# DMATool - Post-Cleanup Quick Reference Card

## ?? Where to Find Things

### Documentation
```
README.md                          # ? Start here
QUICK_REFERENCE.md                 # Quick commands
CLEANUP_COMPLETE.md                # What was cleaned
WORKSPACE_SUMMARY.md               # Project structure

docs/
??? INDEX.md                       # ?? Complete doc index
??? guides/
?   ??? SETUP_GUIDE.md            # Initial setup
?   ??? TROUBLESHOOTING.md        # Problem solving
?   ??? FLASH_TESTING.md          # Flashing guide
??? features/
?   ??? FLASH_TAB.md              # Flash tab docs
?   ??? FTDI_DRIVER.md            # Driver docs
??? development/
    ??? RESOURCE_EMBEDDING.md     # Resource system
    ??? PERFORMANCE.md            # Optimization
```

### Scripts
```
scripts/README.md                  # ?? Scripts guide

Build & Maintenance:
  Clean-Build.ps1                  # Clean artifacts
  Fix-Build-MT-Issue.ps1          # Fix build errors
  Set-Release-NoConsole.ps1       # Console config

Testing:
  Quick-FPGA-Test.ps1             # Test FPGA
  Quick-DMA-Test.ps1              # Test DMA
  Test-FPGAFlash.ps1              # Test flashing

Verification:
  Verify-Resources.ps1            # Check resources
  Verify-Flash.ps1                # Check flash

Debugging:
  Kill-OpenOCD-Processes.ps1      # Kill stuck processes
  Reset-CH347-Adapter.ps1         # Reset adapter
```

### Solution Explorer (After Reopen)
```
Source Files ? Backend, UI, UI\Tabs, Util
Header Files ? Backend, UI, UI\Tabs, Util
vendor ? ImGui\Core, ImGui\Backends
Resources ? RC Files, Manifests
Documentation ? Root Docs, Guides, Features, Development
Scripts ? Build, Testing, Utilities
```

## ?? Common Tasks

| Task | Command/File |
|------|--------------|
| **Getting Started** | `README.md` |
| **Setup Application** | `docs/guides/SETUP_GUIDE.md` |
| **Fix Build Errors** | `.\scripts\Fix-Build-MT-Issue.ps1` |
| **Clean Build** | `.\scripts\Clean-Build.ps1` |
| **Test FPGA** | `.\scripts\Quick-FPGA-Test.ps1` |
| **Find Docs** | `docs/INDEX.md` |
| **Find Scripts** | `scripts/README.md` |
| **Troubleshoot** | `docs/guides/TROUBLESHOOTING.md` |

## ?? Build Configurations

| Config | Console | Optimization | Use For |
|--------|---------|--------------|---------|
| Debug | ? Visible | ? Off | Development |
| Release | ? Hidden | ? On | Distribution |

Details: `CONSOLE_WINDOW_CONFIG.md`

## ?? Archive

**Location**: `archive/20251203/`  
**Contents**: 55 obsolete files (kept for reference)  
**Action**: Can be ignored or deleted after review

## ? Quick Commands

```powershell
# Clean build
.\scripts\Clean-Build.ps1

# Fix build errors
.\scripts\Fix-Build-MT-Issue.ps1

# Test FPGA
.\scripts\Quick-FPGA-Test.ps1

# Verify resources
.\scripts\Verify-Resources.ps1

# Kill stuck OpenOCD
.\scripts\Kill-OpenOCD-Processes.ps1
```

## ?? Solution Explorer

**Filters now match directories!**
- Close Visual Studio
- Reopen `DMATool.vcxproj`
- Solution Explorer = organized ?

## ?? Adding New Files

### Documentation
```powershell
# User guide
docs/guides/YOUR_GUIDE.md

# Feature
docs/features/YOUR_FEATURE.md

# Dev doc
docs/development/YOUR_TOPIC.md

# Update index
# Edit docs/INDEX.md
```

### Scripts
```powershell
# Add script
scripts/YOUR_SCRIPT.ps1

# Update guide
# Edit scripts/README.md
```

## ?? Help

| Issue | Solution |
|-------|----------|
| Build errors | `docs/guides/TROUBLESHOOTING.md` |
| Driver issues | `docs/features/FTDI_DRIVER.md` |
| Flash issues | `docs/guides/FLASH_TESTING.md` |
| Can't find doc | `docs/INDEX.md` |
| Can't find script | `scripts/README.md` |

## ? Checklist

- [ ] Close Visual Studio
- [ ] Reopen DMATool.vcxproj
- [ ] Verify Solution Explorer is organized
- [ ] Read `CLEANUP_COMPLETE.md`
- [ ] Bookmark `docs/INDEX.md`
- [ ] Bookmark `scripts/README.md`
- [ ] (Optional) Commit to Git

## ?? Done!

Workspace is clean, organized, and ready to use!

For full details, see `CLEANUP_COMPLETE.md`
