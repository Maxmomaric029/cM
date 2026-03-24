#pragma once
#include "Globals.h"
#include "Offsets.h"
#include "Sdk.h"

namespace WeaponHooks {
    using tGetUnCharged = int(__fastcall*)(uintptr_t);
    using tGetCharged = int(__fastcall*)(uintptr_t);
    using tGetAdsPercent = float(__fastcall*)(uintptr_t);
    using tGetIsUseCooldown = bool(__fastcall*)(uintptr_t);
    using tUse = void(__fastcall*)(uintptr_t);
    using tGetLethal = int(__fastcall*)(uintptr_t);

    inline tGetUnCharged o_GetUnCharged = nullptr;
    inline tGetCharged o_GetCharged = nullptr;
    inline tGetAdsPercent o_GetAdsPercent = nullptr;
    inline tGetIsUseCooldown o_GetIsUseCooldown = nullptr;
    inline tUse o_Use = nullptr;
    inline tGetLethal o_GetLethal = nullptr;

    inline int __fastcall hk_GetUnCharged(uintptr_t _this) {
        __try {
            if (Globals::InMatch && Globals::Menu::bInfiniteAmmo) {
                Globals::Weapon::LocalWeapon = _this;
                return 999;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetUnCharged ? o_GetUnCharged(_this) : 0;
    }
    inline int __fastcall hk_GetCharged(uintptr_t _this) {
        __try { if (Globals::InMatch && Globals::Menu::bInfiniteAmmo) return 999; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetCharged ? o_GetCharged(_this) : 0;
    }
    inline float __fastcall hk_GetAdsPercent(uintptr_t _this) {
        return o_GetAdsPercent ? o_GetAdsPercent(_this) : 0.f;
    }
    inline bool __fastcall hk_GetIsUseCooldown(uintptr_t _this) {
        __try { if (Globals::InMatch && Globals::Menu::bRapidFire) return false; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        return o_GetIsUseCooldown ? o_GetIsUseCooldown(_this) : false;
    }
    inline void __fastcall hk_Use(uintptr_t _this) { if (o_Use) o_Use(_this); }
    inline int __fastcall hk_GetLethal(uintptr_t _this) {
        __try { if (Globals::InMatch && Globals::Menu::bInfiniteLethals) return 999; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
        return o_GetLethal ? o_GetLethal(_this) : 0;
    }

    inline void Init() {
        if (!Globals::ProjectModule) return;
        o_GetUnCharged = (tGetUnCharged)(Globals::ProjectModule + Offsets::weapon_rva::get_UnChargedAmmoLeft);
        o_GetCharged = (tGetCharged)(Globals::ProjectModule + Offsets::weapon_rva::get_ChargedAmmoLeft);
        o_GetAdsPercent = (tGetAdsPercent)(Globals::ProjectModule + Offsets::weapon_rva::get_AdsPercent);
        o_GetIsUseCooldown = (tGetIsUseCooldown)(Globals::ProjectModule + Offsets::ShootWeapon_rva::get_IsUseCooldown);
        o_Use = (tUse)(Globals::ProjectModule + Offsets::weapon_rva::Use);
        o_GetLethal = (tGetLethal)(Globals::ProjectModule + Offsets::weapon_rva::get_LethalWeaponTotalAmmoLeft);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)o_GetUnCharged, hk_GetUnCharged);
        DetourAttach(&(PVOID&)o_GetCharged, hk_GetCharged);
        DetourAttach(&(PVOID&)o_GetAdsPercent, hk_GetAdsPercent);
        if (o_GetIsUseCooldown) DetourAttach(&(PVOID&)o_GetIsUseCooldown, hk_GetIsUseCooldown);
        if (o_Use) DetourAttach(&(PVOID&)o_Use, hk_Use);
        if (o_GetLethal) DetourAttach(&(PVOID&)o_GetLethal, hk_GetLethal);
        DetourTransactionCommit();
    }
    inline void Unhook() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)o_GetUnCharged, hk_GetUnCharged);
        DetourDetach(&(PVOID&)o_GetCharged, hk_GetCharged);
        DetourDetach(&(PVOID&)o_GetAdsPercent, hk_GetAdsPercent);
        if (o_GetIsUseCooldown) DetourDetach(&(PVOID&)o_GetIsUseCooldown, hk_GetIsUseCooldown);
        if (o_Use) DetourDetach(&(PVOID&)o_Use, hk_Use);
        if (o_GetLethal) DetourDetach(&(PVOID&)o_GetLethal, hk_GetLethal);
        DetourTransactionCommit();
        o_GetUnCharged = nullptr;
        o_GetCharged = nullptr;
        o_GetAdsPercent = nullptr;
        o_GetIsUseCooldown = nullptr;
        o_Use = nullptr;
        o_GetLethal = nullptr;
    }
}
