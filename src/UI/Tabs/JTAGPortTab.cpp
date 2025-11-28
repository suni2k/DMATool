#include "JTAGPortTab.h"
#include "../Theme.h"
#include <imgui.h>

// Integration References:
// - CH347 USB-JTAG adapter: https://github.com/WCHSoftGroup/ch347
// - PCILeech JTAG operations for FPGA programming
// Planned: JTAG chain detection, device identification, boundary scan

namespace DMATool::UI::Tabs
{
    void JTAGPortTab::Render()
    {
        ImGui::BeginChild("JTAGPortContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::Spacing();
        
        // Two column layout
        ImGui::Columns(2, "JTAGColumns", true);
        
        RenderConnectionPanel();
        
        ImGui::NextColumn();
        
        RenderControlPanel();
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Calculate the exact column offset to match the panels above
        // When columns are active, they add an offset on the left equal to the column border
        float columnsOffsetX = ImGui::GetStyle().ItemSpacing.x * 0.5f + ImGui::GetStyle().FramePadding.x;
        
        // Apply the same offset to our bottom panel
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnsOffsetX);
        
        // Reduce the width to account for both left and right offsets
        float panelWidth = ImGui::GetContentRegionAvail().x - columnsOffsetX;
        
        // Calculate height to leave room for bottom spacing
        float bottomSpacing = ImGui::GetStyle().ItemSpacing.y * 2; // Space for two Spacing() calls
        float panelHeight = ImGui::GetContentRegionAvail().y - bottomSpacing;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("StatusPanel", ImVec2(panelWidth, panelHeight), true);
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
        ImGui::TextColored(Colors::Destructive, "Disconnected");
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Devices Found:");
        ImGui::PopStyleColor();
        ImGui::Text("0");
        
        ImGui::NextColumn();
        
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Last Operation:");
        ImGui::PopStyleColor();
        ImGui::Text("None");
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Log output
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Waiting for operations...");
        ImGui::Text("[INFO] JTAG tab initialized");
        ImGui::Text("[INFO] Ready for CH347 adapter connection");
        ImGui::PopStyleColor();
        
        ImGui::EndChild();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::EndChild();
    }

    void JTAGPortTab::RenderConnectionPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("ConnectionPanel", ImVec2(0, 340), true);
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Text("Connection Settings");
        ImGui::Spacing();
        
        // Device selection
        const char* devices[] = { "No Device", "CH347 USB-JTAG", "Custom JTAG Adapter" };
        static int current_device = 0;
        ImGui::Text("JTAG Adapter:");
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##device", &current_device, devices, IM_ARRAYSIZE(devices));
        
        ImGui::Spacing();
        
        // Speed selection
        const char* speeds[] = { "100 kHz", "1 MHz", "6 MHz", "12 MHz" };
        static int current_speed = 2;
        ImGui::Text("TCK Frequency:");
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##speed", &current_speed, speeds, IM_ARRAYSIZE(speeds));
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Connection buttons
        if (Theme::ButtonPrimary("Connect", ImVec2(-1, 40)))
        {
            // Placeholder: Would initialize CH347 connection
        }
        
        ImGui::Spacing();
        
        if (Theme::ButtonDestructive("Disconnect", ImVec2(-1, 40)))
        {
            // Placeholder: Would close connection
        }
        
        ImGui::Spacing();
        
        ImGui::EndChild();
    }

    void JTAGPortTab::RenderControlPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::BeginChild("ControlPanel", ImVec2(0, 340), true);
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Text("JTAG Operations");
        ImGui::Spacing();
        
        // Chain detection
        if (Theme::ButtonSecondary("Detect JTAG Chain", ImVec2(-1, 40)))
        {
            // Placeholder: Would scan JTAG chain
        }
        
        ImGui::Spacing();
        
        // Device ID read
        if (Theme::ButtonSecondary("Read Device ID", ImVec2(-1, 40)))
        {
            // Placeholder: Would read IDCODE
        }
        
        ImGui::Spacing();
        
        // Boundary scan
        if (Theme::ButtonSecondary("Run Boundary Scan", ImVec2(-1, 40)))
        {
            // Placeholder: Would perform boundary scan
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Manual control section
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
        ImGui::Text("Manual Control");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        static bool tms_state = false;
        ImGui::Checkbox("TMS", &tms_state);
        
        static bool tdi_state = false;
        ImGui::Checkbox("TDI", &tdi_state);
        
        static bool tdo_state = false;
        ImGui::BeginDisabled();
        ImGui::Checkbox("TDO (Read)", &tdo_state);
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        ImGui::EndChild();
    }
}
