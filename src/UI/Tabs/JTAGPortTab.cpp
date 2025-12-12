#include "JTAGPortTab.h"
#include "../Theme.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <Windows.h>

// Integration References:
// - CH347 USB-JTAG adapter: https://github.com/WCHSoftGroup/ch347
// - PCILeech JTAG operations for FPGA programming
// - OpenOCD for JTAG chain detection and DNA ID extraction

namespace DMATool::UI::Tabs
{
    // Static member initialization
    Backend::FPGAInfo JTAGPortTab::s_FPGAInfo;
    Backend::DriverInfo JTAGPortTab::s_DriverInfo;
    std::vector<std::string> JTAGPortTab::s_LogMessages;
    bool JTAGPortTab::s_IsDetecting = false;
    bool JTAGPortTab::s_IsAutoDetecting = false;
    bool JTAGPortTab::s_IsCheckingDriver = false;
    bool JTAGPortTab::s_IsInstallingDriver = false;
    bool JTAGPortTab::s_IsUninstallingDriver = false;
    bool JTAGPortTab::s_IsCopyingDNA = false;
    bool JTAGPortTab::s_ResetFrameCounter = false;
    bool JTAGPortTab::s_DriverCheckCompleted = false;
    std::string JTAGPortTab::s_ConnectionStatus = "Disconnected";
    std::string JTAGPortTab::s_DetectionStatus = "Not Detected";
    std::string JTAGPortTab::s_LastOperation = "None";
    std::string JTAGPortTab::s_CurrentProgress = "";

    // Helper function to extract adapter type from driver device name
    Backend::AdapterType GetAdapterTypeFromDriverInfo(const Backend::DriverInfo& driverInfo)
    {
        if (!driverInfo.installed)
            return Backend::AdapterType::Unknown;
        
        std::string deviceName = driverInfo.deviceName;
        std::transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);
        
        if (deviceName.find("ch347") != std::string::npos)
            return Backend::AdapterType::CH347;
        else if (deviceName.find("ftdi") != std::string::npos)
            return Backend::AdapterType::RS232;
        
