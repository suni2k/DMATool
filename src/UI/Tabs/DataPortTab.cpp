#include "DataPortTab.h"
#include "../Theme.h"
#include <imgui.h>
#include <iostream>
#include <thread>

namespace DMATool::UI::Tabs
{
    // Static data
    static Backend::BenchmarkInterface s_Benchmark;
    static Backend::BenchmarkConfig s_Config;
    static Backend::FT601DriverInterface s_FT601Driver;
    static Backend::FT601DriverInfo s_FT601DriverInfo;
    static std::vector<std::string> s_LogMessages;
    static bool s_IsTestRunning = false;
    static int s_CurrentTestType = 0;  // Track selected test type for legend display
    
    // FT601 driver panel state
    static bool s_IsCheckingFT601Driver = false;
    static bool s_IsInstallingFT601Driver = false;
    static bool s_IsUninstallingFT601Driver = false;
    static std::string s_FT601DriverProgress = "";  // Track operation progress

    // Cleanup flag to track if we've cleaned up on exit
    static bool s_HasCleanedUp = false;

    void DataPortTab::AddLog(const std::string& message)
    {
        s_LogMessages.push_back(message);
        std::cout << message << std::endl;
        
        // Keep only last 100 messages
        if (s_LogMessages.size() > 100)
        {
            s_LogMessages.erase(s_LogMessages.begin());
        }
    }

    void DataPortTab::ClearLog()
    {
        s_LogMessages.clear();
    }

    void DataPortTab::StartQuickTest()
    {
        s_IsTestRunning = true;
        ClearLog();
        
        // Configure test based on s_CurrentTestType
        s_Config.testType = (Backend::BenchmarkTestType)s_CurrentTestType;
        s_Config.memoryAddress = "0x1000";
        
        // Set parameters based on test type
        // NOTE: Custom Test (3) and Stress Test (2) use values already set from sliders/dropdowns
        switch (s_CurrentTestType)
        {
        case 0: // Quick Speed Test
            s_Config.durationSeconds = 10;
            s_Config.testSizeMB = 16;
            break;
        case 1: // Throughput Test
            s_Config.durationSeconds = 60;
            s_Config.testSizeMB = 1024;  // 1 GB
            break;
        case 2: // Stress Test - use configured values from UI
            // Duration and read size already set from sliders
            break;
        case 3: // Custom Test - use configured values from UI
            // Duration and read size already set from sliders
            break;
        default:
            s_Config.durationSeconds = 10;
            s_Config.testSizeMB = 16;
            break;
        }
        
        // Start test
        s_Benchmark.StartTest(s_Config, [](const std::string& message) {
            AddLog(message);
        });
    }

    void DataPortTab::StopTest()
    {
        s_Benchmark.StopTest();
        s_IsTestRunning = false;
        AddLog("[INFO] Test stopped by user");
    }

