#include "JTAGFlashTab.h"
#include "../Theme.h"
#include "../../Backend/FlashInterface.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <Windows.h>
#include <commdlg.h>

// Integration References:
// - PCILeech-FPGA firmware: https://github.com/ufrisk/pcileech-fpga
// - OpenOCD for FPGA flash programming via JTAG
// - CH347 JTAG adapter support

namespace DMATool::UI::Tabs
{
    // Static member initialization
    Backend::FlashDeviceInfo JTAGFlashTab::s_FlashInfo;
    Backend::FPGAChipModel JTAGFlashTab::s_SelectedChipModel = Backend::FPGAChipModel::Unknown;  // Start with no selection
    bool JTAGFlashTab::s_AutoDetectChip = true;
    std::string JTAGFlashTab::s_FirmwarePath = "";
    std::vector<std::string> JTAGFlashTab::s_LogMessages;
    bool JTAGFlashTab::s_IsDetecting = false;
    bool JTAGFlashTab::s_IsFlashing = false;
    bool JTAGFlashTab::s_IsVerifying = false;
    std::string JTAGFlashTab::s_CurrentOperation = "None";
    std::string JTAGFlashTab::s_CurrentProgress = "";
    float JTAGFlashTab::s_ProgressPercent = 0.0f;
    JTAGFlashTab::ProgressState JTAGFlashTab::s_ProgressState = JTAGFlashTab::ProgressState::Ready;  // NEW
    
    // SINGLE INSTANCE - Reuse across all operations
    static Backend::FlashInterface* s_FlashInterface = nullptr;
    
    // Prevent re-entry of async operations
    static bool s_FlashThreadRunning = false;
    static bool s_VerifyThreadRunning = false;

    void JTAGFlashTab::AddLog(const std::string& message)
    {
        s_LogMessages.push_back(message);
        std::cout << message << std::endl;
        
        // Keep only last 100 messages
        if (s_LogMessages.size() > 100)
        {
            s_LogMessages.erase(s_LogMessages.begin());
        }
    }

    void JTAGFlashTab::ClearLog()
    {
        s_LogMessages.clear();
    }

    void JTAGFlashTab::UpdateProgress(float percent, const std::string& message)
    {
        s_ProgressPercent = percent;
        s_CurrentProgress = message;
        AddLog("[PROGRESS] " + message);
        
        // Auto-update state based on progress
        if (percent > 0 && percent < 100)
        {
            s_ProgressState = ProgressState::InProgress;  // Yellow for in-progress
        }
    }

    void JTAGFlashTab::SetProgressState(ProgressState state)
    {
        s_ProgressState = state;
    }

