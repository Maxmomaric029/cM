#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include "../Base/Globals.h"
#include "../Base/Sdk.h"
#include "../Base/Utils.h"
#include "../Base/WeaponHooks.h"
#include "../Base/MovementHooks.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace MenuDraw { void Frame(IDXGISwapChain*); }

typedef HRESULT(__stdcall* PresentFn)(IDXGISwapChain*, UINT, UINT);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

namespace Overlay {
    inline PresentFn oPresent = nullptr;
    inline HWND window = nullptr;
    inline WNDPROC oWndProc = nullptr;
    inline ID3D11Device* pDevice = nullptr;
    inline ID3D11DeviceContext* pContext = nullptr;
    inline ID3D11RenderTargetView* mainRTV = nullptr;
    inline bool inited = false;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
        return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
    }

    HRESULT __stdcall hkPresent(IDXGISwapChain* swapChain, UINT sync, UINT flags) {
        static bool unloadRequested = false;
        if (GetAsyncKeyState(Globals::Menu::unloadKey) & 1)
            unloadRequested = true;
        if (unloadRequested && Globals::g_hInjectModule && inited) {
            MovementHooks::Restore();
            if (window && oWndProc)
                SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID&)oPresent, hkPresent);
            DetourTransactionCommit();
            WeaponHooks::Unhook();
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            if (mainRTV) { mainRTV->Release(); mainRTV = nullptr; }
            pContext = nullptr; pDevice = nullptr; window = nullptr; oWndProc = nullptr;
            inited = false;
            CreateThread(nullptr, 0, [](LPVOID m) -> DWORD { FreeLibraryAndExitThread((HMODULE)m, 0); return 0; }, Globals::g_hInjectModule, 0, nullptr);
            return oPresent(swapChain, sync, flags);
        }

        if (!inited) {
            if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
                return oPresent(swapChain, sync, flags);
            }
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            swapChain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* backBuffer = nullptr;
            swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (backBuffer) {
                pDevice->CreateRenderTargetView(backBuffer, nullptr, &mainRTV);
                backBuffer->Release();
            }
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            ImGui::CreateContext();
            ImGui_ImplWin32_Init(window);
            ImGui_ImplDX11_Init(pDevice, pContext);
            inited = true;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DXGI_SWAP_CHAIN_DESC sd;
        if (SUCCEEDED(swapChain->GetDesc(&sd)) && sd.BufferDesc.Width > 0 && sd.BufferDesc.Height > 0) {
            Globals::Screen::ScreenSize.x = (float)sd.BufferDesc.Width;
            Globals::Screen::ScreenSize.y = (float)sd.BufferDesc.Height;
            Globals::Screen::ScreenCenter.x = Globals::Screen::ScreenSize.x / 2.f;
            Globals::Screen::ScreenCenter.y = Globals::Screen::ScreenSize.y / 2.f;
        }

        if (GetAsyncKeyState(Globals::Menu::menuToggleKey) & 1)
            Globals::Menu::bIsOpen = !Globals::Menu::bIsOpen;

        MenuDraw::Frame(swapChain);

        ImGui::Render();
        pContext->OMSetRenderTargets(1, &mainRTV, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        return oPresent(swapChain, sync, flags);
    }

    bool HookPresent() {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "CMBaseHook";
        if (!RegisterClassExA(&wc)) return false;
        HWND hwnd = CreateWindowExA(0, "CMBaseHook", "CMBaseHook", WS_OVERLAPPEDWINDOW, 100, 100, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hwnd) { UnregisterClassA("CMBaseHook", wc.hInstance); return false; }
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.Width = 2;
        sd.BufferDesc.Height = 2;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        IDXGISwapChain* sc = nullptr;
        ID3D11Device* dev = nullptr;
        ID3D11DeviceContext* ctx = nullptr;
        D3D_FEATURE_LEVEL fl;
        const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &sd, &sc, &dev, &fl, &ctx))) {
            DestroyWindow(hwnd); UnregisterClassA("CMBaseHook", wc.hInstance); return false;
        }
        void** vtable = *reinterpret_cast<void***>(sc);
        oPresent = (PresentFn)vtable[8];
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)oPresent, hkPresent);
        DetourTransactionCommit();
        sc->Release(); dev->Release(); ctx->Release();
        DestroyWindow(hwnd);
        UnregisterClassA("CMBaseHook", wc.hInstance);
        return true;
    }
}

#include "MenuDraw.h"
