# Complete Setup Guide - Driver Embedding & Administrator Privileges

## Overview
This guide combines two important fixes for DMATool:
1. **Embedded Driver Files** - Makes DMATool.exe work on any PC without external files
2. **Administrator Privileges** - Ensures DMATool always runs with required permissions

## Quick Start - What You Need to Do

### Step 1: Update DMATool.rc (Add Missing Driver DLLs) ?? REQUIRED
1. Open `DMATool.rc` in a text editor
2. Find the section:
   ```rc
   // CH341/CH347 Driver Files
   ```
3. Add these two lines (highlighted in yellow in your screenshot):
   ```rc
   IDR_CH341_DLL RCDATA "tools\\ch347\\drivers\\CH341DLL.DLL"
   IDR_CH341_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH341DLLA64.DLL"
   ```
4. The final section should have all 9 driver files:
   ```rc
   // CH341/CH347 Driver Files
   IDR_CH341_INF RCDATA "tools\\ch347\\drivers\\CH341WDM.INF"
   IDR_CH341_SYS RCDATA "tools\\ch347\\drivers\\CH341WDM.SYS"
   IDR_CH341_M64_SYS RCDATA "tools\\ch347\\drivers\\CH341M64.SYS"
   IDR_CH341_W64_SYS RCDATA "tools\\ch347\\drivers\\CH341W64.SYS"
   IDR_CH341_CAT RCDATA "tools\\ch347\\drivers\\CH341WDM.CAT"
   IDR_CH341_DLL RCDATA "tools\\ch347\\drivers\\CH341DLL.DLL"        <-- ADD THIS
   IDR_CH341_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH341DLLA64.DLL"  <-- ADD THIS
   IDR_CH347_DLL RCDATA "tools\\ch347\\drivers\\CH347DLL.DLL"
   IDR_CH347_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH347DLLA64.DLL"
   ```
5. Save the file

### Step 2: Link app.manifest (Enable Administrator Mode) ?? REQUIRED
1. Open DMATool in Visual Studio
2. Right-click **DMATool** project ? **Properties**
3. At the top, select **All Configurations**
4. Go to **Configuration Properties** ? **Linker** ? **Manifest File**
5. Set these values:
   - **Embed Manifest**: `Yes (/MANIFEST)`
   - **Additional Manifest Files**: `app.manifest`
6. Click **Apply** ? **OK**

### Step 3: Clean and Rebuild
1. **Build** ? **Clean Solution**
2. **Build** ? **Rebuild Solution**
3. Wait for build to complete successfully

