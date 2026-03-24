#pragma once
#include "../Base/Globals.h"
#include "../Base/Sdk.h"
#include "../Base/Utils.h"
#include "../Base/Offsets.h"
#include "../Base/MovementHooks.h"
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace MenuDraw {
const int kMaxAimMovePerFrame = 500;

static const char* GetKeyName(int vkCode) {
    static char buf[32];
    if (vkCode == VK_LBUTTON) return "Mouse 1";
    if (vkCode == VK_RBUTTON) return "Mouse 2";
    if (vkCode == VK_MBUTTON) return "Mouse 3";
    if (vkCode == VK_XBUTTON1) return "Mouse 4";
    if (vkCode == VK_XBUTTON2) return "Mouse 5";
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
    if (GetKeyNameTextA((LONG)scanCode << 16, buf, 32) == 0) return "?";
    return buf;
}

void HotkeyButton(const char* label, int* currentKey) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    static int* activeKeybind = nullptr;
    char btnLabel[64];
    if (activeKeybind == currentKey) {
        snprintf(btnLabel, sizeof(btnLabel), "[ Press any key ]##%s", label);
        ImGui::Button(btnLabel);
        for (int i = 1; i < 256; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                if (i != VK_ESCAPE) *currentKey = i;
                while (GetAsyncKeyState(i) & 0x8000) {}
                activeKeybind = nullptr;
                return;
            }
        }
    } else {
        snprintf(btnLabel, sizeof(btnLabel), "[ %s ]##%s", GetKeyName(*currentKey), label);
        if (ImGui::Button(btnLabel)) activeKeybind = currentKey;
    }
}

static std::string GetConfigPath() {
    char path[MAX_PATH];
    HMODULE hMod = Globals::g_hInjectModule;
    if (!hMod) hMod = GetModuleHandleA(nullptr);
    if (GetModuleFileNameA(hMod, path, MAX_PATH) == 0) return "";
    std::string s(path);
    size_t last = s.find_last_of("\\/");
    if (last != std::string::npos) s = s.substr(0, last + 1);
    s += "cm_base_config.cfg";
    return s;
}

static void SaveConfig() {
    std::string path = GetConfigPath();
    if (path.empty()) return;
    std::ofstream f(path);
    if (!f) return;
    using namespace Globals;
    f << "bEsp=" << Menu::bEsp << "\nbEspBox=" << Menu::bEspBox << "\nbHealthBar=" << Menu::bHealthBar << "\nbTracers=" << Menu::bTracers
      << "\nbDistance=" << Menu::bDistance << "\nbSnaplines=" << Menu::bSnaplines << "\nbNames=" << Menu::bNames
      << "\nbVisibilityCheck=" << Menu::bVisibilityCheck << "\nbSpawnProtectionIndicator=" << Menu::bSpawnProtectionIndicator
      << "\nbEspShowTeam=" << Menu::bEspShowTeam << "\naimbotFov=" << Menu::aimbotFov << "\naimbotSmoothingX=" << Menu::aimbotSmoothingX
      << "\naimbotSmoothingY=" << Menu::aimbotSmoothingY << "\naimbotBone=" << Menu::aimbotBone << "\naimbotKey=" << Menu::aimbotKey
      << "\naimbotTargeting=" << Menu::aimbotTargeting << "\ntracerOrigin=" << Menu::tracerOrigin
      << "\nmenuToggleKey=" << Menu::menuToggleKey << "\nunloadKey=" << Menu::unloadKey << "\nespBoxThickness=" << Menu::espBoxThickness
      << "\nsnaplineOrigin=" << Menu::snaplineOrigin
      << "\nbNoRecoil=" << Menu::bNoRecoil << "\nbNoCameraShake=" << Menu::bNoCameraShake << "\n";
    f << "colorEspBox=" << Menu::colorEspBox[0] << "," << Menu::colorEspBox[1] << "," << Menu::colorEspBox[2] << "," << Menu::colorEspBox[3] << "\n";
    f << "colorTracers=" << Menu::colorTracers[0] << "," << Menu::colorTracers[1] << "," << Menu::colorTracers[2] << "," << Menu::colorTracers[3] << "\n";
    f << "colorFovCircle=" << Menu::colorFovCircle[0] << "," << Menu::colorFovCircle[1] << "," << Menu::colorFovCircle[2] << "," << Menu::colorFovCircle[3] << "\n";
}

