# CH347 Driver Installation Fix

## Problem
When running DMATool.exe on another PC, the CH347 driver installation fails because it searches for driver files in the local directory structure instead of extracting them from embedded resources.

## Solution Applied

### 1. Updated `src/resource.h`
Added resource IDs for missing DLL files:
```cpp
#define IDR_CH341_DLL                   118
#define IDR_CH341_DLL_A64               119
```

### 2. Updated `src/Backend/OpenOCDInterface.cpp`
Modified `InstallCH347Driver()` to:
- Extract all driver files from embedded resources instead of searching the file system
- Extract CH341DLL.DLL and CH341DLLA64.DLL files (previously missing)

### 3. Created `app.manifest`
Added application manifest to require administrator privileges on launch (needed for driver installation).

## MANUAL STEPS REQUIRED

### Step 1: Update DMATool.rc (Driver Files)

**You MUST manually edit `DMATool.rc` to add the missing driver DLL resources:**

Open `DMATool.rc` in a text editor and find this section:
```rc
// CH341/CH347 Driver Files
IDR_CH341_INF RCDATA "tools\\ch347\\drivers\\CH341WDM.INF"
IDR_CH341_SYS RCDATA "tools\\ch347\\drivers\\CH341WDM.SYS"
IDR_CH341_M64_SYS RCDATA "tools\\ch347\\drivers\\CH341M64.SYS"
IDR_CH341_W64_SYS RCDATA "tools\\ch347\\drivers\\CH341W64.SYS"
IDR_CH341_CAT RCDATA "tools\\ch347\\drivers\\CH341WDM.CAT"
IDR_CH347_DLL RCDATA "tools\\ch347\\drivers\\CH347DLL.DLL"
IDR_CH347_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH347DLLA64.DLL"
```

**ADD these two lines** after the existing driver entries (see the yellow highlighted lines in your screenshot):
```rc
IDR_CH341_DLL RCDATA "tools\\ch347\\drivers\\CH341DLL.DLL"
IDR_CH341_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH341DLLA64.DLL"
```

The final section should look like:
```rc
// CH341/CH347 Driver Files
IDR_CH341_INF RCDATA "tools\\ch347\\drivers\\CH341WDM.INF"
IDR_CH341_SYS RCDATA "tools\\ch347\\drivers\\CH341WDM.SYS"
IDR_CH341_M64_SYS RCDATA "tools\\ch347\\drivers\\CH341M64.SYS"
IDR_CH341_W64_SYS RCDATA "tools\\ch347\\drivers\\CH341W64.SYS"
IDR_CH341_CAT RCDATA "tools\\ch347\\drivers\\CH341WDM.CAT"
IDR_CH341_DLL RCDATA "tools\\ch347\\drivers\\CH341DLL.DLL"
IDR_CH341_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH341DLLA64.DLL"
IDR_CH347_DLL RCDATA "tools\\ch347\\drivers\\CH347DLL.DLL"
IDR_CH347_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH347DLLA64.DLL"
```

### Step 2: Link app.manifest to Project (Administrator Privileges)

**Option A: Via Visual Studio (Easiest)**
1. Open DMATool project in Visual Studio
2. Right-click project in Solution Explorer ? **Properties**
3. Go to **Configuration Properties** ? **Linker** ? **Manifest File**
4. Set **Generate Manifest** to **Yes (/MANIFEST)**
5. In **Additional Manifest Files**, add: `$(ProjectDir)app.manifest`
6. Click **Apply** and **OK**

**Option B: Manual Edit of .vcxproj**
Add this to the `<Link>` section in both Debug and Release configurations:
```xml
<Link>
  <GenerateManifest>true</GenerateManifest>
  <AdditionalManifestFiles>$(ProjectDir)app.manifest</AdditionalManifestFiles>
</Link>
```

See `ADMINISTRATOR_MANIFEST_SETUP.md` for detailed instructions.

## After Making Changes

1. Save `DMATool.rc`
2. Link `app.manifest` to project (via Visual Studio or manual edit)
3. Rebuild the solution: `Build > Rebuild Solution`
4. Verify changes:
   - .exe size increased by ~200-300 KB (embedded DLL files)
   - DMATool.exe shows shield icon (administrator required)
   - UAC prompt appears when running the .exe
5. Test on another PC - driver installation should now work without requiring the `tools/` directory

## How It Works Now

When you click "Install CH347 Driver":
1. Tool extracts all 9 driver files from embedded resources to `%TEMP%\DMATool\drivers\`
2. Runs `pnputil /add-driver` pointing to the temp directory
3. Updates the device driver using PowerShell
4. Verifies installation succeeded

The .exe is now truly standalone - no external files required!

## Files Modified/Created
- ? `src/resource.h` - Added IDR_CH341_DLL and IDR_CH341_DLL_A64
- ? `src/Backend/OpenOCDInterface.cpp` - Extract from resources instead of file system
- ? `app.manifest` - Created (requires administrator privileges)
- ? `DMATool.rc` - **NEEDS MANUAL UPDATE** (add 2 lines for DLL files)
- ? `DMATool.vcxproj` - **NEEDS MANUAL UPDATE** (link app.manifest)

## Testing Checklist
- [ ] Build completes successfully
- [ ] .exe shows shield icon in Windows Explorer
- [ ] UAC prompt appears when launching DMATool.exe
- [ ] Driver installation works on a PC without the source code/tools folder
- [ ] All 9 driver files extracted to temp directory
- [ ] pnputil successfully installs driver
- [ ] Device shows correct driver in Device Manager

