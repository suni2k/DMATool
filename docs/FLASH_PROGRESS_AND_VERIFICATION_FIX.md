# Flash Progress & Verification Fix - Complete Summary

## Problems Identified

From your log, there were **TWO critical issues**:

### 1. **Verification Failure Due to xc7_program Command** ?

The flash programming SUCCEEDED but verification FAILED with:
```
read 2099688 bytes from file ... and flash bank 0 at offset 0x00000000 in 5.060924s (405.159 KiB/s)
contents match  ?

read 2099688 bytes from file ... and flash bank 0 at offset 0x00000000 in 5.058767s (405.331 KiB/s)
contents differ  ?
diff 0 address 0x00000000. Was 0x00 instead of 0xff
...
```

**Cause**: The `xc7_program xc7.tap` command was being run **BETWEEN** `jtagspi_program` and `flash verify_bank`, which **reloaded the FPGA configuration** and invalidated the flash state, causing verification to compare against the wrong data.

### 2. **No Real-Time Progress Updates** ?

The UI stayed frozen at **10% "Programming flash..."** even though OpenOCD was actively working and outputting sector progress like:
```
Info : sector 0 took 228 ms
Info : sector 1 took 261 ms
...
Info : sector 32 took 229 ms
```

**Cause**: `ExecuteOpenOCDCommand` was blocking and only reading output AFTER the process completed, so progress callbacks were never triggered during the operation.

---

## Fixes Applied

### Fix 1: Correct OpenOCD Command Sequence

**File**: `src/Backend/FlashInterface.cpp` ? `ProgramFirmware()`

**Before** (BROKEN):
```cpp
std::vector<std::string> cmds = {
    "init",
    "jtagspi_init 0 \"" + bscanPath + "\"",
    "jtagspi_program \"" + firmwarePathUnix + "\" 0x0",
    "xc7_program xc7.tap",  // ? CORRUPTS FLASH STATE!
    "flash verify_bank 0 \"" + firmwarePathUnix + "\"",  // Verifies WRONG data
    "shutdown"
};
```

**After** (FIXED):
```cpp
std::vector<std::string> cmds = {
    "init",
    "jtagspi_init 0 \"" + bscanPath + "\"",
    "jtagspi_program \"" + firmwarePathUnix + "\" 0x0"
    // NO xc7_program here!
};

if (verifyAfter)
{
    cmds.push_back("flash verify_bank 0 \"" + firmwarePathUnix + "\"");  // ? Verifies correct data
}

// Reload FPGA AFTER verification
cmds.push_back("xc7_program xc7.tap");
cmds.push_back("shutdown");
```

**Result**: Verification now compares against the ACTUAL flashed data, not corrupted/reloaded state.

---

### Fix 2: Real-Time Progress Parsing

**File**: `src/Backend/FlashInterface.cpp` ? `ExecuteOpenOCDCommand()`

Added **line-by-line parsing** of OpenOCD stdout in real-time:

```cpp
bool FlashInterface::ExecuteOpenOCDCommand(
    const std::string& configPath,
    const std::vector<std::string>& commands,
    std::string& output,
    std::string& error,
    FlashProgressCallback progressCallback)  // ? NEW parameter
{
    // ... process launch ...
    
    std::string outputBuffer;  // Accumulate line-by-line
    
    while (true)
    {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);  // Poll every 100ms
        
        // Read stdout in real-time
        PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, NULL);
        if (bytesAvail > 0)
        {
            ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
            output += buffer;
            outputBuffer += buffer;
            std::cout << "[OPENOCD] " << buffer;  // Real-time console output
            
            // Parse complete lines for progress
            size_t newlinePos;
            while ((newlinePos = outputBuffer.find('\n')) != std::string::npos)
            {
                std::string line = outputBuffer.substr(0, newlinePos);
                outputBuffer = outputBuffer.substr(newlinePos + 1);
                
                if (progressCallback)
                {
                    // Parse "Info : sector 15 took 232 ms"
                    std::regex sectorRegex(R"(sector (\d+) took)");
                    std::smatch match;
                    if (std::regex_search(line, match, sectorRegex))
                    {
                        uint64_t sectorNum = std::stoull(match[1].str());
                        progressCallback(sectorNum, 0, "Sector " + std::to_string(sectorNum) + " complete");
                    }
                    // Parse verification results
                    else if (line.find("contents match") != std::string::npos)
                    {
                        progressCallback(0, 0, "Verification passed");
                    }
                    else if (line.find("contents differ") != std::string::npos)
                    {
                        progressCallback(0, 0, "Verification failed");
                    }
                }
            }
        }
        
        if (waitResult == WAIT_OBJECT_0)
            break;
    }
    
    return (exitCode == 0);
}
```

