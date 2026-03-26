#include <windows.h>
#include <thread>
#include "utils/Logger.h"
#include "utils/Console.h"
#include "visuals/Renderer.h"
#include "ui/Config.h"
#include "Global.h"

DWORD WINAPI MainThread(LPVOID lpReserved) {
    Console::Alloc();
    Logger::Log("Nexus Internal - Initializing");

    // Store module handle globally for clean unload
    g_hInjectModule = (HMODULE)lpReserved;

    // CRITICAL: Wait for Project.dll to be loaded before doing anything
    Logger::Log("Waiting for game modules...");
    if (!Memory::Get().WaitForModules(30000)) {
        Logger::Error("FATAL: Could not find Project.dll or GameAssembly.dll after 30 seconds!");
        Console::Free();
        FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
        return FALSE;
    }
    Logger::Log("Game modules found. Base address resolved.");

    Config::Load("nexus_config.json");

    Logger::Log("Nexus Internal Injected. Hooking DX11...");

    // Initialize Renderer (hooks DX11 SwapChain, blocks until unload)
    Renderer::Get().Init();

    Logger::Log("Nexus Internal Unloading...");
    Console::Free();
    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
