# QUICK FIX - Project Won't Load

## The Problem
The PowerShell script broke the .vcxproj file format. Visual Studio won't load it.

## The Solution (Pick ONE)

### Fix #1: Manual Edit (30 seconds) ? EASIEST

1. Close Visual Studio
2. Open `DMATool.vcxproj` in **Notepad**
3. Search for `<Link>` (you'll find 2 sections - Debug and Release)
4. In EACH `<Link>` section, add this line:
   ```xml
   <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
   ```
5. Save the file
6. Open Visual Studio - project will load!

**See `PROJECT_LOAD_FIX.md` for examples showing exactly where to add the line.**

### Fix #2: Restore Backup + Use VS GUI

```powershell
Copy-Item "DMATool.vcxproj.backup" "DMATool.vcxproj" -Force
```

Then:
1. Open Visual Studio
2. Right-click project ? Properties
3. Select "All Configurations"
4. Linker ? Manifest File ? Additional Manifest Files: `app.manifest`
5. Apply ? OK

## After Fixing

1. Build ? Clean Solution
2. Build ? Rebuild Solution
3. Check for shield icon on DMATool.exe
4. Run it - UAC prompt should appear

## Files to Read

- `PROJECT_LOAD_FIX.md` - Detailed fix with examples
- `NEXT_STEPS.md` - Updated instructions

## Why It Happened

Visual Studio is picky about .vcxproj formatting. The XML script changed something VS didn't like.

Manual editing is simpler anyway! ??
