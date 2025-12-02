# Benchmark DMA Tab - Final Fixes

## Issue 1: Custom Test - 3-Column Scale Layout

**Location:** `src/UI/Tabs/DataPortTab.cpp` - `RenderResultsPanel()` function

**Current:** 2 columns (Performance | Combined Scale)
**Target:** 3 columns for Custom Test (Performance | RPS Scale | MB/s Scale)

**Fix:**
```cpp
// Around line 430 - Change column logic
if (s_CurrentTestType == 4)  // Custom Test
{
    ImGui::Columns(3, "CustomPerfScaleColumns", false);
}
else
{
    ImGui::Columns(2, "PerfScaleColumns", false);
}

// ... Performance metrics column stays same ...

ImGui::NextColumn();

// For Custom Test - split scales into 2 separate columns
if (s_CurrentTestType == 4)
{
    // Column 2: RPS Scale
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
    ImGui::Text("RPS Scale");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::SetWindowFontScale(0.85f);
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "7500+ ELITE");
    ImGui::TextColored(Colors::Success, "6500+ AMAZING");
    ImGui::TextColored(Colors::Info, "5200+ GOOD");
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::NextColumn();
    
    // Column 3: MB/s Scale  
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::Info);
    ImGui::Text("MB/s Scale");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::SetWindowFontScale(0.85f);
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "220+ ELITE");
    ImGui::TextColored(Colors::Success, "200+ AMAZING");
    ImGui::TextColored(Colors::Info, "150+ GOOD");
    ImGui::SetWindowFontScale(1.0f);
}
else
{
    // Other tests - single scale column
    // ... existing scale code ...
}
```

---

## Issue 2: Custom Test Configuration Not Applied

**Location:** `src/UI/Tabs/DataPortTab.cpp` - Button click handler

**Problem:** Custom duration/read size values are stored in `s_Config` but then overridden by button handler

**Fix:**
```cpp
// Around line 265 - DON'T override custom config
if (Theme::ButtonPrimary(buttonText.c_str(), ImVec2(-1, 45)))
{
    s_Config.testType = (Backend::BenchmarkTestType)currentTestType;
    s_Config.memoryAddress = "0x1000";
    
    // Only set defaults for non-custom tests
    if (currentTestType != 4)
    {
        s_Config.durationSeconds = 10;
        
        switch (currentTestType)
        {
        case 0: // Quick Speed Test
            s_Config.testSizeMB = 16;
            s_Config.durationSeconds = 10;
            break;
        case 1: // Throughput Test
            s_Config.testSizeMB = 1024;
            s_Config.durationSeconds = 60;
            break;
        case 2: // Mixed Test
            s_Config.testSizeMB = 512;
            s_Config.durationSeconds = 30;
            break;
        }
    }
    // Custom Test (4) uses values already set in s_Config from sliders
    
    StartQuickTest();
}
```

---

## Issue 3: Console Log Bottom Border

**Location:** `src/UI/Tabs/DataPortTab.cpp` - `Render()` function

**Problem:** Console wrapper has left/right margins but bottom border still touches edge

**Fix:**
```cpp
// Around line 110 - Add bottom margin to console wrapper
ImGui::BeginChild("ConsoleLogWrapper", ImVec2(consoleWidth, -ImGui::GetStyle().ItemSpacing.y), false, ImGuiWindowFlags_NoScrollbar);
//                                                          ^^^ Negative value creates bottom margin
RenderConsoleLog(0);
ImGui::EndChild();

ImGui::Spacing();  // This line already exists
```

**Alternative:** Add explicit bottom spacing:
```cpp
ImGui::BeginChild("ConsoleLogWrapper", ImVec2(consoleWidth, ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y), false, ImGuiWindowFlags_NoScrollbar);
```

---

## Testing Checklist

- [ ] Custom Test shows 3 columns: Performance | RPS Scale | MB/s Scale
- [ ] Custom Test duration slider (5s ? 30s) actually runs for configured duration
- [ ] Custom Test read size dropdown actually affects test behavior
- [ ] Console log has proper bottom margin (border not touching window edge)
- [ ] Other tests (Quick/Throughput) still show 2-column layout correctly
- [ ] All other UI elements remain unchanged

---

## Files to Modify

1. `src/UI/Tabs/DataPortTab.cpp` - All 3 fixes in this file
   - `Render()` - Console bottom margin
   - `RenderTestControlsPanel()` - Button click handler
   - `RenderResultsPanel()` - 3-column layout

No header file changes needed.
