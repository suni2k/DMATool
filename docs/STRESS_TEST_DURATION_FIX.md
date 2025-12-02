# Stress Test Duration Fix

## Issue
The Stress Test duration slider wasn't working - tests always ran for 300 seconds regardless of the slider value.

## Root Cause
The `StartQuickTest()` function had a switch statement that **hardcoded** the Stress Test duration to 300 seconds:

```cpp
case 2: // Stress Test
    s_Config.durationSeconds = 300;  // ? HARDCODED - overrides slider value!
    s_Config.testSizeMB = 1024 * 5;  // 5 GB
    break;
```

### Execution Flow (Before Fix)
1. User adjusts slider to 60 seconds ? `s_Config.durationSeconds = 60` ?
2. User clicks "Run Stress Test" button
3. Button handler calls `StartQuickTest()`
4. `StartQuickTest()` executes switch statement
5. **Case 2 overwrites config**: `s_Config.durationSeconds = 300` ?
6. Test runs for 300 seconds (ignoring slider value)

## Solution
Remove the hardcoded duration override for Stress Test (case 2) since the configuration is already properly set by the sliders in `RenderTestControlsPanel()`.

### Code Changes
```cpp
case 2: // Stress Test - use configured values from UI
    // Duration and read size already set from sliders
    break;
```

### Execution Flow (After Fix)
1. User adjusts slider to 60 seconds ? `s_Config.durationSeconds = 60` ?
2. User clicks "Run Stress Test" button
3. Button handler calls `StartQuickTest()`
4. `StartQuickTest()` executes switch statement
5. **Case 2 does nothing** (preserves slider value) ?
6. Test runs for 60 seconds (respects slider value) ?

## Why This Happened
The original implementation assumed **all test types** would have fixed, hardcoded parameters. When we added configurable sliders to Stress Test and Custom Test, we properly updated the button handler to skip the override:

```cpp
// In button handler:
if (currentTestType != 3 && currentTestType != 2)  // Skip Custom and Stress
{
    // Only override for Quick Speed and Throughput
}
```

However, we **forgot to update** the switch statement in `StartQuickTest()`, which still had the old hardcoded values.

## Testing
After this fix, verify:
- [ ] Stress Test with 60s duration runs for ~60 seconds
- [ ] Stress Test with 120s duration runs for ~2 minutes
- [ ] Stress Test with 10s duration runs for ~10 seconds
- [ ] Custom Test still works (duration and read size)
- [ ] Quick Speed Test still runs for 10 seconds (unaffected)
- [ ] Throughput Test still works (unaffected)

## Files Modified
- `src/UI/Tabs/DataPortTab.cpp`:
  - `StartQuickTest()` - Removed hardcoded duration override for Stress Test

## Build Status
? Build successful - all changes compile without errors

## Related Issues Fixed
This also ensures consistency with Custom Test (case 3), which already had the correct implementation (no override).

## Lesson Learned
When adding configurable parameters to tests:
1. Update the **slider/dropdown handlers** to save values ?
2. Update the **button handler** to skip overrides ?
3. Update the **StartQuickTest()** switch to skip overrides ? (This was missed!)

All three locations must be updated for the configuration to work correctly.
