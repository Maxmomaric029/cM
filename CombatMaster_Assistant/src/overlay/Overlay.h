#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <string>
#include <cmath>
#include <vector>

#include "../visuals/WorldToScreen.h"
#include "../visuals/ESP.h"
#include "../ui/Menu.h"
#include "../ui/Theme.h"
#include "../targeting/aimassist.h"
#include "../targeting/assittrigger.h"
#include "../utils/Logger.h"
#include "../utils/Hotkeys.h"
#include "../Global.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ─────────────────────────────────────────────────────────────────────────────
// Overlay — standalone transparent DWM window over the game.
// Uses its own DX11 device + ImGui context (no hooking required).
// ─────────────────────────────────────────────────────────────────────────────
class Overlay {
private:
    HWND        overlayHwnd   = nullptr;
    HWND        gameHwnd      = nullptr;

    ID3D11Device*           pDevice  = nullptr;
    ID3D11DeviceContext*    pContext  = nullptr;
    IDXGISwapChain*         pSwapChain = nullptr;
    ID3D11RenderTargetView* pRTV     = nullptr;

    int screenWidth  = 1920;
    int screenHeight = 1080;
    bool running     = false;
    bool imguiInit   = false;

    Overlay() {}

    // ── Window Procedure ────────────────────────────────────────────────────
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // Forward input to ImGui when menu is open
        if (Menu::bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_SIZE:
            if (Get().pDevice && wParam != SIZE_MINIMIZED) {
                Get().ResizeBuffers(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    // ── Create DX11 device + swapchain for our overlay window ────────────────
    bool CreateDeviceD3D() {
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount                        = 2;
        sd.BufferDesc.Width                   = screenWidth;
        sd.BufferDesc.Height                  = screenHeight;
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 0;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = overlayHwnd;
        sd.SampleDesc.Count                   = 1;
        sd.Windowed                           = TRUE;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, levels, 1, D3D11_SDK_VERSION,
            &sd, &pSwapChain, &pDevice, &featureLevel, &pContext);
        if (FAILED(hr)) return false;

        CreateRenderTarget();
        return true;
    }

    void CreateRenderTarget() {
        ID3D11Texture2D* pBack = nullptr;
        pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBack));
        if (pBack) {
            pDevice->CreateRenderTargetView(pBack, nullptr, &pRTV);
            pBack->Release();
        }
    }

    void CleanupRenderTarget() {
        if (pRTV) { pRTV->Release(); pRTV = nullptr; }
    }

    void ResizeBuffers(UINT w, UINT h) {
        CleanupRenderTarget();
        pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        screenWidth = (int)w; screenHeight = (int)h;
        Visuals::ScreenCenter.x = screenWidth  / 2.0f;
        Visuals::ScreenCenter.y = screenHeight / 2.0f;
        CreateRenderTarget();
    }

