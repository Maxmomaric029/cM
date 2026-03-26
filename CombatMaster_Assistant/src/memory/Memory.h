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
    uintptr_t moduleBase = 0;       // Project.dll base (all offsets are relative to this)
    uintptr_t engineModule = 0;     // GameAssembly.dll
    uintptr_t gdiModule = 0;        // _CombatMaster.GDI.dll (fallback to Project.dll)
    std::mutex memMutex;

    Memory() {
        // Don't init here — call InitModules() after game is loaded
    }
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

public:
    ~Memory() = default;

    static Memory& Get() {
        static Memory instance;
        return instance;
    }

    // Wait for Project.dll to be loaded (call from init thread)
    bool WaitForModules(int timeoutMs = 30000) {
        int elapsed = 0;
        while (elapsed < timeoutMs) {
            moduleBase = (uintptr_t)GetModuleHandleA("Project.dll");
            if (moduleBase) break;
            Sleep(100);
            elapsed += 100;
        }
        if (!moduleBase) {
            // Fallback: try GameAssembly.dll (some Unity builds)
            moduleBase = (uintptr_t)GetModuleHandleA("GameAssembly.dll");
        }
        if (!moduleBase) return false;

        engineModule = (uintptr_t)GetModuleHandleA("GameAssembly.dll");
        gdiModule = (uintptr_t)GetModuleHandleA("_CombatMaster.GDI.dll");
        if (!gdiModule) gdiModule = moduleBase;

        return true;
    }

    bool IsAttached() const { return moduleBase != 0; }
    uintptr_t GetBaseAddress() const { return moduleBase; }
    uintptr_t GetEngineModule() const { return engineModule; }
    uintptr_t GetGdiModule() const { return gdiModule; }

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
