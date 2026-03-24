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

#include "WorldToScreen.h"
#include "../ui/Menu.h"
#include "../ui/Theme.h"
#include "ESP.h"
#include "../game/Game.h"
#include "../utils/Hotkeys.h"
#include "../targeting/aimassist.h"
#include "../targeting/assittrigger.h"

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
            // Block game input when menu is open
            if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_mousewheel)
                return true;
        }
        
        if (Hotkeys::WasKeyPressed(VK_INSERT) || (uMsg == WM_KEYDOWN && wParam == VK_INSERT)) {
            Menu::bShowMenu = !Menu::bShowMenu;
            return true;
        }

        return CallWindowProc(Get().oWndProc, hWnd, uMsg, wParam, lParam);
    }

    static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        auto& renderer = Get();

        if (!renderer.initImgui) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&renderer.pDevice))) {
                renderer.pDevice->GetImmediateContext(&renderer.pContext);
                DXGI_SWAP_CHAIN_DESC sd;
                pSwapChain->GetDesc(&sd);
                renderer.window = sd.OutputWindow;
                
                renderer.screenWidth = GetSystemMetrics(SM_CXSCREEN);
                renderer.screenHeight = GetSystemMetrics(SM_CYSCREEN);

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

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        renderer.DrawCheat();
        Menu::Draw(nullptr);

        ImGui::Render();
        renderer.pContext->OMSetRenderTargets(1, &renderer.mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return renderer.oPresent(pSwapChain, SyncInterval, Flags);
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

    void DrawCheat() {
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
                // Modified triggerbot to pass crosshair and menu info
                Triggerbot::Run(aimTarget.has_value() && Hotkeys::IsPressed(VK_RBUTTON), Menu::bShowMenu);
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
                if (screenPos.X < -500 || screenPos.X > screenWidth + 500 || screenPos.Y < -500 || screenPos.Y > screenHeight + 500) continue; 

                float h = 10000.0f / screenPos.Z; 
                float w = h / 2.0f;
                float x = screenPos.X - (w / 2.0f);
                float y = screenPos.Y - h;

                if (Config::esp_boxes) ESP::DrawCornerBox(x, y, w, h, color, 1.5f);
                if (Config::esp_health) ESP::DrawHealthBar(x - 6, y, 3, h, health, player.GetMaxHealth());

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

                if (Config::esp_lines) ESP::DrawLine(screenWidth / 2, screenHeight, screenPos.X, screenPos.Y + h, color, 1.5f);
            } else {
                if (!isTeam) DrawOffScreenArrow(location, camera, color);
            }
        }
        
        if (Config::aimbot_enabled && Menu::bShowMenu) {
             ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screenWidth / 2.0f, screenHeight / 2.0f), Config::aimbot_fov * 5.0f, ImGui::GetColorU32(ImVec4(0, 1, 1, 0.4f)), 64, 1.0f);
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

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
        
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = GetDesktopWindow(); 
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain* swapChain = nullptr;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;

        if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2, 
                D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context))) 
        {
             Logger::Error("Failed to create dummy D3D11 device. The game might be running DX12 or another API.");
             return;
        }

        void** pVTable = *reinterpret_cast<void***>(swapChain);
        void* targetPresent = pVTable[8]; 

        swapChain->Release();
        device->Release();
        context->Release();

        if (MH_CreateHook(targetPresent, &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) {
             Logger::Error("Failed to create hook for Present");
             return;
        }

        if (MH_EnableHook(targetPresent) != MH_OK) {
             Logger::Error("Failed to enable hook for Present");
             return;
        }
        
        Logger::Log("DX11 Present hooked successfully!");
        
        // Wait permanently so the DLL thread doesn't exit until unloading
        while(true) {
            Sleep(100);
        }
    }
};
