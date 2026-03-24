#pragma once
#include <windows.h>
#include <d3d9.h>
#include <dwmapi.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>
#include <string>
#include <cmath>

#include "WorldToScreen.h"
#include "../ui/Menu.h"
#include "../ui/Theme.h"
#include "ESP.h"
#include "../game/Game.h"
#include "../utils/Hotkeys.h"
#include "../targeting/Aimbot.h"
#include "../targeting/Triggerbot.h"
#include "../utils/ImageLoader.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Renderer {
private:
    HWND hwnd = nullptr;
    IDirect3D9* pD3D = nullptr;
    IDirect3DDevice9* pd3dDevice = nullptr;
    D3DPRESENT_PARAMETERS d3dpp = {};
    int screenWidth = 0;
    int screenHeight = 0;
    IDirect3DTexture9* logoTexture = nullptr;

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_SIZE:
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0; 
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    bool InitD3D(HWND hWnd) {
        if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
            return false;

        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
        d3dpp.EnableAutoDepthStencil = TRUE;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

        if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice) < 0)
            return false;

        return true;
    }

    void CleanupDeviceD3D() {
        if (logoTexture) { logoTexture->Release(); logoTexture = nullptr; }
        if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = nullptr; }
        if (pD3D) { pD3D->Release(); pD3D = nullptr; }
    }
    
    void DrawOffScreenArrow(const FVector& targetLoc, const Camera& camera, ImU32 color) {
        FVector camLoc = camera.GetLocation();
        FRotator camRot = camera.GetRotation();
        
        float deltaX = targetLoc.X - camLoc.X;
        float deltaY = targetLoc.Y - camLoc.Y;
        float dist = std::sqrt(deltaX*deltaX + deltaY*deltaY);
        if (dist < 100.0f) return;
        
        float angleToTarget = std::atan2(deltaY, deltaX);
        float camYawRad = camRot.Yaw * (M_PI / 180.0f);
        
        float diff = angleToTarget - camYawRad;
        
        while (diff > M_PI) diff -= 2 * M_PI;
        while (diff < -M_PI) diff += 2 * M_PI;
        
        float radius = screenHeight / 3.0f; 
        float drawX = (screenWidth / 2.0f) + radius * std::sin(diff);
        float drawY = (screenHeight / 2.0f) - radius * std::cos(diff);
        
        ImVec2 p1(drawX + 15 * std::sin(diff), drawY - 15 * std::cos(diff));
        ImVec2 p2(drawX + 10 * std::sin(diff + 2.3f), drawY - 10 * std::cos(diff + 2.3f));
        ImVec2 p3(drawX + 10 * std::sin(diff - 2.3f), drawY - 10 * std::cos(diff - 2.3f));
        
        ImGui::GetBackgroundDrawList()->AddTriangleFilled(p1, p2, p3, color);
    }

    void DrawESP() {
        if (!Config::esp_enabled) return;
        
        Game game;
        if (!game.Update()) return;
        
        LocalPlayer localPlayer = game.GetLocalPlayer();
        Camera camera = game.GetCamera();
        
        if (!localPlayer.IsValid() || !camera.IsValid()) return;
        
        auto players = game.GetPlayers();
        int localTeam = localPlayer.GetTeamId();
        
        std::optional<Entity> aimTarget = std::nullopt;
        
        if (Config::aimbot_enabled || Config::triggerbot_enabled) {
            aimTarget = Aimbot::GetBestTarget(players, localPlayer, camera, screenWidth, screenHeight);
            
            if (aimTarget.has_value() && Config::aimbot_enabled && Hotkeys::IsPressed(VK_RBUTTON)) { 
                Aimbot::AimAt(aimTarget->GetLocation(), localPlayer, camera);
            }
            
            if (Config::triggerbot_enabled) {
                Triggerbot::Run(aimTarget.has_value() && Hotkeys::IsPressed(VK_RBUTTON));
            }
        }
        
        for (auto& player : players) {
            if (player.GetAddress() == localPlayer.GetAddress()) continue;
            
            float health = player.GetHealth();
            if (health <= 0) continue;
            
            bool isTeam = (player.GetTeamId() == localTeam && localTeam != 255);
            ImU32 color = isTeam ? 
                ImGui::ColorConvertFloat4ToU32(ImVec4(Config::esp_color_team[0], Config::esp_color_team[1], Config::esp_color_team[2], Config::esp_color_team[3])) : 
                ImGui::ColorConvertFloat4ToU32(ImVec4(Config::esp_color_enemy[0], Config::esp_color_enemy[1], Config::esp_color_enemy[2], Config::esp_color_enemy[3]));
            
            FVector location = player.GetLocation();
            FVector screenPos;
            
            float dist = location.Distance(localPlayer.GetLocation()) / 100.0f;
            if (dist > Config::esp_max_distance) continue;
            
            if (Visuals::WorldToScreen(location, camera, screenWidth, screenHeight, screenPos)) {
                
                if (screenPos.X < -500 || screenPos.X > screenWidth + 500 || screenPos.Y < -500 || screenPos.Y > screenHeight + 500) {
                    continue; 
                }

                float h = 10000.0f / screenPos.Z; 
                float w = h / 2.0f;
                float x = screenPos.X - (w / 2.0f);
                float y = screenPos.Y - h;

                if (Config::esp_boxes) {
                    ESP::DrawCornerBox(x, y, w, h, color, 1.5f);
                }

                if (Config::esp_health) {
                    ESP::DrawHealthBar(x - 6, y, 3, h, health, player.GetMaxHealth());
                }

                if (Config::esp_names) {
                    std::wstring wname = player.GetPlayerName();
                    if (!wname.empty()) {
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wname[0], (int)wname.size(), NULL, 0, NULL, NULL);
                        std::string narrowName(size_needed, 0);
                        WideCharToMultiByte(CP_UTF8, 0, &wname[0], (int)wname.size(), &narrowName[0], size_needed, NULL, NULL);
                        ESP::DrawTextCentered(narrowName, screenPos.X, y - 16, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
                    }
                }

                if (Config::esp_distance) {
                    char distStr[32];
                    sprintf_s(distStr, "%.1fm", dist);
                    ESP::DrawTextCentered(distStr, screenPos.X, y + h + 4, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
                }

                if (Config::esp_lines) {
                    ESP::DrawLine(screenWidth / 2, screenHeight, screenPos.X, screenPos.Y + h, color, 1.5f);
                }
            } else {
                if (!isTeam) { 
                     DrawOffScreenArrow(location, camera, color);
                }
            }
        }
        
        if (Config::aimbot_enabled && Menu::bShowMenu) {
             ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screenWidth / 2.0f, screenHeight / 2.0f), Config::aimbot_fov * 5.0f, ImGui::GetColorU32(ImVec4(0, 1, 1, 0.4f)), 64, 1.0f);
        }
    }

