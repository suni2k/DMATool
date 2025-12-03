# Flash Programming Multi-Phase Progress Enhancement

## Summary

Enhanced flash programming progress tracking to show distinct phases with accurate progress based on actual OpenOCD operation stages.

---

## Problem Analysis

From your log, the flash operation has **3 distinct phases**:

### **Phase 1: Sector Erasing** (Fast - ~8 seconds)
```
Info : sector 0 took 228 ms
Info : sector 1 took 261 ms
...
Info : sector 32 took 229 ms
```
- **Duration**: ~230ms per sector × 33 sectors = ~8 seconds
- **What it does**: Erases flash sectors to prepare for writing

### **Phase 2: Writing Flash** (Slow - ~2+ minutes)
```
0x001FFFEC
0x001FFFF0
0x001FFFF4
... (thousands of address lines scrolling down)
```
- **Duration**: ~2 minutes for 2MB firmware (~200 KiB/s)
- **What it does**: Writes firmware data to flash memory
- **Problem**: No OpenOCD output during this phase, so UI appeared frozen at 80%

### **Phase 3: Verification** (Medium - ~5-10 seconds)
```
read 2099688 bytes from file...
contents match
```
- **Duration**: ~5-10 seconds
- **What it does**: Reads back flash and verifies contents

---

## Solution: Multi-Phase Progress Tracking

### **Phase 1: Erasing Sectors (10% ? 40%)**
**Progress**: Based on sector count  
**Display**: "Erasing sector X/33"  
**Duration**: ~8 seconds (predictable)

```
10% ? "Initializing flash..."
12% ? "Erasing sector 1/33"
15% ? "Erasing sector 2/33"
...
38% ? "Erasing sector 32/33"
40% ? "Erasing sector 33/33"
```

### **Phase 2: Writing Flash (40% ? 90%)**
**Progress**: Time-based estimation (200 KiB/s typical)  
**Display**: "Writing flash contents..."  
**Duration**: ~2 minutes (estimated, varies by size)

```
42% ? "Writing flash contents..."  (2 seconds elapsed)
45% ? "Writing flash contents..."  (10 seconds elapsed)
50% ? "Writing flash contents..."  (30 seconds elapsed)
...
85% ? "Writing flash contents..."  (110 seconds elapsed)
90% ? "Writing flash contents..."  (cap at 90%, wait for actual completion)
```

**Key Features**:
- ? **Timer-based updates** every 2 seconds
- ? **Estimated progress** based on typical 200 KiB/s write speed
- ? **Capped at 90%** - won't reach 100% until verification completes
- ? **Prevents UI freeze** - user knows it's still working

### **Phase 3: Verification (90% ? 95%)**
**Progress**: Triggered by OpenOCD message  
**Display**: "Verification passed!"

```
90% ? (waiting for verification to complete)
95% ? "Verification passed!"
```

### **Phase 4: Complete (95% ? 100%)**
**Progress**: Final completion  
**Display**: "Flash programming and verification completed successfully"

```
100% ? "Flash programming and verification completed successfully"
```

---

## Technical Implementation

### **1. Sector Phase Tracking**

```cpp
// PHASE 1: Erasing sectors (10% - 40%)
if (msg.find("Sector") != std::string::npos && msg.find("complete") != std::string::npos)
{
    float sectorProgress = ((float)(sectorNum + 1) / (float)totalSectors) * 30.0f;  // 0-30%
    float totalProgress = 10.0f + sectorProgress;  // 10-40%
    
    std::string progressMsg = "Erasing sector " + std::to_string(sectorNum + 1) + "/" + std::to_string(totalSectors);
    progressCallback((uint64_t)totalProgress, 100, progressMsg);
    
    // Mark when sectors are complete
    if (sectorNum + 1 == totalSectors)
    {
        sectorsComplete = true;
        writingStartTime = std::chrono::steady_clock::now();
    }
}
```

### **2. Writing Phase with Timer-Based Progress**