---

### Fix 3: Progress Percentage Calculation

**File**: `src/Backend/FlashInterface.cpp` ? `ProgramFirmware()`

Calculate firmware size ? total sectors ? percentage based on completed sectors:

```cpp
// Calculate total sectors for progress tracking
uint64_t firmwareSize = fs::file_size(firmwarePath);
uint64_t totalSectors = (firmwareSize + 65535) / 65536;  // Round up (64KB sectors)

std::cout << "[INFO] Firmware size: " << firmwareSize << " bytes (" << totalSectors << " sectors)" << std::endl;

// ...

// Create a progress callback that translates sector numbers to percentages
auto sectorProgressCallback = [&](uint64_t sectorNum, uint64_t unused, const std::string& msg) {
    if (msg.find("Sector") != std::string::npos && msg.find("complete") != std::string::npos)
    {
        // Sector progress: 10% (init) + 0-70% (sectors) + 20% (verify)
        float sectorProgress = ((float)(sectorNum + 1) / (float)totalSectors) * 70.0f;
        float totalProgress = 10.0f + sectorProgress;
        
        std::string progressMsg = "Sector " + std::to_string(sectorNum + 1) + "/" + std::to_string(totalSectors);
        
        if (progressCallback)
            progressCallback((uint64_t)totalProgress, 100, progressMsg);
    }
    else if (msg.find("Firmware loaded") != std::string::npos)
    {
        if (progressCallback)
            progressCallback(80, 100, "Writing to flash...");
    }
    else if (msg.find("Verification passed") != std::string::npos)
    {
        if (progressCallback)
            progressCallback(95, 100, "Verification passed!");
    }
    else if (msg.find("Verification failed") != std::string::npos)
    {
        if (progressCallback)
            progressCallback(90, 100, "Verification failed!");
    }
};

result.success = ExecuteOpenOCDCommand(configPath, cmds, output, error, sectorProgressCallback);
```

---

## Expected Behavior After Fix

### Console Output (Real-Time)
```
[DEBUG] OpenOCD command: "C:\...\openocd.exe" -f "flash_temp_123.cfg" ...
[DEBUG] OpenOCD process launched successfully
[OPENOCD] Open On-Chip Debugger 0.11.0+dev-00706-g822097a35-dirty
[OPENOCD] Info : clock speed 10000 kHz
[OPENOCD] Info : JTAG tap: xc7.tap tap/device found: 0x13632093
[OPENOCD] Info : Found flash device 'win w25q32fv/jv' (ID 0x1640ef)
[OPENOCD] jtagspi_program
[OPENOCD] Info : sector 0 took 228 ms
[OPENOCD] Info : sector 1 took 261 ms
[OPENOCD] Info : sector 2 took 243 ms
...
[OPENOCD] Info : sector 32 took 229 ms
[OPENOCD] read 2099688 bytes from file ... in 5.060924s (405.159 KiB/s)
[OPENOCD] contents match  ?
[OPENOCD] shutdown command invoked
[DEBUG] OpenOCD process exited with code: 0
[SUCCESS] Flash programming succeeded!
[SUCCESS] Verification passed!
```

### UI Progress Bar
```
0%   ? "Preparing to flash firmware..."
10%  ? "Programming flash..."
12%  ? "Sector 1/33"
15%  ? "Sector 2/33"
18%  ? "Sector 3/33"
...
78%  ? "Sector 32/33"
80%  ? "Sector 33/33"
95%  ? "Verification passed!"
100% ? "Flash programming and verification completed successfully"
```

