#pragma once
#include <cstdint>

namespace Offsets {
    constexpr uintptr_t playerRoot = 0x46F5F08;
    constexpr uintptr_t il2cppStaticField = 0xB8;
    constexpr uintptr_t viewMatrix = 0x2FC;

    namespace List {
        constexpr uintptr_t listSize = 0x18;
    }

    namespace PlayerRoot {
        constexpr uintptr_t localPlayer = 0x8;
        constexpr uintptr_t allPlayers = 0x18;
        constexpr uintptr_t playerArming = 0xA8;
        constexpr uintptr_t currentSpectatorPlayer = 0x288;
        constexpr uintptr_t isRealPlayer = 0x132;
        constexpr uintptr_t isVisible = 0x10A;
        constexpr uintptr_t cachedPlayerData = 0x150;
    }

    namespace PlayerMovement {
        constexpr uintptr_t isCrouch = 0x90;
    }

    namespace PlayerConnectData {
        constexpr uintptr_t NickName = 0x18;
    }

    namespace PlayerHealth {
        constexpr uintptr_t currentHealth = 0xD0;
    }

    namespace PlayerArming {
        constexpr uintptr_t activeWeapon = 0x538;
    }

    namespace WeaponBase {
        constexpr uintptr_t infoCached = 0x1C8;
    }

    namespace ShootUseTypeInfoExt {
        constexpr uintptr_t recoilKickPower = 0x60;
        constexpr uintptr_t CamRecoilDuration = 0x78;
        constexpr uintptr_t CamRecoilPowerRange = 0x7C;
    }

    namespace WeaponInfo {
        constexpr uintptr_t useTypeExtension = 0x88;
        constexpr uintptr_t isShootingWeapon = 0x26DD720;
    }

    namespace Object {
        constexpr uintptr_t cachedPtr = 0x10;
    }

    namespace TransformData {
        constexpr uintptr_t rootPosition = 0x90;
    }

    namespace Transform {
        constexpr uintptr_t transformData = 0x28;
    }

    namespace rva {
        constexpr uintptr_t get_TeamId = 0x33D3320;
        constexpr uintptr_t get_fieldOfView = 0x2EE4A70;
        constexpr uintptr_t get_PlayerConnectData = 0x6DEDB0;
        constexpr uintptr_t get_IsInvincible = 0x34BB0F0;
    }

    namespace weapon_rva {
        constexpr uintptr_t get_UnChargedAmmoLeft = 0x33BA1E0;
        constexpr uintptr_t get_ChargedAmmoLeft = 0x33B9910;
        constexpr uintptr_t get_AdsPercent = 0x33B9900;
        constexpr uintptr_t Use = 0x33B97B0;
        constexpr uintptr_t get_LethalWeaponTotalAmmoLeft = 0x36CCF20;
    }

    namespace ShootWeapon_rva {
        constexpr uintptr_t get_IsUseCooldown = 0x339BBD0;
    }

    namespace HostConfig {
        constexpr uintptr_t get_Instance = 0x359A7C0;
        constexpr uintptr_t GdInfo = 0x20;
        constexpr uintptr_t Operators = 0x68;

        namespace OperatorsSection {
            constexpr uintptr_t RunSpeed = 0x9C;
            constexpr uintptr_t SprintSpeed = 0xAC;
        }
    }
}