    void DataPortTab::Render()
    {
        // Update running state
        s_IsTestRunning = s_Benchmark.IsTestRunning();
        
        // Add margin/padding around entire content (matches DNA ID tab)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("DataPortContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        
        // Match font size with other tabs
        ImGui::SetWindowFontScale(1.1f);
        
        ImGui::Spacing();
        
        // RESIZABLE VERTICAL LAYOUT: Top panels vs Bottom panels
        // Use BeginChild with resize enabled for vertical split
        static float topPanelHeightRatio = 0.52f;  // Default 52% for top panels
        
        float availableHeight = ImGui::GetContentRegionAvail().y;
        float topHeight = availableHeight * topPanelHeightRatio;
        
        // Top section with manual resize handle - NO PADDING to prevent gap before separator
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::BeginChild("TopSection", ImVec2(0, topHeight), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleVar();
        
        // Two column layout for top panels (RESIZABLE horizontal separator)
        ImGui::Columns(2, "BenchmarkColumns", true);
        
        // Left column: Test Controls - subtract ItemSpacing to eliminate gap
        float panelHeight = ImGui::GetContentRegionAvail().y;
        RenderTestControlsPanel(panelHeight);
        
        ImGui::NextColumn();
        
        // Right column: FTDI Driver
        RenderFT601DriverPanel(panelHeight);
        
        ImGui::Columns(1);
        
        ImGui::EndChild();
        
        // HORIZONTAL RESIZE HANDLE between top and bottom sections (matches vertical column separator)
        // Get cursor position BEFORE drawing anything
        ImVec2 cursorBeforeSeparator = ImGui::GetCursorScreenPos();
        
        // Draw separator (uses theme colors: Border -> SeparatorHovered -> SeparatorActive)
        ImGui::Separator();
        
        // Get the actual separator position and size
        ImVec2 separatorPos = cursorBeforeSeparator;
        float separatorWidth = ImGui::GetContentRegionAvail().x + ImGui::GetStyle().WindowPadding.x * 2;
        
        // Create invisible button CENTERED on the separator for interaction
        ImGui::SetCursorScreenPos(ImVec2(separatorPos.x, separatorPos.y - 2));  // Center 4px button on 1px line
        ImGui::InvisibleButton("##vsplitter", ImVec2(separatorWidth, 4));  // 4px tall interaction area
        
        // Handle hover and drag
        bool isHovered = ImGui::IsItemHovered();
        bool isActive = ImGui::IsItemActive();
        
        if (isActive)
        {
            float delta = ImGui::GetIO().MouseDelta.y;
            topPanelHeightRatio += delta / availableHeight;
            // Clamp between 30% and 70%
            if (topPanelHeightRatio < 0.3f) topPanelHeightRatio = 0.3f;
            if (topPanelHeightRatio > 0.7f) topPanelHeightRatio = 0.7f;
        }
        
        // Draw colored line overlay when hovered or active (matches ImGuiCol_SeparatorHovered/Active)
        if (isActive)
        {
            // Active: Brand gold light (0.90, 0.75, 0.25) - matches ImGuiCol_SeparatorActive
            ImGui::GetWindowDrawList()->AddLine(
                separatorPos,
                ImVec2(separatorPos.x + separatorWidth, separatorPos.y),
                IM_COL32(230, 191, 64, 255),  // brandGoldLight
                1.5f  // Slightly thicker when active
            );
        }
        else if (isHovered)
        {
            // Hovered: Brand gold (0.83, 0.69, 0.22) - matches ImGuiCol_SeparatorHovered
            ImGui::GetWindowDrawList()->AddLine(
                separatorPos,
                ImVec2(separatorPos.x + separatorWidth, separatorPos.y),
                IM_COL32(212, 176, 56, 255),  // brandGold
                1.5f
            );
        }
        
        // Change cursor when hovering
        if (isHovered || isActive)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }
        
        // Bottom section: Console log (left) + Test Results (right) in 2-column layout (RESIZABLE horizontal separator)
        ImGui::Columns(2, "BottomPanels", true);  // Horizontal resizing enabled
        
        // Left: Console log
        float bottomHeight = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y;
        RenderConsoleLog(bottomHeight);
        
        ImGui::NextColumn();
        
        // Right: Test Results
        RenderResultsPanel(bottomHeight);
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        
        ImGui::EndChild();
        
        // Floating progress notification (toast-style) for driver operations
        if (s_IsCheckingFT601Driver || s_IsInstallingFT601Driver || s_IsUninstallingFT601Driver)
        {
            // Get the MAIN VIEWPORT (entire window) position and size
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 viewportPos = viewport->Pos;
            ImVec2 viewportSize = viewport->Size;
            
            // Center notification with better sizing
            float toastWidth = 420.0f;
            float toastHeight = 140.0f;
            
            ImVec2 toastPos(
                viewportPos.x + (viewportSize.x - toastWidth) * 0.5f,
                viewportPos.y + (viewportSize.y - toastHeight) * 0.5f
            );
            
            // Draw overlay that covers EVERYTHING EXCEPT the notification popup
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            
            // Draw overlay in 4 rectangles around the notification to exclude it
            ImU32 overlayColor = IM_COL32(0, 0, 0, 160);
            
            // Top rectangle (above notification)
            drawList->AddRectFilled(
                viewportPos,
                ImVec2(viewportPos.x + viewportSize.x, toastPos.y),
                overlayColor
            );
            
            // Bottom rectangle (below notification)
            drawList->AddRectFilled(
                ImVec2(viewportPos.x, toastPos.y + toastHeight),
                ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y),
                overlayColor
            );
            
            // Left rectangle (left of notification)
            drawList->AddRectFilled(
                ImVec2(viewportPos.x, toastPos.y),
                ImVec2(toastPos.x, toastPos.y + toastHeight),
                overlayColor
            );
            
            // Right rectangle (right of notification)
            drawList->AddRectFilled(
                ImVec2(toastPos.x + toastWidth, toastPos.y),
                ImVec2(viewportPos.x + viewportSize.x, toastPos.y + toastHeight),
                overlayColor
            );
            
            ImGui::SetNextWindowPos(toastPos);
            ImGui::SetNextWindowSize(ImVec2(toastWidth, toastHeight));
            
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 20));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.12f, 0.14f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.83f, 0.69f, 0.22f, 0.9f));  // Gold border - brand theme
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
            
            ImGui::Begin("##FT601Notification", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoCollapse);
            
            // Title text
            ImGui::SetCursorPos(ImVec2(24, 35));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.97f, 1.0f));  // Near white
            ImGui::SetWindowFontScale(1.4f);
            
            std::string statusText = "Processing Driver";
            if (s_IsCheckingFT601Driver)
                statusText = "Checking Driver";
            else if (s_IsInstallingFT601Driver)
                statusText = "Installing Driver";
            else if (s_IsUninstallingFT601Driver)
                statusText = "Uninstalling Driver";
            
            // Center the title text
            float titleWidth = ImGui::CalcTextSize(statusText.c_str()).x;
            ImGui::SetCursorPosX((toastWidth - titleWidth) * 0.5f);
            ImGui::Text(statusText.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            // Separator line
            ImGui::SetCursorPosX(24);
            ImGui::SetCursorPosY(70);
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.4f, 0.4f, 0.5f, 0.5f));
            ImGui::Separator();
            ImGui::PopStyleColor();
            
            // Animated dots when processing
            ImGui::SetCursorPosY(80);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.7f, 1.0f));
            ImGui::SetWindowFontScale(1.05f);
            
            // Show progress if available, otherwise animated dots
            if (!s_FT601DriverProgress.empty())
            {
                // Center and wrap text properly
                ImGui::PushTextWrapPos(toastWidth - 48);
                float textWidth = ImGui::CalcTextSize(s_FT601DriverProgress.c_str(), nullptr, false, toastWidth - 48).x;
                ImGui::SetCursorPosX((toastWidth - textWidth) * 0.5f);
                ImGui::TextWrapped(s_FT601DriverProgress.c_str());
                ImGui::PopTextWrapPos();
            }
            else
            {
                static float dotTimer = 0.0f;
                dotTimer += ImGui::GetIO().DeltaTime;
                int dotCount = ((int)(dotTimer * 2.0f) % 4);
                
                std::string dots = "Please wait";
                for (int i = 0; i < dotCount; i++) dots += ".";

                float dotsWidth = ImGui::CalcTextSize(dots.c_str()).x;
                ImGui::SetCursorPosX((toastWidth - dotsWidth) * 0.5f);
                ImGui::Text(dots.c_str());
            }
            
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            ImGui::End();
            
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);
        }
    }

    void DataPortTab::RenderTestControlsPanel(float height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("TestControlsPanel", ImVec2(0, height), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("DMA Benchmark Tests");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Test Type Selection
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Test Type");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        const char* testTypes[] = { 
            "Quick Speed Test",
            "Throughput Test",
            "Stress Test",
            "Custom Test"
        };
        static int currentTestType = 0;  // Default to Quick Speed Test
        
        // Push style for dropdown button
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 10));  // Button padding
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));    // Add vertical spacing between items
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 6));  // Add padding inside dropdown popup
        ImGui::SetNextItemWidth(-1);
        
        if (ImGui::BeginCombo("##testtype", testTypes[currentTestType]))
        {
            for (int i = 0; i < IM_ARRAYSIZE(testTypes); i++)
            {
                const bool isSelected = (currentTestType == i);
                if (ImGui::Selectable(testTypes[i], isSelected, 0, ImVec2(0, 0)))
                {
                    currentTestType = i;
                    s_Config.testType = (Backend::BenchmarkTestType)currentTestType;
                    
                    // Clear results when test type changes
                    ClearLog();
                }
                
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            
            ImGui::EndCombo();
        }
        
        ImGui::PopStyleVar(3);  // Pop all 3 style vars (FramePadding, ItemSpacing, WindowPadding)
        
        // Store current test type globally so results panel can access it
        s_CurrentTestType = currentTestType;
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Test description based on selected type
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        switch (currentTestType)
        {
        case 0: // Quick Speed
            ImGui::TextWrapped("10-second speed test measuring Reads Per Second (RPS) and latency.");
            break;
        case 1: // Throughput
            ImGui::TextWrapped("1 GB transfer test measuring throughput in MB/s.");
            break;
        case 2: // Stress
            ImGui::TextWrapped("Extended duration test tracking min/max latency for stability validation.");
            break;
        case 3: // Custom
            ImGui::TextWrapped("User-configurable test with custom duration (1-300s) and read size (1KB-256KB).");
            break;
        }
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Custom Test Configuration (only show when Custom Test is selected)
        if (currentTestType == 3)  // Custom Test (was 4, now 3 after removing Mixed)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
            ImGui::Text("Custom Configuration");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            // Duration input
            static int customDuration = 10;
            ImGui::Text("Duration (seconds):");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::SliderInt("##duration", &customDuration, 1, 300, "%d s"))
            {
                // Update config immediately when slider changes
                s_Config.durationSeconds = customDuration;
            }
            
            ImGui::Spacing();
            
            // Read size input - RE-ADDED with actual implementation
            static int customReadSize = 4096;
            ImGui::Text("Read Size (bytes):");
            ImGui::SetNextItemWidth(-1);
            const char* readSizes[] = { "1 KB (1024)", "4 KB (4096)", "64 KB (65536)", "256 KB (262144)" };
            static int readSizeIndex = 1;  // Default to 4KB
            if (ImGui::Combo("##readsize", &readSizeIndex, readSizes, IM_ARRAYSIZE(readSizes)))
            {
                switch (readSizeIndex)
                {
                case 0: customReadSize = 1024; break;
                case 1: customReadSize = 4096; break;
                case 2: customReadSize = 65536; break;
                case 3: customReadSize = 262144; break;  // 256KB instead of 1MB
                }
                // Update config immediately when selection changes
                s_Config.customReadSizeBytes = customReadSize;
            }
            
            // Ensure config is always up to date (even on first render)
            s_Config.durationSeconds = customDuration;
            s_Config.customReadSizeBytes = customReadSize;
        }
        
        // Stress Test Configuration (only show when Stress Test is selected)
        if (currentTestType == 2)  // Stress Test
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
            ImGui::Text("Stress Test Configuration");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            // Duration input - default to 60 seconds
            static int stressDuration = 60;
            ImGui::Text("Duration (seconds):");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::SliderInt("##stressduration", &stressDuration, 10, 600, "%d s"))
            {
                // Update config immediately when slider changes
                s_Config.durationSeconds = stressDuration;
            }
            
            ImGui::Spacing();
            
            // Read size input - default to 4KB
            static int stressReadSize = 4096;
            ImGui::Text("Read Size (bytes):");
            ImGui::SetNextItemWidth(-1);
            const char* stressReadSizes[] = { "1 KB (1024)", "4 KB (4096)", "64 KB (65536)", "256 KB (262144)" };
            static int stressReadSizeIndex = 1;  // Default to 4KB
            if (ImGui::Combo("##stressreadsize", &stressReadSizeIndex, stressReadSizes, IM_ARRAYSIZE(stressReadSizes)))
            {
                switch (stressReadSizeIndex)
                {
                case 0: stressReadSize = 1024; break;
                case 1: stressReadSize = 4096; break;
                case 2: stressReadSize = 65536; break;
                case 3: stressReadSize = 262144; break;
                }
                // Update config immediately when selection changes
                s_Config.customReadSizeBytes = stressReadSize;
            }
            
            // Ensure config is always up to date (even on first render)
            s_Config.durationSeconds = stressDuration;
            s_Config.customReadSizeBytes = stressReadSize;
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Test buttons (fill remaining space)
        ImGui::BeginDisabled(!Backend::BenchmarkInterface::IsPCILeechAvailable() || s_IsTestRunning);
        
        std::string buttonText = "Run Test";
        if (currentTestType == 0) buttonText = "Run Quick Speed Test";
        else if (currentTestType == 1) buttonText = "Run Throughput Test";
        else if (currentTestType == 2) buttonText = "Run Stress Test";
        else if (currentTestType == 3) buttonText = "Run Custom Test";
        
        if (Theme::ButtonPrimary(buttonText.c_str(), ImVec2(-1, 45)))
        {
            // Set config based on selected test type
            s_Config.testType = (Backend::BenchmarkTestType)currentTestType;
            s_Config.memoryAddress = "0x1000";
            
            // Only override config for non-custom and non-stress tests
            // Custom Test (3) and Stress Test (2) use values already set from sliders
            if (currentTestType != 3 && currentTestType != 2)
            {
                switch (currentTestType)
                {
                case 0: // Quick Speed Test
                    s_Config.durationSeconds = 10;
                    s_Config.testSizeMB = 16;
                    break;
                case 1: // Throughput Test
                    s_Config.testSizeMB = 1024;  // 1 GB
                    s_Config.durationSeconds = 60;  // May take up to 60 seconds
                    break;
                }
            }
            
            StartQuickTest();  // This function starts any test type
        }
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        // Stop button
        ImGui::BeginDisabled(!s_IsTestRunning);
        if (Theme::ButtonDestructive("Stop Test", ImVec2(-1, 45)))
        {
            StopTest();
        }
        ImGui::EndDisabled();
        
        ImGui::EndChild();
    }

    void DataPortTab::RenderResultsPanel(float height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("ResultsPanel", ImVec2(0, height), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Test Results");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        auto results = s_Benchmark.GetCurrentResults();
        
        // Status + Rating (SIDE BY SIDE)
        ImGui::Columns(2, "StatusRatingColumns", false);
        
        // Left: Status
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Status");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (s_IsTestRunning)
        {
            ImGui::TextColored(Colors::Warning, "RUNNING...");
        }
        else if (results.success)
        {
            ImGui::TextColored(Colors::Success, "COMPLETE");
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Not Started");
            ImGui::PopStyleColor();
        }
        
        ImGui::NextColumn();
        
        // Right: Rating
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Rating");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (!results.rating.empty())
        {
            // Color based on rating - NEW SCALE (SWAPPED Amazing/Great)
            ImVec4 ratingColor;
            if (results.rating == "ELITE")
                ratingColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);  // Gold (theme brandGoldLight)
            else if (results.rating == "AMAZING")
                ratingColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green (swapped)
            else if (results.rating == "GREAT")
                ratingColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF Dodger Blue (swapped)
            else if (results.rating == "OKAY")
                ratingColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);  // #5EA6B8 Teal
            else
                ratingColor = Colors::Destructive;  // Red for LOW
            
            // Make rating text larger
            ImGui::SetWindowFontScale(1.5f);
            ImGui::TextColored(ratingColor, results.rating.c_str());
            ImGui::SetWindowFontScale(1.0f);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Pending");
            ImGui::PopStyleColor();
        }
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Performance + Scale (SIDE BY SIDE) - Reduced spacing for compactness
        ImGui::Columns(2, "PerfScaleColumns", false);
        
        // Left: Performance metrics
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Performance");
        ImGui::PopStyleColor();
        
        // Reduce spacing between label and metrics (was ImGui::Spacing())
        ImGui::Dummy(ImVec2(0, 4));
        
        // Compact font for metrics
        ImGui::SetWindowFontScale(0.95f);
        
        // Reads Per Second (PRIMARY METRIC)
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("RPS:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);  // Reduced from 120 to 100
        if (results.readsPerSecond > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%.0f", results.readsPerSecond);
            
            // Color based on rating - use dynamic thresholds based on test type
            ImVec4 rpsColor = Colors::MutedForeground;
            
            if (s_CurrentTestType == 1)  // Throughput Test - use MB/s thresholds
            {
                // For throughput test, color based on throughput rating, not RPS - NEW COLORS
                if (results.rating == "ELITE")
                    rpsColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);  // Gold
                else if (results.rating == "AMAZING")
                    rpsColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green (swapped with Great)
                else if (results.rating == "GREAT")
                    rpsColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF Dodger Blue (swapped with Amazing)
                else if (results.rating == "OKAY")
                    rpsColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);  // #5EA6B8 Teal
                else
                    rpsColor = Colors::Destructive;  // Red
            }
            else if (s_CurrentTestType == 3)  // Custom Test - use read-size-specific thresholds
            {
                uint32_t readSize = s_Config.customReadSizeBytes;
                
                if (readSize <= 4096)  // 1KB-4KB - NEW THRESHOLDS
                {
                    if (results.readsPerSecond >= 7000) rpsColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);  // Gold
                    else if (results.readsPerSecond >= 6000) rpsColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green - Amazing (swapped)
                    else if (results.readsPerSecond >= 5000) rpsColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF - Great (swapped)
                    else if (results.readsPerSecond >= 4000) rpsColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);  // #5EA6B8
                    else rpsColor = Colors::Destructive;
                }
                else if (readSize == 65536)  // 64KB - Proportionally adjusted
                {
                    if (results.readsPerSecond >= 2200) rpsColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);
                    else if (results.readsPerSecond >= 1900) rpsColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green - Amazing (swapped)
                    else if (results.readsPerSecond >= 1600) rpsColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF - Great (swapped)
                    else if (results.readsPerSecond >= 1250) rpsColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);
                    else rpsColor = Colors::Destructive;
                }
                else  // 256KB - Proportionally adjusted
                {
                    if (results.readsPerSecond >= 550) rpsColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);
                    else if (results.readsPerSecond >= 475) rpsColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green - Amazing (swapped)
                    else if (results.readsPerSecond >= 400) rpsColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF - Great (swapped)
                    else if (results.readsPerSecond >= 320) rpsColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);
                    else rpsColor = Colors::Destructive;
                }
            }
            else  // Quick Speed Test / Stress - standard thresholds
            {
                if (results.readsPerSecond >= 7000) rpsColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);  // Gold - Elite
                else if (results.readsPerSecond >= 6000) rpsColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green - Amazing (swapped)
                else if (results.readsPerSecond >= 5000) rpsColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF - Great (swapped)
                else if (results.readsPerSecond >= 4000) rpsColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);  // #5EA6B8 - Okay
                else rpsColor = Colors::Destructive;  // Red - Low
            }
            
            ImGui::TextColored(rpsColor, buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        // Total Reads
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Total Reads:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (results.totalReads > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%llu", results.totalReads);
            ImGui::Text(buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        // Average Latency
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("AVG. Latency:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (results.avgLatencyUs > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%.0f us", results.avgLatencyUs);
            ImGui::Text(buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        // Throughput (ONLY show for Throughput Test)
        if (s_CurrentTestType == 1)  // Only for Throughput Test
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Throughput:");
            ImGui::PopStyleColor();
            ImGui::SameLine(100);
            if (results.throughputMBps > 0)
            {
                char buffer[64];
                sprintf_s(buffer, "%.2f MB/s", results.throughputMBps);
                ImGui::Text(buffer);
            }
            else
            {
                ImGui::Text("---");
            }
        }
        
        // Duration
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Duration:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (results.durationSeconds > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%.1f s", results.durationSeconds);
            ImGui::Text(buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        // Min/Max Latency - ALWAYS show (not conditional)
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Min Latency:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (results.minLatencyUs > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%.0f us", results.minLatencyUs);
            ImGui::Text(buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Max Latency:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (results.maxLatencyUs > 0)
        {
            char buffer[64];
            sprintf_s(buffer, "%.0f us", results.maxLatencyUs);
            ImGui::Text(buffer);
        }
        else
        {
            ImGui::Text("---");
        }
        
        // Reset font scale
        ImGui::SetWindowFontScale(1.0f);

        ImGui::NextColumn();
        
        // Right: Rating Scale (Compact)
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Scale");
        ImGui::PopStyleColor();
        
        // Reduce spacing
        ImGui::Dummy(ImVec2(0, 4));
        
        // Show appropriate rating scale based on SELECTED test type (not just results)
        ImGui::SetWindowFontScale(0.80f);  // Reduced from 0.85f to 0.80f for more compactness
        
        if (s_CurrentTestType == 1)  // Throughput Test
        {
            // Throughput test - show MB/s scale - SWAPPED Amazing/Great colors
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "220+ ELITE");
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "200+ AMAZING");
            ImGui::TextColored(ImVec4(0.118f, 0.565f, 1.0f, 1.0f), "150+ GREAT");
            ImGui::TextColored(ImVec4(0.369f, 0.651f, 0.722f, 1.0f), "125+ OKAY");
            ImGui::TextColored(Colors::Destructive, "<125 LOW");
        }
        else if (s_CurrentTestType == 3)  // Custom Test - show dynamic scale based on read size
        {
            // Custom test scale adjusts based on read size - NEW THRESHOLDS
            uint32_t readSize = s_Config.customReadSizeBytes;
            
            if (readSize <= 4096)  // 1KB-4KB: Use standard RPS scale
            {
                ImGui::TextColored(ImVec4(0.90f, 0.75f, 0.25f, 1.0f), "7000+ ELITE");
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "6000+ AMAZING");
                ImGui::TextColored(ImVec4(0.118f, 0.565f, 1.0f, 1.0f), "5000+ GREAT");
                ImGui::TextColored(ImVec4(0.369f, 0.651f, 0.722f, 1.0f), "4000+ OKAY");
                ImGui::TextColored(Colors::Destructive, "3000+ LOW");
            }
            else if (readSize == 65536)  // 64KB: Adjusted scale
            {
                ImGui::TextColored(ImVec4(0.90f, 0.75f, 0.25f, 1.0f), "2200+ ELITE");
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "1900+ AMAZING");
                ImGui::TextColored(ImVec4(0.118f, 0.565f, 1.0f, 1.0f), "1600+ GREAT");
                ImGui::TextColored(ImVec4(0.369f, 0.651f, 0.722f, 1.0f), "1250+ OKAY");
                ImGui::TextColored(Colors::Destructive, "950+ LOW");
            }
            else  // 256KB: Adjusted scale for chunked reads
            {
                ImGui::TextColored(ImVec4(0.90f, 0.75f, 0.25f, 1.0f), "550+ ELITE");
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "475+ AMAZING");
                ImGui::TextColored(ImVec4(0.118f, 0.565f, 1.0f, 1.0f), "400+ GREAT");
                ImGui::TextColored(ImVec4(0.369f, 0.651f, 0.722f, 1.0f), "320+ OKAY");
                ImGui::TextColored(Colors::Destructive, "240+ LOW");
            }
        }
        else  // Speed test (Quick, Stress)
        {
            // Speed test - show RPS scale - NEW THRESHOLDS
            ImGui::TextColored(ImVec4(0.90f, 0.75f, 0.25f, 1.0f), "7000+ ELITE");
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "6000+ AMAZING");
            ImGui::TextColored(ImVec4(0.118f, 0.565f, 1.0f, 1.0f), "5000+ GREAT");
            ImGui::TextColored(ImVec4(0.369f, 0.651f, 0.722f, 1.0f), "4000+ OKAY");
            ImGui::TextColored(Colors::Destructive, "3000+ LOW");
        }
        
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::Columns(1);
        
        ImGui::EndChild();
    }

    void DataPortTab::RenderConsoleLog(float height)
    {
        // Add padding to match DNA ID and Flash DMA tabs
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("ConsoleLog", ImVec2(0, height), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Console Log");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // REMOVE SPACING between log lines - they're already color-coded
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));  // Zero vertical spacing
        
        // Log output
        ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        if (s_LogMessages.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Waiting for test to start...");
            ImGui::Text("[INFO] Benchmark tab initialized");
            ImGui::Text("[INFO] Ready for testing");
            ImGui::PopStyleColor();
        }
        else
        {
            for (const auto& msg : s_LogMessages)
            {
                // SKIP EMPTY LINES - they cause unwanted spacing
                if (msg.empty())
                    continue;
                
                // Special handling for "Test completed:" messages to color-code rating
                if (msg.find("[SUCCESS] Test completed:") != std::string::npos)
                {
                    // Split message into two parts
                    size_t colonPos = msg.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string successPart = msg.substr(0, colonPos + 1);  // "[SUCCESS] Test completed:"
                        std::string ratingPart = msg.substr(colonPos + 2);      // " ELITE" or " LOW" etc
                        
                        // Display success part in green
                        ImGui::TextColored(Colors::Success, successPart.c_str());
                        ImGui::SameLine();
                        
                        // Display rating part in appropriate color - SWAPPED Amazing/Great
                        ImVec4 ratingColor = Colors::MutedForeground;
                        if (ratingPart.find("ELITE") != std::string::npos)
                            ratingColor = ImVec4(0.90f, 0.75f, 0.25f, 1.0f);  // Gold
                        else if (ratingPart.find("AMAZING") != std::string::npos)
                            ratingColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green (swapped)
                        else if (ratingPart.find("GREAT") != std::string::npos || ratingPart.find("GOOD") != std::string::npos)
                            ratingColor = ImVec4(0.118f, 0.565f, 1.0f, 1.0f);  // #1E90FF Dodger Blue (swapped)
                        else if (ratingPart.find("OKAY") != std::string::npos || ratingPart.find("WARNING") != std::string::npos)
                            ratingColor = ImVec4(0.369f, 0.651f, 0.722f, 1.0f);  // #5EA6B8 Teal
                        else if (ratingPart.find("LOW") != std::string::npos)
                            ratingColor = Colors::Destructive;  // Red
                        
                        ImGui::TextColored(ratingColor, ratingPart.c_str());
                    }
                    else
                    {
                        // Fallback to normal coloring
                        ImGui::TextColored(Colors::Success, msg.c_str());
                    }
                }
                // Color code log messages (NO spacing between lines - already color-coded)
                else if (msg.find("[ERROR]") != std::string::npos)
                    ImGui::TextColored(Colors::Destructive, msg.c_str());
                else if (msg.find("[SUCCESS]") != std::string::npos)
                    ImGui::TextColored(Colors::Success, msg.c_str());
                else if (msg.find("[WARNING]") != std::string::npos)
                    ImGui::TextColored(Colors::Warning, msg.c_str());
                else if (msg.find("[INFO]") != std::string::npos)
                    ImGui::TextColored(Colors::Info, msg.c_str());
                else if (msg.find("[DEBUG]") != std::string::npos)
                    ImGui::TextColored(Colors::MutedForeground, msg.c_str());
                else if (msg.find("[PCILEECH]") != std::string::npos)
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), msg.c_str());
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
                    ImGui::Text(msg.c_str());
                    ImGui::PopStyleColor();
                }
            }
            
            // Auto-scroll to bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        
        ImGui::EndChild();
        
        ImGui::PopStyleVar();  // Pop ItemSpacing
        
        ImGui::EndChild();
    }

    void DataPortTab::RenderFT601DriverPanel(float height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));  // Reduced padding
        ImGui::BeginChild("FT601DriverPanel", ImVec2(0, height), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("FTDI Driver");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Driver Status - 2 COLUMN LAYOUT
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Driver Status");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Compact font for status info
        ImGui::SetWindowFontScale(0.95f);
        
        // 2-COLUMN LAYOUT for status information
        ImGui::Columns(2, "DriverStatusColumns", false);
        
        // LEFT COLUMN
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Status:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        if (s_FT601DriverInfo.installed && s_FT601DriverInfo.isCorrectDriver)
            ImGui::TextColored(Colors::Success, "Installed");
        else if (s_FT601DriverInfo.installed && !s_FT601DriverInfo.isCorrectDriver)
            ImGui::TextColored(Colors::Warning, "Installed (out of date)");
        else if (!s_FT601DriverInfo.deviceName.empty() && !s_FT601DriverInfo.isCorrectDriver)
            ImGui::TextColored(Colors::Warning, "Driver Needed");
        else
            ImGui::Text("Not Detected");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Device:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        if (!s_FT601DriverInfo.deviceName.empty())
        {
            // Wrap long device names
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::Text(s_FT601DriverInfo.deviceName.c_str());
            ImGui::PopTextWrapPos();
        }
        else
            ImGui::Text("Not Detected");
        
        // RIGHT COLUMN
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Version:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        ImGui::Text(s_FT601DriverInfo.version.empty() ? "---" : s_FT601DriverInfo.version.c_str());
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("VID/PID:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        ImGui::Text(s_FT601DriverInfo.vidPid.empty() ? "---" : s_FT601DriverInfo.vidPid.c_str());
        
        // Reset to single column
        ImGui::Columns(1);
        
        ImGui::SetWindowFontScale(1.0f);  // Reset font scale
        
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Driver Management
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Warning);
        ImGui::Text("Management");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));  // 1px spacing after label
        
        // Check Driver Status button
        ImGui::BeginDisabled(s_IsCheckingFT601Driver);
        
        std::string checkButtonText = "Check Driver Status";
        if (s_IsCheckingFT601Driver)
        {
            static float dotTimer = 0.0f;
            dotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(dotTimer * 2.0f) % 4);
            checkButtonText = "Checking";
            for (int i = 0; i < dotCount; i++) checkButtonText += "...";
        }
        
        if (Theme::ButtonSecondary(checkButtonText.c_str(), ImVec2(-1, 40)))
        {
            s_IsCheckingFT601Driver = true;
            AddLog("[INFO] Checking FTDI driver status...");
        }
        ImGui::EndDisabled();
        
        // Check driver operation
        static bool checkQueued = false;
        static int checkFrames = 0;
        
        if (s_IsCheckingFT601Driver)
        {
            if (!checkQueued)
            {
                checkQueued = true;
                checkFrames = 0;
            }
            else
            {
                checkFrames++;
            }
            
            if (checkFrames >= 2)
            {
                s_FT601DriverProgress = "Querying driver status...";
                
                // Small delay to show progress
                Sleep(100);
                
                s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                
                if (s_FT601DriverInfo.installed && s_FT601DriverInfo.isCorrectDriver)
                {
                    AddLog("[SUCCESS] FTDI driver is installed");
                    AddLog("[INFO] Device: " + s_FT601DriverInfo.deviceName);
                    AddLog("[INFO] Driver Version: " + s_FT601DriverInfo.version);
                    AddLog("[INFO] VID/PID: " + s_FT601DriverInfo.vidPid);
                }
                else if (s_FT601DriverInfo.installed && !s_FT601DriverInfo.isCorrectDriver)
                {
                    AddLog("[WARNING] FTDI WinUSB driver is out of date");
                    AddLog("[INFO] Device: " + s_FT601DriverInfo.deviceName);
                    AddLog("[INFO] Current version: " + s_FT601DriverInfo.version);
                    AddLog("[INFO] Required version: 1.4.0.1 or higher");
                    AddLog("[INFO] Action: Click 'Install FTDI Driver' to update to version 1.4.0.1");
                }
                else if (!s_FT601DriverInfo.deviceName.empty() && !s_FT601DriverInfo.isCorrectDriver)
                {
                    AddLog("[WARNING] FTDI WinUSB driver not installed");
                    AddLog("[INFO] Device detected: " + s_FT601DriverInfo.deviceName);
                    AddLog("[INFO] Driver version: Not installed (using default Windows driver)");
                    AddLog("[INFO] Required version: 1.4.0.1 or higher");
                    AddLog("[INFO] Action: Click 'Install FTDI Driver' to install WinUSB driver");
                }
                else
                {
                    AddLog("[WARNING] FTDI device not detected");
                    AddLog("[INFO] Please connect the FTDI device");
                }
                
                s_FT601DriverProgress = "";
                s_IsCheckingFT601Driver = false;
                checkQueued = false;
                checkFrames = 0;
            }
        }
        
        ImGui::Dummy(ImVec2(0, 1));  // 1px spacing between buttons
        
        // Install Driver button
        ImGui::BeginDisabled(s_IsInstallingFT601Driver);
        
        std::string installButtonText = "Install FTDI Driver";
        if (s_IsInstallingFT601Driver)
        {
            static float installDotTimer = 0.0f;
            installDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(installDotTimer * 2.0f) % 4);
            installButtonText = "Installing";
            for (int i = 0; i < dotCount; i++) installButtonText += ".";
        }
        
        if (Theme::ButtonPrimary(installButtonText.c_str(), ImVec2(-1, 40)))  // Increased from 32 to 40
        {
            s_IsInstallingFT601Driver = true;
            AddLog("[INFO] Installing FTDI driver...");
        }
        ImGui::EndDisabled();
        
        // Install driver operation
        static bool installQueued = false;
        static int installFrames = 0;
        static std::thread installThread;
        
        if (s_IsInstallingFT601Driver)
        {
            if (!installQueued)
            {
                installQueued = true;
                installFrames = 0;
                
                // Launch installation on a separate thread
                if (installThread.joinable())
                    installThread.join();
                
                installThread = std::thread([]() {
                    bool success = s_FT601Driver.InstallDriver([](const std::string& progress) {
                        // Update progress string (thread-safe since it's just a string assignment)
                        s_FT601DriverProgress = progress;
                    });
                    
                    if (success)
                    {
                        s_FT601DriverProgress = "Driver installed! Refreshing...";
                        AddLog("[SUCCESS] FTDI driver installation completed");
                        AddLog("[INFO] Driver will be applied automatically");
                        
                        // Refresh driver status
                        Sleep(500);
                        s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                        
                        if (s_FT601DriverInfo.installed)
                        {
                            AddLog("[SUCCESS] Driver installed successfully");
                        }
                        else
                        {
                            AddLog("[INFO] Driver installed - device may need replug");
                        }
                    }
                    else
                    {
                        AddLog("[ERROR] Failed to install FTDI driver");
                        AddLog("[INFO] Check that driver files exist in: dmafiles\\Winusb_D3XX_Release_1.4.0.1");
                    }
                    
                    s_FT601DriverProgress = "";
                    s_IsInstallingFT601Driver = false;
                    installQueued = false;
                });
                installThread.detach();
            }
        }
        
        ImGui::Dummy(ImVec2(0, 1));  // 1px spacing between buttons
        
        // Uninstall Driver button
        ImGui::BeginDisabled(s_IsUninstallingFT601Driver);
        
        std::string uninstallButtonText = "Uninstall FTDI Driver";
        if (s_IsUninstallingFT601Driver)
        {
            static float uninstallDotTimer = 0.0f;
            uninstallDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(uninstallDotTimer * 2.0f) % 4);
            uninstallButtonText = "Uninstalling";
            for (int i = 0; i < dotCount; i++) uninstallButtonText += ".";
        }
        
        if (Theme::ButtonDestructive(uninstallButtonText.c_str(), ImVec2(-1, 40)))  // Increased from 32 to 40
        {
            s_IsUninstallingFT601Driver = true;
            AddLog("[INFO] Uninstalling FTDI driver...");
        }
        ImGui::EndDisabled();
        
        // Uninstall driver operation
        static bool uninstallQueued = false;
        static int uninstallFrames = 0;
        static std::thread uninstallThread;
        
        if (s_IsUninstallingFT601Driver)
        {
            if (!uninstallQueued)
            {
                uninstallQueued = true;
                uninstallFrames = 0;
                
                // Launch uninstallation on a separate thread
                if (uninstallThread.joinable())
                    uninstallThread.join();
                
                uninstallThread = std::thread([]() {
                    bool success = s_FT601Driver.UninstallDriver([](const std::string& progress) {
                        // Update progress string (thread-safe since it's just a string assignment)
                        s_FT601DriverProgress = progress;
                    });
                    
                    if (success)
                    {
                        s_FT601DriverProgress = "Driver removed! Refreshing...";
                        AddLog("[SUCCESS] FTDI driver uninstallation completed");
                        
                        // Refresh driver status
                        Sleep(500);
                        s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                        
                        if (!s_FT601DriverInfo.installed)
                        {
                            AddLog("[SUCCESS] Driver uninstalled successfully");
                        }
                        else
                        {
                            AddLog("[INFO] Driver status refreshing in background");
                        }
                    }
                    else
                    {
                        AddLog("[ERROR] Failed to uninstall FTDI driver");
                    }
                    
                    s_FT601DriverProgress = "";
                    s_IsUninstallingFT601Driver = false;
                    uninstallQueued = false;
                });
                uninstallThread.detach();
            }
        }
        
        ImGui::EndChild();
    }
    
    void DataPortTab::Cleanup()
    {
        if (!s_HasCleanedUp)
        {
            std::cout << "[INFO] DataPortTab: Cleaning up benchmark resources..." << std::endl;
            
            // Force cleanup of benchmark interface
            // This will stop any running tests and release the LeechCore device
            s_Benchmark.ForceCleanup();
            s_IsTestRunning = false;
            
            s_HasCleanedUp = true;
            std::cout << "[INFO] DataPortTab: Cleanup complete - DMA device released" << std::endl;
        }
    }
}
