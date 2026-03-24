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
            if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL)
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
                
                ScreenCenter.x = renderer.screenWidth / 2.0f;
                ScreenCenter.y = renderer.screenHeight / 2.0f;

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
        Menu::Draw((ImTextureID)nullptr);

        ImGui::Render();
        renderer.pContext->OMSetRenderTargets(1, &renderer.mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return renderer.oPresent(pSwapChain, SyncInterval, Flags);
    }

    void DrawCheat() {
        if (!Config::esp_enabled && !Config::aimbot_enabled && !Config::triggerbot_enabled) return;
        
        CPlayer* localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer) return;

        Camera* camera = localPlayer->GetCamera();
        if (!camera) return;

        Matrix4x4 viewMatrix = camera->GetViewMatrix();

        IL2CPP::Array<CPlayer*>* playersArray = CPlayerRoot::GetAllPlayersArray();
        int maxPlayers = CPlayerRoot::GetAllPlayersCount();
        if (!playersArray || maxPlayers <= 0) return;
        
        int localTeam = localPlayer->GetConnectData() ? localPlayer->GetConnectData()->GetTeamId() : 255;
        Vector3 localPos = localPlayer->GetRootPosition();
        
        // --- Aimbot / Triggerbot Engine ---
        if (Config::aimbot_enabled || Config::triggerbot_enabled) {
            CPlayer* aimTarget = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
            
            if (aimTarget && Config::aimbot_enabled && Hotkeys::IsPressed(VK_RBUTTON)) { 
                Aimbot::RunAimbot(aimTarget, viewMatrix, ImGui::GetIO().DeltaTime);
            }
            
            if (Config::triggerbot_enabled) {
                Triggerbot::Run(aimTarget != nullptr && Hotkeys::IsPressed(VK_RBUTTON), Menu::bShowMenu);
            }
        }
        
        if (!Config::esp_enabled) return;

        // --- ESP Engine ---
        for (int i = 0; i < maxPlayers; i++) {
            CPlayer* player = playersArray->vector[i];
            
            if (!player || player == localPlayer) continue;
            
            float health = player->GetHealth();
            if (health <= 0) continue;
            
            int team = player->GetConnectData() ? player->GetConnectData()->GetTeamId() : 255;
            bool isTeam = (team == localTeam && localTeam != 0); // 0 Usually means FFA 

            ImU32 color = isTeam ? 
                ImGui::ColorConvertFloat4ToU32(ImVec4(Config::esp_color_team[0], Config::esp_color_team[1], Config::esp_color_team[2], Config::esp_color_team[3])) : 
                ImGui::ColorConvertFloat4ToU32(ImVec4(Config::esp_color_enemy[0], Config::esp_color_enemy[1], Config::esp_color_enemy[2], Config::esp_color_enemy[3]));
            
            Vector3 location = player->GetRootPosition();
            Vector3 headLocation = location;
            headLocation.y += 1.8f; // Head Offset
            
            float dist = Vector3(localPos.x, localPos.y, localPos.z).Distance(location) / 100.0f;
            if (dist > Config::esp_max_distance) continue;
            
            Vector2 screenPos, headScreenPos;
            
            if (Visuals::WorldToScreen(location, &screenPos, viewMatrix) && Visuals::WorldToScreen(headLocation, &headScreenPos, viewMatrix)) {
                
                // Calculate dynamic box dimensions
                float h = screenPos.y - headScreenPos.y; 
                if (h < 0) h = -h;
                float w = h / 2.0f;
                float x = headScreenPos.x - (w / 2.0f);
                float y = headScreenPos.y;

                if (Config::esp_boxes) ESP::DrawCornerBox(x, y, w, h, color, 1.5f);
                if (Config::esp_health) ESP::DrawHealthBar(x - 6, y, 3, h, health, 100.0f);

                if (Config::esp_names) {
                    if (player->GetConnectData()) {
                        std::wstring wname = player->GetConnectData()->NickName();
                        std::string narrowName(wname.begin(), wname.end());
                        ESP::DrawTextCentered(narrowName, headScreenPos.x, y - 16, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
                    }
                }

                if (Config::esp_distance) {
                    char distStr[32];
                    sprintf_s(distStr, "%.1fm", dist);
                    ESP::DrawTextCentered(distStr, headScreenPos.x, y + h + 4, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
                }

                if (Config::esp_lines) ESP::DrawLine(screenWidth / 2, screenHeight, headScreenPos.x, screenPos.y, color, 1.5f);
            } 
        }
        
        if (Config::aimbot_enabled && Menu::bShowMenu) {
             ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screenWidth / 2.0f, screenHeight / 2.0f), Config::aimbot_fov, ImGui::GetColorU32(ImVec4(0, 1, 1, 0.4f)), 64, 1.0f);
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