### Step 4: Verify Everything Works
1. Navigate to output folder (e.g., `x64\Release\`)
2. Check DMATool.exe:
   - ? Has shield icon (admin required)
   - ? File size increased by ~200-300 KB (embedded drivers)
3. Run DMATool.exe:
   - ? UAC prompt appears
   - ? Runs with administrator privileges
4. Test on another PC (without source code):
   - ? Driver installation works without external files

## What Was Changed

### Files Created
| File | Purpose |
|------|---------|
| `app.manifest` | Application manifest requiring administrator privileges |
| `DRIVER_FIX_REQUIRED.md` | Detailed instructions for driver embedding |
| `ADMINISTRATOR_MANIFEST_SETUP.md` | Detailed instructions for admin setup |
| `COMPLETE_SETUP_GUIDE.md` | This file - combined instructions |

### Code Modified
| File | Changes |
|------|---------|
| `src/resource.h` | Added `IDR_CH341_DLL` (118) and `IDR_CH341_DLL_A64` (119) |
| `src/Backend/OpenOCDInterface.cpp` | Modified `InstallCH347Driver()` to extract from resources |
| `DMATool.rc` | ? **NEEDS MANUAL UPDATE** - Add 2 DLL resource lines |
| `DMATool.vcxproj` | ? **NEEDS MANUAL UPDATE** - Link app.manifest |

## How Driver Installation Works Now

### Before (Broken on Other PCs):
```
User clicks "Install Driver"
  ?
Code searches for "tools\ch347\drivers\" folder
  ?
? FAILS - Folder doesn't exist on user's PC
```

### After (Works Everywhere):
```
User clicks "Install Driver"
  ?
Code extracts 9 driver files from embedded resources to %TEMP%\DMATool\drivers\
  ?
pnputil /add-driver installs from temp folder
  ?
? SUCCESS - Driver installed without external files
```

### Files Extracted to Temp:
1. CH341WDM.INF - Driver installation info
2. CH341WDM.SYS - 32-bit driver binary
3. CH341M64.SYS - 64-bit driver binary (macOS)
4. CH341W64.SYS - 64-bit driver binary (Windows)
5. CH341WDM.CAT - Driver signature catalog
6. CH341DLL.DLL - 32-bit driver DLL
7. CH341DLLA64.DLL - 64-bit driver DLL
8. CH347DLL.DLL - CH347 interface DLL (32-bit)
9. CH347DLLA64.DLL - CH347 interface DLL (64-bit)

## Why Administrator Privileges Are Required

DMATool needs admin rights for:
- **Driver Installation**: `pnputil.exe /add-driver` requires admin
- **Driver Removal**: `pnputil.exe /delete-driver` requires admin
- **Device Updates**: PowerShell `Update-PnpDevice` requires admin
- **Hardware Access**: Direct USB/JTAG communication
- **DMA Operations**: System-level memory access

## Troubleshooting

### Build Error: "LNK1327: failure during running mt.exe"
**Cause**: Manifest file not found or path incorrect

**Solution**:
1. Verify `app.manifest` exists in project root (same folder as DMATool.vcxproj)
2. Clean solution: **Build** ? **Clean Solution**
3. Rebuild: **Build** ? **Rebuild Solution**
4. If still fails, check project properties ? Linker ? Manifest File settings

### Shield Icon Not Showing on .exe
**Cause**: Manifest not properly embedded

**Solution**:
1. Right-click DMATool.exe ? **Properties** ? **Compatibility** tab
2. Check if "Run this program as an administrator" shows (controlled by manifest)
3. If not, rebuild the project
4. Restart Windows Explorer (Ctrl+Shift+Esc ? Restart "Windows Explorer")

### UAC Prompt Not Appearing
**Cause**: Already running as administrator

**Solution**:
1. Check Task Manager ? Details tab ? Find DMATool.exe ? Check "Elevated" column
2. If "Yes", you're already admin (UAC won't show again)
3. Close DMATool and run from regular user mode to test UAC

### Driver Installation Still Fails on Other PC
**Cause**: Missing DLL resources not added to DMATool.rc

**Solution**:
1. Verify you added the 2 DLL lines to `DMATool.rc` (Step 1)
2. Rebuild the project
3. Check .exe file size - should be ~200-300 KB larger
4. Use a resource viewer tool to verify DLLs are embedded

## Testing Checklist

Before distributing DMATool.exe to users:

- [ ] `DMATool.rc` updated with 2 DLL resource lines
- [ ] `app.manifest` linked in project properties
- [ ] Project builds successfully (no errors)
- [ ] .exe file size increased by ~200-300 KB
- [ ] Shield icon visible on DMATool.exe
- [ ] UAC prompt appears when launching
- [ ] On a clean PC (without source code):
  - [ ] Driver installation works
  - [ ] All 9 driver files extracted to temp
  - [ ] pnputil succeeds
  - [ ] Device shows correct driver in Device Manager

## What Happens When User Runs DMATool.exe

```
1. User double-clicks DMATool.exe
   ?
2. Windows reads app.manifest embedded in .exe
   ?
3. Manifest says "requireAdministrator"
   ?
4. Windows shows UAC prompt: "Do you want to allow this app to make changes?"
   ?
5. User clicks "Yes"
   ?
6. DMATool.exe launches with admin privileges
   ?
7. User can now install drivers successfully
```

## Distribution Notes

When distributing DMATool.exe to users:
- ? Single .exe file - no other files needed
- ? Works on any Windows PC (Windows 7+)
- ? Automatically requests admin privileges
- ? All drivers embedded - no manual driver downloads
- ? No installer needed - just run the .exe

## Support

If you encounter issues:
1. Check `DRIVER_FIX_REQUIRED.md` for detailed driver embedding info
2. Check `ADMINISTRATOR_MANIFEST_SETUP.md` for detailed manifest info
3. Verify all steps in this guide were completed
4. Check build output window for specific error messages

## Summary

This setup makes DMATool truly standalone and production-ready:
- ?? Single .exe distribution
- ??? Automatic administrator privileges
- ?? All drivers embedded (no external dependencies)
- ? Works on any PC without source code
- ?? Secure driver installation
- ?? Professional user experience

Both changes are **required** for proper driver installation on end-user machines!
