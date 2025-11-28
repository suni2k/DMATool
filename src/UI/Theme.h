#pragma once

#include <imgui.h>

namespace DMATool::UI
{
    // Shadcn-inspired color palette
    namespace Colors
    {
        // Background colors
        constexpr ImVec4 Background = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);      // hsl(240 10% 3.9%)
        constexpr ImVec4 Foreground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);      // hsl(0 0% 98%)
        constexpr ImVec4 Card = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);            // hsl(240 10% 3.9%)
        constexpr ImVec4 CardForeground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);  // hsl(0 0% 98%)
        constexpr ImVec4 Popover = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);         // hsl(240 10% 3.9%)
        
        // Primary colors
        constexpr ImVec4 Primary = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);         // hsl(0 0% 98%)
        constexpr ImVec4 PrimaryForeground = ImVec4(0.09f, 0.09f, 0.11f, 1.00f); // hsl(240 10% 3.9%)
        
        // Secondary colors
        constexpr ImVec4 Secondary = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);       // hsl(240 3.7% 15.9%)
        constexpr ImVec4 SecondaryForeground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // hsl(0 0% 98%)
        
        // Muted colors
        constexpr ImVec4 Muted = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);           // hsl(240 3.7% 15.9%)
        constexpr ImVec4 MutedForeground = ImVec4(0.64f, 0.64f, 0.66f, 1.00f); // hsl(240 5% 64.9%)
        
        // Accent colors
        constexpr ImVec4 Accent = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);          // hsl(240 3.7% 15.9%)
        constexpr ImVec4 AccentForeground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // hsl(0 0% 98%)
        
        // Destructive
        constexpr ImVec4 Destructive = ImVec4(0.85f, 0.29f, 0.29f, 1.00f);     // hsl(0 62.8% 30.6%)
        constexpr ImVec4 DestructiveForeground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // hsl(0 0% 98%)
        
        // Border
        constexpr ImVec4 Border = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);          // hsl(240 3.7% 15.9%)
        constexpr ImVec4 Input = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);           // hsl(240 3.7% 15.9%)
        constexpr ImVec4 Ring = ImVec4(0.85f, 0.85f, 0.91f, 1.00f);            // hsl(240 4.9% 83.9%)
        
        // Success/Info/Warning
        constexpr ImVec4 Success = ImVec4(0.34f, 0.73f, 0.56f, 1.00f);
        constexpr ImVec4 Info = ImVec4(0.39f, 0.58f, 0.93f, 1.00f);
        constexpr ImVec4 Warning = ImVec4(0.98f, 0.75f, 0.18f, 1.00f);
    }

    class Theme
    {
    public:
        static void ApplyShadcnTheme();
        static void SetupFonts();
        
        // Animation helpers
        static float SmoothStep(float edge0, float edge1, float x);
        static ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t);
        
        // Custom styled widgets
        static bool ButtonPrimary(const char* label, const ImVec2& size = ImVec2(0, 0));
        static bool ButtonSecondary(const char* label, const ImVec2& size = ImVec2(0, 0));
        static bool ButtonDestructive(const char* label, const ImVec2& size = ImVec2(0, 0));
    };
}
