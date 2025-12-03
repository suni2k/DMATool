# Flash Hanging Fix - Summary

## Problem
Flash operation hangs at 10% ("Programming flash...") with no progress updates.

## Root Cause
`ExecuteOpenOCDCommand()` was using **blocking I/O** - it only read stdout/stderr AFTER the process completed. This meant:
- No real-time progress visibility
- No way to detect if OpenOCD was stuck or actually working
- User couldn't see what was happening

## Fix Applied

### File: `src/Backend/FlashInterface.cpp`

**Changed**: `ExecuteOpenOCDCommand()` function

**Before** (Blocking):
```cpp
while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
{
    output += buffer;
}
WaitForSingleObject(pi.hProcess, INFINITE);  // ? Blocks until process exits
```

**After** (Non-blocking with real-time output):
```cpp
while (true)
{
    // Check if process is still running
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);  // Poll every 100ms
    
    // Read stdout if available
    PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, NULL);
    if (bytesAvail > 0)
    {
        ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        output += buffer;
        std::cout << "[OPENOCD] " << buffer;  // ? Real-time console output
    }
    
    // Read stderr if available
    PeekNamedPipe(hStdErrRead, NULL, 0, NULL, &bytesAvail, NULL);
    if (bytesAvail > 0)
    {
        ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        error += buffer;
        std::cout << "[OPENOCD-ERR] " << buffer;  // ? Real-time error output
    }
    
    // Exit if process completed
    if (waitResult == WAIT_OBJECT_0)
        break;
}
```

### Key Changes

1. **Non-blocking pipe reads** using `PeekNamedPipe`
2. **Polling loop** that checks process state every 100ms
3. **Real-time console output** with `[OPENOCD]` prefix
4. **Real-time error output** with `[OPENOCD-ERR]` prefix
5. **Better logging** in `ProgramFirmware()` function

## Benefits

### Now You Can See:
```
[DEBUG] OpenOCD command: "C:\...\openocd.exe" -f "flash_temp_123.cfg" ...
[DEBUG] OpenOCD process launched successfully
[OPENOCD] Open On-Chip Debugger 0.11.0+dev-00706-g822097a35-dirty
[OPENOCD] Info : clock speed 10000 kHz
[OPENOCD] Info : JTAG tap: xc7.tap tap/device found: 0x13632093
[OPENOCD] jtagspi_program
[OPENOCD] Info : sector 0 took 123 ms
[OPENOCD] Info : sector 1 took 125 ms
[OPENOCD] Info : sector 2 took 124 ms
...
[OPENOCD] shutdown command invoked
[DEBUG] OpenOCD process exited with code: 0
[SUCCESS] Flash programming succeeded!
```

### Before (Frozen UI):
```
[PROGRESS] Preparing to flash firmware...
[PROGRESS] Programming flash...
? (nothing for 2 minutes...)
```

## Testing the Fix

### 1. Rebuild
```powershell
# In Visual Studio
Build -> Rebuild Solution
```

### 2. Start Flash Operation
```
Flash Tab -> Select firmware -> Program Firmware
```

### 3. Watch Console Output
You should see real-time `[OPENOCD]` messages showing progress.

### 4. If Still Hangs
Run diagnostic:
```powershell
.\scripts\Check-OpenOCD-Status.ps1
```

Check:
- Is OpenOCD running?
- Is CPU time increasing?
- Is it responding?

## Diagnostic Tool

**New Script**: `scripts/Check-OpenOCD-Status.ps1`

Shows:
- Process ID
- Start time
- CPU usage
- Memory usage
- Responding status
- Command line
- Suggestions based on state

Usage:
```powershell
.\scripts\Check-OpenOCD-Status.ps1
```

## Expected Timeline

| Operation | Time | Notes |
|-----------|------|-------|
| Config generation | < 1s | Instant |
| BSCAN load | 1-2s | One-time per operation |
| Sector erase | 10-30s | Depends on flash size |
| Programming | 1-3 min | Depends on firmware size |
| Verification | 30-60s | If enabled |
| **Total** | **2-4 min** | For typical 4-8 MB firmware |

## If You See Errors

### CH347 Communication Error
```
[OPENOCD-ERR] Error: CH347_Read read data failure.
[OPENOCD-ERR] Error: CH347 clear Buffer Error.
```

**Solution**: Reset adapter
```powershell
.\scripts\Reset-CH347-Adapter.ps1
```

### No Progress After 5 Minutes
```
[OPENOCD] Info : sector 15 took 125 ms
? (stuck here for 5+ minutes)
```

**Solution**: Kill and retry
```powershell
.\scripts\Kill-OpenOCD-Admin.ps1
```
Then try flash again.

### Process Not Found
```
[INFO] No OpenOCD processes found
```

**Cause**: OpenOCD failed to launch

**Check**:
- BSCAN bitstreams in `%TEMP%\DMATool\bscan\`
- OpenOCD.exe in `%TEMP%\DMATool\`
- CH347 driver installed

## Build Status
? Build successful - Ready for testing

## Documentation
- **Full troubleshooting**: `docs/FLASH_HANG_TROUBLESHOOTING.md`
- **Diagnostic script**: `scripts/Check-OpenOCD-Status.ps1`
- **Related fix**: `docs/FLASH_INTERFACE_LOOP_FIX.md`

## Next Steps

1. ? Rebuild project
2. ? Test flash operation
3. ? Monitor console for real-time output
4. ? Verify smooth progress from 0% ? 100%
5. ? Confirm only ONE OpenOCD process during operation

If issues persist, see `docs/FLASH_HANG_TROUBLESHOOTING.md` for detailed debugging steps.
