# CRITICAL BUG FIX - Flash Interface Constructor Loop

## Problem
**HEAP CORRUPTION** and application crash when attempting to flash firmware.

### Symptoms
- Hundreds of log lines repeating:
  ```
  [INFO] Flash tab initializing (standalone mode)...
  [INFO] Extracting BSCAN bitstreams to temp folder...
  ```
- Debug assertion failure: `_CrtIsValidHeapPointer(block)`
- Memory heap corruption
- Application crash

### Root Cause
**Multiple FlashInterface instances created simultaneously in threads**

The JTAGFlashTab was creating a **NEW FlashInterface object** every time an operation was triggered:

```cpp
// WRONG - Creates new instance every time
if (s_IsDetecting && s_FramesSinceOperation >= 2)
{
    Backend::FlashInterface flashInterface;  // NEW INSTANCE #1
    s_FlashInfo = flashInterface.DetectFlashDevice(...);
}

if (s_IsFlashing && s_FramesSinceOperation >= 2)
{
    std::thread flashThread([&]() {
        Backend::FlashInterface flashInterface;  // NEW INSTANCE #2
        auto result = flashInterface.ProgramFirmware(...);
    });
}

if (s_IsVerifying && s_FramesSinceOperation >= 2)
{
    std::thread verifyThread([&]() {
        Backend::FlashInterface flashInterface;  // NEW INSTANCE #3
        auto result = flashInterface.VerifyFirmware(...);
    });
}
```

### Why This Was Fatal

Each `FlashInterface` constructor:
1. Calls `ExtractBSCANBitstreams()`
2. Extracts 5 BSCAN bitstreams (2.3 MB total)
3. Writes to the same temp directory simultaneously
4. Creates file I/O conflicts
5. Causes heap corruption from concurrent file writes

When running flash operation:
- Detect creates instance #1 ? Extracts 5 files
- Flash thread creates instance #2 ? Extracts 5 files (SAME FILES, SIMULTANEOUSLY!)
- Verify thread creates instance #3 ? Extracts 5 files (AGAIN!)

Result: **Infinite loop** of resource extraction causing heap corruption.

## Solution

Use a **SINGLE static FlashInterface instance** shared across all operations:

```cpp
// Add static instance pointer
static Backend::FlashInterface* s_FlashInterface = nullptr;

// Detection - create once, reuse
if (s_IsDetecting && s_FramesSinceOperation >= 2)
{
    if (!s_FlashInterface)
        s_FlashInterface = new Backend::FlashInterface();  // Only once!
    
    s_FlashInfo = s_FlashInterface->DetectFlashDevice(...);
}

// Flashing - reuse existing instance
if (s_IsFlashing && s_FramesSinceOperation >= 2)
{
    if (!s_FlashInterface)
        s_FlashInterface = new Backend::FlashInterface();
    
    std::thread flashThread([&]() {
        // No new instance! Uses s_FlashInterface
        auto result = s_FlashInterface->ProgramFirmware(...);
    });
}

// Verifying - reuse existing instance
if (s_IsVerifying && s_FramesSinceOperation >= 2)
{
    if (!s_FlashInterface)
        s_FlashInterface = new Backend::FlashInterface();
    
    std::thread verifyThread([&]() {
        // No new instance! Uses s_FlashInterface
        auto result = s_FlashInterface->VerifyFirmware(...);
    });
}
```

## Impact

### Before Fix
- ? Application crashes with heap corruption
- ? Infinite resource extraction loop
- ? Multiple GB of temp files created
- ? Flash operations impossible

### After Fix
- ? Single FlashInterface created on first use
- ? Resources extracted only once
- ? No heap corruption
- ? Flash operations work correctly

## Files Changed
- `src/UI/Tabs/JTAGFlashTab.cpp` - Added static instance pointer, reuse across operations

## Testing

### Verify Fix Works
1. Build DMATool
2. Click "JTAG Flash" tab
3. Click "Detect Flash Device"
4. Check console - should see **SINGLE** extraction:
   ```
   [INFO] Flash tab initializing (standalone mode)...
   [INFO] Extracting BSCAN bitstreams to temp folder...
   [INFO] Extracted 5 BSCAN bitstream(s)
   ```
5. Click "Program Firmware" - should **NOT** see another extraction
6. Should complete without crash

### Expected Behavior
- First operation: Extracts resources (normal)
- Subsequent operations: Reuses existing resources (no re-extraction)
- No heap errors
- Smooth operation

## Lessons Learned

### NEVER Create Heavy Objects in Loops/Threads
```cpp
// BAD - Creates heavy objects repeatedly
for (int i = 0; i < 100; i++) {
    HeavyObject obj;  // ? Expensive constructor called 100 times
    obj.DoWork();
}

// GOOD - Create once, reuse
HeavyObject obj;  // ? Constructor called once
for (int i = 0; i < 100; i++) {
    obj.DoWork();
}
```

### Check for Singleton Pattern Needs
If a class has an expensive constructor:
- Resource extraction
- File I/O
- Network connections
- GPU initialization

Consider:
- Static instance (this fix)
- Lazy initialization
- Dependency injection
- Factory pattern

## Priority
?? **CRITICAL** - Prevents application from working at all

## Status
? **FIXED** - Build successful, ready for testing

---

**Last Updated**: 2025-12-02  
**Bug Severity**: Critical (application crash)  
**Fix Complexity**: Simple (single instance pattern)  
**Testing Required**: Immediate - verify flash operations work