public:
    int Run() {
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);

        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Overlay", nullptr };
        ::RegisterClassExW(&wc);
        hwnd = ::CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            wc.lpszClassName, L"Nexus Overlay", WS_POPUP, 
            0, 0, screenWidth, screenHeight, nullptr, nullptr, wc.hInstance, nullptr);

        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY); 
        
        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        if (!InitD3D(hwnd)) {
            CleanupDeviceD3D();
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        // Advanced Custom Font Loading Strategy
        ImFontConfig font_cfg;
        font_cfg.OversampleH = 2;
        font_cfg.OversampleV = 2;

        if (GetFileAttributesA("fonts/Roboto-Medium.ttf") != INVALID_FILE_ATTRIBUTES) {
            io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f, &font_cfg); // Base Text Font
        } else {
            // Fallback if missing
            io.Fonts->AddFontDefault();
        }

        // Texture Loading for Menu Logo
        ImageLoader::LoadTextureFromFile("images/logo.png", pd3dDevice, &logoTexture, nullptr, nullptr);

        Theme::Apply();

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX9_Init(pd3dDevice);

        bool done = false;
        while (!done) {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    done = true;
            }
            if (done) break;

            if (Hotkeys::WasKeyPressed(VK_INSERT)) {
                Menu::bShowMenu = !Menu::bShowMenu;
                
                long style = GetWindowLong(hwnd, GWL_EXSTYLE);
                if (Menu::bShowMenu) {
                    style &= ~WS_EX_TRANSPARENT;
                    SetForegroundWindow(hwnd);
                } else {
                    style |= WS_EX_TRANSPARENT;
                }
                SetWindowLong(hwnd, GWL_EXSTYLE, style);
            }

            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (!Memory::Get().IsAttached()) {
                static DWORD lastAttachTime = 0;
                if (GetTickCount() - lastAttachTime > 2000) {
                    Memory::Get().Attach(L"CombatMaster.exe");
                    lastAttachTime = GetTickCount();
                }
            } else {
                DrawESP();
            }

            // Draw Menu, passing the logo texture id (could be null, handled inside Menu::Draw)
            Menu::Draw((ImTextureID)logoTexture);

            ImGui::EndFrame();

            pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
            pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(0, 0, 0, 0);
            pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

            if (pd3dDevice->BeginScene() >= 0) {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                pd3dDevice->EndScene();
            }
            HRESULT result = pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
            if (result == D3DERR_DEVICELOST && pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                pd3dDevice->Reset(&d3dpp);
        }

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

        return 0;
    }
};
