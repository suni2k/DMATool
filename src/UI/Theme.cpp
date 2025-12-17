#include "Theme.h"
#include <algorithm>
#include <cmath>

namespace DMATool::UI
{
    void Theme::ApplyShadcnTheme()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // DMA Kings brand colors - Gold (#D4AF37) and Black (#0A0A0A)
        ImVec4 brandGold = ImVec4(0.83f, 0.69f, 0.22f, 1.0f);      // #D4AF37
        ImVec4 brandGoldDark = ImVec4(0.70f, 0.58f, 0.18f, 1.0f);  // Darker gold
        ImVec4 brandGoldLight = ImVec4(0.90f, 0.75f, 0.25f, 1.0f); // Lighter gold
        ImVec4 brandBlack = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);     // #0A0A0A
        
        // Rounded corners for modern look
        style.WindowRounding = 12.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        
        // Spacing and padding
        style.WindowPadding = ImVec2(16, 16);
        style.FramePadding = ImVec2(12, 8);
        style.ItemSpacing = ImVec2(12, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 22.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
        
        // Borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;
        
        // Colors - DMA Kings Theme
        ImVec4* colors = style.Colors;
        
        // Windows
        colors[ImGuiCol_WindowBg] = Colors::Background;
        colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        colors[ImGuiCol_PopupBg] = Colors::Popover;
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
        
        // Title
        colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);
        
        // Text
        colors[ImGuiCol_Text] = Colors::Foreground;
        colors[ImGuiCol_TextDisabled] = Colors::MutedForeground;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        
        // Buttons - Gold theme
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
        colors[ImGuiCol_ButtonActive] = brandGoldDark;
        
        // Headers - Gold accents
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
        colors[ImGuiCol_HeaderActive] = brandGold;
        
        // Tabs - Gold for active
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
        colors[ImGuiCol_TabActive] = brandGold;
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        
        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = brandGold;
        
        // Checkboxes and sliders - Gold when active
        colors[ImGuiCol_CheckMark] = brandGold;
        colors[ImGuiCol_SliderGrab] = brandGold;
        colors[ImGuiCol_SliderGrabActive] = brandGoldLight;
        
        // Frames
        colors[ImGuiCol_FrameBg] = Colors::Input;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
        
        // Separators
        colors[ImGuiCol_Separator] = Colors::Border;
        colors[ImGuiCol_SeparatorHovered] = brandGold;
        colors[ImGuiCol_SeparatorActive] = brandGoldLight;
        
        // Resize grip - Gold theme
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.20f, 0.22f, 0.5f);
        colors[ImGuiCol_ResizeGripHovered] = brandGold;
        colors[ImGuiCol_ResizeGripActive] = brandGoldLight;
        
        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        
        // Modals
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
    }

    float Theme::SmoothStep(float edge0, float edge1, float x)
    {
        x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    ImVec4 Theme::LerpColor(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImVec4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

    // Primary button with gold hover/active states
    // Used for main actions like "Detect FPGA & Read DNA"
    bool Theme::ButtonPrimary(const char* label, const ImVec2& size)
    {
        ImVec4 brandGold = ImVec4(0.83f, 0.69f, 0.22f, 1.0f);
        ImVec4 brandGoldLight = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);
        ImVec4 brandGoldDark = ImVec4(0.70f, 0.58f, 0.18f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, brandGoldLight);  // Gold on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, brandGoldDark);     // Dark gold when pressed
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::PrimaryForeground);
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }

    // Secondary button with white text and gold hover/active states
    // Used for utility actions like "Copy DNA to Clipboard"
    bool Theme::ButtonSecondary(const char* label, const ImVec2& size)
    {
        ImVec4 brandGoldLight = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);
        ImVec4 brandGoldDark = ImVec4(0.70f, 0.58f, 0.18f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Secondary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, brandGoldLight);  // Gold on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, brandGoldDark);     // Dark gold when pressed
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Foreground);        // White text for visibility
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }

    // Destructive button with red color scheme
    // Used for dangerous actions like "Uninstall Driver"
    bool Theme::ButtonDestructive(const char* label, const ImVec2& size)
    {
        ImVec4 destructiveLight = ImVec4(0.92f, 0.36f, 0.36f, 1.0f);
        ImVec4 destructiveDark = ImVec4(0.75f, 0.22f, 0.22f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Destructive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, destructiveLight);  // Lighter red on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, destructiveDark);     // Darker red when pressed
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::DestructiveForeground);
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }

    // Gold themed button matching active tab color with gradient overlay
    // Used for important actions like "Check Driver Status"
    bool Theme::ButtonGold(const char* label, const ImVec2& size)
    {
        ImVec4 brandGold = ImVec4(0.83f, 0.69f, 0.22f, 1.0f);      // Same as TabActive (#D4AF37)
        ImVec4 brandGoldDark = ImVec4(0.70f, 0.58f, 0.18f, 1.0f);
        ImVec4 brandGoldLight = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);
        ImVec4 textColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);         // Pure black text for maximum contrast
        
        // Base button colors
        ImGui::PushStyleColor(ImGuiCol_Button, brandGold);          // Base gold color
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, brandGoldLight); // Lighter gold on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, brandGoldDark);   // Darker gold when pressed
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        
        bool result = ImGui::Button(label, size);
        
        // Get actual button rect after rendering for gradient overlay
        ImVec2 buttonMin = ImGui::GetItemRectMin();
        ImVec2 buttonMax = ImGui::GetItemRectMax();
        
        // Draw subtle vertical gradient overlay for depth effect
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Create vertical gradient from lighter gold at top to darker gold at bottom
        ImU32 colorTop = IM_COL32(230, 191, 64, 60);     // Lighter gold, subtle transparency
        ImU32 colorBottom = IM_COL32(178, 148, 46, 90);  // Darker gold, more visible
        
        if (ImGui::IsItemActive())
        {
            // When pressed, use darker gradient to show button is pressed
            colorTop = IM_COL32(178, 148, 46, 80);
            colorBottom = IM_COL32(140, 116, 36, 110);
        }
        else if (ImGui::IsItemHovered())
        {
            // When hovered, use brighter gradient to show interactivity
            colorTop = IM_COL32(240, 200, 74, 70);
            colorBottom = IM_COL32(200, 166, 54, 100);
        }
        
        // Apply gradient as overlay (doesn't replace base color, just adds depth)
        drawList->AddRectFilledMultiColor(
            buttonMin,
            buttonMax,
            colorTop,    // Top-left
            colorTop,    // Top-right
            colorBottom, // Bottom-right
            colorBottom  // Bottom-left
        );
        
        ImGui::PopStyleColor(4);
        return result;
    }
}
