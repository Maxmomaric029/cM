#pragma once
#include <cstdint>

namespace Offsets {
    // ------------------- BASE ADDRESS -------------------
    // Obtener dinamicamente: GetModuleBaseAddress("CombatMaster.exe")

    // ------------------- GLOBAL POINTERS -------------------
    constexpr uintptr_t GWORLD_OFFSET = 0x5E7A2C0;    // GWorld* (8 bytes)
    constexpr uintptr_t GNAMES_OFFSET = 0x5E77140;    // GNames* (8 bytes)

    // ------------------- UWORLD -------------------
    constexpr uintptr_t UWORLD_PERSISTENTLEVEL = 0x30;    // ULevel* (primary level)
    constexpr uintptr_t UWORLD_OWNINGGAMEINSTANCE = 0x1A0; // UGameInstance*
    constexpr uintptr_t UWORLD_GAMESTATE = 0x158;          // AGameStateBase*
    constexpr uintptr_t UWORLD_LEVELS = 0x170;             // TArray<ULevel*>

    // ------------------- ULEVEL -------------------
    constexpr uintptr_t ULEVEL_ACTORS = 0xA0;           // TArray<AActor*>
    constexpr uintptr_t ULEVEL_ACTORCOUNT = 0xA8;       // int32

    // ------------------- GAMEINSTANCE -------------------
    constexpr uintptr_t GAMEINSTANCE_LOCALPLAYERS = 0x38;       // TArray<ULocalPlayer*>
    constexpr uintptr_t LOCALPLAYER_PLAYERCONTROLLER = 0x30;    // APlayerController*
    constexpr uintptr_t PLAYERCONTROLLER_PAWN = 0x3A8;          // APawn*
    constexpr uintptr_t CONTROLLER_PLAYERSTATE = 0x2A8;         // APlayerState*
    constexpr uintptr_t PLAYERSTATE_PLAYERNAME = 0x3A8;         // FString (wchar_t*)

    // ------------------- ACTOR COMPONENTS -------------------
    constexpr uintptr_t ACTOR_ROOTCOMPONENT = 0x188;        // USceneComponent*
    constexpr uintptr_t COMPONENT_WORLDLOCATION = 0x140;    // FVector (x,y,z)
    constexpr uintptr_t COMPONENT_WORLDROTATION = 0x158;    // FRotator (pitch, yaw, roll)

    // ------------------- HEALTH -------------------
    constexpr uintptr_t ACTOR_HEALTH = 0x2A0;               // float
    constexpr uintptr_t ACTOR_HEALTH_MAX = 0x2A4;           // float

    // ------------------- CAMERA -------------------
    constexpr uintptr_t PLAYERCONTROLLER_CAMERAMANAGER = 0x2B8;   // APlayerCameraManager*
    constexpr uintptr_t CAMERAMANAGER_VIEWMATRIX = 0x2A0;         // FMatrix (4x4 floats)

    // ------------------- AIMBOT -------------------
    constexpr uintptr_t PLAYERCONTROLLER_CONTROLROTATION = 0x2C0; // FRotator
    constexpr uintptr_t PAWN_VELOCITY = 0x168;                     // FVector

    // ------------------- WEAPON -------------------
    constexpr uintptr_t PAWN_CURRENTWEAPON = 0x6C0;                // UObject*
    constexpr uintptr_t WEAPON_AMMO = 0x2F0;                       // int32

    // ------------------- TEAM -------------------
    constexpr uintptr_t ACTOR_TEAMID = 0x2F0;                      // int32 (0 friendly, 1 enemy)
}
