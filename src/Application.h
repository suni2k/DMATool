#pragma once

#include <string>
#include <memory>
#include <d3d11.h>
#include <Windows.h>
#include "UI/Theme.h"
#include "UI/MainWindow.h"
#include "Backend/ProjectManager.h"

namespace DMATool
{
    class Application
    {
    public:
        Application(const std::string& title, int width, int height);
        ~Application();

        bool Initialize();
        void Run();
        void Shutdown();

        HWND GetHwnd() const { return m_Hwnd; }

    private:
        void SetupImGui();
        void ApplyTheme();
        void BeginFrame();
        void EndFrame();
        
        bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();

        static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        std::string m_Title;
        int m_Width;
        int m_Height;
        
        // Win32
        HWND m_Hwnd;
        WNDCLASSEXW m_Wc;
        
        // DirectX 11
        ID3D11Device* m_D3dDevice;
        ID3D11DeviceContext* m_D3dDeviceContext;
        IDXGISwapChain* m_SwapChain;
        ID3D11RenderTargetView* m_MainRenderTargetView;
        
        std::unique_ptr<UI::MainWindow> m_MainWindow;
        std::unique_ptr<Backend::ProjectManager> m_ProjectManager;
        
        bool m_Running;
    };
}
