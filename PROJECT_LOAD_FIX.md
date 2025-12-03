# SIMPLE MANUAL FIX - Project Failed to Load

## What Happened
The Configure-Manifest.ps1 script modified the .vcxproj file in a way that Visual Studio doesn't like.

## Quick Fix (30 seconds)

### Step 1: Open the project file in a text editor
1. Close Visual Studio completely
2. Open `DMATool.vcxproj` in **Notepad** or **VS Code** (NOT Visual Studio)

### Step 2: Find and edit the Debug Link section
Find this section (around line 55):
```xml
<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <ClCompile>
    ...
  </ClCompile>
  <Link>
    <SubSystem>Console</SubSystem>
    <GenerateDebugInformation>true</GenerateDebugInformation>
    <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
  </Link>
</ItemDefinitionGroup>
```

**Add this line** inside the `<Link>` section:
```xml
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
```

So it looks like:
```xml
  <Link>
    <SubSystem>Console</SubSystem>
    <GenerateDebugInformation>true</GenerateDebugInformation>
    <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
  </Link>
```

### Step 3: Find and edit the Release Link section
Find this section (around line 70):
```xml
<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
  <ClCompile>
    ...
  </ClCompile>
  <Link>
    <SubSystem>Console</SubSystem>
    <EnableCOMDATFolding>true</EnableCOMDATFolding>
    <OptimizeReferences>true</OptimizeReferences>
    <GenerateDebugInformation>true</GenerateDebugInformation>
    <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
  </Link>
</ItemDefinitionGroup>
```

**Add the same line** inside the `<Link>` section:
```xml
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
```

So it looks like:
```xml
  <Link>
    <SubSystem>Console</SubSystem>
    <EnableCOMDATFolding>true</EnableCOMDATFolding>
    <OptimizeReferences>true</OptimizeReferences>
    <GenerateDebugInformation>true</GenerateDebugInformation>
    <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
    <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
  </Link>
```

### Step 4: Save and test
1. Save the file
2. Open Visual Studio
3. Load the project (should work now!)
4. Build ? Clean Solution
5. Build ? Rebuild Solution

## Alternative: Use the Backup

If you don't want to edit manually:
```powershell
Copy-Item "C:\Users\suni\source\repos\DMATool\DMATool.vcxproj.backup" "C:\Users\suni\source\repos\DMATool\DMATool.vcxproj" -Force
```

Then configure through Visual Studio GUI:
1. Open Visual Studio
2. Load the project
3. Right-click project ? Properties
4. Select "All Configurations"
5. Go to: Configuration Properties ? Linker ? Manifest File
6. Set: **Additional Manifest Files** to `app.manifest`
7. Click Apply ? OK

## Why Did It Fail?

Visual Studio is very picky about .vcxproj file formatting. The XML manipulation in the script changed something (whitespace, encoding, or element order) that VS didn't like.

The manual approach is simpler and guaranteed to work!

## What You're Adding

You're just adding one line to two places in the file:
```xml
<AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
```

That's it! Super simple. ??
