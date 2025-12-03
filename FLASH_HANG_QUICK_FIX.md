# Flash Hanging at 10% - Quick Fix Guide

## What You're Seeing
```
[PROGRESS] Programming flash...
? Stuck at 10% for minutes
```

## Quick Diagnosis (30 seconds)

### Run This NOW:
```powershell
.\scripts\Check-OpenOCD-Status.ps1
```

### Interpret Results:

| Result | Meaning | Action |
|--------|---------|--------|
| **CPU time increasing** | ? Normal - OpenOCD is working | **Wait** (can take 2-4 min) |
| **CPU time frozen** | ? Stuck/Deadlocked | **Kill & Retry** |
| **No process found** | ? Failed to start | **Check setup** |
| **Responding: False** | ? Hung process | **Kill immediately** |

## Quick Fixes

### Fix 1: Kill & Retry
```powershell
.\scripts\Kill-OpenOCD-Admin.ps1
# Then retry flash in DMATool
```

### Fix 2: Reset Adapter
```powershell
.\scripts\Reset-CH347-Adapter.ps1
# Then retry flash
```

### Fix 3: Rebuild App (New Fix Applied)
```
Visual Studio -> Build -> Rebuild Solution
```

## What Changed (Just Applied)

**Before**: No visible progress, UI appears frozen

**After**: Real-time console output showing OpenOCD progress:
```
[OPENOCD] Info : sector 0 took 123 ms
[OPENOCD] Info : sector 1 took 125 ms
[OPENOCD] Info : sector 2 took 124 ms
...
```

## Is It Actually Working?

### Signs Flash is Proceeding Normally:
- ? `[OPENOCD]` messages in console
- ? "sector X took Y ms" messages
- ? CPU time increasing in Check-OpenOCD-Status
- ? Progress percentage advancing (even slowly)

### Signs Flash is Stuck:
- ? No console output for 1+ minute
- ? CPU time not increasing
- ? Progress frozen at 10%
- ? CH347 communication errors

## Expected Duration

- **Small firmware (< 2 MB)**: 1-2 minutes
- **Medium firmware (4-8 MB)**: 2-4 minutes  ? Most common
- **Large firmware (> 10 MB)**: 5-8 minutes

**If > 10 minutes**: Something is wrong, kill and retry

## Emergency Actions

### If Nothing Works:
1. Kill all OpenOCD: `.\scripts\Kill-OpenOCD-Admin.ps1`
2. Unplug CH347 USB cable
3. Wait 10 seconds
4. Plug back in
5. Reset CH347: `.\scripts\Reset-CH347-Adapter.ps1`
6. Close and restart DMATool
7. Try flash again

### If FPGA is Bricked:
1. Power cycle FPGA (unplug power, wait, replug)
2. Use known-good firmware file
3. Flash again

## Full Documentation
- **Detailed troubleshooting**: `docs/FLASH_HANG_TROUBLESHOOTING.md`
- **Fix summary**: `docs/FLASH_HANG_FIX_SUMMARY.md`
- **Diagnostic tool**: `scripts/Check-OpenOCD-Status.ps1`

## Test After Rebuild

1. ? Start flash operation
2. ? Watch console for `[OPENOCD]` messages
3. ? Run `Check-OpenOCD-Status.ps1` in another window
4. ? Verify CPU time is increasing
5. ? Confirm progress advances smoothly

**If you see real-time OpenOCD output, the fix is working!**
