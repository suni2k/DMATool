# DMATool Workspace Summary
*Generated: 2025-12-03 12:41:02*

## Project Structure

```
DMATool/
?? src/                      # Source code
?  ?? Backend/              # Backend logic (OpenOCD, Flash, Drivers)
?  ?? UI/                   # UI components
?  ?  ?? Tabs/              # ImGui tab implementations
?  ?? Util/                 # Utility classes
?? vendor/                   # Third-party libraries
?  ?? imgui/                # ImGui library
?  ?? LeechCore/            # LeechCore (optional)
?? dmafiles/                 # External resources (OpenOCD, drivers, bitstreams)
?? tools/                    # Driver files
?? docs/                     # Documentation
?  ?? guides/               # User guides
?  ?? features/             # Feature documentation
?  ?? development/          # Development guidelines
?? scripts/                  # PowerShell automation scripts
?  ?? Utilities/            # General utilities
?  ?? Testing/              # Test scripts
?  ?? Build/                # Build-related scripts
?? archive/                  # Archived obsolete files
```

## Documentation

### Root Level
- **README.md** - Project overview and quick start
- **QUICK_REFERENCE.md** - Quick reference guide
- **CONSOLE_WINDOW_CONFIG.md** - Console window configuration

### docs/guides/
- **SETUP_GUIDE.md** - Detailed setup instructions
- **TROUBLESHOOTING.md** - Common issues and solutions
- **FLASH_TESTING.md** - FPGA flashing procedures

### docs/features/
- **FLASH_TAB.md** - Flash tab feature documentation
- **FTDI_DRIVER.md** - FTDI driver management
- **BENCHMARK_TAB.md** - Benchmark testing features

### docs/development/
- **RESOURCE_EMBEDDING.md** - How resources are embedded
- **PERFORMANCE.md** - Performance optimization guidelines

### docs/INDEX.md
Complete index of all documentation

## Active Scripts

### Build & Maintenance
- **Clean-Build.ps1** - Clean build artifacts
- **Fix-Build-MT-Issue.ps1** - Fix mt.exe build errors
- **Set-Release-NoConsole.ps1** - Configure console visibility

### Testing
- **Quick-FPGA-Test.ps1** - Quick FPGA test
- **Quick-DMA-Test.ps1** - Quick DMA test
- **Test-FPGAFlash.ps1** - Test flash operations
- **Verify-Resources.ps1** - Verify embedded resources

### Debugging
- **Kill-OpenOCD-Processes.ps1** - Terminate stuck OpenOCD
- **Reset-CH347-Adapter.ps1** - Reset CH347 adapter
- **Check-OpenOCD-Status.ps1** - Check OpenOCD status

### Workspace Management
- **Cleanup-Workspace.ps1** - Archive obsolete files
- **Organize-SolutionFilters.ps1** - Organize Solution Explorer

See **scripts/README.md** for complete usage guide.

## Statistics

```
Total Source Files: 17 .cpp files
Total Header Files: 17 .h files
Total Scripts: 20 .ps1 files
Total Documentation: 120 .md files
```

## Build Configurations

### Debug
- Console window: VISIBLE
- Optimization: Off
- Use for: Development, debugging

### Release
- Console window: HIDDEN
- Optimization: Full
- Use for: Distribution

## Resource Files
- **DMATool.rc** - Windows resource definitions
- **app.manifest** - Application manifest (admin privileges)
- All external files embedded as resources (standalone .exe)

## Archive
Obsolete files archived to: **archive/20251203/**

---
*For more information, see docs/INDEX.md*
