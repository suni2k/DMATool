# Benchmark Tab Improvements

## Changes Made

### 1. Dropdown Menu Spacing ?
**Issue**: Dropdown menu items were too close together, making selection difficult.

**Solution**: 
- Added `ImGuiStyleVar_ItemSpacing` with 8px vertical spacing between dropdown items
- Applied to the Test Type combo box for better visual separation

**Code Changes**:
```cpp
ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));  // Add vertical spacing
```

### 2. Stress Test Configuration ?
**Issue**: Stress test had fixed duration and read size, not matching Custom Test flexibility.

**Solution**:
- Added configurable duration slider (10-600 seconds) with **default of 60 seconds**
- Added read size dropdown (1KB, 4KB, 64KB, 256KB) with **default of 4KB**
- Configuration panel only shows when Stress Test is selected
- Matches the Custom Test UI pattern for consistency

**Code Changes**:
```cpp
if (currentTestType == 2)  // Stress Test
{
    // Duration input - default to 60 seconds
    static int stressDuration = 60;
    ImGui::SliderInt("##stressduration", &stressDuration, 10, 600, "%d s");
    
    // Read size input - default to 4KB
    static int stressReadSize = 4096;
    static int stressReadSizeIndex = 1;  // Default to 4KB
    ImGui::Combo("##stressreadsize", &stressReadSizeIndex, stressReadSizes, IM_ARRAYSIZE(stressReadSizes));
}
```

### 3. Min/Max Latency Tracking (Fastest/Slowest Reads) ?
**Issue**: Only Stress Test showed min/max latency, and it wasn't actually being tracked.

**Solution**:
- Added latency measurement for every read operation in `RunQuickTestLeechCore()`
- Added latency measurement for every chunk in `RunThroughputTest()`
- Min/Max latency now displayed for **ALL test types** in Results Panel
- Logged in console output as "fastest read" and "slowest read"

**Implementation Details**:

#### Quick Speed Test / Custom Test / Stress Test:
```cpp
// Initialize min/max tracking
double minLatency = DBL_MAX;
double maxLatency = 0.0;

// Measure each read
auto readStart = std::chrono::high_resolution_clock::now();
bool readSuccess = leechcore.Read4KB(currentAddress, readBuffer);
auto readEnd = std::chrono::high_resolution_clock::now();

if (readSuccess) {
    double latencyUs = std::chrono::duration<double, std::micro>(readEnd - readStart).count();
    if (latencyUs < minLatency) minLatency = latencyUs;
    if (latencyUs > maxLatency) maxLatency = latencyUs;
}

// Store results
m_CurrentResults.minLatencyUs = minLatency;
m_CurrentResults.maxLatencyUs = maxLatency;
```

#### Throughput Test:
- Measures latency for each **1MB chunk** (not individual 4KB reads)
- Min latency = fastest 1MB chunk transfer
- Max latency = slowest 1MB chunk transfer

**UI Display**:
- Results Panel shows min/max for all completed tests
- Console log shows:
  ```
  - MIN. Latency: 125 us (fastest read)
  - MAX. Latency: 234 us (slowest read)
  ```

## Test Type Defaults

| Test Type | Duration | Read Size | Notes |
|-----------|----------|-----------|-------|
| Quick Speed Test | 10s | 4KB | Fixed |
| Throughput Test | ~60s | 1MB chunks | Time varies based on throughput |
| Stress Test | **60s** (configurable) | **4KB** (configurable) | NEW defaults |
| Custom Test | User-defined | User-defined | Fully configurable |

## Stress Test Configuration Options

### Duration
- Range: 10-600 seconds (10s to 10 minutes)
- Default: **60 seconds**
- Slider control

### Read Size
- Options: 1KB, 4KB, 64KB, 256KB
- Default: **4KB**
- Dropdown control

## Benefits

1. **Better UX**: Dropdown menu is easier to navigate
2. **More Flexible**: Stress Test now matches Custom Test configurability
3. **Better Insights**: Min/Max latency helps identify:
   - Consistency of DMA performance
   - Outliers/spikes in read times
   - Overall stability of the DMA connection

## Testing Recommendations

After these changes, run each test type and verify:
1. Dropdown spacing looks good
2. Stress Test defaults to 60s and 4KB
3. Min/Max latency appears in results for all tests
4. Min latency should be < Avg latency < Max latency (logical)
5. Stress Test config panel only shows when Stress Test is selected

## Future Enhancements

Potential improvements based on min/max latency data:
- Latency distribution graph (histogram)
- Standard deviation calculation
- Jitter measurement (variance in latency)
- Outlier detection (reads > 3x average)
