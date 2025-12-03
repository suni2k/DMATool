#pragma once

#include "../../Backend/FlashInterface.h"
#include <string>
#include <vector>

namespace DMATool::UI::Tabs
{
    class JTAGFlashTab
    {
    public:
        static void Render();
        
    private:
        static void RenderFlashInfoPanel();
        static void RenderFlashOperationsPanel();
        
        // Progress bar states
        enum class ProgressState
        {
            Ready,      // Idle (default gray/white)
            InProgress, // Yellow (operation running)
            Success,    // Green (operation succeeded)
            Failure     // Red (operation failed)
        };
        
        // State management
        static Backend::FlashDeviceInfo s_FlashInfo;
        static Backend::FPGAChipModel s_SelectedChipModel;
        static bool s_AutoDetectChip;
        static std::string s_FirmwarePath;
        static std::vector<std::string> s_LogMessages;
        static bool s_IsDetecting;
        static bool s_IsFlashing;
        static bool s_IsVerifying;
        static std::string s_CurrentOperation;
        static std::string s_CurrentProgress;
        static float s_ProgressPercent;
        static ProgressState s_ProgressState;  // NEW: Track success/failure state
        
        // Helper functions
        static void AddLog(const std::string& message);
        static void ClearLog();
        static void UpdateProgress(float percent, const std::string& message);
        static void SetProgressState(ProgressState state);  // NEW: Set state
    };
}
