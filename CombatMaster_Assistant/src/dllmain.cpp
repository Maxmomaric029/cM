#include <windows.h>
#include <thread>
#include "utils/Logger.h"
#include "utils/Console.h"
#include "visuals/Renderer.h"
#include "ui/Config.h"

DWORD WINAPI MainThread(LPVOID lpReserved) {
    Console::Alloc();
    Logger::Log("Nexus Internal - Initializing Console");

    Config::Load("C:\\Nexus_Config.json"); // Changed path to root to avoid write permission issues in game dir

    Logger::Log("Nexus Internal Injected. Hooking DX11...");

    // Initialize Renderer (Hooking DX11 SwapChain)
    Renderer::Get().Init(); // This will block until the cheat unloads

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
