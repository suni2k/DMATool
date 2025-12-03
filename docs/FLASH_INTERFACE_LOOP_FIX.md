# Flash Interface Infinite Loop Fix

## Problem

When attempting to flash or verify firmware in the JTAG Flash tab, the application would enter an infinite loop, spawning hundreds of OpenOCD processes. The same issue occurred whether the operation succeeded or failed. After aborting, the application needed to be closed and all OpenOCD processes had to be manually terminated using the `Kill-OpenOCD-Admin.ps1` script.

### Symptoms

1. Clicking "Program Firmware" or "Verify Firmware" would trigger the operation
2. The operation would start but then immediately restart
3. Each restart spawned a new OpenOCD process
4. The log would fill with hundreds of identical messages:
   ```
   [PROGRESS] Preparing to flash firmware...
   [INFO] Starting flash programming...
   [INFO] Firmware: C:\Users\...\firmware.bin
   [INFO] Target chip: xc7a75t
   [PROGRESS] Programming flash...
   ```
5. OpenOCD processes would accumulate in the background
6. The only way to stop was to abort and manually kill processes

### Log Output Example

The user would see endless repetitions like this:
```
[PROGRESS] Preparing to flash firmware...
[INFO] Starting flash programming...
[INFO] Firmware: C:\Users\suni\source\repos\DMATool\dmafiles\CH347FPGATool\002ced811686a854_ACE_75T.bin
[INFO] Target chip: xc7a75t
[PROGRESS] Preparing to flash firmware...
[PROGRESS] Programming flash...
[PROGRESS] Preparing to flash firmware...
[INFO] Starting flash programming...
[INFO] Firmware: C:\Users\suni\source\repos\DMATool\dmafiles\CH347FPGATool\002ced811686a854_ACE_75T.bin
[INFO] Target chip: xc7a75t
...
(repeated hundreds of times)
```

## Root Cause

The issue was in `src/UI/Tabs/JTAGFlashTab.cpp`. The async operation flow had a critical race condition:

### The Problematic Flow

1. **User clicks button** ? `s_IsFlashing` set to `true`
2. **Render() called every frame** (60+ times per second)
3. **Frame 1**: Check `s_FramesSinceOperation >= 2` ? false (skip)
4. **Frame 2**: Check `s_FramesSinceOperation >= 2` ? false (skip)
5. **Frame 3**: Check `s_FramesSinceOperation >= 2` ? **true** ? Launch thread
6. **Frame 4**: `s_IsFlashing` still `true` ? Launch ANOTHER thread
7. **Frame 5**: `s_IsFlashing` still `true` ? Launch ANOTHER thread
8. ... **INFINITE LOOP** ...

The flag `s_IsFlashing` was only set to `false` *inside the detached thread*, which ran asynchronously. Meanwhile, the UI continued rendering at 60+ FPS, and each frame would launch a new thread because the condition remained true.

### Code Before Fix

```cpp
// Perform flashing after UI renders
if (s_IsFlashing && s_FramesSinceOperation >= 2)
{
    // Ensure FlashInterface exists
    if (!s_FlashInterface)
    {
        s_FlashInterface = new Backend::FlashInterface();
    }
    
    // Launch flash operation in background thread
    std::thread flashThread([&]() {
        // ... flash operation ...
        
        s_IsFlashing = false;  // ? Too late! Hundreds of threads already launched
    });
    
    flashThread.detach();  // Detached thread runs independently
    s_FramesSinceOperation = 0;
}
```

**The problem**: By the time the thread sets `s_IsFlashing = false`, the next frame has already checked the condition and launched another thread.

## Solution

The fix involves three key changes:

### 1. Immediate Flag Reset

Reset the trigger flag (`s_IsFlashing`) **immediately** before launching the thread:

```cpp
if (s_IsFlashing && s_FramesSinceOperation >= 2)
{
    // CRITICAL FIX: Prevent re-entry! Set flag to false IMMEDIATELY
    s_IsFlashing = false;  // ? Reset before launching thread
    
    // Now launch the thread...
}
```

### 2. Separate Thread Running Tracking

Use dedicated flags to track whether a thread is actually running:

```cpp
static bool s_FlashThreadRunning = false;
static bool s_VerifyThreadRunning = false;
```

These flags are:
- Set to `true` before launching the thread
- Set to `false` at the end of the thread
- Checked to prevent launching multiple threads