    void JTAGFlashTab::Render()
    {
        static bool s_FirstRender = true;
        static int s_FramesSinceOperation = 0;
        
        if (s_FirstRender)
        {
            s_FirstRender = false;
            AddLog("[INFO] Flash programming interface initialized");
            AddLog("[INFO] Supports XC7A35T (RS232), XC7A35T/75T/100T (CH347)");
            AddLog("[INFO] Ready for flash operations");
        }

        // Track frames for async operations
        bool isAnyOperationActive = s_IsDetecting || s_IsFlashing || s_IsVerifying || s_FlashThreadRunning || s_VerifyThreadRunning;
        static bool wasOperationActive = false;

        if (isAnyOperationActive && !wasOperationActive)
        {
            s_FramesSinceOperation = 0;
        }

        if (isAnyOperationActive)
        {
            s_FramesSinceOperation++;
        }

        wasOperationActive = isAnyOperationActive;

        ImGui::BeginChild("JTAGFlashContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::Spacing();
        
        // RESIZABLE VERTICAL LAYOUT: Top panels vs Bottom panel
        static float topPanelHeightRatio = 0.52f;
        
        float availableHeight = ImGui::GetContentRegionAvail().y;
        float topHeight = availableHeight * topPanelHeightRatio;
        
        // Top section
        ImGui::BeginChild("TopSection", ImVec2(0, topHeight), false, ImGuiWindowFlags_NoScrollbar);
        
        // Two column layout for top panels
        ImGui::Columns(2, "FlashColumns", true);
        
        RenderFlashInfoPanel();
        
        ImGui::NextColumn();
        
        RenderFlashOperationsPanel();
        
        ImGui::Columns(1);
        
        ImGui::EndChild();
        
        // HORIZONTAL RESIZE HANDLE
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
        
        // Bottom section: Log & Progress panel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("LogPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Operation Log & Progress");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // ENHANCED PROGRESS BAR with colored states and centered text
        char progressLabel[256];
        bool isOperationActive = s_IsFlashing || s_IsVerifying || s_FlashThreadRunning || s_VerifyThreadRunning;
        
        // Only show progress bar if flash/verify operations are active
        bool showProgressBar = isOperationActive;
        
        if (showProgressBar)
        {
            if (isOperationActive)
            {
                snprintf(progressLabel, sizeof(progressLabel), "%.0f%% - %s", 
                    s_ProgressPercent, s_CurrentProgress.c_str());
            }
            else
            {
                snprintf(progressLabel, sizeof(progressLabel), "%s", s_CurrentProgress.empty() ? "Ready" : s_CurrentProgress.c_str());
            }
            
            // Select color based on state
            ImVec4 progressBarColor;
            
            switch (s_ProgressState)
            {
            case ProgressState::Success:
                // Darker green for success
                progressBarColor = ImVec4(0.15f, 0.65f, 0.15f, 1.0f);     // Darker green bar
                break;
                
            case ProgressState::Failure:
                // Red for failure
                progressBarColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);     // Bright red bar
                break;
                
            case ProgressState::InProgress:
                // Yellow for in-progress
                progressBarColor = ImVec4(0.95f, 0.75f, 0.1f, 1.0f);   // Bright yellow/gold bar
                break;
                
            case ProgressState::Ready:
            default:
                // Default gray/white for ready state
                progressBarColor = ImVec4(0.4f, 0.4f, 0.4f, 0.6f);     // Subtle gray bar
                break;
            }
            
            // Custom progress bar with colored background
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));  // Dark background
            
            float progressFraction = isOperationActive ? (s_ProgressPercent / 100.0f) : 
                                      (s_ProgressState == ProgressState::Success || s_ProgressState == ProgressState::Failure ? 1.0f : 0.0f);
            
            ImGui::ProgressBar(progressFraction, ImVec2(-1, 35), "");  // Empty overlay text
            
            ImGui::PopStyleColor(2);
            
