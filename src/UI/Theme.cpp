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

    bool Theme::ButtonPrimary(const char* label, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, LerpColor(Colors::Primary, Colors::Foreground, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, LerpColor(Colors::Primary, Colors::Background, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::PrimaryForeground);
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }

    bool Theme::ButtonSecondary(const char* label, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Secondary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::Muted);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::SecondaryForeground);
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }

    bool Theme::ButtonDestructive(const char* label, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Destructive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, LerpColor(Colors::Destructive, Colors::Foreground, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, LerpColor(Colors::Destructive, Colors::Background, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::DestructiveForeground);
        
        bool result = ImGui::Button(label, size);
        
        ImGui::PopStyleColor(4);
        return result;
    }
}
