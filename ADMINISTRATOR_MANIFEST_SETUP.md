# How to Make DMATool Require Administrator Privileges

## What This Does
When configured, DMATool.exe will display the UAC (User Account Control) prompt when launched, requiring the user to approve administrator privileges. The .exe will also show a shield icon to indicate it requires elevation.

## Steps to Configure

### 1. Add Application Manifest File (ALREADY DONE ?)
The file `app.manifest` has been created in the project root with:
- `requireAdministrator` execution level
- Windows compatibility for Windows 7-11
- DPI awareness for high-resolution displays

### 2. Link Manifest to Project (REQUIRED BEFORE BUILD)

?? **IMPORTANT: You MUST complete this step before rebuilding the project!**

**Method 1: Via Visual Studio IDE (Recommended)**
1. **Close the current build** (if any errors are showing)
2. Open DMATool project in Visual Studio
3. Right-click on **DMATool** project in Solution Explorer ? **Properties**
4. Select **All Configurations** from the Configuration dropdown at the top
5. Go to **Configuration Properties** ? **Linker** ? **Manifest File**
6. Find **Embed Manifest** and set it to **Yes (/MANIFEST)**
7. Find **Additional Manifest Files** and enter: `app.manifest`
   - Note: Use just `app.manifest` (relative path), NOT `$(ProjectDir)app.manifest`
8. Click **Apply**, then **OK**
9. Clean the solution: **Build** ? **Clean Solution**
10. Rebuild: **Build** ? **Rebuild Solution**

**Method 2: Edit .vcxproj File Manually (Advanced)**
1. **Close Visual Studio** (important - VS locks the .vcxproj file)
2. Open `DMATool.vcxproj` in a text editor (Notepad, VS Code, etc.)
3. Find BOTH `<ItemDefinitionGroup>` sections (one for Debug, one for Release)
4. In EACH section, find or add the `<Link>` tag
5. Add these properties inside `<Link>`:
```xml
<Link>
  ...other settings...
  <GenerateManifest>true</GenerateManifest>
  <EmbedManifest>true</EmbedManifest>
  <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
</Link>
```
6. Save the file
7. Reopen Visual Studio
8. Clean and rebuild the solution

**Example of what it should look like:**
```xml
<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <ClCompile>
    ...
  </ClCompile>
  <Link>
    <SubSystem>Windows</SubSystem>
    <GenerateManifest>true</GenerateManifest>
    <EmbedManifest>true</EmbedManifest>
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
  </Link>
</ItemDefinitionGroup>

<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
  <ClCompile>
    ...
  </ClCompile>
  <Link>
    <SubSystem>Windows</SubSystem>
    <EnableCOMDATFolding>true</EnableCOMDATFolding>
    <OptimizeReferences>true</OptimizeReferences>
    <GenerateManifest>true</GenerateManifest>
    <EmbedManifest>true</EmbedManifest>
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
  </Link>
</ItemDefinitionGroup>
```

### 3. Add Manifest to Project Files (Optional but Recommended)
1. In Visual Studio Solution Explorer, right-click project ? **Add** ? **Existing Item**
2. Select `app.manifest`
3. This ensures it's tracked in source control and visible in IDE

### 4. Rebuild Project
1. **Clean Solution**: **Build** ? **Clean Solution**
2. **Rebuild**: **Build** ? **Rebuild Solution**
3. The manifest will be embedded in DMATool.exe

### 5. Verify It Works
1. Navigate to the output folder (e.g., `x64\Debug\` or `x64\Release\`)
2. Find DMATool.exe
3. Check for:
   - **Shield icon** on the .exe file in Windows Explorer
   - Right-click ? **Properties** ? **Compatibility** tab should show manifest controls admin requirement
4. Run DMATool.exe - you should see:
   - **UAC prompt** asking for administrator approval

## Why This Is Needed

DMATool requires administrator privileges for:
- **Driver Installation**: `pnputil` commands need admin rights
- **Driver Uninstallation**: Removing drivers from Windows driver store
- **Hardware Access**: Direct USB/JTAG communication with hardware
- **Registry Access**: Some operations may require registry access
- **System-level Operations**: DMA operations require elevated privileges

## Manifest File Explanation

```xml
<requestedExecutionLevel level="requireAdministrator" uiAccess="false" />
```
- `level="requireAdministrator"` - Forces UAC prompt on launch
- `uiAccess="false"` - Standard access (not for accessibility tools)

**Alternative Levels:**
- `asInvoker` - Run at user's current privilege level (default, no UAC)
- `highestAvailable` - Run with highest available privileges (admin if available, user otherwise)
- `requireAdministrator` - Always require admin (what we use)

## Troubleshooting

### Build Error: "LNK1327: failure during running mt.exe"
This means the manifest file path is incorrect or not properly configured.

**Solution:**
1. Verify `app.manifest` exists in the project root directory (same folder as DMATool.vcxproj)
2. In project properties, use just `app.manifest` (not full path)
3. Clean and rebuild the solution
4. If using manual edit, ensure the `<Link>` section is properly formatted (no syntax errors)

### Build Error: "Cannot find manifest file"
- Ensure `app.manifest` is in the project root directory
- Check the path in project properties is correct
- Try absolute path if relative doesn't work: `$(ProjectDir)app.manifest`

### No UAC Prompt Shows
- Verify manifest is embedded: Right-click .exe ? Properties ? Compatibility
- Rebuild the project (Clean Solution, then Rebuild)
- Check if you're already running as administrator (UAC won't show if you are already admin)

### Shield Icon Not Showing
- Windows may cache icons - restart Explorer or reboot
- Ensure manifest is properly embedded in the .exe
- Check the manifest was successfully linked (no build warnings about mt.exe)

### "mt.exe" Missing or Not Found
- Ensure Windows SDK is properly installed
- In Visual Studio Installer, verify "Windows 10/11 SDK" is installed
- Repair Visual Studio installation if needed

## Files Modified
- ? Created `app.manifest` with administrator requirement
- ? Need to link manifest in project settings (via Visual Studio or manual edit)

## Security Note
Requesting administrator privileges means:
- Users will be warned before running the app
- The app will run with full system access
- Code must be trusted - any vulnerabilities could affect the entire system

This is necessary for DMATool's driver management features.

## Quick Checklist
- [ ] `app.manifest` file exists in project root
- [ ] Manifest linked in project properties (both Debug and Release configs)
- [ ] Project rebuilt successfully (Clean + Rebuild)
- [ ] Shield icon appears on DMATool.exe
- [ ] UAC prompt shows when running the .exe
- [ ] Administrator privileges confirmed in Task Manager