static void LoadConfig() {
    std::string path = GetConfigPath();
    if (path.empty()) return;
    std::ifstream f(path);
    if (!f) return;
    using namespace Globals;
    std::string line, key, val;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        key = line.substr(0, eq);
        val = line.substr(eq + 1);
        if (key == "bEsp") Menu::bEsp = (val == "1");
        else if (key == "bEspBox") Menu::bEspBox = (val == "1");
        else if (key == "bHealthBar") Menu::bHealthBar = (val == "1");
        else if (key == "bTracers") Menu::bTracers = (val == "1");
        else if (key == "bDistance") Menu::bDistance = (val == "1");
        else if (key == "bSnaplines") Menu::bSnaplines = (val == "1");
        else if (key == "bNames") Menu::bNames = (val == "1");
        else if (key == "bVisibilityCheck") Menu::bVisibilityCheck = (val == "1");
        else if (key == "bSpawnProtectionIndicator") Menu::bSpawnProtectionIndicator = (val == "1");
        else if (key == "bEspShowTeam") Menu::bEspShowTeam = (val == "1");
        else if (key == "aimbotFov") Menu::aimbotFov = (float)atof(val.c_str());
        else if (key == "aimbotSmoothingX") Menu::aimbotSmoothingX = (float)atof(val.c_str());
        else if (key == "aimbotSmoothingY") Menu::aimbotSmoothingY = (float)atof(val.c_str());
        else if (key == "aimbotBone") Menu::aimbotBone = atoi(val.c_str());
        else if (key == "aimbotKey") Menu::aimbotKey = atoi(val.c_str());
        else if (key == "aimbotTargeting") Menu::aimbotTargeting = atoi(val.c_str());
        else if (key == "tracerOrigin") Menu::tracerOrigin = atoi(val.c_str());
        else if (key == "menuToggleKey") Menu::menuToggleKey = atoi(val.c_str());
        else if (key == "unloadKey") Menu::unloadKey = atoi(val.c_str());
        else if (key == "espBoxThickness") Menu::espBoxThickness = (float)atof(val.c_str());
        else if (key == "snaplineOrigin") Menu::snaplineOrigin = atoi(val.c_str());
        else if (key == "bNoRecoil") Menu::bNoRecoil = (val == "1");
        else if (key == "bNoCameraShake") Menu::bNoCameraShake = (val == "1");
        else if (key == "colorEspBox") (void)sscanf_s(val.c_str(), "%f,%f,%f,%f", &Menu::colorEspBox[0], &Menu::colorEspBox[1], &Menu::colorEspBox[2], &Menu::colorEspBox[3]);
        else if (key == "colorTracers") (void)sscanf_s(val.c_str(), "%f,%f,%f,%f", &Menu::colorTracers[0], &Menu::colorTracers[1], &Menu::colorTracers[2], &Menu::colorTracers[3]);
        else if (key == "colorFovCircle") (void)sscanf_s(val.c_str(), "%f,%f,%f,%f", &Menu::colorFovCircle[0], &Menu::colorFovCircle[1], &Menu::colorFovCircle[2], &Menu::colorFovCircle[3]);
    }
}

// POD slot for no-recoil/camera-shake cache (no C++ unwinding in SEH helper)
struct RecoilSlot { uint64_t ext; float recoil; float camRecoilDur; float camRecoilPowerX; float camRecoilPowerY; };

