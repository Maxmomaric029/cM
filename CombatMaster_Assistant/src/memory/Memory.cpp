#include "Memory.h"
#include <TlHelp32.h>
#include <psapi.h>
#include "../utils/Logger.h"

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
    if (!ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size, &bytesRead)) return "";
    size_t nullPos = str.find('\0');
    if (nullPos != std::string::npos) str.resize(nullPos);
    return str;
}

std::wstring Memory::ReadWString(uintptr_t address, size_t size) {
    if (!hProcess || !address) return L"";
    std::wstring str(size, L'\0');
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, (LPCVOID)address, &str[0], size * sizeof(wchar_t), &bytesRead)) return L"";
    size_t nullPos = str.find(L'\0');
    if (nullPos != std::wstring::npos) str.resize(nullPos);
    return str;
}

std::wstring Memory::ReadFString(uintptr_t address) {
    if (!hProcess || !address) return L"";
    FString fstr = Read<FString>(address);
    if (!fstr.Data || fstr.Count <= 0 || fstr.Count > 2048) return L"";
    return ReadWString(fstr.Data, static_cast<size_t>(fstr.Count));
}

std::string Memory::ReadFName(uintptr_t address) {
    return ReadString(address, 256);
}

uintptr_t Memory::FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    std::vector<uint8_t> buffer(size);
    if (!ReadMemoryBlock(base, buffer.data(), size)) return 0;

    size_t patternLength = strlen(mask);
    for (size_t i = 0; i < size - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (mask[j] == 'x' && buffer[i + j] != static_cast<uint8_t>(pattern[j])) {
                found = false;
                break;
            }
        }
        if (found) return base + i;
    }
    return 0;
}
