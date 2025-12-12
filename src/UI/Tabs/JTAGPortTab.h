#pragma once

#include "../../Backend/OpenOCDInterface.h"
#include <string>
#include <vector>

namespace DMATool::UI::Tabs
{
    class JTAGPortTab
    {
    public:
        static void Render();
        
    private:
        static void RenderDeviceInfoPanel();
        static void RenderDriverPanel();
        static void RenderStatusPanel();
        
        // State management
        static Backend::FPGAInfo s_FPGAInfo;
        static Backend::DriverInfo s_DriverInfo;
        static std::vector<std::string> s_LogMessages;
        static bool s_IsDetecting;
        static bool s_IsAutoDetecting;
        static bool s_IsCheckingDriver;
        static bool s_IsInstallingDriver;
        static bool s_IsUninstallingDriver;
        static bool s_IsCopyingDNA;
        static bool s_ResetFrameCounter;  // Flag to reset frame counter for manual detection
        static bool s_DriverCheckCompleted;  // Track if driver check has been done
        static std::string s_ConnectionStatus;
        static std::string s_DetectionStatus;
        static std::string s_LastOperation;
        static std::string s_CurrentProgress;
        
        // Helper functions
        static void AddLog(const std::string& message);
        static void ClearLog();
        static void UpdateProgress(const std::string& progress);
    };
}
