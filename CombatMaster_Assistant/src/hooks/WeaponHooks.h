#pragma once
#include <windows.h>
#include <MinHook.h>
#include "../sdk/SDK.h"
#include "../ui/Config.h"

// Weapon function hooks using MinHook (adapted from reference Detours implementation)
namespace WeaponHooks {
    using tGetUnCharged = int(__fastcall*)(uintptr_t);
    using tGetCharged = int(__fastcall*)(uintptr_t);
    using tGetIsUseCooldown = bool(__fastcall*)(uintptr_t);
    using tGetLethal = int(__fastcall*)(uintptr_t);

    inline tGetUnCharged o_GetUnCharged = nullptr;
    inline tGetCharged o_GetCharged = nullptr;
    inline tGetIsUseCooldown o_GetIsUseCooldown = nullptr;
    inline tGetLethal o_GetLethal = nullptr;

    inline bool g_InMatch = false;
    inline uintptr_t g_LocalWeapon = 0;

    // Hook targets (raw pointers for MH_CreateHook)
    inline void* pGetUnCharged = nullptr;
    inline void* pGetCharged = nullptr;
    inline void* pGetIsUseCooldown = nullptr;
    inline void* pGetLethal = nullptr;

    static int __fastcall hk_GetUnCharged(uintptr_t _this) {
        __try {
            if (g_InMatch && Config::infinite_ammo) {
                g_LocalWeapon = _this;
                return 999;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetUnCharged ? o_GetUnCharged(_this) : 0;
    }

    static int __fastcall hk_GetCharged(uintptr_t _this) {
        __try {
            if (g_InMatch && Config::infinite_ammo) return 999;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetCharged ? o_GetCharged(_this) : 0;
    }

    static bool __fastcall hk_GetIsUseCooldown(uintptr_t _this) {
        __try {
            if (g_InMatch && Config::rapid_fire) return false;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        return o_GetIsUseCooldown ? o_GetIsUseCooldown(_this) : false;
    }

    static int __fastcall hk_GetLethal(uintptr_t _this) {
        __try {
            if (g_InMatch && Config::infinite_lethals) return 999;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetLethal ? o_GetLethal(_this) : 0;
    }

    inline bool Init() {
        uintptr_t base = Memory::Get().GetBaseAddress();
        if (!base) return false;

        pGetUnCharged = (void*)(base + Offsets::weapon_rva::get_UnChargedAmmoLeft);
        pGetCharged = (void*)(base + Offsets::weapon_rva::get_ChargedAmmoLeft);
        pGetIsUseCooldown = (void*)(base + Offsets::ShootWeapon_rva::get_IsUseCooldown);
        pGetLethal = (void*)(base + Offsets::weapon_rva::get_LethalWeaponTotalAmmoLeft);

        bool ok = true;
        if (MH_CreateHook(pGetUnCharged, &hk_GetUnCharged, reinterpret_cast<void**>(&o_GetUnCharged)) != MH_OK) ok = false;
        if (MH_CreateHook(pGetCharged, &hk_GetCharged, reinterpret_cast<void**>(&o_GetCharged)) != MH_OK) ok = false;
        if (MH_CreateHook(pGetIsUseCooldown, &hk_GetIsUseCooldown, reinterpret_cast<void**>(&o_GetIsUseCooldown)) != MH_OK) ok = false;
        if (MH_CreateHook(pGetLethal, &hk_GetLethal, reinterpret_cast<void**>(&o_GetLethal)) != MH_OK) ok = false;

        if (ok) {
            MH_EnableHook(pGetUnCharged);
            MH_EnableHook(pGetCharged);
            MH_EnableHook(pGetIsUseCooldown);
            MH_EnableHook(pGetLethal);
        }
        return ok;
    }

    inline void Unhook() {
        if (pGetUnCharged) MH_DisableHook(pGetUnCharged);
        if (pGetCharged) MH_DisableHook(pGetCharged);
        if (pGetIsUseCooldown) MH_DisableHook(pGetIsUseCooldown);
        if (pGetLethal) MH_DisableHook(pGetLethal);
    }
}
