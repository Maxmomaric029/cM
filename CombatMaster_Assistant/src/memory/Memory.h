#pragma once
#include <windows.h>
#include <string>
#include <vector>

class Memory {
private:
    HANDLE hProcess = nullptr;
    DWORD processId = 0;
    uintptr_t moduleBase = 0;
    Memory() = default;
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

public:
    ~Memory();

    static Memory& Get() {
        static Memory instance;
        return instance;
    }

    bool IsAttached() const { return hProcess != nullptr; }

    bool Attach(const std::wstring& processName);
    uintptr_t GetModuleBase(const std::wstring& moduleName);
    uintptr_t GetBaseAddress() const;

    template <typename T>
    T Read(uintptr_t address) {
        T value{};
        if (!hProcess || !address) return value;
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
        return value;
    }

    template <typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!hProcess || !address) return false;
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), nullptr);
    }
    
    std::string ReadString(uintptr_t address, size_t size);
    std::wstring ReadWString(uintptr_t address, size_t size);
    std::wstring ReadFString(uintptr_t address);
    
    // Pattern scanning utilities directly in Memory manager for robustness
    uintptr_t FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask);
    std::vector<uintptr_t> FindAllPatterns(uintptr_t base, size_t size, const char* pattern, const char* mask);
};

