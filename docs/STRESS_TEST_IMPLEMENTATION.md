# Stress Test Implementation Guide

## Overview
Implementing stress test functionality to track min/max latency for long-duration stability validation.

## Changes Made

### 1. UI Updates (`src/UI/Tabs/DataPortTab.cpp`)
? **Removed Mixed Test option** - Simplified to 4 test types
? **Clear results on test type change** - Added `ClearLog()` when dropdown changes
? **Updated indices** - Custom Test is now index 3 (was 4)
? **Updated button logic** - Stress Test now index 2

### 2. Backend Enum Updates Needed (`src/Backend/BenchmarkInterface.h`)

```cpp
enum class BenchmarkTestType
{
    QuickTest = 0,
    Throughput = 1,
    StressTest = 2,      // NEW: Was MixedTest
    CustomTest = 3       // Was 4
};
```

### 3. Results Structure Updates (`src/Backend/BenchmarkInterface.h`)

```cpp
struct BenchmarkResults
{
    // ... existing fields ...
    
    // Min/Max latency (for Stress Test)
    double minLatencyUs = 0.0;
    double maxLatencyUs = 0.0;
    
    // ... rest of fields ...
};
```

### 4. Stress Test Implementation (`src/Backend/BenchmarkInterface.cpp`)

**Key Features:**
- Track min/max latency for each read
- Use same algorithm as Quick Speed Test
- Default duration: 60 seconds (configurable)
- Display slowest/fastest read times

**Implementation:**
```cpp
bool BenchmarkInterface::RunStressTest(const BenchmarkConfig& config)
{
    AddLog("[INFO] Running Stress Test!");
    AddLog("");
    AddLog("[INFO] Duration: " + std::to_string(config.durationSeconds) + " seconds");
    AddLog("[INFO] Tracking min/max latency for stability analysis...");
    AddLog("");
    
    // Initialize LeechCore
    LeechCoreWrapper leechcore;
    if (!leechcore.Initialize())
    {
        AddLog("[ERROR] Failed to initialize LeechCore");
        return false;
    }
    
    AddLog("[SUCCESS] LeechCore initialized");
    AddLog("");
    AddLog("[INFO] Running stress test...");
    AddLog("");
    
    // Run test with latency tracking
    auto startTime = std::chrono::high_resolution_clock::now();
    uint64_t totalReads = 0;
    int currentSecond = 0;
    uint64_t readsThisSecond = 0;
    
    double minLatency = std::numeric_limits<double>::max();
    double maxLatency = 0.0;
    
    uint64_t baseAddress = std::stoll(config.memoryAddress, nullptr, 16);
    uint8_t buffer[4096];
    
    while (currentSecond < config.durationSeconds)
    {
        if (!m_IsRunning) break;
        
        // Measure latency for each read
        auto readStart = std::chrono::high_resolution_clock::now();
        bool success = leechcore.Read4KB(baseAddress, buffer);
        auto readEnd = std::chrono::high_resolution_clock::now();
        
        if (success)
        {
            totalReads++;
            readsThisSecond++;
            
            // Calculate latency
            double latencyUs = std::chrono::duration<double, std::micro>(readEnd - readStart).count();
            minLatency = std::min(minLatency, latencyUs);
            maxLatency = std::max(maxLatency, latencyUs);
        }
        
        // Log per-second progress
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - startTime).count();
        int newSecond = (int)elapsed;
        
        if (newSecond > currentSecond)
        {
            char buf[128];
            sprintf_s(buf, "[%02d/%02ds]: %llu RPS | Min: %.0f 탎 | Max: %.0f 탎", 
                newSecond, config.durationSeconds, readsThisSecond, minLatency, maxLatency);
            AddLog(buf);
            
            currentSecond = newSecond;
            readsThisSecond = 0;
        }
    }
    
    // Store results
    m_CurrentResults.totalReads = totalReads;
    m_CurrentResults.durationSeconds = currentSecond;
    m_CurrentResults.readsPerSecond = totalReads / (double)currentSecond;
    m_CurrentResults.avgLatencyUs = (1000000.0 / m_CurrentResults.readsPerSecond);
    m_CurrentResults.minLatencyUs = minLatency;
    m_CurrentResults.maxLatencyUs = maxLatency;
    
    // Display results
    AddLog("");
    AddLog("Results:");
    AddLog("- Total Reads: " + std::to_string(totalReads));
    AddLog("");
    AddLog("- Slowest Read: " + std::to_string((int)maxLatency) + " 탎");
    AddLog("- Fastest Read: " + std::to_string((int)minLatency) + " 탎");
    AddLog("- AVG. Latency: " + std::to_string((int)m_CurrentResults.avgLatencyUs) + " 탎");
    AddLog("");
    AddLog("- Reads Per Second (RPS): " + std::to_string((int)m_CurrentResults.readsPerSecond));
    
    return true;
}
```

### 5. Display Min/Max Latency in UI

**Add to Results Panel:**
```cpp
// For Stress Test, show min/max latency
if (s_CurrentTestType == 2 && (results.minLatencyUs > 0 || results.maxLatencyUs > 0))
{
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
    ImGui::Text("Min Latency:");
    ImGui::PopStyleColor();
    ImGui::SameLine(120);
    char buffer[64];
    sprintf_s(buffer, "%.0f us", results.minLatencyUs);
    ImGui::Text(buffer);
    
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::MutedForeground);
    ImGui::Text("Max Latency:");
    ImGui::PopStyleColor();
    ImGui::SameLine(120);
    sprintf_s(buffer, "%.0f us", results.maxLatencyUs);
    ImGui::Text(buffer);
}
```

## Testing Checklist

- [ ] Mixed Test option removed from dropdown
- [ ] Results clear when changing test type
- [ ] Stress Test button appears
- [ ] Stress Test tracks min/max latency
- [ ] Results display slowest/fastest reads
- [ ] Per-second log shows latency range
- [ ] Custom Test still works (index 3)
- [ ] Quick/Throughput tests unaffected

## Expected Output

```
[INFO] Running Stress Test!
[INFO] Duration: 60 seconds
[INFO] Tracking min/max latency for stability analysis...

[SUCCESS] LeechCore initialized

[INFO] Running stress test...

[01/60s]: 5,651 RPS | Min: 128 탎 | Max: 13,688 탎
[02/60s]: 5,516 RPS | Min: 128 탎 | Max: 15,240 탎
...
[60/60s]: 5,508 RPS | Min: 128 탎 | Max: 18,920 탎

Results:
- Total Reads: 336,318

- Slowest Read: 18,920 탎
- Fastest Read: 128 탎
- AVG. Latency: 178 탎

- Reads Per Second (RPS): 5,605 (GOOD)
```

## Next Steps

1. Update `BenchmarkInterface.h` enum
2. Add min/max fields to `BenchmarkResults`
3. Implement `RunStressTest()` method
4. Add min/max display to Results Panel
5. Test all 4 test types

