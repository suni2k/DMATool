#include "MainWindow.h"
#include "Theme.h"
#include "Tabs/JTAGPortTab.h"
#include "Tabs/JTAGFlashTab.h"
#include "Tabs/DataPortTab.h"
#include <imgui.h>
#include <Windows.h>
#include <shellapi.h>

namespace DMATool::UI
{
    MainWindow::MainWindow(Backend::ProjectManager* projectManager)
        : m_ProjectManager(projectManager)
        , m_ShowStartupDialog(true)
        , m_StartupDialogAlpha(0.0f)
        , m_CurrentTab(0)
    {
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::Render()
    {
        if (m_ShowStartupDialog)
        {
            RenderStartupDialog();
        }
        else
        {
            RenderMainContent();
        }
    }

    void MainWindow::RenderStartupDialog()
    {
        // Start fully visible - no fade animation for now
        m_StartupDialogAlpha = 1.0f;
        
        // Full screen splash with dark background
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        // DMA Kings brand colors - Gold/Yellow: #D4AF37, Black: #0A0A0A
        ImVec4 brandGold = ImVec4(0.83f, 0.69f, 0.22f, 1.0f);      // #D4AF37
        ImVec4 brandDarkGold = ImVec4(0.70f, 0.58f, 0.18f, 1.0f);  // Darker gold for hover
        ImVec4 brandBlack = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);     // #0A0A0A
        ImVec4 bgColor = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);        // Background
        
        // Force window background color
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
        
        if (ImGui::Begin("##SplashScreen", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            float windowWidth = ImGui::GetWindowWidth();
            float windowHeight = ImGui::GetWindowHeight();
            
            // === DRAGGABLE AREA (entire window except close button) ===
            ImVec2 dragAreaMin = ImGui::GetCursorScreenPos();
            
            ImGui::SetCursorScreenPos(dragAreaMin);
            ImGui::InvisibleButton("##DragArea", ImVec2(windowWidth - 60, 60));
            
            // Track drag start position to maintain relative offset
            static ImVec2 dragStartMousePos(0, 0);
            static POINT dragStartWindowPos = {0, 0};
            static bool isDragging = false;
            
            if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                HWND hwnd = GetActiveWindow();
                if (hwnd)
                {
                    if (!isDragging)
                    {
                        isDragging = true;
                        POINT pt;
                        GetCursorPos(&pt);
                        dragStartMousePos = ImVec2((float)pt.x, (float)pt.y);
                        
                        RECT rect;
                        GetWindowRect(hwnd, &rect);
                        dragStartWindowPos.x = rect.left;
                        dragStartWindowPos.y = rect.top;
                    }
                    else
                    {
                        POINT pt;
                        GetCursorPos(&pt);
                        
                        int newX = dragStartWindowPos.x + (pt.x - (int)dragStartMousePos.x);
                        int newY = dragStartWindowPos.y + (pt.y - (int)dragStartMousePos.y);
                        
                        SetWindowPos(hwnd, nullptr, newX, newY, 0, 0, 
                            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                    }
                }
            }
            else
            {
                isDragging = false;
            }
            
            // === CLOSE BUTTON (Top Right) - No glow ===
            float closeButtonSize = 36.0f;
            float closeButtonPadding = 12.0f;
            ImVec2 closeButtonPos(windowWidth - closeButtonSize - closeButtonPadding, closeButtonPadding);
            
            ImGui::SetCursorScreenPos(closeButtonPos);
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.76f, 0.20f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            
            ImGui::SetWindowFontScale(1.2f);
            if (ImGui::Button("X", ImVec2(closeButtonSize, closeButtonSize)))
            {
                PostQuitMessage(0);
            }
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);
            
            // Reset cursor for content
            ImGui::SetCursorPosY(windowHeight * 0.3f);
            
            // === DMA KINGS LOGO TEXT ===
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::SetWindowFontScale(4.0f);
            
            float dmaWidth = ImGui::CalcTextSize("DMA").x;
            float kingsWidth = ImGui::CalcTextSize("KINGS").x;
            float totalWidth = dmaWidth + kingsWidth;
            
            ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);
            ImGui::Text("DMA");
            ImGui::PopStyleColor();
            
