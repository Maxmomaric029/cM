#pragma once
#include "Types.h"
#include <cstdint>
#include <windows.h>

namespace Globals {
inline uint64_t ProjectModule = 0;
inline uint64_t EngineModule = 0;
inline uint64_t GdiModule = 0;
inline HMODULE g_hInjectModule = nullptr;
inline bool InMatch = false;

namespace Screen {
inline Vector2 ScreenSize;
inline Vector2 ScreenCenter;
}

namespace Menu {
inline bool bIsOpen = true;
inline int menuToggleKey = 45;   // VK_INSERT
inline int unloadKey = 35;       // VK_END

// ESP
inline bool bEsp = true;
inline bool bEspBox = true;
inline bool bHealthBar = true;
inline bool bTracers = false;
inline bool bDistance = true;
inline bool bSnaplines = false;
inline bool bNames = true;
inline bool bVisibilityCheck = true;
inline bool bSpawnProtectionIndicator = true;
inline bool bEspShowTeam = false;  // false = enemies only, true = show teammates too

// Aimbot
inline bool bAimbot = true;
inline bool bDrawFov = true;
inline bool bAimbotVisCheck = true;
inline float aimbotFov = 383.f;
inline float aimbotSmoothingX = 3.f;
inline float aimbotSmoothingY = 3.f;
inline int aimbotBone = 0;         // 0=Head, 1=Neck, 2=Chest, 3=Pelvis
inline int aimbotKey = VK_RBUTTON;
inline int aimbotTargeting = 0;   // 0=FOV, 1=Distance, 2=Both
inline bool bSkipSpawnProtection = true;
inline bool bTeamCheck = true;
inline bool bAimbotFollowCrouched = true;
inline int aimbotPathStyle = 0;   // 0=Linear, 1=Human

// ESP visuals
inline float espBoxThickness = 2.25f;
inline int snaplineOrigin = 1;    // 0=Top, 1=Center, 2=Bottom
inline int tracerOrigin = 2;      // 0=Top, 1=Center, 2=Bottom (where tracers start)
inline float colorEspBox[4] = {0.7f, 0.2f, 1.0f, 1.0f};
inline float colorTracers[4] = {0.0f, 1.0f, 1.0f, 1.0f};
inline float colorFovCircle[4] = {1.0f, 1.0f, 1.0f, 1.0f};
inline float colorTargetOrb[4] = {0.7f, 0.2f, 1.0f, 0.8f};
inline bool bTargetTracer = false;
inline bool bTargetOrb = false;

// Extra (weapon/movement hooks; no recoil & no camera shake are client-side)
inline bool bNoRecoil = false;
inline bool bNoCameraShake = false;
inline bool bInfiniteAmmo = false;
inline bool bRapidFire = false;
inline bool bInfiniteLethals = false;
inline bool bMovementSpeed = false;
inline float movementRunMult = 1.5f;
inline float movementSprintMult = 1.5f;
}
namespace Weapon {
inline uintptr_t LocalWeapon = 0;
}

inline void Init() {
  ProjectModule = reinterpret_cast<uint64_t>(GetModuleHandleA("Project.dll"));
  EngineModule = reinterpret_cast<uint64_t>(GetModuleHandleA("GameAssembly.dll"));
  GdiModule = reinterpret_cast<uint64_t>(GetModuleHandleA("_CombatMaster.GDI.dll"));
  if (!GdiModule) GdiModule = ProjectModule;

  Screen::ScreenSize.x = (float)GetSystemMetrics(SM_CXSCREEN);
  Screen::ScreenSize.y = (float)GetSystemMetrics(SM_CYSCREEN);
  Screen::ScreenCenter = Screen::ScreenSize / 2.f;
}

} // namespace Globals
using namespace Globals;
