#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <optional>
#include "../utils/Logger.h"

// FString structure for Unreal Engine 4/5
struct FString {
    uintptr_t Data;
    int32_t Count;
    int32_t Max;
};

// TArray structure for Unreal Engine 4/5
template<class T>
struct TArray {
    uintptr_t Data;
    int32_t Count;
    int32_t Max;
};

class Memory {
private:
    HANDLE hProcess = nullptr;
    DWORD processId = 0;
    uintptr_t moduleBase = 0;
    std::mutex memMutex;

    Memory() = default;
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    
    // Internal cache system for batch processing
    std::vector<uint8_t> bufferCache;
    uintptr_t lastCacheAddress = 0;
    size_t lastCacheSize = 0;

public:
    ~Memory();

    static Memory& Get() {
        static Memory instance;
        return instance;
    }

    bool IsAttached() const { return hProcess != nullptr; }
    DWORD GetProcessId() const { return processId; }
    
    // Core attachment routines
    bool Attach(const std::wstring& processName);
    bool AttachByWindow(const std::wstring& windowName);
    void Detach();
    
    uintptr_t GetModuleBase(const std::wstring& moduleName);
    uintptr_t GetBaseAddress() const;

    // Advanced Reading Templates with validation
    template <typename T>
    T Read(uintptr_t address) {
        T value{};
        if (!hProcess || !address || address > 0x7FFFFFFFFFFF) return value; // Prevent kernel reading crash
        
        std::lock_guard<std::mutex> lock(memMutex);
        SIZE_T bytesRead = 0;
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), &bytesRead);
        return value;
    }
    
    template <typename T>
    std::optional<T> ReadSafe(uintptr_t address) {
        T value{};
        if (!hProcess || !address || address > 0x7FFFFFFFFFFF) return std::nullopt;
        
        std::lock_guard<std::mutex> lock(memMutex);
        SIZE_T bytesRead = 0;
        if (ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), &bytesRead) && bytesRead == sizeof(T)) {
            return value;
        }
        return std::nullopt;
    }

    // Advanced Writing Templates with strict protection
    template <typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!hProcess || !address || address > 0x7FFFFFFFFFFF) return false;
        
        std::lock_guard<std::mutex> lock(memMutex);
        SIZE_T bytesWritten = 0;
        
        // Change memory protection before writing (failsafe for protected addresses)
        DWORD oldProtect;
        if (VirtualProtectEx(hProcess, (LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            bool success = WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), &bytesWritten);
            VirtualProtectEx(hProcess, (LPVOID)address, sizeof(T), oldProtect, &oldProtect);
            return success;
        }
        
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), &bytesWritten);
    }
    
    // Array Reading
    template <typename T>
    std::vector<T> ReadArray(uintptr_t address, size_t count) {
        std::vector<T> result;
        if (!hProcess || !address || count == 0 || count > 10000) return result;
        
        result.resize(count);
        std::lock_guard<std::mutex> lock(memMutex);
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)address, result.data(), count * sizeof(T), &bytesRead)) {
            result.clear();
        }
        return result;
    }

    // UE4 Specific Methods
    std::string ReadString(uintptr_t address, size_t size = 128);
    std::wstring ReadWString(uintptr_t address, size_t size = 128);
    std::wstring ReadFString(uintptr_t address);
    std::string ReadFName(uintptr_t address); // For UE4 GNames reading
    
    // Pointer Chain resolution
    uintptr_t FindPointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

    // Advanced Memory Scanning & Signature Matching
    bool ReadMemoryBlock(uintptr_t address, void* buffer, size_t size);
    uintptr_t FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask);
    uintptr_t FindPatternInModule(const std::wstring& moduleName, const char* pattern, const char* mask);
    std::vector<uintptr_t> FindAllPatterns(uintptr_t base, size_t size, const char* pattern, const char* mask);
};

