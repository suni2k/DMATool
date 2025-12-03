# FINAL SIMPLE FIX - Manifest Setup

## The Issue
- Build fails with `LNK1327: failure during running mt.exe`
- This happens because Visual Studio has cached manifest settings

## Solution (3 Easy Steps)

### Step 1: Clean Everything
Close Visual Studio, then run:
```powershell
.\scripts\Clean-Build.ps1
```

### Step 2: Configure Manifest Through Visual Studio GUI
1. **Open Visual Studio** (as admin - you're already doing this ?)
2. **Load the DMATool project**
3. **Right-click the project** ? **Properties**
4. At the top, select **"All Configurations"** dropdown
5. Navigate to: **Configuration Properties** ? **Linker** ? **Manifest File**
6. Find **"Additional Manifest Files"** 
7. Type: `$(ProjectDir)app.manifest`
8. Click **Apply** ? **OK**

### Step 3: Rebuild
- **Build** ? **Rebuild Solution**

## Why Visual Studio GUI?
- Visual Studio knows exactly how to configure manifest settings
- No XML editing = no formatting issues
- No PowerShell scripts breaking things
- Just works! ?

## What This Does
- Embeds `app.manifest` into DMATool.exe
- Makes the .exe request administrator privileges on launch
- Shows the UAC shield icon

## If It Still Fails
Try removing and re-adding the manifest:
1. Project Properties ? Linker ? Manifest File
2. Clear the "Additional Manifest Files" field
3. Apply
4. Rebuild (should work now)
5. Add `$(ProjectDir)app.manifest` back
6. Apply ? Rebuild

## Summary
? `app.manifest` exists (you verified this)
? DMATool.rc has driver DLLs (you added them)
? Just need to configure through VS GUI (safest way)

That's it! The GUI is the way to go. ??
