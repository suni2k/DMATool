# Quick Guide: Publishing DMATool to GitHub

## Step 1: Run the Setup Script

```powershell
cd C:\Users\suni\source\repos\DMATool
.\scripts\Setup-Public-Repo.ps1
```

This creates a clean public repository at: `C:\Users\suni\source\repos\DMATool-Public\`

---

## Step 2: Create GitHub Repository

1. **Go to**: https://github.com/new

2. **Fill in**:
   - **Repository name**: `DMATool`
   - **Description**: `Professional DMA Card Management Suite - JTAG, Flash, and Benchmark Tools`
   - **Visibility**: ? **Public**
   - **Initialize**: ? Do NOT check "Add README" (we have one!)

3. **Click**: "Create repository"

---

## Step 3: Push to GitHub

Open PowerShell and run:

```powershell
cd ..\DMATool-Public
git remote add origin https://github.com/suni2k/DMATool.git
git push -u origin main
```

---

## Step 4: Create First Release

1. **Go to**: https://github.com/suni2k/DMATool/releases/new

2. **Fill in**:
   - **Tag**: `v1.0.0`
   - **Release title**: `DMATool v1.0.0 - Initial Release`
   - **Description**:
     ```markdown
     ## ?? DMATool v1.0.0 - Initial Release

     Professional DMA card management suite with JTAG, Flash, and Benchmark capabilities.

     ### ? Features
     - ? FPGA Detection (XC7A35T, XC7A75T, XC7A100T)
     - ? DNA ID Reading
     - ? Firmware Programming
     - ? DMA Benchmarking
     - ? CH347 Driver Management
     - ? Protected with VMProtect Ultimate

     ### ?? Download
     Download `DMATool.exe` below and run as Administrator.

     ### ?? System Requirements
     - Windows 10/11 (64-bit)
     - Administrator privileges
     - DMA card (PCILeech-compatible)
     - JTAG adapter (CH347 or FTDI)

     ### ??? Security
     This release is protected against reverse engineering using VMProtect Ultimate.

     ---
     **Full documentation**: https://github.com/suni2k/DMATool#readme
     ```

3. **Attach Files**:
   - Click "Attach binaries by dropping them here"
   - Upload `DMATool-Public\releases\DMATool.exe`

4. **Click**: "Publish release"

---

## Step 5: Verify Everything

### Check Repository
Visit: https://github.com/suni2k/DMATool

You should see:
- ? Professional README with screenshots
- ? LICENSE file
- ? assets/ folder with images
- ? .github/ templates

### Check Release
Visit: https://github.com/suni2k/DMATool/releases/latest

You should see:
- ? v1.0.0 release
- ? DMATool.exe download link
- ? Release notes

---

## Optional: Add Screenshots

If you haven't already, copy your screenshots to the assets folder:

```powershell
# Copy screenshots from your main repo
Copy-Item "C:\Users\suni\source\repos\DMATool\assets\*" `
          "C:\Users\suni\source\repos\DMATool-Public\assets\" -Force

# Commit and push
cd ..\DMATool-Public
git add assets/
git commit -m "Add application screenshots"
git push
```

**Recommended screenshots**:
- `jtag_tab.png` - JTAG port interface
- `flash_tab.png` - Flash programming interface
- `data_tab.png` - Benchmarking interface
- `banner.png` - Header image for README

---

## Repository Structure

After setup, your public repo will look like:

```
DMATool-Public/
??? README.md                    # Professional documentation
??? LICENSE                      # Proprietary license
??? .gitignore                   # Git ignore rules
??? assets/                      # Screenshots
?   ??? jtag_tab.png
?   ??? flash_tab.png
?   ??? data_tab.png
?   ??? banner.png
??? releases/                    # Contains exe (for reference)
?   ??? DMATool.exe
??? .github/
    ??? ISSUE_TEMPLATE/          # Bug/feature templates
        ??? bug_report.md
        ??? feature_request.md
```

---

## Future Updates

### When you release a new version:

1. **Build and protect** the new exe
2. **Update version** in README.md
3. **Commit changes**:
   ```powershell
   cd ..\DMATool-Public
   git add README.md releases/DMATool.exe
   git commit -m "Release v1.1.0: Added new features"
   git push
   ```
4. **Create new release** on GitHub with new tag (e.g., `v1.1.0`)

---

## Troubleshooting

### "Remote already exists"
```powershell
git remote remove origin
git remote add origin https://github.com/suni2k/DMATool.git
```

### "Authentication failed"
1. Make sure you're logged into GitHub in your browser
2. Or use Personal Access Token instead of password

### Screenshots not showing in README
1. Make sure files are in `assets/` folder
2. Commit and push the assets folder
3. Check image paths in README.md match your filenames

---

## Summary Checklist

- [ ] Run `Setup-Public-Repo.ps1` script
- [ ] Create GitHub repository (Public, no README)
- [ ] Push to GitHub
- [ ] Create v1.0.0 release
- [ ] Upload DMATool.exe to release
- [ ] Add screenshots to assets/ folder
- [ ] Verify README displays correctly
- [ ] Test download link works

---

**You're done!** Your DMATool is now publicly available on GitHub! ??

Visit: **https://github.com/suni2k/DMATool**
