#pragma once
#include <windows.h>
#include <cstdint>
#include "../memory/Offsets.h"
#include "../memory/Memory.h"

// No-recoil and no-camera-shake: patch weapon data each frame (client-side)
// Uses POD struct and __try/__except for crash safety (no C++ objects in SEH)
namespace RecoilPatch {

    struct RecoilSlot {
        uint64_t ext;
        float recoil;
        float camRecoilDur;
        float camRecoilPowerX;
        float camRecoilPowerY;
    };

    // Pure C implementation to avoid C2712 with __try
    static void ApplyImpl(uintptr_t localPlayerPtr, uintptr_t projectModule, int bNoRecoil, int bNoCameraShake) {
        __try {
            uint64_t playerArming = *(uint64_t*)(localPlayerPtr + Offsets::PlayerRoot::playerArming);
            if (!playerArming) return;
            uint64_t activeWeapon = *(uint64_t*)(playerArming + Offsets::PlayerArming::activeWeapon);
            if (!activeWeapon) return;
            uint64_t infoCached = *(uint64_t*)(activeWeapon + Offsets::WeaponBase::infoCached);
            if (!infoCached) return;
            uint64_t useTypeExt = *(uint64_t*)(infoCached + Offsets::WeaponInfo::useTypeExtension);
            if (!useTypeExt) return;

            typedef bool(*is_shooting_t)(uint64_t);
            is_shooting_t isShootingFn = (is_shooting_t)(projectModule + Offsets::WeaponInfo::isShootingWeapon);
            if (!isShootingFn(infoCached)) return;

            static RecoilSlot s1 = {}, s2 = {};
            RecoilSlot* orig = nullptr;
            if (s1.ext == useTypeExt) orig = &s1;
            else if (s2.ext == useTypeExt) orig = &s2;

            if (bNoRecoil || bNoCameraShake) {
                if (!orig) {
                    RecoilSlot* slot = (s1.ext == 0) ? &s1 : &s2;
                    slot->ext = useTypeExt;
                    slot->recoil = *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::recoilKickPower);
                    slot->camRecoilDur = *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilDuration);
                    slot->camRecoilPowerX = *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange);
                    slot->camRecoilPowerY = *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange + 4);
                    orig = slot;
                }
            }

            if (bNoRecoil)
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::recoilKickPower) = 0.f;
            else if (orig)
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::recoilKickPower) = orig->recoil;

            if (bNoCameraShake) {
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilDuration) = 0.f;
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange) = 0.f;
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange + 4) = 0.f;
            } else if (orig) {
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilDuration) = orig->camRecoilDur;
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange) = orig->camRecoilPowerX;
                *(float*)(useTypeExt + Offsets::ShootUseTypeInfoExt::CamRecoilPowerRange + 4) = orig->camRecoilPowerY;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void Apply(bool inMatch, bool noRecoil, bool noCameraShake) {
        if (!inMatch && !noRecoil && !noCameraShake) return;

        CPlayer* localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer) return;

        uintptr_t base = Memory::Get().GetBaseAddress();
        if (!base) return;

        ApplyImpl((uintptr_t)localPlayer, base, noRecoil ? 1 : 0, noCameraShake ? 1 : 0);
    }
}
