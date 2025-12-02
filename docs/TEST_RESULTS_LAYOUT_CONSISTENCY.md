# Test Results Layout Consistency Fix

## Issue
Min/Max Latency fields only appeared in the Test Results panel **after** a test completed, causing the layout to shift and look inconsistent.

## Problem Description
**Before:**
- When no test has run: Fields show RPS, Total Reads, AVG Latency, Duration (4 metrics)
- After test completes: Min/Max Latency suddenly appear (6 metrics)
- Layout shifts vertically, which feels jarring

**User Experience Issue:**
When selecting different test types from the dropdown, the results panel should show a consistent structure with all fields present (showing "---" for empty values).

## Solution
Always display Min/Max Latency fields in the Test Results panel, showing "---" when no data is available yet.

### Code Changes

**Before (Conditional Display):**
```cpp
// Show min/max latency for ALL tests
if (results.minLatencyUs > 0 || results.maxLatencyUs > 0)
{
    // Only show these fields if we have data
    ImGui::Text("Min Latency:");
    // ...
    ImGui::Text("Max Latency:");
    // ...
}
```

**After (Always Display):**
```cpp
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
    ImGui::Text("---");  // Show placeholder when no data
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
    ImGui::Text("---");  // Show placeholder when no data
}
```

## Visual Improvements

### Test Results Panel Structure (Now Consistent)

**Performance Column:**
1. RPS: ---
2. Total Reads: ---
3. AVG. Latency: ---
4. Throughput: --- (Throughput Test only)
5. Duration: ---
6. **Min Latency: ---** ? Always visible
7. **Max Latency: ---** ? Always visible

**Benefits:**
- ? Consistent layout regardless of test state
- ? No vertical shifting when test completes
- ? Users know what metrics to expect
- ? Professional, polished appearance
- ? "---" clearly indicates "not available yet"

## Impact on Test Types

All test types now show Min/Max Latency fields:

| Test Type | Min/Max Latency Tracked | Display |
|-----------|------------------------|---------|
| Quick Speed Test | ? Yes | Always shown |
| Throughput Test | ? Yes | Always shown |
| Stress Test | ? Yes | Always shown |
| Custom Test | ? Yes | Always shown |

## User Experience Flow

1. **User selects test type** ? All metrics visible with "---"
2. **User clicks "Run Test"** ? Status changes to "RUNNING..."
3. **Test completes** ? Metrics populate with actual values
4. **User selects different test** ? Layout stays consistent

## Files Modified
- `src/UI/Tabs/DataPortTab.cpp`:
  - `RenderResultsPanel()` - Removed conditional check, always display Min/Max Latency

## Testing Checklist
After build succeeds:
- [ ] Test Results panel shows Min/Max Latency as "---" before any test runs
- [ ] Layout doesn't shift when test completes
- [ ] Min/Max values populate correctly after Quick Speed Test
- [ ] Min/Max values populate correctly after Throughput Test
- [ ] Min/Max values populate correctly after Stress Test
- [ ] Min/Max values populate correctly after Custom Test
- [ ] Switching between test types maintains consistent layout

## Build Status
? Pending - Close DMATool.exe and rebuild to verify changes compile successfully
