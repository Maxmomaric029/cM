#include "pch.h"
#include "Base/WeaponHooks.h"
#include "Base/MovementHooks.h"
#include "Overlay/D3DHook.h"

static DWORD WINAPI InitThread(LPVOID param) {
    HMODULE hModule = (HMODULE)param;
    Globals::Init();
    Globals::g_hInjectModule = hModule;
    WeaponHooks::Init();
    Overlay::HookPresent();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitThread, hModule, 0, nullptr);
    }
    return TRUE;
}
