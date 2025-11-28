# DMATool - Setup Guide

## Quick Start

This guide will help you set up and build the DMATool application using **DirectX 11** for native Windows rendering.

## Prerequisites

1. **Visual Studio 2019 or later** with:
   - C++ Desktop Development workload
   - Windows 10 SDK (includes DirectX 11)

2. **Git** (for cloning dependencies)

## Step 1: Set Up Dependencies

### Download ImGui

1. Go to https://github.com/ocornut/imgui/releases
2. Download the latest release (v1.89 or later)
3. Extract to `DMATool/vendor/imgui/`

Your directory structure should look like:
```
DMATool/
??? vendor/
?   ??? imgui/
?       ??? imgui.h
?       ??? imgui.cpp
?       ??? imgui_demo.cpp
?       ??? imgui_draw.cpp
?       ??? imgui_tables.cpp
?       ??? imgui_widgets.cpp
?       ??? imgui_internal.h
?       ??? imconfig.h
?       ??? imstb_rectpack.h
?       ??? imstb_textedit.h
?       ??? imstb_truetype.h
?       ??? backends/
?           ??? imgui_impl_win32.h
?           ??? imgui_impl_win32.cpp
?           ??? imgui_impl_dx11.h
?           ??? imgui_impl_dx11.cpp
```

**Note**: We're using **DirectX 11** with **Win32** backends (not OpenGL/GLFW). This provides native Windows rendering with excellent performance.

### DirectX 11 SDK

DirectX 11 is included with the Windows 10 SDK, which should already be installed with Visual Studio. No additional downloads needed!

## Step 2: Verify Directory Structure

Your final structure should be:

```
DMATool/
??? src/
?   ??? main.cpp
?   ??? Application.h
?   ??? Application.cpp
?   ??? UI/
?   ?   ??? Theme.h
?   ?   ??? Theme.cpp
?   ?   ??? MainWindow.h
?   ?   ??? MainWindow.cpp
?   ?   ??? Tabs/
?   ?       ??? JTAGPortTab.h
?   ?       ??? JTAGPortTab.cpp
?   ?       ??? JTAGFlashTab.h
?   ?       ??? JTAGFlashTab.cpp
?   ?       ??? DataPortTab.h
?   ?       ??? DataPortTab.cpp
?   ??? Backend/
?       ??? ProjectManager.h
?       ??? ProjectManager.cpp
?       ??? DMAInterface.h
?       ??? DMAInterface.cpp
?       ??? JTAGInterface.h
?       ??? JTAGInterface.cpp
??? vendor/
?   ??? imgui/
?       ??? [ImGui files as shown above]
??? DMATool.sln
??? DMATool.vcxproj
??? DMATool.vcxproj.filters
??? README.md
```

## Step 3: Build the Project

1. Open `DMATool.sln` in Visual Studio

2. Select build configuration:
   - **Debug|x64** for development
   - **Release|x64** for optimized builds

3. Build the solution:
   - Press `F7` or
   - Menu: Build ? Build Solution

4. The executable will be created in:
   - Debug: `bin/Debug-x64/DMATool.exe`
   - Release: `bin/Release-x64/DMATool.exe`

## Step 4: Run the Application

1. Press `F5` to run with debugging, or `Ctrl+F5` to run without debugging

2. The application window should appear with:
   - Native Windows window with DirectX 11 rendering
   - A startup dialog prompting to create or load a project
   - Three tabs: JTAG Port of DMA, JTAG Flash Tool, Data Port of DMA
   - Smooth 60 FPS rendering with vsync

## DirectX 11 Features

? **Native Windows Performance** - Direct hardware acceleration  
? **Better Multi-Monitor Support** - ImGui viewports enabled  
? **Familiar DirectX Pipeline** - If you've used DirectX before  
? **Windows Graphics Debugger** - Use Visual Studio's graphics tools  
? **No External Dependencies** - Everything included with Windows SDK  

## Troubleshooting

### Error: Cannot open include file 'imgui.h'

**Solution**: Verify that ImGui files are in `vendor/imgui/` and the include paths in the project are correct.

### Error: Cannot open include file 'd3d11.h'

**Solution**: 
- Ensure Windows 10 SDK is installed (comes with Visual Studio)
- Check that SDK version matches in project settings

### Linker Error: Cannot open file 'd3d11.lib'

**Solution**: 
- Verify Windows 10 SDK is installed
- Check project properties ? Linker ? Input ? Additional Dependencies includes `d3d11.lib`

### Application crashes on startup with DirectX error

**Solution**:
- Ensure you're building for x64 (not x86)
- Update your graphics drivers
- Check Windows Update for latest DirectX runtime
- Try running as Administrator

### Black screen or no rendering

**Solution**:
- Verify DirectX 11 support: Run `dxdiag` and check DirectX version
- Update graphics drivers
- Check Event Viewer for DirectX errors

## Performance Tips

### For Best Performance:
1. Build in **Release** mode for production
2. Enable vsync (already set to 1 in Present call)
3. Use **D3D11_CREATE_DEVICE_DEBUG** flag only in Debug builds
4. Profile with Visual Studio Graphics Debugger if needed

### Graphics Debugging:
1. Graphics ? Start Diagnostics (Alt+F5)
2. Capture frames to analyze DirectX calls
3. Check render target views and shader pipeline
4. Verify draw calls and state changes

## Next Steps

Once the application is running:

1. **Explore the UI**: Navigate through the three tabs to see the layout
2. **Review the code**: Check out the DirectX 11 initialization in `Application.cpp`
3. **Use Graphics Debugger**: Capture frames to see the rendering pipeline
4. **Plan integration**: See README.md for planned hardware integrations

## Why DirectX 11?

**Perfect for Windows-focused tools like DMATool:**
- Native Windows API integration
- Better compatibility with Windows Driver Kit (WDK)
- Excellent debugging tools in Visual Studio
- Familiar if you've worked with DirectX before
- No external library dependencies (GLFW, etc.)

## Hardware Integration (Future)

This is currently a UI skeleton. To integrate actual hardware:

1. **For DMA operations**:
   - Install LeechCore library
   - Link against leechcore.dll
   - Implement real methods in `DMAInterface.cpp`

2. **For JTAG operations**:
   - Install CH347 drivers
   - Link against CH347DLL.dll
   - Implement real methods in `JTAGInterface.cpp`

See the repository links in README.md for detailed integration guides.

## Advanced: Graphics Debugging

Since you're using DirectX, you have access to powerful debugging tools:

```cpp
// In Debug builds, enable graphics debugging
#ifdef _DEBUG
createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
```

**To use Graphics Debugger:**
1. Set breakpoint in rendering code
2. Graphics ? Start Graphics Debugging
3. Capture frame with Print Screen
4. Analyze pipeline, shaders, buffers, textures
5. Check performance metrics

This is invaluable for debugging complex UI layouts!
