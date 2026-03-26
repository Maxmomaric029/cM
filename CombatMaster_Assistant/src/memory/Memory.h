#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <psapi.h>
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
    HANDLE hProcess    = nullptr;
    uintptr_t moduleBase   = 0;  // Project.dll base
    uintptr_t engineModule = 0;  // GameAssembly.dll
    uintptr_t gdiModule    = 0;  // _CombatMaster.GDI.dll
    DWORD gamePid      = 0;
    std::mutex memMutex;

    Memory() {}
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    // Enumerate modules in the target process to find a DLL base
    uintptr_t GetRemoteModuleBase(const char* moduleName) {
        if (!hProcess || !gamePid) return 0;
        HMODULE hMods[1024];
        DWORD cbNeeded = 0;
        if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) return 0;
        DWORD count = cbNeeded / sizeof(HMODULE);
        char modName[MAX_PATH];
        for (DWORD i = 0; i < count; i++) {
            if (GetModuleBaseNameA(hProcess, hMods[i], modName, sizeof(modName))) {
                if (_stricmp(modName, moduleName) == 0)
                    return (uintptr_t)hMods[i];
            }
        }
        return 0;
    }

public:
    ~Memory() {
        if (hProcess) { CloseHandle(hProcess); hProcess = nullptr; }
    }

    static Memory& Get() {
        static Memory instance;
        return instance;
    }

    // --- Wait for the game process and open a handle ---
    bool WaitForProcess(const char* processName, int timeoutMs = 60000) {
        Logger::Log("[Memory] Waiting for process: " + std::string(processName));
        int elapsed = 0;
        while (elapsed < timeoutMs) {
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snap != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe{}; pe.dwSize = sizeof(pe);
                if (Process32First(snap, &pe)) {
                    do {
                        if (_stricmp(pe.szExeFile, processName) == 0) {
                            gamePid = pe.th32ProcessID;
                            CloseHandle(snap);
                            goto found;
                        }
                    } while (Process32Next(snap, &pe));
                }
                CloseHandle(snap);
            }
            Sleep(500);
            elapsed += 500;
        }
        Logger::Error("[Memory] Timed out waiting for process.");
        return false;

    found:
        hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION
                               | PROCESS_QUERY_INFORMATION, FALSE, gamePid);
        if (!hProcess) {
            Logger::Error("[Memory] OpenProcess failed. Run as Administrator.");
            return false;
        }
        Logger::Log("[Memory] Process opened. PID=" + std::to_string(gamePid));
        return true;
    }

    // --- Resolve module bases inside the game process ---
    bool InitModules() {
        // Allow a moment for modules to load
        Sleep(1000);
        int retries = 10;
        while (retries-- > 0) {
            moduleBase = GetRemoteModuleBase("Project.dll");
            if (moduleBase) break;
            Sleep(500);
        }
        if (!moduleBase) {
            moduleBase = GetRemoteModuleBase("GameAssembly.dll");
        }
        if (!moduleBase) {
            Logger::Error("[Memory] Could not find Project.dll or GameAssembly.dll");
            return false;
        }
        engineModule = GetRemoteModuleBase("GameAssembly.dll");
        gdiModule    = GetRemoteModuleBase("_CombatMaster.GDI.dll");
        if (!gdiModule) gdiModule = moduleBase;

        Logger::Log("[Memory] Project.dll base: 0x" + [&]{
            char buf[32]; snprintf(buf, 32, "%llX", (unsigned long long)moduleBase); return std::string(buf);
        }());
        return true;
    }

    bool IsAttached()         const { return hProcess != nullptr && moduleBase != 0; }
    HANDLE GetProcessHandle() const { return hProcess; }
    DWORD  GetPID()           const { return gamePid; }
    uintptr_t GetBaseAddress()  const { return moduleBase; }
    uintptr_t GetEngineModule() const { return engineModule; }
    uintptr_t GetGdiModule()    const { return gdiModule; }

    // --- Core RPM ---
    template<typename T>
    T Read(uintptr_t address) {
        T value{};
        if (!hProcess || !address) return value;
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
        return value;
    }

    template<typename T>
    std::optional<T> ReadSafe(uintptr_t address) {
        T value{};
        if (!hProcess || !address) return std::nullopt;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), &bytesRead)
            || bytesRead != sizeof(T))
            return std::nullopt;
        return value;
    }

    // --- Core WPM ---
    template<typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!hProcess || !address) return false;
        SIZE_T bytesWritten = 0;
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), &bytesWritten)
               && bytesWritten == sizeof(T);
    }

    template<typename T>
    std::vector<T> ReadArray(uintptr_t address, size_t count) {
        std::vector<T> result;
        if (!hProcess || !address || count == 0 || count > 10000) return result;
        result.resize(count);
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)address, result.data(), count * sizeof(T), &bytesRead))
            result.clear();
        return result;
    }

    bool ReadMemoryBlock(uintptr_t address, void* buffer, size_t size) {
        if (!hProcess || !address || !buffer || size == 0) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead)
               && bytesRead == size;
    }

    // Follows a pointer chain: base → deref → +off[0] → deref → +off[1] → ...
    uintptr_t FindPointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

    std::string  ReadString (uintptr_t address, size_t size = 128);
    std::wstring ReadWString(uintptr_t address, size_t size = 128);
    std::wstring ReadFString(uintptr_t address);
    std::string  ReadFName  (uintptr_t address);

    uintptr_t FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask);
};
