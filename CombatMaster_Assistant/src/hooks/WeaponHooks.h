#pragma once
#include <windows.h>
#include <MinHook.h>
#include "../memory/Memory.h"
#include "../memory/Offsets.h"
#include "../ui/Config.h"

// Weapon function hooks using MinHook
// All addresses are RVAs relative to Project.dll (Memory::Get().GetBaseAddress())
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

    // Track which hooks succeeded for proper cleanup
    inline void* pGetUnCharged = nullptr;
    inline void* pGetCharged = nullptr;
    inline void* pGetIsUseCooldown = nullptr;
    inline void* pGetLethal = nullptr;
    inline bool hookUnCharged = false;
    inline bool hookCharged = false;
    inline bool hookCooldown = false;
    inline bool hookLethal = false;

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

    // Helper: check if an address looks like valid executable code
    static bool IsValidCodeAddress(void* addr) {
        if (!addr) return false;
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) return false;
        return (mbi.State == MEM_COMMIT) &&
               (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
    }

    inline bool Init() {
        uintptr_t base = Memory::Get().GetBaseAddress();
        if (!base) return false;

        int hookCount = 0;
        int totalHooks = 4;

        // Validate each address before attempting to hook
        pGetUnCharged = (void*)(base + Offsets::weapon_rva::get_UnChargedAmmoLeft);
        if (IsValidCodeAddress(pGetUnCharged)) {
            if (MH_CreateHook(pGetUnCharged, &hk_GetUnCharged, reinterpret_cast<void**>(&o_GetUnCharged)) == MH_OK) {
                MH_EnableHook(pGetUnCharged);
                hookUnCharged = true;
                hookCount++;
            }
        }

        pGetCharged = (void*)(base + Offsets::weapon_rva::get_ChargedAmmoLeft);
        if (IsValidCodeAddress(pGetCharged)) {
            if (MH_CreateHook(pGetCharged, &hk_GetCharged, reinterpret_cast<void**>(&o_GetCharged)) == MH_OK) {
                MH_EnableHook(pGetCharged);
                hookCharged = true;
                hookCount++;
            }
        }

        pGetIsUseCooldown = (void*)(base + Offsets::ShootWeapon_rva::get_IsUseCooldown);
        if (IsValidCodeAddress(pGetIsUseCooldown)) {
            if (MH_CreateHook(pGetIsUseCooldown, &hk_GetIsUseCooldown, reinterpret_cast<void**>(&o_GetIsUseCooldown)) == MH_OK) {
                MH_EnableHook(pGetIsUseCooldown);
                hookCooldown = true;
                hookCount++;
            }
        }

        pGetLethal = (void*)(base + Offsets::weapon_rva::get_LethalWeaponTotalAmmoLeft);
        if (IsValidCodeAddress(pGetLethal)) {
            if (MH_CreateHook(pGetLethal, &hk_GetLethal, reinterpret_cast<void**>(&o_GetLethal)) == MH_OK) {
                MH_EnableHook(pGetLethal);
                hookLethal = true;
                hookCount++;
            }
        }

        return hookCount > 0; // At least some hooks worked
    }

    inline void Unhook() {
        if (hookUnCharged && pGetUnCharged) MH_DisableHook(pGetUnCharged);
        if (hookCharged && pGetCharged) MH_DisableHook(pGetCharged);
        if (hookCooldown && pGetIsUseCooldown) MH_DisableHook(pGetIsUseCooldown);
        if (hookLethal && pGetLethal) MH_DisableHook(pGetLethal);
        hookUnCharged = hookCharged = hookCooldown = hookLethal = false;
    }
}
