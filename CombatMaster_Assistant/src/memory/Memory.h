#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include "../utils/Logger.h"

struct FString {
    uintptr_t Data;
    int32_t Count;
    int32_t Max;
};

template<class T>
struct TArray {
    uintptr_t Data;
    int32_t Count;
    int32_t Max;
};

class Memory {
private:
    uintptr_t moduleBase = 0;
    std::mutex memMutex;

    Memory() {
        moduleBase = (uintptr_t)GetModuleHandle(NULL);
    }
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

public:
    ~Memory() = default;

    static Memory& Get() {
        static Memory instance;
        return instance;
    }

    bool IsAttached() const { return moduleBase != 0; }
    uintptr_t GetBaseAddress() const { return moduleBase; }

    template <typename T>
    T Read(uintptr_t address) {
        if (!address || IsBadReadPtr((const void*)address, sizeof(T))) return T{};
        return *reinterpret_cast<T*>(address);
    }
    
    template <typename T>
    std::optional<T> ReadSafe(uintptr_t address) {
        if (!address || IsBadReadPtr((const void*)address, sizeof(T))) return std::nullopt;
        return *reinterpret_cast<T*>(address);
    }

    template <typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!address) return false;
        
        DWORD oldProtect;
        if (VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *reinterpret_cast<T*>(address) = value;
            VirtualProtect((LPVOID)address, sizeof(T), oldProtect, &oldProtect);
            return true;
        }
        return false;
    }
    
    template <typename T>
    std::vector<T> ReadArray(uintptr_t address, size_t count) {
        std::vector<T> result;
        if (!address || count == 0 || count > 10000 || IsBadReadPtr((const void*)address, count * sizeof(T))) return result;
        
        result.resize(count);
        memcpy(result.data(), (void*)address, count * sizeof(T));
        return result;
    }

    std::string ReadString(uintptr_t address, size_t size = 128);
    std::wstring ReadWString(uintptr_t address, size_t size = 128);
    std::wstring ReadFString(uintptr_t address);
    std::string ReadFName(uintptr_t address);
    
    uintptr_t FindPointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

    bool ReadMemoryBlock(uintptr_t address, void* buffer, size_t size);
    uintptr_t FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask);
};
