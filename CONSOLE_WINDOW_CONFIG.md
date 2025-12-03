# Console Window Configuration

## Summary
The project has been configured to show the console window in **Debug** mode and hide it in **Release** mode.

## Configuration Changes

### Debug Build (Development)
- **SubSystem**: `Console`
- **Console Window**: VISIBLE ?
- **Debug Logs**: All `std::cout` messages visible in console
- **Use Case**: Development, debugging, testing

### Release Build (Distribution)
- **SubSystem**: `Windows`
- **Console Window**: HIDDEN ?
- **Debug Logs**: Not visible (clean UI for end users)
- **Use Case**: Distribution to end users

## Files Modified

### DMATool.vcxproj
**Line 67** (Debug Configuration):
```xml
<SubSystem>Console</SubSystem>
```

**Line 87** (Release Configuration):
```xml
<SubSystem>Windows</SubSystem>  <!-- Changed from Console -->
```

**Backup Created**: `DMATool.vcxproj.backup_20251203_123601`

## Next Steps

1. **Close the solution** in Visual Studio
2. **Reopen** `DMATool.vcxproj`
3. **Test Debug build**:
   - Build in Debug configuration
   - Run the application
   - Console window should appear with debug logs
4. **Test Release build**:
   - Build in Release configuration
   - Run the application
   - No console window should appear (clean UI only)

## Alternative Logging for Release Mode

If you need to debug Release builds, you have several options:

### Option 1: Use OutputDebugString (Recommended)
Instead of `std::cout`, use Windows debug output:
```cpp
#include <windows.h>

void DebugLog(const std::string& message) {
    OutputDebugStringA((message + "\n").c_str());
}

// Usage:
DebugLog("[INFO] Application started");
```

View logs with **DebugView** tool (download from Microsoft Sysinternals).

### Option 2: Log to File
Create a log file for Release mode:
```cpp
#ifdef NDEBUG  // Release mode
    std::ofstream logFile("DMATool_log.txt", std::ios::app);
    logFile << "[INFO] Message here" << std::endl;
#else  // Debug mode
    std::cout << "[INFO] Message here" << std::endl;
#endif
```

### Option 3: Temporarily Switch to Console
For troubleshooting Release builds, temporarily change line 87 back to `Console`, rebuild, and test.

## Build Comparison

| Feature | Debug Build | Release Build |
|---------|-------------|---------------|
| Console Window | ? Visible | ? Hidden |
| Optimization | Off | Full |
| Debug Symbols | Full | Minimal |
| Size | Larger | Smaller |
| Performance | Slower | Faster |
| Logs Visible | ? Yes | ? No |
| For Users | ? No | ? Yes |

## Troubleshooting

### Console still appears in Release mode
1. Make sure you're building **Release** configuration (not Debug)
2. Clean solution: `Build` ? `Clean Solution`
3. Rebuild: `Build` ? `Rebuild Solution`
4. Check line 87 in `DMATool.vcxproj` is `<SubSystem>Windows</SubSystem>`

### Need logs in Release mode
Use **OutputDebugString** + **DebugView** as described above.

### Want to revert changes
```powershell
Copy-Item DMATool.vcxproj.backup_20251203_123601 DMATool.vcxproj -Force
```

---
Generated: 2025-12-03 12:36:01
