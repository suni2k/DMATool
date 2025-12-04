#include "Application.h"
#include "UI/Tabs/DataPortTab.h"  // For cleanup of DMA resources
#include "VMProtectConfig.h"  // VMProtect SDK integration
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <tchar.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DMATool
{
    Application::Application(const std::string& title, int width, int height)
        : m_Title(title)
        , m_Width(width)
        , m_Height(height)
        , m_Hwnd(nullptr)
        , m_D3dDevice(nullptr)
        , m_D3dDeviceContext(nullptr)
        , m_SwapChain(nullptr)
        , m_MainRenderTargetView(nullptr)
        , m_Running(false)
    {
        ZeroMemory(&m_Wc, sizeof(m_Wc));
    }

    Application::~Application()
    {
    }

    bool Application::Initialize()
    {
        VMPROTECT_ULTRA_FUNCTION("AppInitialize");
        
        // Create completely borderless window (no title bar, no resize handles)
        m_Wc = { sizeof(m_Wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"DMATool", nullptr };
        ::RegisterClassExW(&m_Wc);
        
        // WS_POPUP = no title bar, no border, no system menu
        // Remove WS_THICKFRAME to prevent resizing
        m_Hwnd = ::CreateWindowW(
            m_Wc.lpszClassName,
            L"DMA Kings Tool",
            WS_POPUP | WS_VISIBLE, // Only popup, no thick frame
            100, 100, m_Width, m_Height,
            nullptr, nullptr, m_Wc.hInstance, nullptr
        );

        if (!m_Hwnd)
        {
            return false;
        }

        // Apply rounded corners (Windows 11 style)
        typedef enum _DWM_WINDOW_CORNER_PREFERENCE {
            DWMWCP_DEFAULT = 0,
            DWMWCP_DONOTROUND = 1,
            DWMWCP_ROUND = 2,
            DWMWCP_ROUNDSMALL = 3
        } DWM_WINDOW_CORNER_PREFERENCE;

        typedef HRESULT(WINAPI* DwmSetWindowAttributeProc)(HWND, DWORD, LPCVOID, DWORD);
        HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
        if (dwmapi)
        {
            DwmSetWindowAttributeProc DwmSetWindowAttribute = 
                (DwmSetWindowAttributeProc)GetProcAddress(dwmapi, "DwmSetWindowAttribute");
            if (DwmSetWindowAttribute)
            {
                DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
                DwmSetWindowAttribute(m_Hwnd, 33, &preference, sizeof(preference)); // 33 = DWMWA_WINDOW_CORNER_PREFERENCE
                
                // Set dark title bar (even though it's hidden, this helps with border color)
                BOOL useDarkMode = TRUE;
                DwmSetWindowAttribute(m_Hwnd, 20, &useDarkMode, sizeof(useDarkMode)); // 20 = DWMWA_USE_IMMERSIVE_DARK_MODE
            }
        }

        // Initialize Direct3D
        if (!CreateDeviceD3D(m_Hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClassW(m_Wc.lpszClassName, m_Wc.hInstance);
            return false;
        }

        // Show the window
        ::ShowWindow(m_Hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(m_Hwnd);

        // Setup ImGui
        SetupImGui();
        ApplyTheme();

        // Initialize managers
        m_ProjectManager = std::make_unique<Backend::ProjectManager>();
        m_MainWindow = std::make_unique<UI::MainWindow>(m_ProjectManager.get());

        m_Running = true;
        
        VMPROTECT_END_FUNCTION();
        return true;
    }

    void Application::SetupImGui()
    {
        // NOTE: UI setup is NOT protected - would cause lag
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // Note: Docking and Viewports require imgui_internal.h and special build
        // For now, keep it simple without these features
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(m_Hwnd);
        ImGui_ImplDX11_Init(m_D3dDevice, m_D3dDeviceContext);
        
        // Load fonts with better quality
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
    }

    void Application::ApplyTheme()
    {
        UI::Theme::ApplyShadcnTheme();
    }

    void Application::Run()
    {
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));

        while (m_Running && msg.message != WM_QUIT)
        {
            // Poll and handle messages
            if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            BeginFrame();
            m_MainWindow->Render();
            EndFrame();
        }
    }

    void Application::BeginFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Application::EndFrame()
    {
        ImGui::Render();
        
        const float clear_color[4] = { 0.09f, 0.09f, 0.11f, 1.00f };
        m_D3dDeviceContext->OMSetRenderTargets(1, &m_MainRenderTargetView, nullptr);
        m_D3dDeviceContext->ClearRenderTargetView(m_MainRenderTargetView, clear_color);
        
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        m_SwapChain->Present(1, 0); // Present with vsync
    }

    void Application::Shutdown()
    {
        // Clean up tab resources BEFORE destroying MainWindow
        // This ensures LeechCore device is released properly
        UI::Tabs::DataPortTab::Cleanup();
        
        m_MainWindow.reset();
        m_ProjectManager.reset();

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(m_Hwnd);
        ::UnregisterClassW(m_Wc.lpszClassName, m_Wc.hInstance);
    }

    bool Application::CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &m_SwapChain,
            &m_D3dDevice,
            &featureLevel,
            &m_D3dDeviceContext
        );

        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        {
            res = D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_WARP,
                nullptr,
                createDeviceFlags,
                featureLevelArray,
                2,
                D3D11_SDK_VERSION,
                &sd,
                &m_SwapChain,
                &m_D3dDevice,
                &featureLevel,
                &m_D3dDeviceContext
            );
        }

        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void Application::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        
        if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
        if (m_D3dDeviceContext) { m_D3dDeviceContext->Release(); m_D3dDeviceContext = nullptr; }
        if (m_D3dDevice) { m_D3dDevice->Release(); m_D3dDevice = nullptr; }
    }

    void Application::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        m_D3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_MainRenderTargetView);
        pBackBuffer->Release();
    }

    void Application::CleanupRenderTarget()
    {
        if (m_MainRenderTargetView) { m_MainRenderTargetView->Release(); m_MainRenderTargetView = nullptr; }
    }

    LRESULT WINAPI Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                // Handle window resize - need to recreate render target
                // This would need access to the Application instance
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_NCHITTEST:
            // Return HTCAPTION for nowhere to prevent resize cursors
            // Let ImGui handle all mouse events
            return HTCLIENT;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}
