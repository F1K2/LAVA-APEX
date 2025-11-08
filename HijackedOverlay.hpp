// overlay/HijackedOverlay.hpp
#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include "../OS-ImGui/imgui/imgui.h"
#include "../OS-ImGui/imgui/imgui_impl_win32.h"
#include "../OS-ImGui/imgui/imgui_impl_dx11.h"
#include <windowsx.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// Forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace HijackedOverlay {
    // Variables globales
    inline HWND g_OverlayWindow = nullptr;
    inline HWND g_TargetWindow = nullptr;
    inline ID3D11Device* g_pd3dDevice = nullptr;
    inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    inline IDXGISwapChain* g_pSwapChain = nullptr;
    inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    inline bool g_Initialized = false;
    inline bool g_IsInteractive = false; // Mode transparent par défaut
    inline bool g_MenuOpen = false; // État du menu

    // Déclarations
    void CleanupRenderTarget();
    void CreateRenderTarget();
    void Cleanup();
    void UpdateInteractiveMode();
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void CreateRenderTarget() {
        if (!g_pSwapChain || !g_pd3dDevice) return;

        CleanupRenderTarget(); // Nettoyer l'ancien

        ID3D11Texture2D* pBackBuffer = nullptr;
        HRESULT hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (SUCCEEDED(hr) && pBackBuffer) {
            hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }
    }

    void CleanupRenderTarget() {
        if (g_mainRenderTargetView) {
            g_mainRenderTargetView->Release();
            g_mainRenderTargetView = nullptr;
        }
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // ImGui gère d'abord les messages
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_SIZE:
            if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                HRESULT hr = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                if (SUCCEEDED(hr)) {
                    CreateRenderTarget();
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_NCHITTEST:
            if (g_IsInteractive) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &pt);

                RECT titleBar = { 0, 0, 560, 30 }; // hauteur approximative du header
                if (PtInRect(&titleBar, pt))
                    return HTCAPTION; // déplacer fenêtre uniquement sur le header
                return HTCLIENT; // sinon clic normal
            }
            return HTTRANSPARENT; // Mode transparent

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void Cleanup() {
        if (!g_Initialized) return;

        g_Initialized = false;

        // Shutdown ImGui
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        // Cleanup D3D
        CleanupRenderTarget();

        if (g_pSwapChain) {
            g_pSwapChain->Release();
            g_pSwapChain = nullptr;
        }

        if (g_pd3dDeviceContext) {
            g_pd3dDeviceContext->Release();
            g_pd3dDeviceContext = nullptr;
        }

        if (g_pd3dDevice) {
            g_pd3dDevice->Release();
            g_pd3dDevice = nullptr;
        }

        // Cleanup Window
        if (g_OverlayWindow) {
            DestroyWindow(g_OverlayWindow);
            g_OverlayWindow = nullptr;
        }

        UnregisterClassW(L"OverlayClass", GetModuleHandle(nullptr));
        g_TargetWindow = nullptr;
    }

    bool Initialize(const char* targetWindowName, const char* targetClassName = nullptr) {
        // Cleanup si déjà initialisé
        if (g_Initialized) {
            Cleanup();
        }

        // Trouver la fenêtre cible
        g_TargetWindow = FindWindowA(targetClassName, targetWindowName);
        if (!g_TargetWindow || !IsWindow(g_TargetWindow)) {
            return false;
        }

        // Obtenir les dimensions
        RECT rect;
        if (!GetWindowRect(g_TargetWindow, &rect)) {
            return false;
        }

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        if (width <= 0 || height <= 0) {
            width = 800;
            height = 600;
        }

        // Enregistrer la classe de fenêtre
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr; // Pas de background brush
        wc.lpszClassName = L"OverlayClass";

        if (!RegisterClassExW(&wc)) {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS) {
                return false;
            }
        }

        // Créer la fenêtre overlay
        g_OverlayWindow = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
            L"OverlayClass",
            L"OverlayWindow",
            WS_POPUP,
            rect.left, rect.top,
            width, height,
            nullptr, nullptr, wc.hInstance, nullptr
        );

        if (!g_OverlayWindow) {
            return false;
        }

        // Configuration de la transparence
        SetLayeredWindowAttributes(g_OverlayWindow, RGB(0, 0, 0), 255, LWA_ALPHA);

        // DWM pour le blur et la transparence
        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(g_OverlayWindow, &margins);

        // Activer le blur (optionnel)
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = nullptr;
        DwmEnableBlurBehindWindow(g_OverlayWindow, &bb);

        ShowWindow(g_OverlayWindow, SW_SHOW);
        UpdateWindow(g_OverlayWindow);

        // Configuration D3D11
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_OverlayWindow;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            4,
            D3D11_SDK_VERSION,
            &sd,
            &g_pSwapChain,
            &g_pd3dDevice,
            &featureLevel,
            &g_pd3dDeviceContext
        );

        if (FAILED(hr)) {
            Cleanup();
            return false;
        }

        CreateRenderTarget();

        if (!g_mainRenderTargetView) {
            Cleanup();
            return false;
        }

        // Initialisation ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        // Style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.Alpha = 0.95f;

        // Couleurs avec transparence
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.92f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);

        // Backends
        if (!ImGui_ImplWin32_Init(g_OverlayWindow)) {
            Cleanup();
            return false;
        }

        if (!ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext)) {
            Cleanup();
            return false;
        }

        g_Initialized = true;
        return true;
    }

    void UpdateOverlayPosition() {
        if (!g_TargetWindow || !g_OverlayWindow || !IsWindow(g_TargetWindow))
            return;

        RECT targetRect;
        if (!GetWindowRect(g_TargetWindow, &targetRect))
            return;

        RECT overlayRect;
        if (!GetWindowRect(g_OverlayWindow, &overlayRect))
            return;

        // Vérifier si la position/taille a changé
        if (overlayRect.left != targetRect.left ||
            overlayRect.top != targetRect.top ||
            (overlayRect.right - overlayRect.left) != (targetRect.right - targetRect.left) ||
            (overlayRect.bottom - overlayRect.top) != (targetRect.bottom - targetRect.top)) {

            SetWindowPos(g_OverlayWindow, HWND_TOPMOST,
                targetRect.left, targetRect.top,
                targetRect.right - targetRect.left,
                targetRect.bottom - targetRect.top,
                SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
        }
    }

    bool ProcessMessages() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                return false;
            }
        }
        return true;
    }

    void BeginFrame() {
        if (!g_Initialized) return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame() {
        if (!g_Initialized || !g_pd3dDeviceContext || !g_mainRenderTargetView)
            return;

        ImGui::Render();

        // Clear avec transparence totale
        const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);

        // Render ImGui
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        if (g_pSwapChain) {
            g_pSwapChain->Present(1, 0);
        }
    }

    bool IsInitialized() {
        return g_Initialized;
    }

    // Fonctions utilitaires supplémentaires
    void SetInteractive(bool interactive) {
        g_IsInteractive = interactive;
        if (g_OverlayWindow) {
            LONG_PTR exStyle = GetWindowLongPtr(g_OverlayWindow, GWL_EXSTYLE);
            if (interactive) {
                exStyle &= ~WS_EX_TRANSPARENT;
            }
            else {
                exStyle |= WS_EX_TRANSPARENT;
            }
            SetWindowLongPtr(g_OverlayWindow, GWL_EXSTYLE, exStyle);
        }
    }

    bool IsTargetWindowValid() {
        return g_TargetWindow && IsWindow(g_TargetWindow);
    }

    HWND GetOverlayWindow() {
        return g_OverlayWindow;
    }

    HWND GetTargetWindow() {
        return g_TargetWindow;
    }
}