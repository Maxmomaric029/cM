#include "Memory.h"
#include <TlHelp32.h>
#include "../utils/Logger.h"

Memory mem;

Memory::~Memory() {
    if (hProcess) {
        CloseHandle(hProcess);
        hProcess = nullptr;
    }
}

bool Memory::Attach(const std::wstring& processName) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        Logger::Error("Failed to create process snapshot.");
        return false;
    }

    bool found = false;
    if (Process32FirstW(snapshot, &entry) == TRUE) {
        do {
            if (wcscmp(entry.szExeFile, processName.c_str()) == 0) {
                processId = entry.th32ProcessID;
                hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
                
                if (!hProcess) {
                    Logger::Error("Found process but Failed to open handle.");
                    break;
                }

                moduleBase = GetModuleBase(processName);
                if (moduleBase == 0) {
                     Logger::Error("Failed to get module base address.");
                     CloseHandle(hProcess);
                     hProcess = nullptr;
                     break;
                }
                
                Logger::Log("Successfully attached to process. PID: " + std::to_string(processId));
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry) == TRUE);
    }
    
    CloseHandle(snapshot);
    return found;
}

uintptr_t Memory::GetModuleBase(const std::wstring& moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (wcscmp(moduleEntry.szModule, moduleName.c_str()) == 0) {
                    baseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snapshot, &moduleEntry));
        } else {
             Logger::Error("Module32First failed. Error code: " + std::to_string(GetLastError()));
        }
        CloseHandle(snapshot);
    } else {
         Logger::Error("Failed to create module snapshot. Error code: " + std::to_string(GetLastError()));
    }
    
    return baseAddress;
}

uintptr_t Memory::GetBaseAddress() const {
    return moduleBase;
}

std::string Memory::ReadString(uintptr_t address, size_t size) {
    if (!hProcess || !address) return "";
    
    std::string str(size, '\0');
    SIZE_T bytesRead = 0;
    if (ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size, &bytesRead)) {
        str.resize(bytesRead);
        
        // Find null terminator
        size_t nullPos = str.find('\0');
        if (nullPos != std::string::npos) {
            str.resize(nullPos);
        }
    } else {
        str.clear();
    }
    return str;
}

std::wstring Memory::ReadWString(uintptr_t address, size_t size) {
    if (!hProcess || !address) return L"";
    
    std::wstring str(size, L'\0');
    SIZE_T bytesRead = 0;
    if (ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size * sizeof(wchar_t), &bytesRead)) {
        // Adjust size based on actual bytes read
        str.resize(bytesRead / sizeof(wchar_t));
        
        // Find null terminator
        size_t nullPos = str.find(L'\0');
        if (nullPos != std::wstring::npos) {
            str.resize(nullPos);
        }
    } else {
        str.clear();
    }
    return str;
}

std::wstring Memory::ReadFString(uintptr_t address) {
    if (!address) return L"";
    
    uintptr_t arrayPtr = Read<uintptr_t>(address);
    if (!arrayPtr) return L"";
    
    int32_t length = Read<int32_t>(address + 8);
    // Sanity check length to prevent huge allocations on bad reads
    if (length <= 0 || length > 2048) return L"";
    
    return ReadWString(arrayPtr, length);
}

uintptr_t Memory::FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    // Robust pattern scanner implementation
    std::vector<uint8_t> buffer(size);
    if (!ReadProcessMemory(hProcess, (LPCVOID)base, buffer.data(), size, nullptr)) {
        return 0; // Failed to read memory region
    }

    size_t patternLength = strlen(mask);
    
    for (size_t i = 0; i < size - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (mask[j] == 'x' && buffer[i + j] != static_cast<uint8_t>(pattern[j])) {
                found = false;
                break;
            }
        }
        
        if (found) {
            return base + i;
        }
    }
    
    return 0;
}

std::vector<uintptr_t> Memory::FindAllPatterns(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    std::vector<uintptr_t> results;
    std::vector<uint8_t> buffer(size);
    
    if (!ReadProcessMemory(hProcess, (LPCVOID)base, buffer.data(), size, nullptr)) {
        return results;
    }

    size_t patternLength = strlen(mask);
    
    for (size_t i = 0; i < size - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (mask[j] == 'x' && buffer[i + j] != static_cast<uint8_t>(pattern[j])) {
                found = false;
                break;
            }
        }
        
        if (found) {
            results.push_back(base + i);
        }
    }
    
    return results;
}
