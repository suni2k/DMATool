#pragma once
#include "../../Backend/BenchmarkInterface.h"
#include "../../Backend/FT601DriverInterface.h"
#include <vector>
#include <string>

namespace DMATool::UI::Tabs
{
    class DataPortTab
    {
    public:
        static void Render();
        static void Cleanup();  // Clean up benchmark resources (release DMA device)
        
    private:
        static void RenderTestControlsPanel(float height);
        static void RenderResultsPanel(float height);
        static void RenderConsoleLog(float height);
        static void RenderFT601DriverPanel(float height);
        
        // Test management
        static void StartQuickTest();
        static void StopTest();
        static void AddLog(const std::string& message);
        static void ClearLog();
        
        // Note: Static state is managed in DataPortTab.cpp via file-scope statics
    };
}