### Operation Log Panel (Real-Time Updates)
```
[INFO] User initiated flash programming
[INFO] Creating OpenOCD config for xc7a75t
[INFO] BSCAN bitstream: C:\...\bscan_spi_xc7a75t.bit
[INFO] Firmware file: C:\...\002ced811686a854_ACE_75T.bin
[INFO] Firmware size: 2099688 bytes (33 sectors)
[PROGRESS] Preparing to flash firmware...
[PROGRESS] Programming flash...
[PROGRESS] Sector 1/33
[PROGRESS] Sector 2/33
...
[PROGRESS] Sector 33/33
[PROGRESS] Verification passed!
[SUCCESS] Flash programming and verification completed successfully
[INFO] Bytes written: 2099688
[INFO] Duration: 15.2 seconds
```

---

## What Was Wrong With Your Previous Flash

Looking at your log again:

```
contents match  ?  ? First verification (during jtagspi_program)
contents differ ?  ? Second verification (after xc7_program corrupted state)
```

The flash was **actually programmed correctly**, but the second verification command was comparing against **corrupted data** because `xc7_program` had already reloaded the FPGA with a different configuration.

**The fix ensures**:
1. Flash is programmed ?
2. Verification runs immediately (before FPGA reload) ?
3. FPGA is reloaded ONLY after verification passes ?

---

## Testing the Fix

### 1. Rebuild
```
Visual Studio ? Build ? Rebuild Solution
```

### 2. Run Flash Operation
```
Flash Tab ? Detect Flash Device
Flash Tab ? Select firmware file
Flash Tab ? Program Firmware (with "Verify after programming" enabled)
```

### 3. Expected Results
- ? Progress bar advances smoothly from 0% ? 100%
- ? Operation log shows real-time sector progress
- ? Console shows `[OPENOCD]` messages in real-time
- ? Verification PASSES (no "contents differ" errors)
- ? Total duration: ~15-30 seconds for 2MB firmware

---

## Files Modified

1. **`src/Backend/FlashInterface.h`**
   - Added `progressCallback` parameter to `ExecuteOpenOCDCommand()`

2. **`src/Backend/FlashInterface.cpp`**
   - Fixed `ProgramFirmware()` command sequence (moved `xc7_program` to end)
   - Added real-time progress parsing in `ExecuteOpenOCDCommand()`
   - Added sector-to-percentage calculation in `ProgramFirmware()`

---

## Build Status
? **Build successful** - Ready for testing

---

## Related Documentation
- `docs/FLASH_HANG_TROUBLESHOOTING.md` - Comprehensive troubleshooting guide
- `docs/FLASH_HANG_FIX_SUMMARY.md` - Previous hang fix summary
- `FLASH_HANG_QUICK_FIX.md` - Quick reference card
- `scripts/Check-OpenOCD-Status.ps1` - Diagnostic tool

---

## Why This Fix Works

### Problem 1 Solution: Command Ordering
**Before**: `program ? reload FPGA ? verify` (verifies WRONG data)
**After**: `program ? verify ? reload FPGA` (verifies CORRECT data)

### Problem 2 Solution: Real-Time Parsing
**Before**: Read all output after process exits (no progress)
**After**: Parse stdout line-by-line while running (live progress)

### Problem 3 Solution: Accurate Percentages
**Before**: Static "10% Programming flash..." (no updates)
**After**: Dynamic "Sector 15/33 (45%)" based on actual progress

---

## Next Steps

1. ? Test the flash operation with your XC7A75T
2. ? Verify the progress bar updates smoothly
3. ? Confirm verification PASSES
4. ? Check console for real-time OpenOCD output
5. ? Report if any issues persist

The flash should now:
- Show real-time progress ?
- Display accurate sector counts ?  
- Pass verification ?
- Complete in ~15-30 seconds ?

Let me know if you encounter any issues!
