# Console Window Hidden in Release Build

## Problem
The Release build was showing a black console window when launching the application, even though it's a GUI application.

## Solution
Configured the Release build to use the **Windows subsystem** with the **mainCRTStartup** entry point. This allows the app to:
- Use the existing `main()` function (no code changes needed)
- Hide the console window in Release builds
- Keep the console visible in Debug builds for logging

## Configuration

### Debug Build
```xml
<SubSystem>Console</SubSystem>
<!-- No EntryPointSymbol needed - uses main() with console -->
```
- ? Console window **VISIBLE**
- ? All `std::cout` debug logs visible
- ? Perfect for development and debugging

### Release Build
```xml
<SubSystem>Windows</SubSystem>
<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
```
- ? Console window **HIDDEN**
- ? Clean GUI-only experience for end users
- ? Still uses `main()` function (no WinMain needed)
- ? Requires administrator privileges

## How It Works
The `mainCRTStartup` entry point tells the linker:
- "Use the Windows subsystem (no console)"
- "But start execution at `main()` instead of `WinMain()`"

This is the standard way to create a GUI app that uses `main()` instead of `WinMain()`.

## Verification
Both builds now:
- ? Compile successfully
- ? Require administrator privileges
- ? Work correctly with the existing `main()` entry point

**Debug**: Shows console for logging
**Release**: No console, clean GUI only

## Files Modified
- `DMATool.vcxproj` - Added `<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>` to Release configuration

## Scripts Created
- `scripts/Hide-Release-Console.ps1` - Automates the configuration

## Testing
1. Build Release configuration
2. Run `bin\Release-x64\DMATool.exe`
3. Verify:
   - UAC prompt appears (admin required) ?
   - NO console window appears ?
   - Only the ImGui/DirectX window shows ?

---
**Note**: If you need to debug the Release build, you can:
1. Use `OutputDebugString()` + DebugView tool
2. Implement file-based logging
3. Temporarily change Release SubSystem to Console for testing
