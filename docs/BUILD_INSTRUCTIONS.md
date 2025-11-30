# DMATool - Build Instructions

## Prerequisites

### Required Software
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Workload: "Desktop development with C++"
  - C++17 standard support
- **Git** (for cloning repository)
- **Windows 10/11** (x64)

### Optional Software
- **CMake 3.20+** (if using CMake build system)
- **Windows SDK** (usually included with Visual Studio)

## Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/akwanmn/DMATool.git
cd DMATool
```

### 2. Open in Visual Studio
```bash
# Double-click to open
DMATool.sln
```

Or from Visual Studio:
- File ? Open ? Project/Solution
- Navigate to `DMATool.sln`
- Click "Open"

### 3. Select Configuration
- **Debug** - For development (slower, includes debug symbols)
- **Release** - For production (optimized, smaller binary)

Configuration dropdown in toolbar:
```
[Debug ?] [x64 ?]
```

### 4. Build
**Method 1: Keyboard Shortcut**
```
Ctrl + Shift + B
```

**Method 2: Menu**
```
Build ? Build Solution
```

**Method 3: Right-click**
```
Solution Explorer ? Right-click "DMATool" ? Build
```

### 5. Run
**Method 1: Keyboard Shortcut**
```
F5 (Debug mode - with debugger)
Ctrl + F5 (Release mode - without debugger)
```

**Method 2: Menu**
```
Debug ? Start Debugging (F5)
Debug ? Start Without Debugging (Ctrl+F5)
```

## Build Output

### Output Locations
```
DMATool/
?? bin/
?  ?? Debug-x64/
?  ?  ?? DMATool.exe         # Debug build
?  ?? Release-x64/
?     ?? DMATool.exe         # Release build
?? bin-int/                  # Intermediate files (obj)
```

### Standalone Distribution
**What to include:**
```
Distribution/
?? DMATool.exe               # Main executable
?? tools/
   ?? ch347/
      ?? drivers/            # Driver files (optional - embedded in .exe)
         ?? CH341WDM.INF
         ?? CH341WDM.SYS
         ?? CH341DLL.DLL
         ?? CH341DLLA64.DLL
         ?? ... (all driver files)
```

**Note:** OpenOCD and config files are **embedded in DMATool.exe** - no external files needed!

## Build Configurations

### Debug Build
- **Optimization:** Disabled (`/Od`)
- **Debug Info:** Full (`/Zi`)
- **Runtime:** Debug DLL (`/MDd`)
- **Size:** ~2-3 MB
- **Speed:** Slower (no optimizations)
- **Use Case:** Development, debugging

### Release Build
- **Optimization:** Maximum (`/O2`)
- **Debug Info:** None or minimal
- **Runtime:** Release DLL (`/MD`)
- **Size:** ~800 KB - 1 MB
- **Speed:** Optimized
- **Use Case:** Production, distribution

## Dependencies

### Automatically Handled
The following are included in the repository (`vendor/` folder):
- **ImGui** - UI library
- **GLFW** - Window management
- **glad** - OpenGL loader

### Embedded Resources
The following are embedded in `DMATool.exe` at build time:
- `openocd.exe` - JTAG interface tool
- `libusb-1.0.dll` - USB library
- `libhidapi-0.dll` - HID library
- OpenOCD configuration files (`.cfg`)

**No external dependencies at runtime!**

## Common Build Issues

### Issue 1: "Cannot open include file: 'imgui.h'"
**Cause:** ImGui vendor path not set correctly

**Solution:**
1. Check `DMATool.vcxproj` includes:
   ```xml
   <AdditionalIncludeDirectories>
     vendor\imgui;
     vendor\imgui\backends;
     vendor\glfw\include;
     vendor\glad\include;
   </AdditionalIncludeDirectories>
   ```

2. Verify files exist:
   ```
   vendor/imgui/imgui.h
   vendor/imgui/backends/imgui_impl_glfw.h
   vendor/imgui/backends/imgui_impl_opengl3.h
   ```

### Issue 2: "LNK1104: cannot open file 'glfw3.lib'"
**Cause:** GLFW library not built or path incorrect

**Solution:**
1. Check `vendor/glfw/lib/` contains:
   - `glfw3.lib` (Release)
   - `glfw3d.lib` (Debug)

2. If missing, rebuild GLFW:
   ```bash
   cd vendor/glfw
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

### Issue 3: "RC.exe not found"
**Cause:** Windows SDK not installed or path incorrect

**Solution:**
1. Open Visual Studio Installer
2. Modify Visual Studio 2022
3. Ensure "Windows 10 SDK" is checked
4. Click "Modify" to install

### Issue 4: "Embedded resource not found"
**Cause:** Resource files missing from `dmafiles/` folder

