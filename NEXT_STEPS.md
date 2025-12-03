# Manual Steps to Complete Setup

## Step 1: Update DMATool.rc ?

? You've already added these lines (highlighted in yellow):
```rc
IDR_CH341_DLL RCDATA "tools\\ch347\\drivers\\CH341DLL.DLL"
IDR_CH341_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH341DLLA64.DLL"
```

Good! That's done. ?

## Step 2: Configure the Manifest

?? **UPDATE**: The PowerShell script caused the project to fail loading in Visual Studio.

**Use this simple manual approach instead:**

### Option A: Manual Edit (Recommended - 30 seconds)

1. Close Visual Studio completely
2. Open `DMATool.vcxproj` in **Notepad** or **VS Code** (NOT Visual Studio)
3. Find the `<Link>` section for **Debug** configuration (around line 55)
4. Add this line inside `<Link>`:
   ```xml
   <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
   ```
5. Find the `<Link>` section for **Release** configuration (around line 70)
6. Add the same line inside `<Link>`
7. Save the file

**See `PROJECT_LOAD_FIX.md` for detailed instructions with examples!**

### Option B: Use Visual Studio GUI (Safer)

1. Restore the backup first:
   ```powershell
   Copy-Item "DMATool.vcxproj.backup" "DMATool.vcxproj" -Force
   ```
2. Open Visual Studio
3. Right-click DMATool project ? Properties
4. Select **"All Configurations"** at the top
5. Go to: **Configuration Properties ? Linker ? Manifest File**
6. Set **Additional Manifest Files** to: `app.manifest`
7. Click **Apply** ? **OK**

## Step 3: Rebuild the Project

After manual editing or GUI config:

1. Open Visual Studio
2. Load the project (should work now!)
3. **Build ? Clean Solution**
4. **Build ? Rebuild Solution**

## Step 4: Verify It Worked

Check that:
- ? Project loads successfully in Visual Studio
- ? Build completes successfully (no errors)
- ? DMATool.exe shows a **shield icon** in Windows Explorer
- ? When you run DMATool.exe, a **UAC prompt** appears
- ? File size increased by ~200-300 KB (embedded driver DLLs)

## What Went Wrong?

The PowerShell script modified the .vcxproj file using XML manipulation, which changed the formatting in a way Visual Studio didn't like. 

**The manual approach is simpler and guaranteed to work!**

## Files Created

- ? `PROJECT_LOAD_FIX.md` - **READ THIS for detailed fix instructions**
- ? `.github/copilot-instructions.md` - Instructions for GitHub Copilot
- ? `app.manifest` - Application manifest (already existed)
- ? `COMPLETE_SETUP_GUIDE.md` - Full documentation
- ? `ADMINISTRATOR_MANIFEST_SETUP.md` - Detailed manifest instructions
- ? `DRIVER_FIX_REQUIRED.md` - Driver embedding instructions

## Summary

You're almost done! Just:
1. ? DMATool.rc updated (you did this)
2. ? Manually add manifest line to .vcxproj (see `PROJECT_LOAD_FIX.md`)
3. ? Rebuild the project
4. ? Test it works

**It's just adding one line to two places in the .vcxproj file!** Super simple. ??
