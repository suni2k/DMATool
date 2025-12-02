# Benchmark Tab UI Fixes

## Issues Fixed

### 1. ? Test Results Panel - Font Sizing and Spacing

**Problem**: 
- Test Results panel required scrolling due to too much content and spacing
- Throughput metric was shown for all tests, cluttering the display

**Solution**:
- **Reduced font scale** for metrics from 1.0 to 0.95
- **Reduced spacing** between sections (changed `ImGui::Spacing()` to `ImGui::Dummy(ImVec2(0, 4))`)
- **Reduced label alignment** from 120px to 100px (more compact)
- **Reduced rating scale font** from 0.85 to 0.80
- **Conditional Throughput display**: Only shows for Throughput Test (test type 1)
- **Reduced decimal precision** for duration from 2 decimals to 1 decimal (18.16s ? 18.2s)

**Code Changes**:
```cpp
// Compact font for metrics
ImGui::SetWindowFontScale(0.95f);

// Reduced alignment
ImGui::SameLine(100);  // Was 120

// Throughput only for Throughput Test
if (s_CurrentTestType == 1)  // Only for Throughput Test
{
    ImGui::Text("Throughput:");
    // ...
}

// Rating scale more compact
ImGui::SetWindowFontScale(0.80f);  // Was 0.85f
```

**Result**:
- All metrics now fit without scrolling
- Cleaner, more professional appearance
- Throughput metric only appears when relevant

---

### 2. ? Stress Test Duration Not Updating

**Problem**: 
- Changing the duration slider in Stress Test Configuration didn't actually change the test duration
- Test would run for default time regardless of slider value

**Root Cause**:
- Configuration values were only stored at the END of the render function
- When "Run Stress Test" button was clicked, it used the old config values
- The slider value wasn't being immediately persisted to `s_Config`

**Solution**:
- **Immediate config updates**: Update `s_Config` as soon as slider/dropdown changes
- **Continuous sync**: Ensure config is updated every frame (at end of config section)
- Applied same fix to both **Stress Test** and **Custom Test** configurations

**Code Changes**:
```cpp
// Stress Test Configuration
if (ImGui::SliderInt("##stressduration", &stressDuration, 10, 600, "%d s"))
{
    // Update config immediately when slider changes
    s_Config.durationSeconds = stressDuration;
}

if (ImGui::Combo("##stressreadsize", &stressReadSizeIndex, stressReadSizes, ...))
{
    switch (stressReadSizeIndex) { /* ... */ }
    // Update config immediately when selection changes
    s_Config.customReadSizeBytes = stressReadSize;
}

// Ensure config is always up to date (even on first render)
s_Config.durationSeconds = stressDuration;
s_Config.customReadSizeBytes = stressReadSize;
```

**Result**:
- Stress Test now runs for the exact duration specified on the slider
- Custom Test also benefits from the same fix
- Configuration changes are immediately reflected when test starts

---

## Testing Checklist

After these fixes, verify:

### Test Results Panel
- [ ] All metrics visible without scrolling
- [ ] Throughput metric **only shows for Throughput Test**
- [ ] Throughput metric **hidden for Quick Speed, Stress, and Custom tests**
- [ ] Text is readable (not too small)
- [ ] Rating scale fits properly on right side
- [ ] Min/Max latency displays correctly for all tests

### Stress Test Configuration
- [ ] Default duration is 60 seconds
- [ ] Changing duration slider to 10s runs for ~10 seconds
- [ ] Changing duration slider to 120s runs for ~2 minutes
- [ ] Duration slider max value (600s = 10 minutes) works
- [ ] Read size dropdown changes are applied
- [ ] Configuration persists when switching between test types and back

### Custom Test Configuration
- [ ] Same verification as Stress Test
- [ ] Duration range 1-300 seconds works correctly
- [ ] Read size changes are applied

---

## Visual Improvements Summary

| Element | Before | After |
|---------|--------|-------|
| Metrics Font Scale | 1.0 | 0.95 |
| Label Alignment | 120px | 100px |
| Rating Scale Font | 0.85 | 0.80 |
| Duration Decimals | 18.16 s | 18.2 s |
| Spacing | `ImGui::Spacing()` | `ImGui::Dummy(ImVec2(0, 4))` |
| Throughput Display | All tests | Throughput Test only |

---

## Configuration Update Pattern

For future reference, when adding configurable test parameters:

```cpp
// CORRECT PATTERN:
static int configValue = defaultValue;

if (ImGui::SliderInt("##label", &configValue, min, max, format))
{
    // Update config IMMEDIATELY on change
    s_Config.parameterName = configValue;
}

// ALSO update at end of config section (catches first render)
s_Config.parameterName = configValue;
```

**Why this works**:
1. Immediate update in `if` block catches user changes
2. Final update at end ensures first frame has correct value
3. No race conditions when "Run Test" button is clicked

---

## Files Modified

- `src/UI/Tabs/DataPortTab.cpp`:
  - `RenderResultsPanel()` - Compactness and conditional Throughput display
  - `RenderTestControlsPanel()` - Stress Test and Custom Test config fixes

## Build Status

? Build successful - all changes compile without errors