### 3. Re-entry Protection

Add a guard to prevent launching a new thread if one is already running:

```cpp
if (s_IsFlashing && s_FramesSinceOperation >= 2)
{
    s_IsFlashing = false;  // Reset trigger immediately
    
    // Don't launch if already running
    if (s_FlashThreadRunning)
    {
        AddLog("[WARNING] Flash operation already in progress!");
        s_FramesSinceOperation = 0;
    }
    else
    {
        s_FlashThreadRunning = true;  // Mark thread as running
        
        std::thread flashThread([&]() {
            // ... perform flash operation ...
            
            s_FlashThreadRunning = false;  // Mark thread as finished
        });
        
        flashThread.detach();
    }
}
```

## Files Modified

### src/UI/Tabs/JTAGFlashTab.cpp

**Changes:**

1. **Added thread tracking flags** (line ~38):
   ```cpp
   static bool s_FlashThreadRunning = false;
   static bool s_VerifyThreadRunning = false;
   ```

2. **Updated operation active check** (line ~82):
   ```cpp
   bool isAnyOperationActive = s_IsDetecting || s_IsFlashing || s_IsVerifying 
                               || s_FlashThreadRunning || s_VerifyThreadRunning;
   ```

3. **Fixed flash operation** (line ~315):
   ```cpp
   if (s_IsFlashing && s_FramesSinceOperation >= 2)
   {
       s_IsFlashing = false;  // CRITICAL: Reset immediately
       
       if (!s_FlashThreadRunning)
       {
           s_FlashThreadRunning = true;
           // ... launch thread ...
           // Thread sets s_FlashThreadRunning = false when done
       }
   }
   ```

4. **Fixed verify operation** (line ~360):
   ```cpp
   if (s_IsVerifying && s_FramesSinceOperation >= 2)
   {
       s_IsVerifying = false;  // CRITICAL: Reset immediately
       
       if (!s_VerifyThreadRunning)
       {
           s_VerifyThreadRunning = true;
           // ... launch thread ...
           // Thread sets s_VerifyThreadRunning = false when done
       }
   }
   ```

5. **Updated button states** to include thread running flags:
   - Detect button: Disabled when `s_FlashThreadRunning || s_VerifyThreadRunning`
   - Flash button: Disabled when `s_FlashThreadRunning || s_VerifyThreadRunning`
   - Verify button: Disabled when `s_FlashThreadRunning || s_VerifyThreadRunning`
   - Button labels show "Flashing..." or "Verifying..." when thread is running

## Testing

### Test Cases

1. **Single Flash Operation**
   - Click "Program Firmware"
   - Operation should execute exactly once
   - No duplicate log messages
   - Only one OpenOCD process spawned

2. **Single Verify Operation**
   - Click "Verify Firmware"
   - Operation should execute exactly once
   - No duplicate log messages
   - Only one OpenOCD process spawned

3. **Button Spamming**
   - Rapidly click "Program Firmware" multiple times
   - Only one operation should execute
   - UI should show "Flashing..." during operation
   - Button should be disabled during operation

4. **Error Handling**
   - Trigger a flash error (wrong firmware, disconnected device, etc.)
   - Error should be reported exactly once
   - No runaway loop
   - Single OpenOCD process should cleanly exit

### Verification

After applying the fix:
- ? Flash operations execute exactly once
- ? No infinite loops
- ? No process accumulation
- ? Clean error handling
- ? Proper button state management
- ? Accurate progress reporting

## Related Issues

This fix also resolves:
- OpenOCD process leaks
- Memory usage spikes during flash operations
- Application hangs requiring force-kill
- Need for manual process cleanup scripts

## Pattern Application

The same pattern was applied to both:
- **Flash Programming** (`ProgramFirmware`)
- **Firmware Verification** (`VerifyFirmware`)

This pattern should be used for any future async operations in the Flash tab or other tabs.

## Prevention

To prevent similar issues in the future:

1. **Always reset trigger flags immediately** before launching async operations
2. **Use separate state tracking** for operation execution vs. UI triggers
3. **Add re-entry guards** for async operations
4. **Test with rapid button clicking** to catch race conditions
5. **Monitor process spawning** during development
6. **Use proper state machines** for complex async flows

## Keywords

- Infinite loop
- Runaway process
- OpenOCD leak
- Flash programming
- Race condition
- Thread management
- Async operations
- Re-entry protection
- State machine