            // Render centered text overlay on progress bar - WHITE TEXT ONLY
            ImVec2 progressBarMin = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
            ImVec2 progressBarMax = ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y);
            ImVec2 progressBarCenter = ImVec2((progressBarMin.x + progressBarMax.x) * 0.5f, 
                                              (progressBarMin.y + progressBarMax.y) * 0.5f);
            
            ImVec2 textSize = ImGui::CalcTextSize(progressLabel);
            ImVec2 textPos = ImVec2(progressBarCenter.x - textSize.x * 0.5f, 
                                    progressBarCenter.y - textSize.y * 0.5f);
            
            // Draw centered white text (no shadow)
            ImGui::GetWindowDrawList()->AddText(
                textPos,
                IM_COL32(255, 255, 255, 255),  // Pure white text
                progressLabel
            );
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        
        // Log output with scrolling
        ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        if (s_LogMessages.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("Waiting for operations...");
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
                else if (msg.find("[PROGRESS]") != std::string::npos)
                    ImGui::TextColored(Colors::Warning, msg.c_str());
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
        
        ImGui::EndChild();  // End LogPanel
        
        // Perform detection after UI renders (similar to JTAG tab pattern)
        if (s_IsDetecting && s_FramesSinceOperation >= 2)
        {
            // Create FlashInterface only once
            if (!s_FlashInterface)
            {
                s_FlashInterface = new Backend::FlashInterface();
            }
            
            // No progress bar for detection, just log messages
            
            s_FlashInfo = s_FlashInterface->DetectFlashDevice([](uint64_t current, uint64_t total, const std::string& msg) {
                // Just log, don't update progress bar for detection
                AddLog("[PROGRESS] " + msg);
            });
            
            if (s_FlashInfo.detected)
            {
                AddLog("[SUCCESS] FPGA detected!");
                AddLog("[INFO] Chip: " + s_FlashInfo.fpgaModelString);
                AddLog("[INFO] Manufacturer: " + s_FlashInfo.manufacturer);
                AddLog("[INFO] Family: " + s_FlashInfo.model);
                
                // Auto-select chip model if enabled
                if (s_AutoDetectChip)
                {
                    s_SelectedChipModel = s_FlashInfo.fpgaModel;
                    AddLog("[SUCCESS] Auto-selected: " + Backend::FlashInterface::ChipModelToString(s_SelectedChipModel));
                }
                
                // Auto-populate the flash device info display
                AddLog("[INFO] Flash ready for programming");
            }
            else
            {
                AddLog("[ERROR] Failed to detect flash device");
                AddLog("[INFO] Please check:");
                AddLog("[INFO]   1. CH347 adapter is connected");
                AddLog("[INFO]   2. JTAG cable is properly connected");
                AddLog("[INFO]   3. Target device is powered on");
            }
            
            s_IsDetecting = false;
            s_FramesSinceOperation = 0;
        }
        
        // Perform flashing after UI renders
        if (s_IsFlashing && s_FramesSinceOperation >= 2)
        {
            // CRITICAL FIX: Prevent re-entry! Set flag to false IMMEDIATELY
            s_IsFlashing = false;
            
            // Don't launch if already running
            if (s_FlashThreadRunning)
            {
                AddLog("[WARNING] Flash operation already in progress!");
                s_FramesSinceOperation = 0;
            }
            else
            {
                s_FlashThreadRunning = true;
                
                // Ensure FlashInterface exists
                if (!s_FlashInterface)
                {
                    s_FlashInterface = new Backend::FlashInterface();
                }
                
                // Launch flash operation in background thread to avoid UI freeze
                std::thread flashThread([&]() {
                    SetProgressState(ProgressState::InProgress);  // YELLOW
                    UpdateProgress(0, "Preparing to flash firmware...");
                    AddLog("[INFO] Starting flash programming...");
                    AddLog("[INFO] Firmware: " + s_FirmwarePath);
                    AddLog("[INFO] Target chip: " + Backend::FlashInterface::ChipModelToString(s_SelectedChipModel));
                    
                    auto result = s_FlashInterface->ProgramFirmware(
                        s_FirmwarePath,
                        s_SelectedChipModel,
                        true,  // Verify after
                        false, // No backup for now
                        [](uint64_t current, uint64_t total, const std::string& msg) {
                            float percent = total > 0 ? (float)current / (float)total * 100.0f : 0.0f;
                            UpdateProgress(percent, msg);
                        }
                    );
                    
                    if (result.success)
                    {
                        AddLog("[SUCCESS] ===============================================");
                        AddLog("[SUCCESS] Flash programming and verification completed successfully!");
                        AddLog("[SUCCESS] ===============================================");
                        AddLog("[SUCCESS] Bytes written: " + std::to_string(result.bytesProcessed));
                        AddLog("[SUCCESS] Duration: " + std::to_string(result.durationSeconds) + " seconds");
                        UpdateProgress(100, "Complete!");
                        SetProgressState(ProgressState::Success);  // GREEN
                    }
                    else
                    {
                        AddLog("[ERROR] Flash programming failed!");
                        AddLog("[ERROR] " + result.message);
                        UpdateProgress(0, "Flash failed");
                        SetProgressState(ProgressState::Failure);  // RED
                    }
                    
                    s_FlashThreadRunning = false;
                });
                
                flashThread.detach();  // Let thread run independently
                s_FramesSinceOperation = 0;
            }
        }
        
        // Perform verification after UI renders
        if (s_IsVerifying && s_FramesSinceOperation >= 2)
        {
            // CRITICAL FIX: Prevent re-entry! Set flag to false IMMEDIATELY
            s_IsVerifying = false;
            
            // Don't launch if already running
            if (s_VerifyThreadRunning)
            {
                AddLog("[WARNING] Verification operation already in progress!");
                s_FramesSinceOperation = 0;
            }
            else
            {
                s_VerifyThreadRunning = true;
                
                // Ensure FlashInterface exists
                if (!s_FlashInterface)
                {
                    s_FlashInterface = new Backend::FlashInterface();
                }
                
                // Launch verify operation in background thread
                std::thread verifyThread([&]() {
                    SetProgressState(ProgressState::InProgress);  // YELLOW
                    UpdateProgress(0, "Preparing to verify firmware...");
                    
                    AddLog("[INFO] ========================================");
                    AddLog("[INFO] Flash Verification");
                    AddLog("[INFO] ========================================");
                    AddLog("[INFO] Original file: " + s_FirmwarePath);
                    AddLog("[INFO] Target chip: " + Backend::FlashInterface::ChipModelToString(s_SelectedChipModel));
                    
                    auto result = s_FlashInterface->VerifyFirmware(
                        s_FirmwarePath,
                        s_SelectedChipModel,
                        [](uint64_t current, uint64_t total, const std::string& msg) {
                            float percent = total > 0 ? (float)current / (float)total * 100.0f : 0.0f;
                            UpdateProgress(percent, msg);
                        }
                    );
                    
                    if (result.success)
                    {
                        AddLog("[SUCCESS] ============================================");
                        AddLog("[SUCCESS] VERIFICATION PASSED!");
                        AddLog("[SUCCESS] ============================================");
                        
                        // Extract SHA256 hashes from message
                        std::string msg = result.message;
                        size_t origPos = msg.find("Original SHA256: ");
                        size_t readPos = msg.find("Readback SHA256: ");
                        size_t speedPos = msg.find("Speed: ");
                        
                        if (origPos != std::string::npos && readPos != std::string::npos)
                        {
                            std::string origHash = msg.substr(origPos + 17, 64);
                            std::string readHash = msg.substr(readPos + 17, 64);
                            AddLog("[SUCCESS] Original SHA256:  " + origHash);
                            AddLog("[SUCCESS] Readback SHA256:  " + readHash);
                        }
                        
                        AddLog("[SUCCESS] Bytes verified: " + std::to_string(result.bytesProcessed));
                        AddLog("[SUCCESS] Duration: " + std::to_string(result.durationSeconds) + " seconds");
                        
                        // Extract and display speed if available
                        if (speedPos != std::string::npos)
                        {
                            std::string speed = msg.substr(speedPos + 7);
                            if (speed.find('\n') != std::string::npos)
                                speed = speed.substr(0, speed.find('\n'));
                            AddLog("[SUCCESS] Speed: " + speed);
                        }
                        
                        UpdateProgress(100, "Complete!");
                        SetProgressState(ProgressState::Success);  // GREEN
                    }
                    else
                    {
                        AddLog("[ERROR] ============================================");
                        AddLog("[ERROR] VERIFICATION FAILED!");
                        AddLog("[ERROR] ============================================");
                        
                        // Extract SHA256 hashes from message
                        std::string msg = result.message;
                        size_t origPos = msg.find("Original SHA256: ");
                        size_t readPos = msg.find("Readback SHA256: ");
                        
                        if (origPos != std::string::npos && readPos != std::string::npos)
                        {
                            std::string origHash = msg.substr(origPos + 17, 64);
                            std::string readHash = msg.substr(readPos + 17, 64);
                            AddLog("[ERROR] Original SHA256:  " + origHash);
                            AddLog("[ERROR] Readback SHA256:  " + readHash);
                        }
                        
                        AddLog("[WARNING] Troubleshooting:");
                        AddLog("[WARNING]   1. Try reflashing with slower clock speed");
                        AddLog("[WARNING]   2. Check JTAG connections");
                        AddLog("[WARNING]   3. Verify hardware is working correctly");
                        
                        UpdateProgress(0, "Verification failed!");
                        SetProgressState(ProgressState::Failure);  // RED
                    }
                    
                    s_VerifyThreadRunning = false;
                });
                
                verifyThread.detach();
                s_FramesSinceOperation = 0;
            }
        }
        
        ImGui::EndChild();  // End JTAGFlashContent
    }

    void JTAGFlashTab::RenderFlashInfoPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 10));
        ImGui::BeginChild("FlashInfoPanel", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        
        ImGui::Text("Flash Device & Chip Selection");
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));
        
        // FPGA Chip Selection Section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("FPGA Chip Model");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));
        
        // Auto-detect toggle
        ImGui::Checkbox("Auto-detect chip model", &s_AutoDetectChip);
        ImGui::Dummy(ImVec2(0, 2));
        
        // Chip model dropdown (disabled if auto-detect is on AND device is detected)
        ImGui::BeginDisabled(s_AutoDetectChip && s_FlashInfo.detected);
        
        auto supportedChips = Backend::FlashInterface::GetSupportedChipModels();
        
        // Find the display name for the currently selected chip
        std::string currentChipDisplay = "--- Select DMA Chipset ---";
        
        // Only show selected chip if it's not Unknown
        if (s_SelectedChipModel != Backend::FPGAChipModel::Unknown)
        {
            for (const auto& [model, displayName] : supportedChips)
            {
                if (model == s_SelectedChipModel)
                {
                    currentChipDisplay = displayName;
                    break;
                }
            }
        }
        
        // Chip model selection dropdown with improved spacing
        // HeightLarge flag makes dropdown taller to prevent clipping
        if (ImGui::BeginCombo("##chipmodel", currentChipDisplay.c_str(), ImGuiComboFlags_HeightLarge))
        {
            // Add padding to the popup window itself for breathing room
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 12));  // Top/bottom padding for popup
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 8));    // Spacing between items
            
            // Add spacing at the top to prevent first item from being flush with border
            ImGui::Dummy(ImVec2(0, 4));
            
            // Render each supported chip model
            for (const auto& [model, displayName] : supportedChips)
            {
                bool isSelected = (s_SelectedChipModel == model);
                if (ImGui::Selectable(displayName.c_str(), isSelected, 0, ImVec2(0, 0)))
                {
                    s_SelectedChipModel = model;
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            
            // Add spacing at the bottom to prevent last item from being flush with border
            ImGui::Dummy(ImVec2(0, 4));
            
            ImGui::PopStyleVar(2);
            ImGui::EndCombo();
        }
        
        ImGui::EndDisabled();
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));
        
        // Flash Device Info Section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Success);
        ImGui::Text("Flash Device Info");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 1));
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Manufacturer:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FlashInfo.detected)
            ImGui::Text(s_FlashInfo.manufacturer.c_str());
        else
            ImGui::Text("Not Detected");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Model:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FlashInfo.detected)
            ImGui::Text(s_FlashInfo.model.c_str());
        else
            ImGui::Text("---");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Detected FPGA:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        if (s_FlashInfo.detected && !s_FlashInfo.fpgaModelString.empty())
            ImGui::TextColored(Colors::Success, s_FlashInfo.fpgaModelString.c_str());
        else
            ImGui::Text("---");
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));
        
        // Detect button
        ImGui::BeginDisabled(s_IsDetecting || s_IsFlashing || s_IsVerifying || s_FlashThreadRunning || s_VerifyThreadRunning);
        
        std::string detectButtonText = s_IsDetecting ? "Detecting..." : "Detect Flash Device";
        
        if (Theme::ButtonPrimary(detectButtonText.c_str(), ImVec2(-1, 36)))
        {
            s_IsDetecting = true;
            ClearLog();
            s_CurrentOperation = "Flash Detection";
            // No progress state for detection
            AddLog("[INFO] Starting flash device detection...");
        }
        
        ImGui::EndDisabled();
        
        ImGui::EndChild();
    }

    void JTAGFlashTab::RenderFlashOperationsPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 10));
        ImGui::BeginChild("FlashOperationsPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Flash Operations");
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 1));
        
        // Firmware File Selection
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Firmware File");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        // File path display
        static char firmwarePathBuf[512] = "";
        if (!s_FirmwarePath.empty())
        {
            strncpy_s(firmwarePathBuf, s_FirmwarePath.c_str(), sizeof(firmwarePathBuf) - 1);
        }
        else
        {
            strcpy_s(firmwarePathBuf, "No file selected");
        }
        
        ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(-110);  // Leave space for Browse button
        ImGui::InputText("##firmware", firmwarePathBuf, IM_ARRAYSIZE(firmwarePathBuf));
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        if (Theme::ButtonSecondary("Browse...", ImVec2(100, 0)))
        {
            // Open file dialog for .bin files
            OPENFILENAMEA ofn = { 0 };
            char szFile[260] = { 0 };
            
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "Firmware Files (*.bin)\0*.BIN\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = ".\\dmafiles\\CH347FPGATool";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            if (GetOpenFileNameA(&ofn) == TRUE)
            {
                s_FirmwarePath = szFile;
                AddLog("[INFO] Selected firmware: " + s_FirmwarePath);
            }
        }
        
        // File info
        if (!s_FirmwarePath.empty() && std::filesystem::exists(s_FirmwarePath))
        {
            auto fileSize = std::filesystem::file_size(s_FirmwarePath);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
            ImGui::Text("File Size: %.2f MB", fileSize / 1024.0 / 1024.0);
            ImGui::PopStyleColor();
        }
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 1));
        
        // Flash Programming Section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Warning);
        ImGui::Text("Programming Options");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        // Verify after programming checkbox
        static bool verifyAfterWrite = true;
        ImGui::Checkbox("Verify after programming", &verifyAfterWrite);
        
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 1));
        
        // Reduced spacing to align buttons without scroll bar
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Program Firmware button
        ImGui::BeginDisabled(s_IsFlashing || s_IsDetecting || s_FirmwarePath.empty() || s_FlashThreadRunning || s_VerifyThreadRunning);
        
        std::string flashButtonText = (s_IsFlashing || s_FlashThreadRunning) ? "Flashing..." : "Program Firmware";
        
        if (Theme::ButtonPrimary(flashButtonText.c_str(), ImVec2(-1, 40)))
        {
            s_IsFlashing = true;
            s_CurrentOperation = "Flash Programming";
            s_ProgressState = ProgressState::InProgress;  // YELLOW when starting
            AddLog("[INFO] User initiated flash programming");
        }
        
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        // Verify Firmware button
        ImGui::BeginDisabled(s_IsFlashing || s_IsDetecting || s_IsVerifying || s_FirmwarePath.empty() || !s_FlashInfo.detected || s_FlashThreadRunning || s_VerifyThreadRunning);
        
        std::string verifyButtonText = (s_IsVerifying || s_VerifyThreadRunning) ? "Verifying..." : "Verify Firmware";
        
        if (Theme::ButtonSecondary(verifyButtonText.c_str(), ImVec2(-1, 35)))
        {
            s_IsVerifying = true;
            s_CurrentOperation = "Firmware Verification";
            s_ProgressState = ProgressState::InProgress;  // YELLOW when starting
            AddLog("[INFO] User initiated firmware verification");
        }
        
        ImGui::EndDisabled();
        
        ImGui::EndChild();
    }
}
