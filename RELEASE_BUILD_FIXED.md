# Release Build Fixed - Subsystem Mismatch Resolved

## Problem
After setting up administrator mode, the **Release build** failed with:
```
error LNK2001: unresolved external symbol WinMain
fatal error LNK1120: 1 unresolved externals
```

## Root Cause
The Release configuration was set to use the **Windows subsystem** (`<SubSystem>Windows</SubSystem>`), which expects a GUI application with a `WinMain()` entry point. However, DMATool uses a `main()` entry point, which requires the **Console subsystem**.

The Debug configuration was correctly set to Console, but Release was not.

## Solution
Changed the Release configuration's subsystem from `Windows` to `Console` to match the Debug configuration and the application's actual entry point.

### Before
```xml
<!-- Debug -->
<SubSystem>Console</SubSystem>

<!-- Release -->
<SubSystem>Windows</SubSystem>  ? Wrong!
```

### After
```xml
<!-- Debug -->
<SubSystem>Console</SubSystem>

<!-- Release -->
<SubSystem>Console</SubSystem>  ? Correct!
```

## Verification
Both Debug and Release builds now:
- ? Build successfully without errors
- ? Use the Console subsystem
- ? Require administrator privileges
- ? Work correctly with `main()` entry point

## Note
Even though it uses the Console subsystem, DMATool is primarily a GUI application (ImGui/DirectX). The console window can be hidden if needed by:
1. Using `FreeConsole()` in code, or
2. Using `/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup` linker flags

But for now, Console subsystem is correct for the current `main()` entry point.
