#include "JTAGFlashTab.h"
#include "../Theme.h"
#include <imgui.h>

// Integration References:
// - PCILeech-FPGA firmware: https://github.com/ufrisk/pcileech-fpga
// - FPGA bitstream programming via JTAG
// Planned: Flash chip detection, firmware upload, verification

namespace DMATool::UI::Tabs
{
    void JTAGFlashTab::Render()
    {
        ImGui::BeginChild("JTAGFlashContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::Spacing();
        
        // RESIZABLE VERTICAL LAYOUT: Top panels vs Bottom panel
        static float topPanelHeightRatio = 0.52f;  // Default 52% for top panels
        
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
        
        // Bottom section: Operation Progress panel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("ProgressPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Text("Operation Progress");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Progress bar
        static float progress = 0.0f;
        ImGui::ProgressBar(progress, ImVec2(-1, 30), "Ready");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Status columns
        ImGui::Columns(4, "ProgressColumns", false);
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Status:");
        ImGui::PopStyleColor();
        ImGui::Text("Idle");
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Bytes Transferred:");
        ImGui::PopStyleColor();
        ImGui::Text("0 / 0");
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Speed:");
        ImGui::PopStyleColor();
        ImGui::Text("--- KB/s");
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Time Remaining:");
        ImGui::PopStyleColor();
        ImGui::Text("---");
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Log output
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("[INFO] Flash programming interface initialized");
        ImGui::Text("[INFO] Supports PCILeech FPGA firmware formats");
        ImGui::Text("[INFO] Ready for flash operations");
        ImGui::PopStyleColor();
        
        ImGui::EndChild();
        
        ImGui::EndChild();  // End JTAGFlashContent
    }

    void JTAGFlashTab::RenderFlashInfoPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("FlashInfoPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Text("Flash Device Information");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Flash info display
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Manufacturer:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        ImGui::Text("Not Detected");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Device ID:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        ImGui::Text("---");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Capacity:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        ImGui::Text("---");
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Sector Size:");
        ImGui::PopStyleColor();
        ImGui::SameLine(150);
        ImGui::Text("---");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Detection button
        if (Theme::ButtonPrimary("Detect Flash Device", ImVec2(-1, 40)))
        {
            // Placeholder: Would detect flash chip via JTAG
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Firmware file selection
        ImGui::Text("Firmware File:");
        ImGui::Spacing();
        
        static char firmware_path[256] = "No file selected";
        ImGui::BeginDisabled();
        ImGui::InputText("##firmware", firmware_path, IM_ARRAYSIZE(firmware_path));
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        if (Theme::ButtonSecondary("Browse...", ImVec2(100, 0)))
        {
            // Placeholder: Would open file dialog for .bin/.bit files
        }
        
        ImGui::Spacing();
        
        // File info
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("File Size: ---");
        ImGui::Text("File Type: ---");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        ImGui::EndChild();
    }

    void JTAGFlashTab::RenderFlashOperationsPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("FlashOperationsPanel", ImVec2(0, 0), true);
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Text("Flash Operations");
        ImGui::Spacing();
        
        // Read operations
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
        ImGui::Text("Read Operations");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (Theme::ButtonSecondary("Read Flash ID", ImVec2(-1, 35)))
        {
            // Placeholder: Read flash identification
        }
        
        ImGui::Spacing();
        
        if (Theme::ButtonSecondary("Read Full Flash", ImVec2(-1, 35)))
        {
            // Placeholder: Dump entire flash contents
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Write operations
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Warning);
        ImGui::Text("Write Operations");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (Theme::ButtonPrimary("Program Firmware", ImVec2(-1, 35)))
        {
            // Placeholder: Program selected firmware to flash
        }
        
        ImGui::Spacing();
        
        if (Theme::ButtonSecondary("Verify Firmware", ImVec2(-1, 35)))
        {
            // Placeholder: Verify programmed data
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Dangerous operations
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Destructive);
        ImGui::Text("Destructive Operations");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (Theme::ButtonDestructive("Erase Chip", ImVec2(-1, 35)))
        {
            // Placeholder: Chip erase operation
        }
        
        ImGui::Spacing();
        
        // Options
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        static bool verify_after_write = true;
        ImGui::Checkbox("Verify after programming", &verify_after_write);
        
        static bool backup_before_write = true;
        ImGui::Checkbox("Backup before programming", &backup_before_write);
        
        ImGui::Spacing();
        
        ImGui::EndChild();
    }
}