**In ExecuteOpenOCDCommand**:
```cpp
// Track when sectors complete to start periodic writing updates
bool sectorsCompleted = false;
auto lastProgressUpdate = std::chrono::steady_clock::now();

while (true)
{
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);  // Poll every 100ms
    
    // Send periodic "Writing progress" updates during the writing phase (every 2 seconds)
    auto now = std::chrono::steady_clock::now();
    double timeSinceLastUpdate = std::chrono::duration<double>(now - lastProgressUpdate).count();
    
    if (sectorsCompleted && timeSinceLastUpdate >= 2.0)
    {
        if (progressCallback)
            progressCallback(0, 0, "Writing progress");  // Trigger progress update
        lastProgressUpdate = now;
    }
    
    // ... read OpenOCD output ...
}
```

**In ProgramFirmware callback**:
```cpp
// Estimate writing time based on typical speed (~200 KiB/s)
double estimatedWriteSeconds = (firmwareSize / 1024.0) / 200.0;

// PHASE 2: Writing flash (40% - 90%)
else if (msg.find("Writing progress") != std::string::npos)
{
    if (sectorsComplete && !writingComplete)
    {
        auto now = std::chrono::steady_clock::now();
        double elapsedWrite = std::chrono::duration<double>(now - writingStartTime).count();
        
        // Progress from 40% to 90% based on estimated write time
        double ratio = elapsedWrite / estimatedWriteSeconds;
        if (ratio > 1.0) ratio = 1.0;  // Cap at 100% of estimate
        double writeProgress = ratio * 50.0;  // 0-50%
        float totalProgress = 40.0f + (float)writeProgress;  // 40-90%
        
        progressCallback((uint64_t)totalProgress, 100, "Writing flash contents...");
    }
}
```

### **3. Verification and Completion**

```cpp
// PHASE 3: Verification (90% - 95%)
else if (msg.find("Verification passed") != std::string::npos)
{
    writingComplete = true;
    progressCallback(95, 100, "Verification passed!");
}

// PHASE 4: Complete (100%)
if (result.success && verifyAfter && verifyPassed)
{
    progressCallback(100, 100, "Flash programming and verification completed successfully");
}
```

---

## Progress Timeline Example

**For 2MB Firmware (~33 sectors)**:

| Time | Progress | Phase | Message |
|------|----------|-------|---------|
| 0s | 0% | Init | "Preparing to flash firmware..." |
| 1s | 10% | Sector | "Initializing flash..." |
| 2s | 12% | Sector | "Erasing sector 1/33" |
| 3s | 15% | Sector | "Erasing sector 2/33" |
| ... | ... | ... | ... |
| 9s | 40% | Sector | "Erasing sector 33/33" |
| 11s | 42% | **Writing** | "Writing flash contents..." |
| 15s | 44% | **Writing** | "Writing flash contents..." |
| 30s | 50% | **Writing** | "Writing flash contents..." |
| 60s | 60% | **Writing** | "Writing flash contents..." |
| 90s | 70% | **Writing** | "Writing flash contents..." |
| 120s | 80% | **Writing** | "Writing flash contents..." |
| 130s | 85% | **Writing** | "Writing flash contents..." |
| 135s | 88% | **Writing** | "Writing flash contents..." |
| 140s | 90% | **Writing** | "Writing flash contents..." (capped) |
| 145s | 95% | Verify | "Verification passed!" |
| 147s | 100% | Complete | "Flash programming and verification completed successfully" |

**Total Duration**: ~147 seconds (2 minutes 27 seconds)

---

## User Experience Improvements

### **Before** ?:
```
10% ? "Programming flash..."
...
80% ? "Sector 33/33"
...
(2 minute freeze - user thinks it's hung!)
...
100% ? "Complete!"
```

### **After** ?:
```
10% ? "Initializing flash..."
12% ? "Erasing sector 1/33"
...
40% ? "Erasing sector 33/33"
42% ? "Writing flash contents..."  (shows it's working!)
45% ? "Writing flash contents..."  (still working!)
50% ? "Writing flash contents..."  (progress visible!)
...
90% ? "Writing flash contents..."  (almost done!)
95% ? "Verification passed!"
100% ? "Flash programming and verification completed successfully"
```

**Benefits**:
- ? **No more UI freeze** - progress updates every 2 seconds
- ? **Accurate phase indication** - user knows what's happening
- ? **Realistic timing** - doesn't jump to 100% prematurely
- ? **Clear completion** - distinct phases for each operation

