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
#include "../Global.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC_t)(HWND, UINT, WPARAM, LPARAM);

class Renderer {
private:
    Present_t oPresent = nullptr;
    WNDPROC_t oWndProc = nullptr;
    HWND window = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    bool initImgui = false;

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
            g_UnloadRequested = true;

        if (g_UnloadRequested && renderer.initImgui) {
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

                // Load fonts
                ImGuiIO& io = ImGui::GetIO();
                char path[MAX_PATH];
                GetModuleFileNameA(g_hInjectModule, path, MAX_PATH);
                std::string dllDir = std::string(path);
                size_t lastSlash = dllDir.find_last_of("\\/");
                if (lastSlash != std::string::npos) dllDir = dllDir.substr(0, lastSlash);

                std::string oswaldPath = dllDir + "\\fonts\\Oswald-Regular.ttf";
                std::string iconsPath = dllDir + "\\fonts\\MaterialIcons-Regular.ttf";

                if (GetFileAttributesA(oswaldPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    io.Fonts->AddFontFromFileTTF(oswaldPath.c_str(), 18.0f);
                }
                
                if (GetFileAttributesA(iconsPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    static const ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
                    ImFontConfig icons_config;
                    icons_config.MergeMode = true;
                    icons_config.PixelSnapH = true;
                    icons_config.GlyphMinAdvanceX = 18.0f; // Force icons to have width
                    io.Fonts->AddFontFromFileTTF(iconsPath.c_str(), 20.0f, &icons_config, icon_ranges);
                }
                

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

        // FOV circle
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

        if (!entities.empty()) inMatch = true;
        WeaponHooks::g_InMatch = inMatch;
        RecoilPatch::Apply(inMatch, Config::no_recoil, Config::no_camera_shake);
        MovementHooks::ApplyIfNeeded(inMatch);

        CPlayer* closestPlayer = nullptr;
        bool aimActive = (GetAsyncKeyState(Config::aimbot_key) & 0x8000) != 0;
        if (Config::aimbot_enabled && aimActive) {
            closestPlayer = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
        }

        if (Config::triggerbot_enabled) {
            CPlayer* trigTarget = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
            Triggerbot::Run(trigTarget != nullptr && (GetAsyncKeyState(Config::aimbot_key) & 0x8000), Menu::bShowMenu);
        }

        if (Config::esp_enabled) {
            for (auto& entity : entities) {
                CPlayerHealth* playerHealth = entity->GetPlayerHealth();
                if (!playerHealth) continue;

                float healthPercent = playerHealth->GetHealthPercent();
                Vector3 rootPosition = entity->GetRootPosition();
                Vector3 headPos = rootPosition; headPos.y += 1.6f;

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

                ImU32 colBox;
                if (Config::esp_show_team && isTeam) {
                    colBox = IM_COL32((int)(Config::esp_color_team[0]*255), (int)(Config::esp_color_team[1]*255), (int)(Config::esp_color_team[2]*255), 255);
                } else {
                    colBox = IM_COL32((int)(Config::esp_color_enemy[0]*255), (int)(Config::esp_color_enemy[1]*255), (int)(Config::esp_color_enemy[2]*255), 255);
                }

                if (Config::esp_boxes) ESP::DrawCornerBox(boxX, boxY, width, height, colBox, Config::esp_box_thickness);
                if (Config::esp_health) ESP::DrawHealthBar(boxX - 6, boxY, 3, height, healthPercent, 100.0f);

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
                        DrawUtils::DrawGlowText(BackgroundDrawList, ImGui::GetFont(), ImGui::GetFontSize(), 
                            ImVec2(headW2sPos.x - ts.x * 0.5f, headW2sPos.y - ts.y - 4.f), 
                            IM_COL32(255, 255, 255, 255), line, 0.4f);
                    }
                }
            }
        }

        if (Config::aimbot_enabled && closestPlayer) {
            Aimbot::RunAimbot(closestPlayer, viewMatrix, ImGui::GetIO().DeltaTime);
        }
    }

public:
    static Renderer& Get() {
        static Renderer instance;
        return instance;
    }

    void Init() {
        if (MH_Initialize() != MH_OK) return;
        WeaponHooks::Init();

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        DXGI_SWAP_CHAIN_DESC sd; ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.Width = 2; sd.BufferDesc.Height = 2;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        WNDCLASSEXA wc = {}; wc.cbSize = sizeof(wc); wc.style = CS_CLASSDC;
        wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "NexusHook"; RegisterClassExA(&wc);
        HWND hwnd = CreateWindowExA(0, "NexusHook", "NexusHook", WS_OVERLAPPEDWINDOW, 100, 100, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        sd.OutputWindow = hwnd;

        IDXGISwapChain* swapChain = nullptr; ID3D11Device* device = nullptr; ID3D11DeviceContext* context = nullptr;
        if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context))) {
            DestroyWindow(hwnd); UnregisterClassA("NexusHook", wc.hInstance); return;
        }

        void** pVTable = *reinterpret_cast<void***>(swapChain);
        void* targetPresent = pVTable[8];
        swapChain->Release(); device->Release(); context->Release();
        DestroyWindow(hwnd); UnregisterClassA("NexusHook", wc.hInstance);

        MH_CreateHook(targetPresent, &hkPresent, reinterpret_cast<void**>(&oPresent));
        MH_EnableHook(targetPresent);
        while (!g_UnloadRequested) Sleep(100);
        Sleep(500);
    }
};
