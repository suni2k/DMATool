# Visual Studio Manifest Configuration - EXACT STEPS

## What You See vs What You Need

### ? WRONG - What's Currently Selected
You have **"Additional Manifest Dependencies"** selected (the dropdown icon is on that row)

### ? CORRECT - What You Need to Select
You need to click on **"Additional Manifest Files"** (the row just below it)

## Exact Steps

1. In the property grid, look for these two rows:
   ```
   Additional Manifest Dependencies    [dropdown icon here - WRONG]
   Additional Manifest Files           [empty - THIS IS THE ONE YOU NEED]
   ```

2. **Click on "Additional Manifest Files"** (not Dependencies!)

3. A text box should appear on the right side

4. Type exactly: `$(ProjectDir)app.manifest`

5. Press **Enter**

6. Click **Apply**

7. Click **OK**

8. **Build ? Rebuild Solution**

## What Each Field Does

| Field | Purpose |
|-------|---------|
| **Additional Manifest Dependencies** | For specifying DLL dependencies (not what we need) |
| **Additional Manifest Files** | For embedding a custom manifest file (THIS is what we need!) |

## Quick Check

After entering the value, the properties should show:
```
Generate Manifest:                  Yes (/MANIFEST)
Manifest File:                      $(IntDir)$(TargetName)$(TargetExt).intermediate.manifest
Additional Manifest Files:          $(ProjectDir)app.manifest    <-- YOU SHOULD SEE THIS
Allow Isolation:                    Yes
Enable User Account Control (UAC):  Yes (/MANIFESTUAC)
```

## Still Not Working?

If the build still fails after this:
1. Close Visual Studio completely
2. Run: `.\scripts\Clean-Build.ps1`
3. Reopen Visual Studio
4. Try the configuration again

The key is clicking the **correct row** - it's easy to click "Dependencies" by mistake! ??
