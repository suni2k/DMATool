# DMATool Scripts Guide

## Active Utility Scripts

### Build & Maintenance
- **Clean-Build.ps1** - Clean build artifacts and temp files
- **Fix-Build-MT-Issue.ps1** - Fix manifest tool (mt.exe) build errors
- **Set-Release-NoConsole.ps1** - Configure console window visibility

### Development & Testing
- **Quick-FPGA-Test.ps1** - Quick FPGA detection test
- **Quick-DMA-Test.ps1** - Quick DMA functionality test
- **Test-FPGAFlash.ps1** - Test FPGA flashing
- **Test-DeviceMasking.ps1** - Test device masking features

### Verification
- **Verify-Resources.ps1** - Verify embedded resources
- **Verify-OpenOCD-Resources.ps1** - Verify OpenOCD resources
- **Verify-Flash-Resources.ps1** - Verify flash resources
- **Verify-Flash.ps1** - Verify flash operations

### Debugging
- **Check-OpenOCD-Status.ps1** - Check OpenOCD process status
- **Kill-OpenOCD-Processes.ps1** - Terminate stuck OpenOCD processes
- **Kill-OpenOCD-Admin.ps1** - Terminate OpenOCD with admin privileges
- **Reset-CH347-Adapter.ps1** - Reset CH347 USB adapter
- **Fix-OpenOCD-Version.ps1** - Fix OpenOCD version issues

### Workspace Management
- **Cleanup-Workspace.ps1** - Archive obsolete files and organize workspace

## Archived Scripts
Obsolete and one-time setup scripts are archived in:
```
archive/20251203/old-scripts/
```

These are kept for historical reference but are no longer needed for regular development.