---

## Files Modified

1. **`src/Backend/FlashInterface.cpp`**
   - Added multi-phase progress tracking in `ProgramFirmware()`
   - Added timer-based "Writing progress" updates in `ExecuteOpenOCDCommand()`
   - Estimated write time calculation based on 200 KiB/s typical speed
   - Progress capping at 90% until verification completes

---

## Build Status
? **Build successful** - Ready for testing

---

## Testing Instructions

### Test Flash Programming
```
Flash Tab ? Detect Flash Device
Flash Tab ? Browse ? Select: 002ced811686a854_ACE_75T.bin
Flash Tab ? Program Firmware (with "Verify after programming" enabled)
```

**Expected Progress**:
1. **0-10%**: "Preparing to flash firmware..."
2. **10-40%**: "Erasing sector X/33" (fast, ~8 seconds)
3. **40-90%**: "Writing flash contents..." (slow, ~2 minutes, updates every 2 seconds)
4. **90-95%**: "Verification passed!"
5. **95-100%**: "Flash programming and verification completed successfully"

**Expected Duration**: ~147 seconds (2 minutes 27 seconds) for 2MB firmware

---

## Progress Bar Color States

| Phase | Progress | Color | Message |
|-------|----------|-------|---------|
| Init | 0-10% | ?? Yellow | "Preparing..." |
| Erase | 10-40% | ?? Yellow | "Erasing sector X/33" |
| Write | 40-90% | ?? Yellow | "Writing flash contents..." |
| Verify | 90-95% | ?? Yellow | "Verification passed!" |
| Complete | 100% | ?? **Darker Green** | "Flash programming and verification completed successfully" |

---

## Key Features

### **1. Timer-Based Writing Updates**
- Updates every **2 seconds** during writing phase
- Prevents UI from appearing frozen
- User knows operation is still progressing

### **2. Estimated Progress**
- Based on typical **200 KiB/s** write speed
- Adjusts for firmware size
- Capped at **90%** to avoid premature 100%

### **3. Phase Separation**
- **10-40%**: Erasing (fast, predictable)
- **40-90%**: Writing (slow, estimated)
- **90-95%**: Verification (fast, actual)
- **95-100%**: Complete

### **4. Accurate Sector Tracking**
- Shows "Erasing sector X/33"
- Real-time sector completion
- Based on actual OpenOCD output

---

## Technical Notes

### **Why 200 KiB/s?**
From your log:
```
read 2099688 bytes from file ... in 5.060924s (405.159 KiB/s)
```

Typical flash write speeds:
- **Read**: ~400 KiB/s (fast)
- **Write**: ~200 KiB/s (slower - conservative estimate)

### **Why Cap at 90%?**
- **Prevents premature 100%** - Users might think it's done
- **Waits for actual verification** - Only goes to 100% after "contents match"
- **Better UX** - Clear distinction between writing and verification

### **Why 2-Second Updates?**
- **Not too frequent** - Doesn't spam the log
- **Not too slow** - User sees progress
- **Balanced** - Good for 2-minute operations

---

## Summary

**What Changed**:
1. ? Added multi-phase progress tracking (erase/write/verify)
2. ? Added timer-based writing updates every 2 seconds
3. ? Estimated progress based on typical write speed
4. ? Capped progress at 90% until verification
5. ? Clear phase messages for user awareness

**User Benefits**:
- ?? **No more UI freeze** - Progress updates every 2 seconds during writing
- ?? **Realistic progress** - Doesn't jump to 100% prematurely
- ?? **Phase awareness** - User knows what's happening (erasing/writing/verifying)
- ?? **Estimated timing** - Progress reflects actual operation duration
- ? **Professional UX** - Smooth, informative progress tracking

**Result**: Flash programming now has professional multi-phase progress tracking with no UI freeze! ??

---

## Next Steps

1. ? Rebuild solution (already done)
2. ? Test flash programming - verify progress updates every 2 seconds
3. ? Confirm progress stops at 90% and waits for verification
4. ? Check completion at 100% after "Verification passed!"
5. ? Verify total duration matches expectations (~2-3 minutes for 2MB)

**Enjoy the smooth, informative flash progress!** ??