    // ── Create the transparent overlay window ───────────────────────────────
    bool CreateOverlayWindow() {
        // Try to find the game window to mirror its position/size
        gameHwnd = FindWindowA("UnityWndClass", nullptr);
        if (!gameHwnd) gameHwnd = FindWindowA(nullptr, "Combat Master");

        if (gameHwnd) {
            RECT r;
            if (GetClientRect(gameHwnd, &r)) {
                POINT topLeft = { r.left, r.top };
                ClientToScreen(gameHwnd, &topLeft);
                screenWidth  = r.right  - r.left;
                screenHeight = r.bottom - r.top;
            }
        } else {
            // Fallback to primary monitor size
            screenWidth  = GetSystemMetrics(SM_CXSCREEN);
            screenHeight = GetSystemMetrics(SM_CYSCREEN);
        }

        Visuals::ScreenCenter.x = screenWidth  / 2.0f;
        Visuals::ScreenCenter.y = screenHeight / 2.0f;

        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = GetModuleHandleA(nullptr);
        wc.lpszClassName = "NexusOverlay";
        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExA(&wc);

        overlayHwnd = CreateWindowExA(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
            "NexusOverlay", "NexusOverlay",
            WS_POPUP,
            0, 0, screenWidth, screenHeight,
            nullptr, nullptr, wc.hInstance, nullptr);

        if (!overlayHwnd) return false;

        // Make the window fully transparent (black = transparent)
        SetLayeredWindowAttributes(overlayHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        // Extend DWM frame — this makes the window's background truly transparent
        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(overlayHwnd, &margins);

        ShowWindow(overlayHwnd, SW_SHOW);
        UpdateWindow(overlayHwnd);
        return true;
    }

    // ── Initialize ImGui ────────────────────────────────────────────────────
    void InitImGui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        Theme::Apply();

        ImGuiIO& io = ImGui::GetIO();

        // Try to load fonts from alongside the exe
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string dir = exePath;
        size_t lastSlash = dir.find_last_of("\\/");
        if (lastSlash != std::string::npos) dir = dir.substr(0, lastSlash);

        std::string oswaldPath = dir + "\\fonts\\Oswald-Regular.ttf";
        std::string iconsPath  = dir + "\\fonts\\MaterialIcons-Regular.ttf";

        bool fontLoaded = false;
        if (GetFileAttributesA(oswaldPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            io.Fonts->AddFontFromFileTTF(oswaldPath.c_str(), 18.0f);
            fontLoaded = true;
        }
        if (GetFileAttributesA(iconsPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            static const ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
            ImFontConfig cfg;
            cfg.MergeMode       = true;
            cfg.PixelSnapH      = true;
            cfg.GlyphMinAdvanceX = 18.0f;
            io.Fonts->AddFontFromFileTTF(iconsPath.c_str(), 20.0f, &cfg, icon_ranges);
        }
        if (!fontLoaded) io.Fonts->AddFontDefault();

        ImGui_ImplWin32_Init(overlayHwnd);
        ImGui_ImplDX11_Init(pDevice, pContext);
        imguiInit = true;
    }

    // ── Per-frame input passthrough toggle ──────────────────────────────────
    void UpdateClickThrough() {
        LONG exStyle = GetWindowLongA(overlayHwnd, GWL_EXSTYLE);
        if (Menu::bShowMenu) {
            // Remove WS_EX_TRANSPARENT so the window can receive mouse input
            if (exStyle & WS_EX_TRANSPARENT)
                SetWindowLongA(overlayHwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
        } else {
            // Re-add WS_EX_TRANSPARENT so clicks pass through to the game
            if (!(exStyle & WS_EX_TRANSPARENT))
                SetWindowLongA(overlayHwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
        }
    }

    // ── Try to keep overlay on top & aligned with game window ───────────────
    void SnapToGame() {
        if (!gameHwnd) return;
        RECT r;
        if (GetWindowRect(gameHwnd, &r)) {
            int w = r.right  - r.left;
            int h = r.bottom - r.top;
            SetWindowPos(overlayHwnd, HWND_TOPMOST, r.left, r.top, w, h, SWP_NOACTIVATE);
            if (w != screenWidth || h != screenHeight)
                ResizeBuffers(w, h);
        }
    }

    // ── Main draw call (ESP + Menu) ─────────────────────────────────────────
    void DrawFrame() {
        // Hotkey polling
        if (GetAsyncKeyState(Config::menu_toggle_key) & 1)
            Menu::bShowMenu = !Menu::bShowMenu;
        if (GetAsyncKeyState(Config::unload_key) & 1)
            g_ExitRequested = true;

        UpdateClickThrough();
        SnapToGame();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawCheat();
        Menu::Draw(nullptr);

        ImGui::Render();

        const float clearColor[4] = { 0.f, 0.f, 0.f, 0.f }; // Fully transparent background
        pContext->OMSetRenderTargets(1, &pRTV, nullptr);
        pContext->ClearRenderTargetView(pRTV, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        pSwapChain->Present(1, 0);
    }

    // ── Game logic (ESP, Aimbot, Triggerbot) ─────────────────────────────────
    void DrawCheat() {
        ImDrawList* bg = ImGui::GetBackgroundDrawList();

        // FOV circle
        if (Config::aimbot_draw_fov && Config::aimbot_enabled && Visuals::ScreenCenter.y >= 10.f) {
            ImU32 fovCol = IM_COL32(
                (int)(Config::esp_color_fov[0]*255), (int)(Config::esp_color_fov[1]*255),
                (int)(Config::esp_color_fov[2]*255), (int)(Config::esp_color_fov[3]*255));
            bg->AddCircle(ImVec2(Visuals::ScreenCenter.x, Visuals::ScreenCenter.y),
                          Config::aimbot_fov, fovCol, 100, 1.5f);
        }

        CPlayer localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer.Valid()) return;

        // Spectator passthrough
        CPlayer viewPlayer = localPlayer;
        if (localPlayer.IsDead()) {
            CPlayer spec = CPlayerRoot::GetSpectatorPlayer(localPlayer);
            if (spec.Valid()) {
                Camera cam = spec.GetCamera();
                if (cam.Valid()) viewPlayer = spec;
            }
        }

        Camera viewCamera = viewPlayer.GetCamera();
        if (!viewCamera.Valid()) return;

        Matrix4x4 viewMatrix = viewCamera.GetViewMatrix();
        Vector3 localPos     = viewPlayer.GetRootPosition();

        int maxPlayers = CPlayerRoot::GetAllPlayersCount();
        if (maxPlayers <= 0) return;

        std::vector<CPlayer> entities;
        entities.reserve(maxPlayers);
        for (int i = 0; i < maxPlayers; i++) {
            CPlayer player = CPlayerRoot::GetPlayerAt(i);
            if (!player.Valid() || player.GetAddress() == localPlayer.GetAddress()) continue;
            if (!Config::esp_show_team && player.IsTeammate(localPlayer)) continue;
            if (player.IsDead()) continue;
            entities.push_back(player);
        }

        // Aimbot / Triggerbot
        bool aimActive = (GetAsyncKeyState(Config::aimbot_key) & 0x8000) != 0;
        CPlayer* closestPlayer = nullptr;
        static CPlayer closestBuf(0);
        if (Config::aimbot_enabled && aimActive) {
            closestBuf = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
            if (closestBuf.Valid()) closestPlayer = &closestBuf;
        }
        if (Config::triggerbot_enabled) {
            CPlayer trigTarget = Aimbot::GetBestTarget(viewMatrix, screenWidth, screenHeight);
            Triggerbot::Run(trigTarget.Valid() && aimActive, Menu::bShowMenu);
        }

        // ESP
        if (Config::esp_enabled) {
            for (auto& entity : entities) {
                auto playerHealth = entity.GetPlayerHealth();
                if (!playerHealth.Valid()) continue;
                float healthPct = playerHealth.GetHealthPercent();

                Vector3 rootPos = entity.GetRootPosition();
                Vector3 headPos = rootPos; headPos.y += 1.6f;

                float dist = Vector3(localPos.x, localPos.y, localPos.z).Distance(rootPos);
                if (dist / 100.f > Config::esp_max_distance) continue;

                Vector2 headW2s, rootW2s;
                if (!Visuals::WorldToScreen(headPos, &headW2s, viewMatrix) ||
                    !Visuals::WorldToScreen(rootPos,  &rootW2s, viewMatrix)) continue;

                float height = rootW2s.y - headW2s.y;
                float width  = height / 2.0f;
                float boxX   = headW2s.x - (width / 2.0f);
                float boxY   = headW2s.y;

                bool isTeam = entity.IsTeammate(localPlayer);
                ImU32 colBox;
                if (Config::esp_show_team && isTeam)
                    colBox = IM_COL32((int)(Config::esp_color_team[0]*255),(int)(Config::esp_color_team[1]*255),(int)(Config::esp_color_team[2]*255),255);
                else
                    colBox = IM_COL32((int)(Config::esp_color_enemy[0]*255),(int)(Config::esp_color_enemy[1]*255),(int)(Config::esp_color_enemy[2]*255),255);

                if (Config::esp_boxes)  ESP::DrawCornerBox(boxX, boxY, width, height, colBox, Config::esp_box_thickness);
                if (Config::esp_health) ESP::DrawHealthBar(boxX - 6, boxY, 3, height, healthPct, 100.0f);

                if (Config::esp_names || Config::esp_distance) {
                    char line[128] = "";
                    if (Config::esp_names) {
                        auto cd = entity.GetConnectData();
                        if (cd.Valid()) {
                            std::string name = cd.GetNickName();
                            strncpy_s(line, 128, name.c_str(), 60);
                            line[60] = '\0';
                            if (!entity.isRealPlayer()) strcat_s(line, " [BOT]");
                        }
                    }
                    if (Config::esp_distance) {
                        char d[24]; snprintf(d, 24, " %.0fm", dist);
                        strcat_s(line, d);
                    }
                    if (line[0]) {
                        ImVec2 ts = ImGui::CalcTextSize(line);
                        DrawUtils::DrawGlowText(bg, ImGui::GetFont(), ImGui::GetFontSize(),
                            ImVec2(headW2s.x - ts.x * 0.5f, headW2s.y - ts.y - 4.f),
                            IM_COL32(255,255,255,255), line, 0.4f);
                    }
                }
            }
        }

        if (Config::aimbot_enabled && closestPlayer)
            Aimbot::RunAimbot(*closestPlayer, viewMatrix, ImGui::GetIO().DeltaTime);
    }

    // ── Cleanup ──────────────────────────────────────────────────────────────
    void Shutdown() {
        if (imguiInit) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            imguiInit = false;
        }
        CleanupRenderTarget();
        if (pSwapChain) { pSwapChain->Release(); pSwapChain = nullptr; }
        if (pContext)   { pContext->Release();   pContext   = nullptr; }
        if (pDevice)    { pDevice->Release();    pDevice    = nullptr; }
        if (overlayHwnd) {
            DestroyWindow(overlayHwnd);
            UnregisterClassA("NexusOverlay", GetModuleHandleA(nullptr));
            overlayHwnd = nullptr;
        }
    }

public:
    static Overlay& Get() { static Overlay inst; return inst; }

    void Init() {
        if (!CreateOverlayWindow()) {
            Logger::Error("[Overlay] Failed to create overlay window.");
            return;
        }
        if (!CreateDeviceD3D()) {
            Logger::Error("[Overlay] Failed to create D3D11 device.");
            Shutdown();
            return;
        }
        InitImGui();
        Logger::Log("[Overlay] Initialized. Running render loop.");

        running = true;
        MSG msg{};
        while (!g_ExitRequested) {
            // Pump Win32 messages
            while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                if (msg.message == WM_QUIT) g_ExitRequested = true;
            }
            if (g_ExitRequested) break;

            // Check if game is still alive
            if (gameHwnd && !IsWindow(gameHwnd)) {
                Logger::Log("[Overlay] Game window closed. Exiting.");
                break;
            }

            DrawFrame();
        }

        running = false;
        Shutdown();
        Logger::Log("[Overlay] Shutdown complete.");
    }
};