// No C++ objects/lambdas here so __try/__except is allowed (C2712)
static void ApplyNoRecoilAndNoCameraShakeImpl(uintptr_t localPlayerPtr, uintptr_t projectModule, int bNoRecoil, int bNoCameraShake) {
    __try {
        uint64_t playerArming = *(uint64_t*)(localPlayerPtr + Offsets::PlayerRoot::playerArming);
        if (!playerArming) return;
        uint64_t activeWeapon = *(uint64_t*)(playerArming + Offsets::PlayerArming::activeWeapon);
        if (!activeWeapon) return;
        uint64_t infoCached = *(uint64_t*)(activeWeapon + Offsets::WeaponBase::infoCached);
        if (!infoCached) return;
        uint64_t useTypeExt = *(uint64_t*)(infoCached + Offsets::WeaponInfo::useTypeExtension);
        if (!useTypeExt) return;
        typedef bool (*is_shooting_t)(uint64_t);
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

// No recoil / no camera shake: patch ShootUseTypeInfoExt each frame (client-side; no [HOST])
static void ApplyNoRecoilAndNoCameraShake() {
    using namespace Globals;
    if (!InMatch && !Menu::bNoRecoil && !Menu::bNoCameraShake) return;
    PlayerRoot* localPlayer = PlayerRoot::GetLocalPlayer();
    if (!localPlayer || !Globals::ProjectModule) return;
    ApplyNoRecoilAndNoCameraShakeImpl((uintptr_t)localPlayer, (uintptr_t)Globals::ProjectModule,
        Menu::bNoRecoil ? 1 : 0, Menu::bNoCameraShake ? 1 : 0);
}

void Frame(IDXGISwapChain* swapChain) {
    using namespace Globals;
    ImDrawList* BackgroundDrawList = ImGui::GetBackgroundDrawList();

    // FOV circle when not in match (lobby) or fallback — same as original
    if (Screen::ScreenCenter.y >= 10.f && Menu::bDrawFov && Menu::bAimbot) {
        ImU32 fovCol = IM_COL32((int)(Menu::colorFovCircle[0] * 255),
                                (int)(Menu::colorFovCircle[1] * 255),
                                (int)(Menu::colorFovCircle[2] * 255),
                                (int)(Menu::colorFovCircle[3] * 255));
        BackgroundDrawList->AddCircle(
            ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y),
            Menu::aimbotFov, fovCol, 100, 1.5f);
    }

    PlayerRoot* localPlayer = PlayerRoot::GetLocalPlayer();
    Globals::InMatch = false;

    if (localPlayer) {
        PlayerRoot* viewPlayer = localPlayer;
        if (localPlayer->IsDead()) {
            PlayerRoot* spectated = *(PlayerRoot**)((uintptr_t)localPlayer + Offsets::PlayerRoot::currentSpectatorPlayer);
            if (spectated && spectated->GetCamera())
                viewPlayer = spectated;
        }
        Camera* viewCamera = viewPlayer->GetCamera();
        if (viewCamera) {
            ViewMatrix viewMatrix = viewCamera->GetViewMatrix();
            float effectiveAimbotFov = Menu::aimbotFov;

            if (Menu::bDrawFov && Menu::bAimbot) {
                ImU32 fovCol = IM_COL32((int)(Menu::colorFovCircle[0] * 255),
                                        (int)(Menu::colorFovCircle[1] * 255),
                                        (int)(Menu::colorFovCircle[2] * 255),
                                        (int)(Menu::colorFovCircle[3] * 255));
                BackgroundDrawList->AddCircle(
                    ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y),
                    effectiveAimbotFov, fovCol, 100, 1.5f);
            }

            static std::vector<PlayerRoot*> s_entities;
            GetEntitiesTo(s_entities, false, Menu::bEspShowTeam);
            const std::vector<PlayerRoot*>& entities = s_entities;
            bool aimActive = (GetAsyncKeyState(Menu::aimbotKey) & 0x8000) != 0;
            PlayerRoot* closestPlayer = (Menu::bAimbot && aimActive)
                ? ClosestInFOV(effectiveAimbotFov, Menu::bAimbotVisCheck, s_entities, -1)
                : nullptr;

            if (!s_entities.empty())
                Globals::InMatch = true;

            MovementHooks::ApplyIfNeeded();
            ApplyNoRecoilAndNoCameraShake();

            for (auto& entity : entities) {
                if (!Menu::bEsp) continue;

                PlayerHealth* playerHealth = entity->GetPlayerHealth();
                if (!playerHealth) continue;

                float healthPercent = playerHealth->GetHealthPercent();
                Vector3 rootPosition = entity->GetRootPosition();

                Vector3 headPos = rootPosition; headPos.y += 1.6f;
                Vector3 rootW2sPos = rootPosition;

                Vector2 headW2sPos, rootW2sPos2;
                float distance = Vector3::Distance(viewPlayer->GetRootPosition(), rootPosition);
                if (!WorldToScreen(headPos, &headW2sPos, viewMatrix) ||
                    !WorldToScreen(rootPosition, &rootW2sPos2, viewMatrix))
                    continue;

                ImVec2 headW2s(headW2sPos.x, headW2sPos.y);
                ImVec2 rootW2s(rootW2sPos2.x, rootW2sPos2.y);

                float height = rootW2s.y - headW2s.y;
                float width = height / 2.0f;
                ImVec2 boxTopLeft(headW2s.x - width / 2.0f, headW2s.y);
                ImVec2 boxBottomRight(headW2s.x + width / 2.0f, rootW2s.y);

                ImU32 colTracer = IM_COL32((int)(Menu::colorTracers[0] * 255),
                                           (int)(Menu::colorTracers[1] * 255),
                                           (int)(Menu::colorTracers[2] * 255),
                                           (int)(Menu::colorTracers[3] * 255));
                ImU32 colBox = IM_COL32((int)(Menu::colorEspBox[0] * 255),
                                        (int)(Menu::colorEspBox[1] * 255),
                                        (int)(Menu::colorEspBox[2] * 255),
                                        (int)(Menu::colorEspBox[3] * 255));

                bool isVis = entity->IsVisible();
                bool isTeam = entity->IsTeammate();

                if (Menu::bEspShowTeam && isTeam) {
                    if (Menu::bVisibilityCheck && !isVis)
                        colBox = IM_COL32(139, 0, 0, 200);
                    else
                        colBox = IM_COL32(100, 255, 255, 255);
                    colTracer = colBox;
                } else if (Menu::bVisibilityCheck && !isVis) {
                    colBox = IM_COL32(200, 30, 30, 150);
                    colTracer = IM_COL32(200, 30, 30, 150);
                }

                if (Menu::bSpawnProtectionIndicator) {
                    typedef bool(__fastcall *IsInvincible_t)(uint64_t);
                    static IsInvincible_t isInvFn = nullptr;
                    if (!isInvFn)
                        isInvFn = (IsInvincible_t)(Globals::ProjectModule + Offsets::rva::get_IsInvincible);
                    if (playerHealth && isInvFn && isInvFn((uint64_t)playerHealth)) {
                        float cx = headW2s.x, cy = headW2s.y - 18.f, s = 10.f;
                        BackgroundDrawList->AddQuadFilled(
                            ImVec2(cx, cy - s), ImVec2(cx + s, cy), ImVec2(cx, cy + s), ImVec2(cx - s, cy),
                            IM_COL32(255, 165, 0, 255));
                    }
                }

                if (Menu::bSnaplines) {
                    ImVec2 origin;
                    if (Menu::snaplineOrigin == 0) origin = ImVec2(Screen::ScreenCenter.x, 0.f);
                    else if (Menu::snaplineOrigin == 1) origin = ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y);
                    else origin = ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y * 2.f);
                    BackgroundDrawList->AddLine(origin, headW2s, colTracer, 1.0f);
                }

                if (Menu::bEspBox) {
                    float th = Menu::espBoxThickness;
                    if (th < 0.5f) th = 0.5f;
                    if (th > 6.f) th = 6.f;
                    BackgroundDrawList->AddRect(boxTopLeft, boxBottomRight, colBox, 0.0f, 0, th);
                }

                if (Menu::bHealthBar) {
                    float frac = (healthPercent / 100.f); if (frac > 1.f) frac = 1.f;
                    ImVec2 barL(boxTopLeft.x - 5.f, boxTopLeft.y), barR(boxTopLeft.x - 1.f, boxBottomRight.y);
                    BackgroundDrawList->AddRectFilled(barL, barR, IM_COL32(0, 0, 0, 180));
                    ImVec2 fillL(barL.x + 1.f, boxBottomRight.y - (height * frac)), fillR(barR.x - 1.f, boxBottomRight.y - 1.f);
                    BackgroundDrawList->AddRectFilled(fillL, fillR, IM_COL32((int)((1.f - frac) * 255), (int)(frac * 255), 0, 255));
                }

                if (Menu::bNames || Menu::bDistance) {
                    char line[128] = "";
                    if (Menu::bNames) {
                        NetPlayerData* nd = entity->GetNetPlayerData();
                        if (nd) {
                            PlayerConnectData* cd = nd->GetPlayerConnectData();
                            if (cd) {
                                UnityString* us = cd->GetNickName();
                                if (us) { strncpy_s(line, 128, us->ToString().c_str(), 60); line[60] = '\0'; }
                            }
                            if (!entity->IsRealPlayer()) strcat_s(line, " [BOT]");
                        }
                    }
                    if (Menu::bDistance) {
                        char d[24]; snprintf(d, 24, " %.0fm", distance);
                        strcat_s(line, d);
                    }
                    if (line[0]) {
                        ImVec2 ts = ImGui::CalcTextSize(line);
                        BackgroundDrawList->AddText(ImVec2(headW2s.x - ts.x * 0.5f, headW2s.y - ts.y - 2.f), IM_COL32(255, 255, 255, 255), line);
                    }
                }

                if (Menu::bTracers) {
                    ImVec2 origin;
                    if (Menu::tracerOrigin == 0) origin = ImVec2(Screen::ScreenCenter.x, 0.f);
                    else if (Menu::tracerOrigin == 1) origin = ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y);
                    else origin = ImVec2(Screen::ScreenCenter.x, Screen::ScreenSize.y);
                    BackgroundDrawList->AddLine(origin, headW2s, colTracer, 1.2f);
                }
            }

            if (Menu::bAimbot && closestPlayer) {
                Vector3 targetPos = closestPlayer->GetRootPosition();
                int bone = Menu::aimbotBone;
                if (Menu::bAimbotFollowCrouched) {
                    PlayerMovement* pm = closestPlayer->GetPlayerMovement();
                    if (pm && pm->IsCrouch()) bone = 2;
                }
                if (bone == 0) targetPos.y += 1.6f;
                else if (bone == 1) targetPos.y += 1.35f;
                else if (bone == 2) targetPos.y += 1.1f;
                else if (bone == 3) targetPos.y += 0.8f;

                Vector2 outPos;
                if (WorldToScreen(targetPos, &outPos, viewMatrix)) {
                    if (Menu::bTargetTracer) {
                        ImU32 colTracer = IM_COL32((int)(Menu::colorTracers[0] * 255), (int)(Menu::colorTracers[1] * 255), (int)(Menu::colorTracers[2] * 255), (int)(Menu::colorTracers[3] * 255));
                        BackgroundDrawList->AddLine(ImVec2(Screen::ScreenCenter.x, Screen::ScreenCenter.y), ImVec2(outPos.x, outPos.y), colTracer, 2.5f);
                    }
                    if (Menu::bTargetOrb) {
                        ImU32 colOrb = IM_COL32((int)(Menu::colorTargetOrb[0] * 255), (int)(Menu::colorTargetOrb[1] * 255), (int)(Menu::colorTargetOrb[2] * 255), (int)(Menu::colorTargetOrb[3] * 255));
                        BackgroundDrawList->AddCircleFilled(ImVec2(outPos.x, outPos.y), 8.f, colOrb);
                    }

                    if (aimActive) {
                        float dx = outPos.x - Screen::ScreenCenter.x;
                        float dy = outPos.y - Screen::ScreenCenter.y;
                        float distToTarget = sqrtf(dx * dx + dy * dy);
                        if (distToTarget >= 2.0f) {
                            float smoothX = Menu::aimbotSmoothingX >= 1.f ? Menu::aimbotSmoothingX : 1.f;
                            float smoothY = Menu::aimbotSmoothingY >= 1.f ? Menu::aimbotSmoothingY : 1.f;
                            float dt = ImGui::GetIO().DeltaTime;
                            if (dt <= 0.f || dt > 0.05f) dt = 0.016f;
                            if (dt < 0.001f) dt = 0.001f;
                            float strengthX = 30.0f / smoothX;
                            float strengthY = 30.0f / smoothY;
                            float stepFactorX = 1.0f - expf(-strengthX * dt);
                            float stepFactorY = 1.0f - expf(-strengthY * dt);
                            if (Menu::aimbotPathStyle == 0) {
                                if (stepFactorX > 0.92f) stepFactorX = 0.92f;
                                if (stepFactorY > 0.92f) stepFactorY = 0.92f;
                            } else {
                                stepFactorX *= 1.06f; stepFactorY *= 1.06f;
                                if (stepFactorX > 1.10f) stepFactorX = 1.10f;
                                if (stepFactorY > 1.10f) stepFactorY = 1.10f;
                            }
                            int moveX = (int)(dx * stepFactorX);
                            int moveY = (int)(dy * stepFactorY);
                            if (moveX != 0 || moveY != 0) {
                                if (moveX > kMaxAimMovePerFrame) moveX = kMaxAimMovePerFrame; else if (moveX < -kMaxAimMovePerFrame) moveX = -kMaxAimMovePerFrame;
                                if (moveY > kMaxAimMovePerFrame) moveY = kMaxAimMovePerFrame; else if (moveY < -kMaxAimMovePerFrame) moveY = -kMaxAimMovePerFrame;
                                mouse_event(MOUSEEVENTF_MOVE, moveX, moveY, 0, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    if (!Menu::bIsOpen) return;

    // Distinct style: dark slate + amber accent, larger padding, rounder
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.f;
    style.FrameRounding = 6.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 6.f;
    style.WindowPadding = ImVec2(14.f, 12.f);
    style.FramePadding = ImVec2(10.f, 6.f);
    style.ItemSpacing = ImVec2(10.f, 8.f);
    style.ScrollbarSize = 14.f;
    style.GrabMinSize = 12.f;
    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.96f);
    c[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.13f, 0.17f, 0.95f);
    c[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.98f);
    c[ImGuiCol_Border] = ImVec4(0.35f, 0.28f, 0.15f, 0.6f);
    c[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.17f, 0.20f, 1.f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.24f, 0.18f, 1.f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.26f, 0.14f, 1.f);
    c[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.14f, 0.08f, 1.f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.18f, 0.06f, 1.f);
    c[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.65f, 0.15f, 1.f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.9f, 0.55f, 0.1f, 1.f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 0.7f, 0.2f, 1.f);
    c[ImGuiCol_Button] = ImVec4(0.22f, 0.18f, 0.12f, 1.f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.28f, 0.10f, 1.f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.32f, 0.08f, 1.f);
    c[ImGuiCol_Header] = ImVec4(0.28f, 0.22f, 0.12f, 1.f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.40f, 0.30f, 0.12f, 1.f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.35f, 0.10f, 1.f);
    c[ImGuiCol_Tab] = ImVec4(0.20f, 0.16f, 0.10f, 1.f);
    c[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.25f, 0.08f, 1.f);
    c[ImGuiCol_TabActive] = ImVec4(0.28f, 0.20f, 0.06f, 1.f);
    c[ImGuiCol_Text] = ImVec4(0.92f, 0.90f, 0.85f, 1.f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.48f, 0.45f, 1.f);

    ImGui::SetNextWindowSize(ImVec2(480, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("CM Base Internal", &Menu::bIsOpen, ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox("Enable ESP", &Menu::bEsp);
            ImGui::Checkbox("Boxes", &Menu::bEspBox);
            ImGui::Checkbox("Tracers", &Menu::bTracers);
            if (Menu::bTracers) {
                const char* tracerOrigins[] = { "Top", "Center", "Bottom" };
                ImGui::Combo("Tracer origin", &Menu::tracerOrigin, tracerOrigins, 3);
            }
            ImGui::Checkbox("Show teammates", &Menu::bEspShowTeam);
            ImGui::Checkbox("Names", &Menu::bNames);
            ImGui::Checkbox("Distance", &Menu::bDistance);
            ImGui::Checkbox("Health bar", &Menu::bHealthBar);
            ImGui::Checkbox("Snaplines", &Menu::bSnaplines);
            ImGui::Checkbox("Spawn protection indicator", &Menu::bSpawnProtectionIndicator);
            ImGui::Checkbox("Visibility check", &Menu::bVisibilityCheck);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Aim")) {
            ImGui::Checkbox("Enable Aimbot", &Menu::bAimbot);
            ImGui::Checkbox("Draw FOV", &Menu::bDrawFov);
            ImGui::SliderFloat("FOV radius", &Menu::aimbotFov, 1.f, 850.f, "%.0f");
            ImGui::Checkbox("Visibility check", &Menu::bAimbotVisCheck);
            ImGui::SliderFloat("Smoothing X", &Menu::aimbotSmoothingX, 1.f, 20.f, "%.1f");
            ImGui::SliderFloat("Smoothing Y", &Menu::aimbotSmoothingY, 1.f, 20.f, "%.1f");
            const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
            ImGui::Combo("Target bone", &Menu::aimbotBone, bones, 4);
            const char* targeting[] = { "Closest to crosshair", "Closest distance", "Both" };
            ImGui::Combo("Target method", &Menu::aimbotTargeting, targeting, 3);
            ImGui::Checkbox("Skip spawn protection", &Menu::bSkipSpawnProtection);
            ImGui::Checkbox("Team check", &Menu::bTeamCheck);
            ImGui::Checkbox("Follow crouched (chest)", &Menu::bAimbotFollowCrouched);
            ImGui::Checkbox("Target tracer", &Menu::bTargetTracer);
            ImGui::Checkbox("Target orb", &Menu::bTargetOrb);
            HotkeyButton("Aimbot key (default: Mouse 2)", &Menu::aimbotKey);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Extra")) {
            ImGui::Checkbox("No recoil", &Menu::bNoRecoil);
            ImGui::Checkbox("No camera shake", &Menu::bNoCameraShake);
            ImGui::Separator();
            ImGui::Checkbox("Infinite ammo", &Menu::bInfiniteAmmo);
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");
            ImGui::Checkbox("Rapid fire", &Menu::bRapidFire);
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");
            ImGui::Checkbox("Infinite lethals", &Menu::bInfiniteLethals);
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");
            ImGui::Checkbox("Movement speed", &Menu::bMovementSpeed);
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");
            if (Menu::bMovementSpeed) {
                ImGui::SliderFloat("Run mult", &Menu::movementRunMult, 0.5f, 5.f, "%.2f");
                ImGui::SliderFloat("Sprint mult", &Menu::movementSprintMult, 0.5f, 5.f, "%.2f");
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Config")) {
            ImGui::Text("Save or load settings to a file next to the DLL.");
            if (ImGui::Button("Save config", ImVec2(160, 0))) SaveConfig();
            ImGui::SameLine();
            if (ImGui::Button("Load config", ImVec2(160, 0))) LoadConfig();
            ImGui::Spacing();
            ImGui::TextDisabled("File: cm_base_config.cfg");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings")) {
            HotkeyButton("Menu toggle", &Menu::menuToggleKey);
            HotkeyButton("Unload", &Menu::unloadKey);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
}
