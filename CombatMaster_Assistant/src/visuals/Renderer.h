#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <MinHook.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <string>
#include <cmath>
#include <vector>

#include "WorldToScreen.h"
#include "../ui/Menu.h"
#include "../ui/Theme.h"
#include "ESP.h"

#include "../utils/Hotkeys.h"
#include "../utils/Logger.h"
#include "../targeting/aimassist.h"
#include "../targeting/assittrigger.h"
#include "../hooks/WeaponHooks.h"
#include "../hooks/RecoilPatch.h"
#include "../hooks/MovementHooks.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC_t)(HWND, UINT, WPARAM, LPARAM);

// Global module handle for clean unload
inline HMODULE g_hInjectModule = nullptr;

class Renderer {
private:
    Present_t oPresent = nullptr;
    WNDPROC_t oWndProc = nullptr;
    HWND window = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    bool initImgui = false;
    bool unloadRequested = false;

    int screenWidth = 0;
    int screenHeight = 0;

    Renderer() {}

    static LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (Menu::bShowMenu) {
            ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP ||
                uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL)
                return true;
        }
        return CallWindowProc(Get().oWndProc, hWnd, uMsg, wParam, lParam);
    }

    static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        auto& renderer = Get();

        // --- Toggle menu ---
        if (GetAsyncKeyState(Config::menu_toggle_key) & 1)
            Menu::bShowMenu = !Menu::bShowMenu;

        // --- Clean unload on DELETE key ---
        if (GetAsyncKeyState(Config::unload_key) & 1)
            renderer.unloadRequested = true;

        if (renderer.unloadRequested && renderer.initImgui) {
            // Restore movement hooks
            MovementHooks::Restore();
            // Unhook weapon hooks
            WeaponHooks::Unhook();
            // Restore WndProc
            if (renderer.window && renderer.oWndProc)
                SetWindowLongPtr(renderer.window, GWLP_WNDPROC, (LONG_PTR)renderer.oWndProc);
            // Shutdown ImGui
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            // Release D3D resources
            if (renderer.mainRenderTargetView) {
                renderer.mainRenderTargetView->Release();
                renderer.mainRenderTargetView = nullptr;
            }
            renderer.pContext = nullptr;
            renderer.pDevice = nullptr;
            renderer.window = nullptr;
            renderer.oWndProc = nullptr;
            renderer.initImgui = false;
            // Disable Present hook
            MH_DisableHook(MH_ALL_HOOKS);
            // Schedule FreeLibrary on separate thread
            if (g_hInjectModule) {
                CreateThread(nullptr, 0, [](LPVOID m) -> DWORD {
                    Sleep(100);
                    FreeLibraryAndExitThread((HMODULE)m, 0);
                    return 0;
                }, g_hInjectModule, 0, nullptr);
            }
            return renderer.oPresent(pSwapChain, SyncInterval, Flags);
        }

        if (!renderer.initImgui) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&renderer.pDevice))) {
                renderer.pDevice->GetImmediateContext(&renderer.pContext);
                DXGI_SWAP_CHAIN_DESC sd;
                pSwapChain->GetDesc(&sd);
                renderer.window = sd.OutputWindow;

                // Get screen size from swap chain (handles windowed mode correctly)
                renderer.screenWidth = sd.BufferDesc.Width;
                renderer.screenHeight = sd.BufferDesc.Height;
                if (renderer.screenWidth == 0 || renderer.screenHeight == 0) {
                    renderer.screenWidth = GetSystemMetrics(SM_CXSCREEN);
                    renderer.screenHeight = GetSystemMetrics(SM_CYSCREEN);
                }

                Visuals::ScreenCenter.x = renderer.screenWidth / 2.0f;
                Visuals::ScreenCenter.y = renderer.screenHeight / 2.0f;

                ID3D11Texture2D* pBackBuffer;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
                renderer.pDevice->CreateRenderTargetView(pBackBuffer, NULL, &renderer.mainRenderTargetView);
                pBackBuffer->Release();

                renderer.oWndProc = (WNDPROC_t)SetWindowLongPtr(renderer.window, GWLP_WNDPROC, (LONG_PTR)WndProc);

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                Theme::Apply();

                ImGui_ImplWin32_Init(renderer.window);
                ImGui_ImplDX11_Init(renderer.pDevice, renderer.pContext);

                renderer.initImgui = true;
            } else {
                return renderer.oPresent(pSwapChain, SyncInterval, Flags);
            }
        }

        // Update screen size from swap chain each frame
        DXGI_SWAP_CHAIN_DESC sd;
        if (SUCCEEDED(pSwapChain->GetDesc(&sd)) && sd.BufferDesc.Width > 0 && sd.BufferDesc.Height > 0) {
            renderer.screenWidth = sd.BufferDesc.Width;
            renderer.screenHeight = sd.BufferDesc.Height;
            Visuals::ScreenCenter.x = renderer.screenWidth / 2.0f;
            Visuals::ScreenCenter.y = renderer.screenHeight / 2.0f;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        renderer.DrawCheat();
        Menu::Draw((ImTextureID)nullptr);

        ImGui::Render();
        renderer.pContext->OMSetRenderTargets(1, &renderer.mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return renderer.oPresent(pSwapChain, SyncInterval, Flags);
    }

    void DrawCheat() {
        ImDrawList* BackgroundDrawList = ImGui::GetBackgroundDrawList();
        bool inMatch = false;

        // FOV circle (always draw if enabled even outside match)
        if (Visuals::ScreenCenter.y >= 10.f && Config::aimbot_draw_fov && Config::aimbot_enabled) {
            ImU32 fovCol = IM_COL32(
                (int)(Config::esp_color_fov[0] * 255), (int)(Config::esp_color_fov[1] * 255),
                (int)(Config::esp_color_fov[2] * 255), (int)(Config::esp_color_fov[3] * 255));
            BackgroundDrawList->AddCircle(
                ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y),
                Config::aimbot_fov, fovCol, 100, 1.5f);
        }

        CPlayer* localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer) return;

        // Spectator camera support when dead
        CPlayer* viewPlayer = localPlayer;
        if (localPlayer->IsDead()) {
            CPlayer* spectated = CPlayerRoot::GetSpectatorPlayer(localPlayer);
            if (spectated && spectated->GetCamera())
                viewPlayer = spectated;
        }

        Camera* viewCamera = viewPlayer->GetCamera();
        if (!viewCamera) return;

        Matrix4x4 viewMatrix = viewCamera->GetViewMatrix();
        Vector3 localPos = viewPlayer->GetRootPosition();

        // Build entity list
        IL2CPP::Array<CPlayer*>* playersArray = CPlayerRoot::GetAllPlayersArray();
        int maxPlayers = CPlayerRoot::GetAllPlayersCount();
        if (!playersArray || maxPlayers <= 0) return;

        std::vector<CPlayer*> entities;
        entities.reserve(maxPlayers);
        for (int i = 0; i < maxPlayers; i++) {
            CPlayer* player = playersArray->vector[i];
            if (!player || player == localPlayer) continue;
            if (!Config::esp_show_team && player->IsTeammate(localPlayer)) continue;
            if (player->IsDead()) continue;
            entities.push_back(player);
        }

        if (!entities.empty())
            inMatch = true;

        // Update weapon hooks state
        WeaponHooks::g_InMatch = inMatch;

        // Apply per-frame patches
        RecoilPatch::Apply(inMatch, Config::no_recoil, Config::no_camera_shake);
        MovementHooks::ApplyIfNeeded(inMatch);

        // --- Aimbot ---
        CPlayer* closestPlayer = nullptr;
        bool aimActive = (GetAsyncKeyState(Config::aimbot_key) & 0x8000) != 0;
        if (Config::aimbot_enabled && aimActive) {
            closestPlayer = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
        }

        // --- Triggerbot ---
        if (Config::triggerbot_enabled) {
            CPlayer* trigTarget = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
            Triggerbot::Run(trigTarget != nullptr && (GetAsyncKeyState(Config::aimbot_key) & 0x8000), Menu::bShowMenu);
        }

        // --- ESP Rendering ---
        if (Config::esp_enabled) {
            for (auto& entity : entities) {
                CPlayerHealth* playerHealth = entity->GetPlayerHealth();
                if (!playerHealth) continue;

                float healthPercent = playerHealth->GetHealthPercent();
                Vector3 rootPosition = entity->GetRootPosition();
                Vector3 headPos = rootPosition;
                headPos.y += 1.6f;

                float distance = Vector3(localPos.x, localPos.y, localPos.z).Distance(rootPosition);
                if (distance / 100.f > Config::esp_max_distance) continue;

                Vector2 headW2sPos, rootW2sPos;
                if (!Visuals::WorldToScreen(headPos, &headW2sPos, viewMatrix) ||
                    !Visuals::WorldToScreen(rootPosition, &rootW2sPos, viewMatrix))
                    continue;

                float height = rootW2sPos.y - headW2sPos.y;
                float width = height / 2.0f;
                float boxX = headW2sPos.x - (width / 2.0f);
                float boxY = headW2sPos.y;

                bool isVis = entity->isVisible();
                bool isTeam = entity->IsTeammate(localPlayer);

                // Color logic with visibility check (from reference)
                ImU32 colBox, colTracer;
                if (Config::esp_show_team && isTeam) {
                    if (Config::esp_visibility_check && !isVis) {
                        colBox = IM_COL32(139, 0, 0, 200);
                    } else {
                        colBox = IM_COL32(
                            (int)(Config::esp_color_team[0] * 255), (int)(Config::esp_color_team[1] * 255),
                            (int)(Config::esp_color_team[2] * 255), (int)(Config::esp_color_team[3] * 255));
                    }
                    colTracer = colBox;
                } else {
                    if (Config::esp_visibility_check && !isVis) {
                        colBox = IM_COL32(200, 30, 30, 150);
                        colTracer = IM_COL32(200, 30, 30, 150);
                    } else {
                        colBox = IM_COL32(
                            (int)(Config::esp_color_enemy[0] * 255), (int)(Config::esp_color_enemy[1] * 255),
                            (int)(Config::esp_color_enemy[2] * 255), (int)(Config::esp_color_enemy[3] * 255));
                        colTracer = colBox;
                    }
                }

                // Spawn protection indicator (orange diamond)
                if (Config::esp_spawn_protection_indicator && entity->IsInvincible()) {
                    float cx = headW2sPos.x, cy = headW2sPos.y - 18.f, s = 10.f;
                    BackgroundDrawList->AddQuadFilled(
                        ImVec2(cx, cy - s), ImVec2(cx + s, cy), ImVec2(cx, cy + s), ImVec2(cx - s, cy),
                        IM_COL32(255, 165, 0, 255));
                }

                // Snaplines
                if (Config::esp_lines) {
                    ImVec2 origin;
                    if (Config::esp_snapline_origin == 0) origin = ImVec2(Visuals::ScreenCenter.x, 0.f);
                    else if (Config::esp_snapline_origin == 1) origin = ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y);
                    else origin = ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y * 2.f);
                    BackgroundDrawList->AddLine(origin, ImVec2(headW2sPos.x, headW2sPos.y), colTracer, 1.0f);
                }

                // Boxes
                if (Config::esp_boxes) {
                    ESP::DrawCornerBox(boxX, boxY, width, height, colBox, Config::esp_box_thickness);
                }

                // Health bar
                if (Config::esp_health) {
                    ESP::DrawHealthBar(boxX - 6, boxY, 3, height, healthPercent, 100.0f);
                }

                // Names & Distance
                if (Config::esp_names || Config::esp_distance) {
                    char line[128] = "";
                    if (Config::esp_names) {
                        CPlayerConnectData* cd = entity->GetConnectData();
                        if (cd) {
                            UnityString* us = cd->GetNickName();
                            if (us) {
                                std::string name = us->ToString();
                                strncpy_s(line, 128, name.c_str(), 60);
                                line[60] = '\0';
                            }
                            if (!entity->isRealPlayer()) strcat_s(line, " [BOT]");
                        }
                    }
                    if (Config::esp_distance) {
                        char d[24]; snprintf(d, 24, " %.0fm", distance);
                        strcat_s(line, d);
                    }
                    if (line[0]) {
                        ImVec2 ts = ImGui::CalcTextSize(line);
                        BackgroundDrawList->AddText(
                            ImVec2(headW2sPos.x - ts.x * 0.5f, headW2sPos.y - ts.y - 2.f),
                            IM_COL32(255, 255, 255, 255), line);
                    }
                }

                // Tracers
                if (Config::esp_tracers) {
                    ImVec2 origin;
                    if (Config::esp_tracer_origin == 0) origin = ImVec2(Visuals::ScreenCenter.x, 0.f);
                    else if (Config::esp_tracer_origin == 1) origin = ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y);
                    else origin = ImVec2(Visuals::ScreenCenter.x, (float)screenHeight);
                    BackgroundDrawList->AddLine(origin, ImVec2(headW2sPos.x, headW2sPos.y), colTracer, 1.2f);
                }
            }
        }

        // --- Aimbot Execution & Target Visuals ---
        if (Config::aimbot_enabled && closestPlayer) {
            Aimbot::RunAimbot(closestPlayer, viewMatrix, ImGui::GetIO().DeltaTime);

            // Target tracer & orb
            Vector3 targetPos = closestPlayer->GetRootPosition();
            int bone = Config::aimbot_bone;
            if (Config::aimbot_follow_crouched && closestPlayer->isCrouch()) bone = 2;
            if (bone == 0) targetPos.y += 1.6f;
            else if (bone == 1) targetPos.y += 1.35f;
            else if (bone == 2) targetPos.y += 1.1f;
            else targetPos.y += 0.8f;

            Vector2 outPos;
            if (Visuals::WorldToScreen(targetPos, &outPos, viewMatrix)) {
                if (Config::aimbot_target_tracer) {
                    BackgroundDrawList->AddLine(
                        ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y),
                        ImVec2(outPos.x, outPos.y),
                        IM_COL32((int)(Config::esp_color_enemy[0]*255), (int)(Config::esp_color_enemy[1]*255),
                                 (int)(Config::esp_color_enemy[2]*255), (int)(Config::esp_color_enemy[3]*255)), 2.5f);
                }
                if (Config::aimbot_target_orb) {
                    ImU32 colOrb = IM_COL32(
                        (int)(Config::esp_color_target_orb[0]*255), (int)(Config::esp_color_target_orb[1]*255),
                        (int)(Config::esp_color_target_orb[2]*255), (int)(Config::esp_color_target_orb[3]*255));
                    BackgroundDrawList->AddCircleFilled(ImVec2(outPos.x, outPos.y), 8.f, colOrb);
                }
            }
        }
    }