            ImGui::SameLine(0, 0);
            
            ImGui::PushStyleColor(ImGuiCol_Text, brandGold);
            ImGui::Text("KINGS");
            ImGui::PopStyleColor();
            
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Subtitle
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            float subtitleWidth = ImGui::CalcTextSize("Professional DMA Hardware Interface Tool").x;
            ImGui::SetCursorPosX((windowWidth - subtitleWidth) * 0.5f);
            ImGui::Text("Professional DMA Hardware Interface Tool");
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            
            // === ENTER TOOL BUTTON ===
            float buttonWidth = 400.0f;
            float buttonHeight = 55.0f;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            
            ImGui::PushStyleColor(ImGuiCol_Button, brandGold);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.82f, 0.35f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, brandDarkGold);
            ImGui::PushStyleColor(ImGuiCol_Text, brandBlack);
            
            ImGui::SetWindowFontScale(1.5f);
            if (ImGui::Button("ENTER TOOL", ImVec2(buttonWidth, buttonHeight)))
            {
                m_ShowStartupDialog = false;
                m_CurrentTab = 0; // Switch to DNA ID tab immediately
            }
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::PopStyleColor(4);
            
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            
            // === COMMUNITY LINKS ===
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            float linksWidth = ImGui::CalcTextSize("Join our community:").x;
            ImGui::SetCursorPosX((windowWidth - linksWidth) * 0.5f);
            ImGui::Text("Join our community:");
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            
            float linkButtonWidth = 180.0f;
            float linkButtonHeight = 40.0f;
            float totalLinksWidth = linkButtonWidth * 3 + 40.0f;
            float linksStartX = (windowWidth - totalLinksWidth) * 0.5f;
            
            ImGui::SetCursorPosX(linksStartX);
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, brandGold);
            
            if (ImGui::Button("Website", ImVec2(linkButtonWidth, linkButtonHeight)))
            {
                ShellExecuteA(nullptr, "open", "https://www.dmakings.com", nullptr, nullptr, SW_SHOWNORMAL);
            }
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(linksStartX + linkButtonWidth + 20.0f);
            
            if (ImGui::Button("Discord", ImVec2(linkButtonWidth, linkButtonHeight)))
            {
                ShellExecuteA(nullptr, "open", "https://discord.gg/MfH9UHxkdP", nullptr, nullptr, SW_SHOWNORMAL);
            }
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(linksStartX + (linkButtonWidth + 20.0f) * 2);
            
            if (ImGui::Button("Setup Guide", ImVec2(linkButtonWidth, linkButtonHeight)))
            {
                ShellExecuteA(nullptr, "open", "https://injectkings.gitbook.io/dma-kings", nullptr, nullptr, SW_SHOWNORMAL);
            }
            
            ImGui::PopStyleColor(4);
            
            // Version/Copyright at bottom
            ImGui::SetCursorPos(ImVec2(20, windowHeight - 40));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::Text("DMA Kings Tool v1.0 | (C) 2025 DMA Kings");
            ImGui::PopStyleColor();
        }
        ImGui::End();
        
        ImGui::PopStyleColor(); // WindowBg
        ImGui::PopStyleVar(2);
    }

    void MainWindow::RenderMainContent()
    {
        // Full screen main window - no padding or rounding
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        // DMA Kings brand colors
        ImVec4 brandGold = ImVec4(0.83f, 0.69f, 0.22f, 1.0f);
        ImVec4 bgDark = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgDark);
        
        ImGui::Begin("##MainWindow", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar);
        
        // === CUSTOM TITLE BAR (Draggable AREA) ===
        float titleBarHeight = 45.0f;
        ImVec2 titleBarMin = ImGui::GetCursorScreenPos();
        ImVec2 titleBarMax = ImVec2(titleBarMin.x + ImGui::GetWindowWidth(), titleBarMin.y + titleBarHeight);
        
        // Draw title bar background with subtle gradient
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(titleBarMin, titleBarMax, 
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.12f, 0.12f, 0.14f, 1.0f)));
        
        // Draw bottom border of title bar (gold accent)
        drawList->AddLine(
            ImVec2(titleBarMin.x, titleBarMax.y), 
            ImVec2(titleBarMax.x, titleBarMax.y), 
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.83f, 0.69f, 0.22f, 0.3f)), 
            2.0f
        );
        
        // Make title bar draggable (everywhere except close button area)
        ImGui::SetCursorScreenPos(titleBarMin);
        ImGui::InvisibleButton("##TitleBar", ImVec2(ImGui::GetWindowWidth() - 60, titleBarHeight));
        
        // Track drag start position to maintain relative offset
        static ImVec2 dragStartMousePos(0, 0);
        static POINT dragStartWindowPos = {0, 0};
        static bool isDraggingMain = false;
        
        if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            HWND hwnd = GetActiveWindow();
            if (hwnd)
            {
                if (!isDraggingMain)
                {
                    // First frame of drag - record starting positions
                    isDraggingMain = true;
                    POINT pt;
                    GetCursorPos(&pt);
                    dragStartMousePos = ImVec2((float)pt.x, (float)pt.y);
                    
                    RECT rect;
                    GetWindowRect(hwnd, &rect);
                    dragStartWindowPos.x = rect.left;
                    dragStartWindowPos.y = rect.top;
                }
                else
                {
                    // Dragging - calculate offset from start and apply
                    POINT pt;
                    GetCursorPos(&pt);
                    
                    int newX = dragStartWindowPos.x + (pt.x - (int)dragStartMousePos.x);
                    int newY = dragStartWindowPos.y + (pt.y - (int)dragStartMousePos.y);
                    
                    SetWindowPos(hwnd, nullptr, newX, newY, 0, 0, 
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
        }
        else
        {
            isDraggingMain = false;
        }
        
        // Title text (centered vertically)
        ImGui::SetCursorScreenPos(ImVec2(titleBarMin.x + 20, titleBarMin.y + 10));
        
        // "DMA" in white
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetWindowFontScale(1.4f);
        ImGui::Text("DMA");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        
        // "KINGS TOOL" in gold - on same line
        ImGui::SameLine(0, 5); // 5px spacing
        ImGui::PushStyleColor(ImGuiCol_Text, brandGold);
        ImGui::SetWindowFontScale(1.4f);
        ImGui::Text("KINGS TOOL");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        // Close button (top right) - Beautiful Design
        float closeButtonSize = 36.0f;
        float closeButtonPadding = 8.0f;
        ImVec2 closeButtonPos(titleBarMax.x - closeButtonSize - closeButtonPadding, titleBarMin.y + 4);
        
        ImGui::SetCursorScreenPos(closeButtonPos);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.76f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        
        ImGui::SetWindowFontScale(1.2f);
        if (ImGui::Button("X", ImVec2(closeButtonSize, closeButtonSize)))
        {
            PostQuitMessage(0);
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        
        // Move cursor below title bar for content
        ImGui::SetCursorPosY(titleBarHeight + 8);
        ImGui::SetCursorPosX(0);
        
        // Content area - full width, no scrollbar on main body
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 0));
        ImGui::BeginChild("##ContentArea", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        
        RenderTabs();
        
        ImGui::EndChild();
        
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }

    void MainWindow::RenderTabs()
    {
        // Force select DNA ID tab on first frame after entering tool
        static bool s_FirstRender = true;
        
        if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("DNA ID"))
            {
                m_CurrentTab = 0;
                Tabs::JTAGPortTab::Render();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Flash DMA"))
            {
                m_CurrentTab = 1;
                Tabs::JTAGFlashTab::Render();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Benchmark DMA"))
            {
                m_CurrentTab = 2;
                Tabs::DataPortTab::Render();
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        // Reset first render flag after first frame
        if (s_FirstRender)
        {
            s_FirstRender = false;
        }
    }
}