**Solution:**
1. Verify files exist:
   ```
   dmafiles/ch347/CH347FPGATool/OpenOCD_CH347/bin/openocd.exe
   dmafiles/ch347/CH347FPGATool/OpenOCD_CH347/bin/*.dll
   dmafiles/ch347/CH347FPGATool/OpenOCD_CH347/bin/*.cfg
   ```

2. Check `DMATool.rc` has correct paths:
   ```rc
   IDR_OPENOCD_EXE RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\openocd.exe"
   ```

### Issue 5: "Driver files not found at runtime"
**Cause:** Driver files missing from `tools/ch347/drivers/`

**Solution:**
1. Copy complete driver package:
   ```
   Source: C:\Users\[You]\Desktop\dma\75T-driver\Driver Installer (open)\
   Dest:   C:\Users\[You]\source\repos\DMATool\tools\ch347\drivers\
   ```

2. Verify all files present:
   - `CH341WDM.INF`
   - `CH341WDM.SYS`
   - `CH341DLL.DLL` ? Important!
   - `CH341DLLA64.DLL` ? Important!
   - `CH347DLL.DLL`
   - `CH347DLLA64.DLL`

## Advanced Build Options

### CMake Build (Optional)
```bash
# Generate Visual Studio project
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release

# Output: build/Release/DMATool.exe
```

### Command Line Build
```bash
# Open "Developer Command Prompt for VS 2022"
cd C:\path\to\DMATool

# Build Release
msbuild DMATool.sln /p:Configuration=Release /p:Platform=x64

# Build Debug
msbuild DMATool.sln /p:Configuration=Debug /p:Platform=x64
```

### Clean Build
```bash
# Visual Studio
Build ? Clean Solution

# Command Line
msbuild DMATool.sln /t:Clean
```

## Packaging for Distribution

### Create Release Package
1. Build in **Release** configuration
2. Copy `bin/Release-x64/DMATool.exe` to distribution folder
3. (Optional) Include `tools/ch347/drivers/` folder
4. Create ZIP or installer

### Example Distribution Structure
```
DMATool-v1.0-Release.zip
?? DMATool.exe               # Main executable (~800KB)
?? README.md                 # User guide
?? tools/                    # (Optional - already in .exe)
   ?? ch347/
      ?? drivers/
         ?? ... (driver files)
```

### Installer Creation (Future)
- **Inno Setup** - Free installer creator
- **WiX Toolset** - Windows Installer XML
- **NSIS** - Nullsoft Scriptable Install System

## Testing the Build

### Quick Test Checklist
1. ? Application starts without errors
2. ? Splash screen displays correctly
3. ? "ENTER TOOL" button works
4. ? DNA ID tab loads
5. ? Driver check works (no errors in console)
6. ? Auto-detection runs (if driver installed)
7. ? Manual detection button clickable
8. ? Copy DNA button clickable

### With CH347 Hardware
1. ? Plug in CH347 adapter
2. ? Driver installation works
3. ? FPGA detection succeeds
4. ? DNA ID extracted correctly
5. ? Copy to clipboard works

## Troubleshooting

### Runtime Errors

**Error: "openocd.exe not found"**
- Check resource extraction: `%TEMP%\DMATool\openocd.exe` should exist
- Verify `DMATool.rc` has `IDR_OPENOCD_EXE` defined

**Error: "Driver files not found"**
- Check `tools/ch347/drivers/` folder exists
- Verify all required DLL files present (especially `CH341DLL.DLL`)

**Error: "Access Denied" when installing driver**
- Run DMATool as Administrator
- Or approve UAC prompt when it appears

### Performance Issues

**Slow auto-detection**
- Normal: ~3 seconds for complete detection
- If longer: Check if multiple USB devices connected

**UI freezes during detection**
- Normal: Operations run synchronously
- UI updates every frame but input blocked during operations

## Build Metrics

### Typical Build Times
- **Clean Build (Debug):** ~10-15 seconds
- **Clean Build (Release):** ~15-20 seconds
- **Incremental Build:** ~2-5 seconds
- **Full Rebuild:** ~20-30 seconds

### Binary Sizes
- **Debug Build:** ~2-3 MB
- **Release Build:** ~800 KB - 1 MB
- **With Embedded Resources:** +5 MB (OpenOCD, DLLs, configs)

## CI/CD (Future)

### GitHub Actions
```yaml
# Future: .github/workflows/build.yml
name: Build DMATool
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - uses: microsoft/setup-msbuild@v1
      - run: msbuild DMATool.sln /p:Configuration=Release
      - uses: actions/upload-artifact@v2
        with:
          name: DMATool-Release
          path: bin/Release-x64/DMATool.exe
```

---

**Need Help?**
- **Discord:** https://discord.gg/MfH9UHxkdP
- **GitHub Issues:** https://github.com/akwanmn/DMATool/issues
- **Documentation:** See `docs/` folder

**Last Updated:** January 2025
**Version:** 1.0
