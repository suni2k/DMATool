# CORRECT Location for "Additional Manifest Files"

## You're Currently Looking at the WRONG Page!

You're on: **Linker ? Manifest File** ?

This page only shows:
- Generate Manifest
- Manifest File
- Additional Manifest Dependencies
- Allow Isolation
- Enable User Account Control (UAC)
- UAC Execution Level
- UAC Bypass UI Protection

**BUT** this page does NOT have "Additional Manifest Files"!

## Where to Actually Find It

### Option 1: Linker ? Input Page
1. In the left panel, click **Linker** (expand if collapsed)
2. Click **Input** (under Linker)
3. Look for **"Additional Manifest Files"** in the property grid
4. Enter: `$(ProjectDir)app.manifest`
5. Apply ? OK

### Option 2: Linker ? General Page  
1. In the left panel, click **Linker** (expand if collapsed)
2. Click **General** (under Linker)
3. Scroll down to find **"Additional Manifest Files"**
4. Enter: `$(ProjectDir)app.manifest`
5. Apply ? OK

### Option 3: Search for It
1. At the top of the Property Pages window, there's a **Search** box
2. Type: `manifest files`
3. It should highlight the correct property
4. Enter: `$(ProjectDir)app.manifest`
5. Apply ? OK

## Alternative: Just Edit the .vcxproj File Manually

Since Visual Studio's property pages are confusing, here's the **simplest approach**:

1. **Close Visual Studio**
2. Open `DMATool.vcxproj` in **Notepad**
3. Find the `<Link>` section for Debug (around line 55)
4. Add this line inside `<Link>`:
   ```xml
   <AdditionalManifestFiles>$(ProjectDir)app.manifest</AdditionalManifestFiles>
   ```
5. Find the `<Link>` section for Release (around line 70)
6. Add the same line
7. **Save the file**
8. **Reopen Visual Studio**
9. **Build ? Rebuild Solution**

## Example of What to Add

```xml
<Link>
  <SubSystem>Console</SubSystem>
  <GenerateDebugInformation>true</GenerateDebugInformation>
  <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
  <AdditionalManifestFiles>$(ProjectDir)app.manifest</AdditionalManifestFiles>  <!-- ADD THIS LINE -->
</Link>
```

## Why This Is Confusing

Visual Studio has **TWO** different manifest-related properties:
- **Additional Manifest Dependencies** (on Manifest File page) - for DLL dependencies
- **Additional Manifest Files** (on Input or General page) - for custom manifest files ? **THIS IS WHAT YOU NEED**

They're on different pages! ??

## Recommendation

**Just edit the .vcxproj file manually** - it's faster and clearer than hunting through VS property pages!
