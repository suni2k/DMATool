# Quick Fix for OpenOCD Process Leak

## The Problem
You have 50+ orphaned `openocd.exe` processes running in Task Manager from previous testing.

## Quick Solution

### Option 1: PowerShell Admin (Recommended)
1. Right-click PowerShell ? "Run as Administrator"
2. Navigate to your project:
   ```powershell
   cd C:\Users\suni\source\repos\DMATool
   ```
3. Run the cleanup script:
   ```powershell
   .\scripts\Kill-OpenOCD-Admin.ps1
   ```

### Option 2: Task Manager
1. Open Task Manager (Ctrl+Shift+Esc)
2. Go to "Details" tab
3. Find all "openocd.exe" processes
4. Right-click ? "End Task" for each one
5. Or select one, then Ctrl+Click others, right-click ? "End Task"

### Option 3: Command Line (Admin)
```cmd
taskkill /F /IM openocd.exe /T
```

## What Was Fixed

The code now properly:
- ? Waits for OpenOCD to finish before continuing
- ? Terminates the process if it times out
- ? Cleans up all process handles
- ? Prevents orphaned processes

## Future Prevention

After rebuilding with the fix:
- OpenOCD will automatically terminate after each operation
- No more manual cleanup needed
- Processes won't accumulate in Task Manager

## Verify the Fix Works

1. Kill all current orphaned processes (using steps above)
2. Run the new build: `bin\Debug-x64\DMATool.exe`
3. Click "Detect FPGA" a few times
4. Check Task Manager - should see 0 openocd.exe processes when idle
5. You might see 1 process briefly during detection, but it should disappear

---

**Next Steps**:
1. Clean up orphaned processes now (Option 1 recommended)
2. Test the new build
3. Verify no processes accumulate
