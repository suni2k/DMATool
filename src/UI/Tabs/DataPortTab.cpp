#include "DataPortTab.h"
#include "../Theme.h"
#include <imgui.h>
#include <iostream>

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
        
        // Two column layout for top panels
        ImGui::Columns(2, "BenchmarkColumns", true);
        
        // Left column: Test Controls
        float panelHeight = ImGui::GetContentRegionAvail().y - (ImGui::GetStyle().ItemSpacing.y * 2);
        RenderTestControlsPanel(panelHeight * 0.45f);  // Reduced from 0.6 to 0.45
        
        ImGui::NextColumn();
        
        // Right column: Results
        RenderResultsPanel(panelHeight * 0.45f);  // Reduced from 0.6 to 0.45
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Bottom section: Console log (left) + FT601 Driver (right) in 2-column layout
        ImGui::Columns(2, "BottomPanels", true);
        
        // Left: Console log (60% width)
        float bottomHeight = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y;
        RenderConsoleLog(bottomHeight);
        
        ImGui::NextColumn();
        
        // Right: FT601 Driver panel (40% width)
        RenderFT601DriverPanel(bottomHeight);
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        
        ImGui::EndChild();
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
        
        ImGui::PopStyleVar(2);  // Pop both padding and spacing
        
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
            // Color based on rating
            ImVec4 ratingColor;
            if (results.rating == "ELITE")
                ratingColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
            else if (results.rating == "AMAZING")
                ratingColor = Colors::Success;
            else if (results.rating == "GOOD")
                ratingColor = Colors::Info;
            else if (results.rating == "WARNING")
                ratingColor = Colors::Warning;
            else
                ratingColor = Colors::Destructive;
            
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
                // For throughput test, color based on throughput rating, not RPS
                if (results.rating == "ELITE")
                    rpsColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
                else if (results.rating == "AMAZING")
                    rpsColor = Colors::Success;
                else if (results.rating == "GOOD")
                    rpsColor = Colors::Info;
                else if (results.rating == "WARNING")
                    rpsColor = Colors::Warning;
                else
                    rpsColor = Colors::Destructive;
            }
            else if (s_CurrentTestType == 3)  // Custom Test - use read-size-specific thresholds
            {
                uint32_t readSize = s_Config.customReadSizeBytes;
                
                if (readSize <= 4096)  // 1KB-4KB
                {
                    if (results.readsPerSecond >= 7500) rpsColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                    else if (results.readsPerSecond >= 6500) rpsColor = Colors::Success;
                    else if (results.readsPerSecond >= 5200) rpsColor = Colors::Info;
                    else if (results.readsPerSecond >= 4000) rpsColor = Colors::Warning;
                    else rpsColor = Colors::Destructive;
                }
                else if (readSize == 65536)  // 64KB
                {
                    if (results.readsPerSecond >= 2400) rpsColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                    else if (results.readsPerSecond >= 2200) rpsColor = Colors::Success;
                    else if (results.readsPerSecond >= 1800) rpsColor = Colors::Info;
                    else if (results.readsPerSecond >= 1400) rpsColor = Colors::Warning;
                    else rpsColor = Colors::Destructive;
                }
                else  // 256KB
                {
                    if (results.readsPerSecond >= 600) rpsColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                    else if (results.readsPerSecond >= 550) rpsColor = Colors::Success;
                    else if (results.readsPerSecond >= 480) rpsColor = Colors::Info;
                    else if (results.readsPerSecond >= 380) rpsColor = Colors::Warning;
                    else rpsColor = Colors::Destructive;
                }
            }
            else  // Quick Speed Test / Stress - standard thresholds
            {
                if (results.readsPerSecond >= 7500) rpsColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                else if (results.readsPerSecond >= 6500) rpsColor = Colors::Success;
                else if (results.readsPerSecond >= 5200) rpsColor = Colors::Info;
                else if (results.readsPerSecond >= 4000) rpsColor = Colors::Warning;
                else rpsColor = Colors::Destructive;
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
            // Throughput test - show MB/s scale
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "220+ ELITE");
            ImGui::TextColored(Colors::Success, "200+ AMAZING");
            ImGui::TextColored(Colors::Info, "150+ GOOD");
            ImGui::TextColored(Colors::Warning, "125+ WARNING");
            ImGui::TextColored(Colors::Destructive, "<125 LOW");
        }
        else if (s_CurrentTestType == 3)  // Custom Test - show dynamic scale based on read size
        {
            // Custom test scale adjusts based on read size
            uint32_t readSize = s_Config.customReadSizeBytes;
            
            if (readSize <= 4096)  // 1KB-4KB: Use standard RPS scale
            {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "7500+ ELITE");
                ImGui::TextColored(Colors::Success, "6500+ AMAZING");
                ImGui::TextColored(Colors::Info, "5200+ GOOD");
                ImGui::TextColored(Colors::Warning, "4000+ WARNING");
                ImGui::TextColored(Colors::Destructive, "<4000 LOW");
            }
            else if (readSize == 65536)  // 64KB: Adjusted scale
            {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "2400+ ELITE");
                ImGui::TextColored(Colors::Success, "2200+ AMAZING");
                ImGui::TextColored(Colors::Info, "1800+ GOOD");
                ImGui::TextColored(Colors::Warning, "1400+ WARNING");
                ImGui::TextColored(Colors::Destructive, "<1400 LOW");
            }
            else  // 256KB: Adjusted scale for chunked reads
            {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "600+ ELITE");
                ImGui::TextColored(Colors::Success, "550+ AMAZING");
                ImGui::TextColored(Colors::Info, "480+ GOOD");
                ImGui::TextColored(Colors::Warning, "380+ WARNING");
                ImGui::TextColored(Colors::Destructive, "<380 LOW");
            }
        }
        else  // Speed test (Quick, Stress)
        {
            // Speed test - show RPS scale
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "7500+ ELITE");
            ImGui::TextColored(Colors::Success, "6500+ AMAZING");
            ImGui::TextColored(Colors::Info, "5200+ GOOD");
            ImGui::TextColored(Colors::Warning, "4000+ WARNING");
            ImGui::TextColored(Colors::Destructive, "<4000 LOW");
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
                        
                        // Display rating part in appropriate color
                        ImVec4 ratingColor = Colors::MutedForeground;
                        if (ratingPart.find("ELITE") != std::string::npos)
                            ratingColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
                        else if (ratingPart.find("AMAZING") != std::string::npos)
                            ratingColor = Colors::Success;
                        else if (ratingPart.find("GOOD") != std::string::npos)
                            ratingColor = Colors::Info;
                        else if (ratingPart.find("WARNING") != std::string::npos)
                            ratingColor = Colors::Warning;
                        else if (ratingPart.find("LOW") != std::string::npos)
                            ratingColor = Colors::Destructive;
                        
                        ImGui::TextColored(ratingColor, ratingPart.c_str());
                    }
                    else
                    {
                        // Fallback to normal coloring
                        ImGui::TextColored(Colors::Success, msg.c_str());
                    }
                }
                // Color code log messages
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
        
        ImGui::EndChild();
    }

    void DataPortTab::RenderFT601DriverPanel(float height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));  // Reduced padding
        ImGui::BeginChild("FT601DriverPanel", ImVec2(0, height), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("FTDI FT601 Driver");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Driver Status
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Driver Status");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Compact font for status info
        ImGui::SetWindowFontScale(0.95f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Status:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (s_FT601DriverInfo.installed && s_FT601DriverInfo.isCorrectDriver)
            ImGui::TextColored(Colors::Success, "Installed");
        else if (!s_FT601DriverInfo.deviceName.empty() && !s_FT601DriverInfo.isCorrectDriver)
            ImGui::TextColored(Colors::Warning, "Driver Needed");
        else
            ImGui::TextColored(Colors::Destructive, "Not Detected");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Device:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        if (!s_FT601DriverInfo.deviceName.empty())
        {
            // Wrap long device names
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::Text(s_FT601DriverInfo.deviceName.c_str());
            ImGui::PopTextWrapPos();
        }
        else
            ImGui::Text("Not Detected");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Version:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        ImGui::Text(s_FT601DriverInfo.version.empty() ? "---" : s_FT601DriverInfo.version.c_str());
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("VID/PID:");
        ImGui::PopStyleColor();
        ImGui::SameLine(100);
        ImGui::Text(s_FT601DriverInfo.vidPid.empty() ? "---" : s_FT601DriverInfo.vidPid.c_str());
        
        ImGui::SetWindowFontScale(1.0f);  // Reset font scale
        
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Driver Management
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Warning);
        ImGui::Text("Management");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Check Driver button
        ImGui::BeginDisabled(s_IsCheckingFT601Driver);
        
        std::string checkButtonText = "Check Driver Status";
        if (s_IsCheckingFT601Driver)
        {
            static float dotTimer = 0.0f;
            dotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(dotTimer * 2.0f) % 4);
            checkButtonText = "Checking";
            for (int i = 0; i < dotCount; i++) checkButtonText += ".";
        }
        
        if (Theme::ButtonSecondary(checkButtonText.c_str(), ImVec2(-1, 32)))  // Reduced button height
        {
            s_IsCheckingFT601Driver = true;
            AddLog("[INFO] Checking FT601 driver status...");
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
                s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                
                if (s_FT601DriverInfo.installed && s_FT601DriverInfo.isCorrectDriver)
                {
                    AddLog("[SUCCESS] FT601 driver is installed");
                    AddLog("[INFO] Device: " + s_FT601DriverInfo.deviceName);
                    if (!s_FT601DriverInfo.version.empty())
                        AddLog("[INFO] Version: " + s_FT601DriverInfo.version);
                    if (!s_FT601DriverInfo.vidPid.empty())
                        AddLog("[INFO] VID/PID: " + s_FT601DriverInfo.vidPid);
                }
                else if (!s_FT601DriverInfo.deviceName.empty() && !s_FT601DriverInfo.isCorrectDriver)
                {
                    AddLog("[WARNING] FT601 driver not installed");
                    AddLog("[INFO] Current device: " + s_FT601DriverInfo.deviceName);
                    AddLog("[INFO] This is the default device name - driver needed");
                    AddLog("[INFO] Action: Click 'Install FT601 Driver' to install proper driver");
                }
                else
                {
                    AddLog("[WARNING] FT601 device not detected");
                    AddLog("[INFO] Please connect the FT601 device");
                }
                
                s_IsCheckingFT601Driver = false;
                checkQueued = false;
                checkFrames = 0;
            }
        }
        
        ImGui::Spacing();
        
        // Install Driver button
        ImGui::BeginDisabled(s_IsInstallingFT601Driver);
        
        std::string installButtonText = "Install FT601 Driver";
        if (s_IsInstallingFT601Driver)
        {
            static float installDotTimer = 0.0f;
            installDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(installDotTimer * 2.0f) % 4);
            installButtonText = "Installing";
            for (int i = 0; i < dotCount; i++) installButtonText += ".";
        }
        
        if (Theme::ButtonPrimary(installButtonText.c_str(), ImVec2(-1, 32)))  // Reduced button height
        {
            s_IsInstallingFT601Driver = true;
            AddLog("[INFO] Installing FT601 driver...");
        }
        ImGui::EndDisabled();
        
        // Install driver operation
        static bool installQueued = false;
        static int installFrames = 0;
        
        if (s_IsInstallingFT601Driver)
        {
            if (!installQueued)
            {
                installQueued = true;
                installFrames = 0;
            }
            else
            {
                installFrames++;
            }
            
            if (installFrames >= 2)
            {
                if (s_FT601Driver.InstallDriver())
                {
                    AddLog("[SUCCESS] FT601 driver installation initiated");
                    AddLog("[INFO] Please follow UAC prompts if they appear");
                    
                    // Refresh status after install
                    Sleep(2000);
                    s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                    
                    if (s_FT601DriverInfo.installed)
                    {
                        AddLog("[SUCCESS] Driver installed successfully");
                    }
                }
                else
                {
                    AddLog("[ERROR] Failed to install FT601 driver");
                    AddLog("[INFO] Check that driver files exist in: dmafiles\\Winusb_D3XX_Release_1.4.0.1");
                }
                
                s_IsInstallingFT601Driver = false;
                installQueued = false;
                installFrames = 0;
            }
        }
        
        ImGui::Spacing();
        
        // Uninstall Driver button
        ImGui::BeginDisabled(s_IsUninstallingFT601Driver);
        
        std::string uninstallButtonText = "Uninstall FT601 Driver";
        if (s_IsUninstallingFT601Driver)
        {
            static float uninstallDotTimer = 0.0f;
            uninstallDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(uninstallDotTimer * 2.0f) % 4);
            uninstallButtonText = "Uninstalling";
            for (int i = 0; i < dotCount; i++) uninstallButtonText += ".";
        }
        
        if (Theme::ButtonDestructive(uninstallButtonText.c_str(), ImVec2(-1, 32)))  // Reduced button height
        {
            s_IsUninstallingFT601Driver = true;
            AddLog("[INFO] Uninstalling FT601 driver...");
        }
        ImGui::EndDisabled();
        
        // Uninstall driver operation
        static bool uninstallQueued = false;
        static int uninstallFrames = 0;
        
        if (s_IsUninstallingFT601Driver)
        {
            if (!uninstallQueued)
            {
                uninstallQueued = true;
                uninstallFrames = 0;
            }
            else
            {
                uninstallFrames++;
            }
            
            if (uninstallFrames >= 2)
            {
                if (s_FT601Driver.UninstallDriver())
                {
                    AddLog("[SUCCESS] FT601 driver uninstallation initiated");
                    
                    // Refresh status after uninstall
                    Sleep(2000);
                    s_FT601DriverInfo = s_FT601Driver.CheckDriver();
                    
                    if (!s_FT601DriverInfo.installed)
                    {
                        AddLog("[SUCCESS] Driver uninstalled successfully");
                    }
                }
                else
                {
                    AddLog("[ERROR] Failed to uninstall FT601 driver");
                }
                
                s_IsUninstallingFT601Driver = false;
                uninstallQueued = false;
                uninstallFrames = 0;
            }
        }
        
        ImGui::EndChild();
    }
}