        return Backend::AdapterType::Unknown;
    }

    void JTAGPortTab::AddLog(const std::string& message)
    {
        s_LogMessages.push_back(message);
        
        // Also output to console for debugging
        std::cout << message << std::endl;
        
        // Keep only last 100 messages
        if (s_LogMessages.size() > 100)
        {
            s_LogMessages.erase(s_LogMessages.begin());
        }
    }

    void JTAGPortTab::ClearLog()
    {
        s_LogMessages.clear();
    }

    void JTAGPortTab::UpdateProgress(const std::string& progress)
    {
        s_CurrentProgress = progress;
        AddLog("[INFO] " + progress);
    }

    void JTAGPortTab::Render()
    {
        // DISABLE AUTO-DETECTION: User must manually detect FPGA
        // Reason: Tool might be launched while in UPDATE port, causing detection to fail
        static bool s_HasAutoDetected = true;  // Set to true to disable auto-detection
        static bool s_AutoDetectQueued = false;
        static int s_FramesSinceFirstRender = 0;
        static int s_FramesSinceDetectionQueued = 0;
        static bool s_FirstRenderOfSession = true;
        
        // Track frames since first render (for initial delay)
        if (s_FirstRenderOfSession)
        {
            s_FirstRenderOfSession = false;
            s_FramesSinceFirstRender = 0;
        }
        else
        {
            s_FramesSinceFirstRender++;
        }
        
        // AUTO-DETECTION DISABLED - User must click "Detect FPGA & Read DNA" button
        // (Queue auto-detection code commented out)
        /*
        if (s_FramesSinceFirstRender == 2 && !s_AutoDetectQueued && 
            !s_IsDetecting && !s_HasAutoDetected)
        {
            s_AutoDetectQueued = true;
            s_HasAutoDetected = true;
        }
        */
        
        // Start detection (sets flag, actual work happens below after UI renders)
        // AUTO-DETECTION DISABLED - This block will never execute
        if (s_AutoDetectQueued && !s_IsDetecting && !s_IsAutoDetecting)
        {
            s_AutoDetectQueued = false;
            s_IsAutoDetecting = true;
            s_IsDetecting = true;
            s_FramesSinceDetectionQueued = 0;
            ClearLog();
            s_LastOperation = "Auto-Detection";
            s_CurrentProgress = "Initializing...";
            AddLog("[INFO] Auto-detecting FPGA and adapter...");
            
            // Detection will happen on next frame
        }
        
        // Track when a new operation starts
        static bool s_WasDetecting = false;
        static bool s_WasCheckingDriver = false;
        static bool s_WasInstallingDriver = false;
        static bool s_WasUninstallingDriver = false;
        
        bool isAnyOperationActive = s_IsDetecting || s_IsCheckingDriver || s_IsInstallingDriver || s_IsUninstallingDriver;
        bool wasAnyOperationActive = s_WasDetecting || s_WasCheckingDriver || s_WasInstallingDriver || s_WasUninstallingDriver;
        
        // Detect when a NEW operation starts (transition from false to true)
        if (isAnyOperationActive && !wasAnyOperationActive)
        {
            s_FramesSinceDetectionQueued = 0;
        }
        
        // Reset frame counter when manual detection button is clicked (BEFORE increment)
        if (s_ResetFrameCounter)
        {
            s_FramesSinceDetectionQueued = 0;
            s_ResetFrameCounter = false;
        }
        
        // Count frames since detection was queued (AFTER reset check)
        if (isAnyOperationActive)
        {
            s_FramesSinceDetectionQueued++;
        }
        
        // Update previous state for next frame
        s_WasDetecting = s_IsDetecting;
        s_WasCheckingDriver = s_IsCheckingDriver;
        s_WasInstallingDriver = s_IsInstallingDriver;
        s_WasUninstallingDriver = s_IsUninstallingDriver;

        // Render the UI first
        ImGui::BeginChild("JTAGPortContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::Spacing();
        
        // RESIZABLE VERTICAL LAYOUT: Top panels vs Bottom panel
        static float topPanelHeightRatio = 0.52f;  // Default 52% for top panels
        
        float availableHeight = ImGui::GetContentRegionAvail().y;
        float topHeight = availableHeight * topPanelHeightRatio;
        
        // Top section
        ImGui::BeginChild("TopSection", ImVec2(0, topHeight), false, ImGuiWindowFlags_NoScrollbar);
        
        // Two column layout for top panels
        ImGui::Columns(2, "JTAGColumns", true);
        
        RenderDeviceInfoPanel();
        
        ImGui::NextColumn();
        
        RenderDriverPanel();
        
        ImGui::Columns(1);
        
        ImGui::EndChild();
        
        // HORIZONTAL RESIZE HANDLE between top and bottom sections (matches vertical column separator)
        ImVec2 cursorBeforeSeparator = ImGui::GetCursorScreenPos();
        ImGui::Separator();
        
        ImVec2 separatorPos = cursorBeforeSeparator;
        float separatorWidth = ImGui::GetContentRegionAvail().x + ImGui::GetStyle().WindowPadding.x * 2;
        
        ImGui::SetCursorScreenPos(ImVec2(separatorPos.x, separatorPos.y - 2));
        ImGui::InvisibleButton("##vsplitter", ImVec2(separatorWidth, 4));
        
        bool isHovered = ImGui::IsItemHovered();
        bool isActive = ImGui::IsItemActive();
        
        if (isActive)
        {
            float delta = ImGui::GetIO().MouseDelta.y;
            topPanelHeightRatio += delta / availableHeight;
            if (topPanelHeightRatio < 0.3f) topPanelHeightRatio = 0.3f;
            if (topPanelHeightRatio > 0.7f) topPanelHeightRatio = 0.7f;
        }
        
        if (isActive)
        {
            ImGui::GetWindowDrawList()->AddLine(
                separatorPos,
                ImVec2(separatorPos.x + separatorWidth, separatorPos.y),
                IM_COL32(230, 191, 64, 255),  // brandGoldLight
                1.5f
            );
        }
        else if (isHovered)
        {
            ImGui::GetWindowDrawList()->AddLine(
                separatorPos,
                ImVec2(separatorPos.x + separatorWidth, separatorPos.y),
                IM_COL32(212, 176, 56, 255),  // brandGold
                1.5f
            );
        }
        
        if (isHovered || isActive)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }
        
        // Bottom section: Status & Log panel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("StatusPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Status & Log");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Status indicators
        ImGui::Columns(3, "StatusColumns", false);
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Connection:");
        ImGui::PopStyleColor();
        
        if (s_ConnectionStatus == "Connected")
            ImGui::TextColored(Colors::Success, s_ConnectionStatus.c_str());
        else
            ImGui::TextColored(Colors::Destructive, s_ConnectionStatus.c_str());
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Detection Status:");
        ImGui::PopStyleColor();
        
        if (s_DetectionStatus == "Detected")
            ImGui::TextColored(Colors::Success, s_DetectionStatus.c_str());
        else
            ImGui::Text(s_DetectionStatus.c_str());
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Last Operation:");
        ImGui::PopStyleColor();
        ImGui::Text(s_LastOperation.c_str());
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Log output with scrolling
        ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        if (s_LogMessages.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Waiting for operations...");
            ImGui::Text("[INFO] DNA ID tab initialized");
            ImGui::Text("[INFO] Click 'Detect FPGA & Read DNA' to start");
            ImGui::Text("[INFO] Note: Ensure adapter is in DATA port, not UPDATE port");
            ImGui::PopStyleColor();
        }
        else
        {
            for (const auto& msg : s_LogMessages)
            {
                // Color code log messages
                if (msg.find("[ERROR]") != std::string::npos)
                    ImGui::TextColored(Colors::Destructive, msg.c_str());
                else if (msg.find("[SUCCESS]") != std::string::npos)
                    ImGui::TextColored(Colors::Success, msg.c_str());
                else if (msg.find("[WARNING]") != std::string::npos)
                    ImGui::TextColored(Colors::Warning, msg.c_str());
                else if (msg.find("[INFO]") != std::string::npos)
                    ImGui::TextColored(Colors::Info, msg.c_str());
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
        
        // Perform auto-detection AFTER UI has rendered AND notification has shown for 1 frame
        if (s_IsAutoDetecting && s_FramesSinceDetectionQueued >= 2)
        {
            UpdateProgress("Detecting hardware...");
            Backend::OpenOCDInterface openocd;
            
            // STEP 1: Detect DMA card by VID/PID (driver-agnostic)
            AddLog("[INFO] Scanning for DMA cards...");
            Backend::CardInfo cardInfo = openocd.DetectDMACard();
            
            if (!cardInfo.detected)
            {
                AddLog("[ERROR] No DMA card detected");
                AddLog("[INFO] Please ensure your DMA card is properly connected");
                AddLog("[INFO] Supported cards: 35T (RS232), 75T (CH347), 100T (CH347)");
                
                s_DetectionStatus = "No Card";
                s_ConnectionStatus = "Disconnected";
                s_CurrentProgress = "No DMA card found!";
                
                s_IsDetecting = false;
                s_IsAutoDetecting = false;
                s_FramesSinceDetectionQueued = 0;
                Sleep(3000);
                s_CurrentProgress = "";
                return;
            }
            
            // Card detected! Set adapter type early so UI updates correctly
            s_FPGAInfo.adapterType = cardInfo.adapterType;
            
            AddLog("[SUCCESS] Detected: " + cardInfo.cardTypeString + " DMA card");
            AddLog("[INFO] VID:PID = " + cardInfo.vidPid);
            AddLog("[INFO] Device: " + cardInfo.deviceName);
            
            // STEP 2: Check driver status for THIS specific card
            UpdateProgress("Checking driver status...");
            std::string driverTypeName = (cardInfo.adapterType == Backend::AdapterType::RS232) ? "RS232" : "CH347";
            AddLog("[INFO] Checking " + driverTypeName + " driver status...");
            
            s_DriverInfo = (cardInfo.adapterType == Backend::AdapterType::RS232)
                ? openocd.CheckRS232Driver()
                : openocd.CheckCH347Driver();
            
            bool canDetectFPGA = false;
            
            if (s_DriverInfo.installed)
            {
                // Driver found - check if it's the CORRECT driver
                bool isCorrectDriver = false;
                
                if (cardInfo.adapterType == Backend::AdapterType::CH347)
                {
                    // CH347: Need "HighSpeed-JTAG" driver
                    isCorrectDriver = (s_DriverInfo.deviceName.find("HighSpeed-JTAG") != std::string::npos);
                }
                else if (cardInfo.adapterType == Backend::AdapterType::RS232)
                {
                    // RS232: CheckRS232Driver() already validated - WinUSB = installed, FTDIBUS = not installed
                    // If we're here and installed=true, it means WinUSB is present
                    isCorrectDriver = true;
                }
                
                if (isCorrectDriver)
                {
                    AddLog("[SUCCESS] Correct " + driverTypeName + " driver detected");
                    AddLog("[INFO] Device: " + s_DriverInfo.deviceName);
                    AddLog("[INFO] Version: " + s_DriverInfo.version);
                    canDetectFPGA = true;
                }
                else
                {
                    // Wrong driver installed
                    AddLog("[WARNING] Wrong " + driverTypeName + " driver detected: " + s_DriverInfo.deviceName);
                    AddLog("[ERROR] Cannot detect FPGA with current driver");
                    AddLog("[INFO] ");
                    AddLog("[INFO] ===== ACTION REQUIRED =====");
                    AddLog("[INFO] Please install the correct " + driverTypeName + " driver:");
                    AddLog("[INFO] 1. Click 'Uninstall " + driverTypeName + " Driver' below");
                    AddLog("[INFO] 2. Wait for uninstall to complete");
                    AddLog("[INFO] 3. Click 'Install " + driverTypeName + " Driver'");
                    AddLog("[INFO] 4. Follow installation prompts");
                    AddLog("[INFO] 5. Click 'Detect FPGA & Read DNA' to retry");
                    AddLog("[INFO] =============================");
                    
                    s_DetectionStatus = "Driver Issue";
                    s_ConnectionStatus = "Wrong Driver";
                    s_CurrentProgress = "Driver installation required!";
                    canDetectFPGA = false;
                }
            }
            else
            {
                // No driver installed or wrong driver (FTDIBUS for RS232)
                AddLog("[ERROR] " + driverTypeName + " driver not installed");
                if (cardInfo.adapterType == Backend::AdapterType::RS232 && !s_DriverInfo.deviceName.empty())
                {
                    AddLog("[INFO] Current: " + s_DriverInfo.deviceName);
                    AddLog("[INFO] Required: WinUSB driver for JTAG operations");
                }
                AddLog("[INFO] Cannot detect FPGA without correct driver");
                AddLog("[INFO] ");
                AddLog("[INFO] ===== ACTION REQUIRED =====");
                AddLog("[INFO] Please install the " + driverTypeName + " driver:");
                AddLog("[INFO] 1. Click 'Install " + driverTypeName + " Driver' below");
                AddLog("[INFO] 2. Wait for installation to complete");
                AddLog("[INFO] 3. Click 'Detect FPGA & Read DNA' to retry");
                AddLog("[INFO] =============================");
                
                s_DetectionStatus = "No Driver";
                s_ConnectionStatus = "Disconnected";
                s_CurrentProgress = "Driver installation required!";
                canDetectFPGA = false;
            }
            
            // STEP 3: Proceed with FPGA detection if driver is correct
            if (canDetectFPGA)
            {
                UpdateProgress("Driver OK! Searching for OpenOCD...");
                UpdateProgress("Detecting JTAG adapter...");
                s_FPGAInfo = openocd.DetectFPGA([](const std::string& msg) {
                    AddLog(msg);
                    // Extract progress from log messages
                    if (msg.find("Detecting") != std::string::npos)
                        s_CurrentProgress = msg.substr(msg.find("]") + 2);
                    else if (msg.find("Found") != std::string::npos)
                        s_CurrentProgress = msg.substr(msg.find("]") + 2);
                });
                
                if (s_FPGAInfo.detected)
                {
                    UpdateProgress("FPGA detected! All systems ready.");
                    s_DetectionStatus = "Detected";
                    s_ConnectionStatus = "Connected";
                    AddLog("[SUCCESS] FPGA detected: " + s_FPGAInfo.partNumber);
                    AddLog("[INFO] DNA ID: " + s_FPGAInfo.dnaId);
                }
                else
                {
                    UpdateProgress("Detection failed. Check connections.");
                    s_DetectionStatus = "Failed";
                    s_ConnectionStatus = "Disconnected";
                }
            }
            
            s_IsDetecting = false;
            s_IsAutoDetecting = false;
            s_FramesSinceDetectionQueued = 0;
            
            // Keep progress message visible for 3 seconds if driver issue
            if (!canDetectFPGA)
            {
                Sleep(3000);
            }
            s_CurrentProgress = "";
        }
        
        // Perform manual FPGA detection (triggered by button)
        if (!s_IsAutoDetecting && s_IsDetecting && !s_IsCheckingDriver && 
            !s_IsInstallingDriver && !s_IsUninstallingDriver && s_FramesSinceDetectionQueued >= 2)
        {
            Backend::OpenOCDInterface openocd;
            
            // STEP 1: Detect DMA card by VID/PID (driver-agnostic)
            UpdateProgress("Detecting hardware...");
            AddLog("[INFO] Scanning for DMA cards...");
            Backend::CardInfo cardInfo = openocd.DetectDMACard();
            
            if (!cardInfo.detected)
            {
                AddLog("[ERROR] No DMA card detected");
                AddLog("[INFO] Please ensure your DMA card is properly connected");
                AddLog("[INFO] Supported cards: 35T (RS232), 75T (CH347), 100T (CH347)");
                
                s_DetectionStatus = "No Card";
                s_ConnectionStatus = "Disconnected";
                s_CurrentProgress = "No DMA card found!";
                
                s_IsDetecting = false;
                s_FramesSinceDetectionQueued = 0;
                Sleep(2000);
                s_CurrentProgress = "";
            }
            else
            {
                // Card detected! Set adapter type
                s_FPGAInfo.adapterType = cardInfo.adapterType;
                
                AddLog("[SUCCESS] Detected: " + cardInfo.cardTypeString + " DMA card");
                AddLog("[INFO] VID:PID = " + cardInfo.vidPid);
                
                // STEP 2: Proceed with FPGA detection
                UpdateProgress("Searching for OpenOCD executable...");
                s_FPGAInfo = openocd.DetectFPGA([](const std::string& msg) {
                    AddLog(msg);
                });
                
                if (s_FPGAInfo.detected)
                {
                    UpdateProgress("FPGA detected successfully!");
                    s_DetectionStatus = "Detected";
                    s_ConnectionStatus = "Connected";
                    AddLog("[SUCCESS] FPGA detected: " + s_FPGAInfo.partNumber);
                    AddLog("[INFO] DNA ID: " + s_FPGAInfo.dnaId);
                    s_CurrentProgress = "Detection complete!";
                }
                else
                {
                    UpdateProgress("Detection failed!");
                    s_DetectionStatus = "Failed";
                    s_ConnectionStatus = "Disconnected";
                    AddLog("[ERROR] Failed to detect FPGA. Check connections.");
                    s_CurrentProgress = "Detection failed.";
                }
                
                s_IsDetecting = false;
                s_FramesSinceDetectionQueued = 0;
                s_CurrentProgress = "";
            }
        }

        // Floating progress notification (toast-style) - MUST render even if detection hasn't started yet
        if (s_IsDetecting || s_IsCheckingDriver || s_IsInstallingDriver || s_IsUninstallingDriver)
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
            
            ImGui::Begin("##ScanningNotification", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoCollapse);
            
            // Title text - no spinner, just text
            ImGui::SetCursorPos(ImVec2(24, 35));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.97f, 1.0f));  // Near white
            ImGui::SetWindowFontScale(1.4f);
            
            std::string statusText = "Scanning Device";
            if (s_IsCheckingDriver)
                statusText = "Checking Driver";
            else if (s_IsInstallingDriver)
                statusText = "Installing Driver";
            else if (s_IsUninstallingDriver)
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
            
            // Current progress with better styling
            ImGui::SetCursorPosY(80);
            if (!s_CurrentProgress.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.75f, 0.8f, 1.0f));  // Light blue-gray
                ImGui::SetWindowFontScale(1.05f);
                
                // Center and wrap text properly
                ImGui::PushTextWrapPos(toastWidth - 48);
                float textWidth = ImGui::CalcTextSize(s_CurrentProgress.c_str(), nullptr, false, toastWidth - 48).x;
                ImGui::SetCursorPosX((toastWidth - textWidth) * 0.5f);
                ImGui::TextWrapped(s_CurrentProgress.c_str());
                ImGui::PopTextWrapPos();
                
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            }
            else
            {
                // Animated dots when no specific progress
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.7f, 1.0f));
                ImGui::SetWindowFontScale(1.05f);
                
                static float dotTimer = 0.0f;
                dotTimer += ImGui::GetIO().DeltaTime;
                int dotCount = ((int)(dotTimer * 2.0f) % 4);
                
                std::string dots = "Please wait";
                for (int i = 0; i < dotCount; i++) dots += ".";

                float dotsWidth = ImGui::CalcTextSize(dots.c_str()).x;
                ImGui::SetCursorPosX((toastWidth - dotsWidth) * 0.5f);
                ImGui::Text(dots.c_str());
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            }
            
            ImGui::End();
            
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);
        }
        
        ImGui::EndChild();  // End JTAGPortContent
    }

    void JTAGPortTab::RenderDeviceInfoPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 10));
        ImGui::BeginChild("DeviceInfoPanel", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        
        ImGui::Text("FPGA Device Information");
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // FPGA Details Section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("FPGA Details");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Chip Model:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FPGAInfo.detected)
            ImGui::TextColored(Colors::Success, s_FPGAInfo.partNumber.c_str());
        else
            ImGui::Text("Unknown");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Adapter:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        
        // Derive adapter from driver info first, then FPGA detection as fallback
        Backend::AdapterType adapterType = GetAdapterTypeFromDriverInfo(s_DriverInfo);
        if (adapterType == Backend::AdapterType::Unknown && s_FPGAInfo.detected)
        {
            adapterType = s_FPGAInfo.adapterType;
        }
        
        if (adapterType != Backend::AdapterType::Unknown)
        {
            // Display adapter type
            const char* adapterName = "Unknown";
            ImVec4 adapterColor = Colors::MutedForeground;
            
            switch (adapterType)
            {
            case Backend::AdapterType::CH347:
                adapterName = "CH347";
                adapterColor = Colors::Success;
                break;
            case Backend::AdapterType::RS232:
                adapterName = "RS232/FTDI";
                adapterColor = Colors::Info;
                break;
            case Backend::AdapterType::Unknown:
            default:
                adapterName = "Unknown";
                adapterColor = Colors::MutedForeground;
                break;
            }
            
            ImGui::TextColored(adapterColor, adapterName);
        }
        else
            ImGui::Text("---");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Manufacturer:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FPGAInfo.detected)
            ImGui::Text(s_FPGAInfo.manufacturer.c_str());
        else
            ImGui::Text("Xilinx");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Family:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FPGAInfo.detected)
            ImGui::Text(s_FPGAInfo.family.c_str());
        else
            ImGui::Text("Artix-7");
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // DNA ID Section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Success);
        ImGui::Text("DNA ID");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Unique DNA:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FPGAInfo.detected && !s_FPGAInfo.dnaId.empty())
            ImGui::TextColored(Colors::Warning, s_FPGAInfo.dnaId.c_str());
        else
            ImGui::Text("---");
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Reduced spacing
        
        // Detection button - DISABLED until driver check is done AND correct driver is installed
        ImGui::BeginDisabled((s_IsDetecting && !s_IsCheckingDriver) || !s_DriverCheckCompleted || !s_DriverInfo.installed);
        
        std::string buttonText = "Detect FPGA & Read DNA";
        if (!s_DriverCheckCompleted)
        {
            buttonText = "Detect FPGA & Read DNA (Check Driver First)";
        }
        else if (!s_DriverInfo.installed)
        {
            buttonText = "Detect FPGA & Read DNA (Install Driver First)";
        }
        else if (s_IsDetecting && !s_IsCheckingDriver && !s_IsInstallingDriver && !s_IsUninstallingDriver)
        {
            // Animated dots - only when THIS button's operation is running
            static float detectDotTimer = 0.0f;
            detectDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(detectDotTimer * 2.0f) % 4);
            buttonText = "Detecting";
            for (int i = 0; i < dotCount; i++) buttonText += ".";
        }
        
        if (Theme::ButtonPrimary(buttonText.c_str(), ImVec2(-1, 36)))  // Reduced button height
        {
            s_IsDetecting = true;
            s_IsAutoDetecting = false;  // Explicitly mark as manual detection
            s_ResetFrameCounter = true;  // Signal to reset frame counter in Render()
            ClearLog();
            s_LastOperation = "FPGA Detection";
            s_CurrentProgress = "Starting detection...";
            AddLog("[INFO] Starting FPGA detection...");
        }
        
        // Show tooltip when disabled
        if (!s_DriverCheckCompleted && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Please click 'Check Driver Status' first to detect your DMA card type");
        }
        else if (s_DriverCheckCompleted && !s_DriverInfo.installed && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Please install the correct driver first. Click 'Install RS232 Driver' or 'Install CH347 Driver' below.");
        }
        
        ImGui::EndDisabled();
        
        // Reduce spacing between buttons
        ImGui::Dummy(ImVec2(0, 1));
        
        // Copy DNA button with feedback
        ImGui::BeginDisabled(s_IsCopyingDNA || !s_FPGAInfo.detected || s_FPGAInfo.dnaId.empty());
        
        std::string copyButtonText = s_IsCopyingDNA ? "Copied!" : "Copy DNA to Clipboard";
        
        if (Theme::ButtonSecondary(copyButtonText.c_str(), ImVec2(-1, 36)))  // Reduced button height
        {
            if (!s_FPGAInfo.dnaId.empty())
            {
                s_IsCopyingDNA = true;
                
                // Copy to clipboard
                if (OpenClipboard(nullptr))
                {
                    EmptyClipboard();
                    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s_FPGAInfo.dnaId.size() + 1);
                    if (hg)
                    {
                        memcpy(GlobalLock(hg), s_FPGAInfo.dnaId.c_str(), s_FPGAInfo.dnaId.size() + 1);
                        GlobalUnlock(hg);
                        SetClipboardData(CF_TEXT, hg);
                        AddLog("[SUCCESS] DNA ID copied to clipboard: " + s_FPGAInfo.dnaId);
                        s_LastOperation = "Copy DNA";
                    }
                    CloseClipboard();
                }
            }
        }
        
        // Reset copy feedback after delay
        static int copyFeedbackFrames = 0;
        if (s_IsCopyingDNA)
        {
            if (copyFeedbackFrames > 0)
                copyFeedbackFrames--;
            else
                s_IsCopyingDNA = false;
        }
        else
        {
            copyFeedbackFrames = 60;  // Reset when button is clicked
        }
        
        ImGui::EndDisabled();
        
        ImGui::EndChild();
    }

    void JTAGPortTab::RenderDriverPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 10));
        ImGui::BeginChild("DriverPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("JTAG Driver Information");
        ImGui::Dummy(ImVec2(0, 1));  // Match FPGA panel spacing
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Match FPGA panel spacing
        
        // Driver Status - Use 2 columns for better layout
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Driver Status");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));  // Add spacing after section header

        ImGui::Columns(2, "DriverInfoColumns", false);
        
        // Left column
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Status:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        if (s_DriverInfo.installed)
            ImGui::TextColored(Colors::Success, "Installed");
        else if (s_DriverCheckCompleted)
            ImGui::TextColored(Colors::Warning, "Driver Install Needed");
        else
            ImGui::Text("---");
        
        // Right column
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Version:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        ImGui::Text(s_DriverInfo.version.empty() ? "---" : s_DriverInfo.version.c_str());
        
        // Back to left column
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Adapter:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        
        // Derive adapter from driver info first, then FPGA detection as fallback
        Backend::AdapterType adapterType = s_FPGAInfo.adapterType;  // Use FPGAInfo (set by card detection)
        if (adapterType == Backend::AdapterType::Unknown)
        {
            adapterType = GetAdapterTypeFromDriverInfo(s_DriverInfo);
        }
        
        if (adapterType != Backend::AdapterType::Unknown)
        {
            const char* adapterName = "Unknown";
            ImVec4 adapterColor = Colors::MutedForeground;
            
            switch (adapterType)
            {
            case Backend::AdapterType::CH347:
                adapterName = "CH347";
                adapterColor = Colors::Success;
                break;
            case Backend::AdapterType::RS232:
                adapterName = "RS-232";
                adapterColor = Colors::Info;
                break;
            case Backend::AdapterType::Unknown:
            default:
                adapterName = "Unknown";
                adapterColor = Colors::MutedForeground;
                break;
            }
            
            ImGui::TextColored(adapterColor, adapterName);
        }
        else
            ImGui::Text("---");
        
        // Right column for Device
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Device:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        if (!s_DriverInfo.deviceName.empty())
        {
            // Show full device name with text wrapping (regardless of installed status)
            float availableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + availableWidth);
            ImGui::Text(s_DriverInfo.deviceName.c_str());
            ImGui::PopTextWrapPos();
        }
        else
            ImGui::Text("Not Detected");
        
        ImGui::Columns(1);
        
        // VID/PID on its own line
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("VID/PID:");
        ImGui::PopStyleColor();
        ImGui::SameLine(70);
        ImGui::Text(s_DriverInfo.vidPid.empty() ? "---" : s_DriverInfo.vidPid.c_str());
        
        ImGui::Dummy(ImVec2(0, 1));  // Add spacing before separator
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));  // Add spacing after separator
        
        // Driver Management Buttons - Dynamic based on adapter type
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Warning);
        ImGui::Text("Management");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Determine adapter type for button labels
        // Use detected adapter type (set by card detection OR FPGA detection)
        bool cardDetected = (s_FPGAInfo.adapterType != Backend::AdapterType::Unknown);
        std::string driverName = "DMA Card";  // Default if unknown
        if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
        {
            driverName = "RS232";
        }
        else if (s_FPGAInfo.adapterType == Backend::AdapterType::CH347)
        {
            driverName = "CH347";
        }
        
        // Check driver status button (always enabled)
        ImGui::BeginDisabled(s_IsCheckingDriver);
        
        std::string checkButtonText = "Check Driver Status";
        if (s_IsCheckingDriver)
        {
            static float checkDotTimer = 0.0f;
            checkDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(checkDotTimer * 2.0f) % 4);
            checkButtonText = "Checking";
            for (int i = 0; i < dotCount; i++) checkButtonText += ".";
        }
        
        if (Theme::ButtonSecondary(checkButtonText.c_str(), ImVec2(-1, 40)))
        {
            s_IsCheckingDriver = true;
            s_IsDetecting = true;
            s_LastOperation = "Check Driver";
            s_CurrentProgress = "Checking driver status...";
            AddLog("[INFO] Checking driver status...");
        }
        
        ImGui::EndDisabled();
        
        // Check driver operation - run after notification shows
        static bool checkDriverQueued = false;
        static int checkDriverFrames = 0;
        
        if (s_IsCheckingDriver && s_IsDetecting)
        {
            if (!checkDriverQueued)
            {
                checkDriverQueued = true;
                checkDriverFrames = 0;
            }
            else
            {
                checkDriverFrames++;
            }
            
            if (checkDriverFrames >= 2)
            {
                Backend::OpenOCDInterface openocd;
                
                // First detect hardware to know which driver to check
                AddLog("[INFO] Detecting DMA card hardware...");
                Backend::CardInfo cardInfo = openocd.DetectDMACard();
                
                if (!cardInfo.detected)
                {
                    AddLog("[ERROR] No DMA card detected");
                    AddLog("[INFO] Please connect a DMA card (35T/75T/100T)");
                    s_CurrentProgress = "No card found.";
                }
                else
                {
                    // Update adapter type so buttons show correct driver
                    s_FPGAInfo.adapterType = cardInfo.adapterType;
                    
                    AddLog("[SUCCESS] Detected: " + cardInfo.cardTypeString + " DMA card");
                    AddLog("[INFO] VID:PID = " + cardInfo.vidPid);
                    
                    // Check driver for this specific card
                    std::string driverType = (cardInfo.adapterType == Backend::AdapterType::RS232) ? "RS232" : "CH347";
                    AddLog("[INFO] Checking " + driverType + " driver...");
                    
                    s_DriverInfo = (cardInfo.adapterType == Backend::AdapterType::RS232)
                        ? openocd.CheckRS232Driver()
                        : openocd.CheckCH347Driver();
                    
                    if (s_DriverInfo.installed)
                    {
                        AddLog("[SUCCESS] Correct " + driverType + " driver is installed");
                        AddLog("[INFO] Device: " + s_DriverInfo.deviceName);
                        AddLog("[INFO] Version: " + s_DriverInfo.version);
                        AddLog("[INFO] Provider: " + s_DriverInfo.provider);
                        s_CurrentProgress = "Driver check complete!";
                    }
                    else
                    {
                        AddLog("[WARNING] Correct " + driverType + " driver is NOT installed");
                        if (!s_DriverInfo.deviceName.empty())
                        {
                            AddLog("[INFO] Current: " + s_DriverInfo.deviceName);
                        }
                        AddLog("[INFO] Click 'Install " + driverType + " Driver' to proceed");
                        s_CurrentProgress = "Driver not found.";
                    }
                }
                
                s_IsCheckingDriver = false;
                s_IsDetecting = false;
                checkDriverQueued = false;
                checkDriverFrames = 0;
                s_CurrentProgress = "";
                s_DriverCheckCompleted = true;  // Mark driver check as completed
            }
        }
        
        ImGui::Spacing();
        
        // Install driver button - DISABLED if no card detected
        ImGui::BeginDisabled(s_IsInstallingDriver || !cardDetected);
        
        std::string installButtonText = "Install " + driverName + " Driver";
        if (!cardDetected)
        {
            installButtonText = "Install Driver (No Card Detected)";
        }
        else if (s_IsInstallingDriver)
        {
            static float installDotTimer = 0.0f;
            installDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(installDotTimer * 2.0f) % 4);
            installButtonText = "Installing";
            for (int i = 0; i < dotCount; i++) installButtonText += ".";
        }
        
        if (Theme::ButtonPrimary(installButtonText.c_str(), ImVec2(-1, 40)))
        {
            s_IsInstallingDriver = true;
            s_IsDetecting = true;
            s_LastOperation = "Install Driver";
            s_CurrentProgress = "Installing drivers...";
            AddLog("[INFO] Launching " + driverName + " driver installer...");
        }
        
        // Show tooltip when disabled due to no card
        if (!cardDetected && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Please detect DMA card first using 'Check Driver Status' or 'Detect FPGA & Read DNA'");
        }
        
        ImGui::EndDisabled();
        
        // Install driver operation - run after notification shows
        static bool installDriverQueued = false;
        static int installDriverFrames = 0;
        
        if (s_IsInstallingDriver && s_IsDetecting)
        {
            if (!installDriverQueued)
            {
                installDriverQueued = true;
                installDriverFrames = 0;
            }
            else
            {
                installDriverFrames++;
            }
            
            if (installDriverFrames >= 2)
            {
                Backend::OpenOCDInterface openocd;
                
                // Call appropriate installer based on adapter type
                bool success = false;
                if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
                {
                    success = openocd.InstallRS232Driver();
                }
                else if (s_FPGAInfo.adapterType == Backend::AdapterType::CH347)
                {
                    success = openocd.InstallCH347Driver();
                }
                else
                {
                    AddLog("[ERROR] Unknown adapter type - cannot install driver");
                    success = false;
                }
                
                if (success)
                {
                    AddLog("[SUCCESS] Driver installation completed!");
                    s_CurrentProgress = "Driver installed. Refreshing status...";
                }
                else
                {
                    if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
                    {
                        AddLog("[INFO] Please follow the instructions in your browser.");
                        AddLog("[INFO] Use Zadig to install WinUSB driver for RS232.");
                    }
                    else
                    {
                        AddLog("[INFO] Please follow the instructions in your browser.");
                        AddLog("[INFO] Download and install the CH347 driver.");
                        AddLog("[INFO] Restart DMATool after installation completes.");
                    }
                    s_CurrentProgress = "See browser for instructions. Refreshing status...";
                }
                
                // AUTO-REFRESH: Re-check driver status after installation
                Sleep(1000);  // Give Windows time to update
                AddLog("[INFO] Re-checking driver status...");
                
                // Check the correct driver type
                if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
                {
                    s_DriverInfo = openocd.CheckRS232Driver();
                }
                else
                {
                    s_DriverInfo = openocd.CheckCH347Driver();
                }
                
                if (s_DriverInfo.installed)
                {
                    AddLog("[SUCCESS] Driver status updated - Driver is installed");
                    AddLog("[INFO] Device: " + s_DriverInfo.deviceName);
                    AddLog("[INFO] Version: " + s_DriverInfo.version);
                }
                else
                {
                    AddLog("[WARNING] Driver status updated - Driver not detected");
                }
                
                s_IsInstallingDriver = false;
                s_IsDetecting = false;
                installDriverQueued = false;
                installDriverFrames = 0;
                s_CurrentProgress = "";
            }
        }
        
        ImGui::Spacing();
        
        // Uninstall driver button - DISABLED if no card detected
        ImGui::BeginDisabled(s_IsUninstallingDriver || !cardDetected);
        
        std::string uninstallButtonText = "Uninstall " + driverName + " Driver";
        if (!cardDetected)
        {
            uninstallButtonText = "Uninstall Driver (No Card Detected)";
        }
        else if (s_IsUninstallingDriver)
        {
            static float uninstallDotTimer = 0.0f;
            uninstallDotTimer += ImGui::GetIO().DeltaTime;
            int dotCount = ((int)(uninstallDotTimer * 2.0f) % 4);
            uninstallButtonText = "Uninstalling";
            for (int i = 0; i < dotCount; i++) uninstallButtonText += ".";
        }
        
        if (Theme::ButtonDestructive(uninstallButtonText.c_str(), ImVec2(-1, 40)))
        {
            s_IsUninstallingDriver = true;
            s_IsDetecting = true;
            s_LastOperation = "Uninstall Driver";
            s_CurrentProgress = "Uninstalling driver...";
            AddLog("[INFO] Uninstalling " + driverName + " driver...");
        }
        
        // Show tooltip when disabled due to no card
        if (!cardDetected && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Please detect DMA card first using 'Check Driver Status' or 'Detect FPGA & Read DNA'");
        }
        
        ImGui::EndDisabled();
        
        // Uninstall driver operation - run after notification shows
        static bool uninstallDriverQueued = false;
        static int uninstallDriverFrames = 0;
        
        if (s_IsUninstallingDriver && s_IsDetecting)
        {
            if (!uninstallDriverQueued)
            {
                uninstallDriverQueued = true;
                uninstallDriverFrames = 0;
            }
            else
            {
                uninstallDriverFrames++;
            }
            
            if (uninstallDriverFrames >= 2)
            {
                Backend::OpenOCDInterface openocd;
                
                // Call appropriate uninstaller based on adapter type
                bool success = false;
                if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
                {
                    success = openocd.UninstallRS232Driver();
                }
                else if (s_FPGAInfo.adapterType == Backend::AdapterType::CH347)
                {
                    success = openocd.UninstallCH347Driver();
                }
                else
                {
                    AddLog("[ERROR] Unknown adapter type - cannot uninstall driver");
                    success = false;
                }
                
                if (success)
                {
                    AddLog("[SUCCESS] Driver uninstall initiated.");
                    s_CurrentProgress = "Uninstall complete! Refreshing status...";
                }
                else
                {
                    AddLog("[ERROR] Failed to uninstall driver. Administrator privileges required.");
                    s_CurrentProgress = "Uninstall failed. Refreshing status...";
                }
                
                // AUTO-REFRESH: Re-check driver status after uninstallation
                Sleep(1000);  // Give Windows time to update
                AddLog("[INFO] Re-checking driver status...");
                
                // Check the correct driver type
                if (s_FPGAInfo.adapterType == Backend::AdapterType::RS232)
                {
                    s_DriverInfo = openocd.CheckRS232Driver();
                }
                else
                {
                    s_DriverInfo = openocd.CheckCH347Driver();
                }
                
                if (!s_DriverInfo.installed)
                {
                    AddLog("[SUCCESS] Driver status updated - Driver is uninstalled");
                }
                else
                {
                    AddLog("[WARNING] Driver status updated - Driver still detected");
                    AddLog("[INFO] Device: " + s_DriverInfo.deviceName);
                }
                
                s_IsUninstallingDriver = false;
                s_IsDetecting = false;
                uninstallDriverQueued = false;
                uninstallDriverFrames = 0;
                s_CurrentProgress = "";
            }
        }
        
        ImGui::EndChild();
    }
}
