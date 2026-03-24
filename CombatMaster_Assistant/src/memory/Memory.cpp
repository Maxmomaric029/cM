#include "Memory.h"
#include <TlHelp32.h>
#include <psapi.h>
#include "../utils/Logger.h"

Memory::~Memory() {
    Detach();
}

void Memory::Detach() {
    if (hProcess) {
        CloseHandle(hProcess);
        hProcess = nullptr;
    }
    processId = 0;
    moduleBase = 0;
}

bool Memory::Attach(const std::wstring& processName) {
    if (hProcess) {
        Detach();
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        Logger::Error("Failed to create process snapshot. Error: " + std::to_string(GetLastError()));
        return false;
    }

    bool found = false;
    if (Process32FirstW(snapshot, &entry) == TRUE) {
        do {
            if (wcscmp(entry.szExeFile, processName.c_str()) == 0) {
                processId = entry.th32ProcessID;
                
                // Elevate privileges attempting PROCESS_ALL_ACCESS first
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                
                // Fallback to VM permissions if ALL_ACCESS fails (e.g., lightweight anticheat)
                if (!hProcess) {
                    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
                }
                
                if (!hProcess) {
                    Logger::Error("Found process but Failed to open handle. GetLastError: " + std::to_string(GetLastError()));
                    break;
                }

                moduleBase = GetModuleBase(processName);
                if (moduleBase == 0) {
                     Logger::Error("Failed to get module base address. Process might be protected or bitness mismatch.");
                     Detach();
                     break;
                }
                
                Logger::Log("Successfully attached to process. PID: " + std::to_string(processId) + " | Base: 0x" + std::to_string(moduleBase));
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry) == TRUE);
    }
    
    CloseHandle(snapshot);
    return found;
}

bool Memory::AttachByWindow(const std::wstring& windowName) {
    HWND hWnd = FindWindowW(NULL, windowName.c_str());
    if (!hWnd) return false;
    
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == 0) return false;
    
    processId = pid;
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    
    if (hProcess) {
        // Need to find main module name for GetModuleBase
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32W moduleEntry;
            moduleEntry.dwSize = sizeof(moduleEntry);
            if (Module32FirstW(snapshot, &moduleEntry)) {
                moduleBase = (uintptr_t)moduleEntry.modBaseAddr;
                Logger::Log("Attached via Window! Base: 0x" + std::to_string(moduleBase));
                CloseHandle(snapshot);
                return true;
            }
            CloseHandle(snapshot);
        }
    }
    return false;
}

uintptr_t Memory::GetModuleBase(const std::wstring& moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (_wcsicmp(moduleEntry.szModule, moduleName.c_str()) == 0) {
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

bool Memory::ReadMemoryBlock(uintptr_t address, void* buffer, size_t size) {
    if (!hProcess || !address || !buffer || size == 0) return false;
    std::lock_guard<std::mutex> lock(memMutex);
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
}

uintptr_t Memory::FindPointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
    if (!hProcess || !baseAddress) return 0;
    
    uintptr_t pointer = baseAddress;
    for (size_t i = 0; i < offsets.size(); ++i) {
        pointer = Read<uintptr_t>(pointer);
        if (!pointer) return 0;
        pointer += offsets[i];
    }
    return pointer;
}

std::string Memory::ReadString(uintptr_t address, size_t size) {
    if (!hProcess || !address) return "";
    
    std::string str(size, '\0');
    SIZE_T bytesRead = 0;
    std::lock_guard<std::mutex> lock(memMutex);
    if (ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size, &bytesRead)) {
        str.resize(bytesRead);
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
    std::lock_guard<std::mutex> lock(memMutex);
    if (ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size * sizeof(wchar_t), &bytesRead)) {
        str.resize(bytesRead / sizeof(wchar_t));
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
    
    // Read the entire struct to minimize RPM calls
    FString fstr = Read<FString>(address);
    if (!fstr.Data || fstr.Count <= 0 || fstr.Count > 2048) return L"";
    
    return ReadWString(fstr.Data, static_cast<size_t>(fstr.Count));
}

std::string Memory::ReadFName(uintptr_t address) {
    // Advanced GNames reading logic. Requires accurate offsets to evaluate correctly.
    // Placeholder length bounds
    return ReadString(address, 256);
}

uintptr_t Memory::FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    std::vector<uint8_t> buffer(size);
    if (!ReadMemoryBlock(base, buffer.data(), size)) {
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

uintptr_t Memory::FindPatternInModule(const std::wstring& moduleName, const char* pattern, const char* mask) {
    // Fetches base and size of a specific module to scan internally
    uintptr_t baseAddr = GetModuleBase(moduleName);
    if (!baseAddr) return 0;
    
    // Assuming typical module size for game logic (e.g., 0x5000000). To be perfectly accurate, we should parse the PE header for SizeOfImage.
    // We use a safe arbitrary large size or fetch PE Header.
    
    // Minimal PE Header validation to get true size
    uint32_t peOffset = Read<uint32_t>(baseAddr + 0x3C);
    uint32_t imageSize = Read<uint32_t>(baseAddr + peOffset + 0x50);
    
    if (imageSize == 0 || imageSize > 0x10000000) {
        imageSize = 0x5000000; // Fallback size 80MB
    }
    
    return FindPattern(baseAddr, imageSize, pattern, mask);
}

std::vector<uintptr_t> Memory::FindAllPatterns(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    std::vector<uintptr_t> results;
    std::vector<uint8_t> buffer(size);
    
    if (!ReadMemoryBlock(base, buffer.data(), size)) {
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