public:
    static Renderer& Get() {
        static Renderer instance;
        return instance;
    }

    void Init() {
        if (MH_Initialize() != MH_OK) {
            Logger::Error("MinHook failed to initialize");
            return;
        }

        // Initialize weapon hooks
        if (!WeaponHooks::Init()) {
            Logger::Log("Warning: Some weapon hooks failed to initialize");
        }

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.Width = 2;
        sd.BufferDesc.Height = 2;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

        // Create hidden window for vtable discovery
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "NexusHook";
        RegisterClassExA(&wc);
        HWND hwnd = CreateWindowExA(0, "NexusHook", "NexusHook", WS_OVERLAPPEDWINDOW,
            100, 100, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain* swapChain = nullptr;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;

        if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2,
                D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context)))
        {
            Logger::Error("Failed to create dummy D3D11 device.");
            DestroyWindow(hwnd);
            UnregisterClassA("NexusHook", wc.hInstance);
            return;
        }

        void** pVTable = *reinterpret_cast<void***>(swapChain);
        void* targetPresent = pVTable[8];

        swapChain->Release();
        device->Release();
        context->Release();
        DestroyWindow(hwnd);
        UnregisterClassA("NexusHook", wc.hInstance);

        if (MH_CreateHook(targetPresent, &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) {
            Logger::Error("Failed to create hook for Present");
            return;
        }

        if (MH_EnableHook(targetPresent) != MH_OK) {
            Logger::Error("Failed to enable hook for Present");
            return;
        }

        Logger::Log("DX11 Present hooked successfully!");

        // Wait for unload signal instead of infinite loop
        while (!unloadRequested) {
            Sleep(100);
        }
        // Small delay to let the unload in hkPresent complete
        Sleep(500);
    }
};
