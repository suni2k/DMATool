# RESOURCE_EMBEDDING_IMPLEMENTATION.md
# Complete Resource Embedding Implementation for PCILeech and FTDI Drivers

## Summary

This document outlines the complete implementation to embed PCILeech and FTDI driver resources into DMATool.exe, making it fully standalone.

## Implementation Steps Completed

### ? Step 1: Resource IDs Added (src/resource.h)
```cpp
// PCILeech executable for benchmark tests
#define IDR_PCILEECH_EXE                120
#define IDR_VMM_DLL                     121
#define IDR_DBGHELP_DLL                 122

// FT601 Driver Files (already exists)
#define IDR_FT601_INF                   116
#define IDR_FT601_CAT                   117

// LeechCore DLLs (already exists)
#define IDR_LEECHCORE_DLL               200
#define IDR_FTD3XX_DLL                  201
```

### ? Step 2: Resources Embedded (DMATool.rc)
```rc
// PCILeech Files for Benchmark Tests
IDR_PCILEECH_EXE RCDATA "dmafiles\\pcileech\\pcileech.exe"
IDR_LEECHCORE_DLL RCDATA "dmafiles\\pcileech\\leechcore.dll"
IDR_FTD3XX_DLL RCDATA "dmafiles\\pcileech\\FTD3XX.dll"
IDR_VMM_DLL RCDATA "dmafiles\\pcileech\\vmm.dll"
IDR_DBGHELP_DLL RCDATA "dmafiles\\pcileech\\dbghelp.dll"

// FT601 Driver Files
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

### ? Step 3: Files Copied to dmafiles
All necessary files copied from C:\Tools\PCILeech to dmafiles\pcileech:
- pcileech.exe (314 KB)
- leechcore.dll (147 KB)
- FTD3XX.dll (502 KB)
- vmm.dll (2,322 KB)
- dbghelp.dll (1,522 KB)

## Remaining Steps

### ?? Step 4: Modify BenchmarkInterface.cpp

Replace the `GetPCILeechPath()` function with resource extraction:

```cpp
std::string BenchmarkInterface::GetPCILeechPath()
{
    // Extract PCILeech from embedded resources to temp directory
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    
    std::string pcileechDir = std::string(tempPath) + "DMATool_PCILeech\\";
    std::string pcileechExe = pcileechDir + "pcileech.exe";
    
    // Check if already extracted
    if (std::filesystem::exists(pcileechExe))
    {
        return pcileechExe;
    }
    
    // Create temp directory
    std::filesystem::create_directories(pcileechDir);
    
    // Extract PCILeech and dependencies from resources
    if (!ExtractResourceToFile(IDR_PCILEECH_EXE, pcileechExe))
    {
        std::cout << "[ERROR] Failed to extract pcileech.exe from resources" << std::endl;
        return "";
    }
    
    ExtractResourceToFile(IDR_LEECHCORE_DLL, pcileechDir + "leechcore.dll");
    ExtractResourceToFile(IDR_FTD3XX_DLL, pcileechDir + "FTD3XX.dll");
    ExtractResourceToFile(IDR_VMM_DLL, pcileechDir + "vmm.dll");
    ExtractResourceToFile(IDR_DBGHELP_DLL, pcileechDir + "dbghelp.dll");
    
    std::cout << "[INFO] PCILeech extracted to: " << pcileechDir << std::endl;
    
    return pcileechExe;
}
```

Add the `ExtractResourceToFile` helper function:

```cpp
bool BenchmarkInterface::ExtractResourceToFile(int resourceId, const std::string& outputPath)
{
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hResource)
        return false;

    HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
    if (!hLoadedResource)
        return false;

    LPVOID pLockedResource = LockResource(hLoadedResource);
    if (!pLockedResource)
        return false;

    DWORD dwResourceSize = SizeofResource(NULL, hResource);
    if (dwResourceSize == 0)
        return false;

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open())
        return false;

    outFile.write(static_cast<const char*>(pLockedResource), dwResourceSize);
    outFile.close();

    return true;
}
```

### ?? Step 5: Modify FT601DriverInterface.cpp

Update the InstallDriver method to extract from resources:

```cpp
bool FT601DriverInterface::InstallDriver(std::function<void(const std::string&)> progressCallback)
{
    if (progressCallback)
        progressCallback("Extracting driver files...");
    
    // Get temp directory
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    
    std::string driverDir = std::string(tempPath) + "DMATool_FT601_Driver\\";
    std::string infPath = driverDir + "FTD3XXWU.Inf";
    std::string catPath = driverDir + "FTD3XXWU.cat";
    
    // Create temp directory
    std::filesystem::create_directories(driverDir);
    
    // Extract driver files from embedded resources
    if (!ExtractResourceToFile(IDR_FT601_INF, infPath))
    {
        std::cout << "[ERROR] Failed to extract FTDI driver INF file" << std::endl;
        return false;
    }
    
    if (!ExtractResourceToFile(IDR_FT601_CAT, catPath))
    {
        std::cout << "[ERROR] Failed to extract FTDI driver CAT file" << std::endl;
        return false;
    }
    
    std::cout << "[INFO] FTDI driver files extracted to: " << driverDir << std::endl;
    
    if (progressCallback)
        progressCallback("Installing driver to Windows driver store...");
    
    // Use extracted files for installation
    std::string command = "pnputil /add-driver \"" + infPath + "\" /install";
    // ... rest of installation code ...
}
```

Add the `ExtractResourceToFile` helper (same as BenchmarkInterface):

```cpp
bool FT601DriverInterface::ExtractResourceToFile(int resourceId, const std::string& outputPath)
{
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hResource)
        return false;

    HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
    if (!hLoadedResource)
        return false;

    LPVOID pLockedResource = LockResource(hLoadedResource);
    if (!pLockedResource)
        return false;

    DWORD dwResourceSize = SizeofResource(NULL, hResource);
    if (dwResourceSize == 0)
        return false;

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open())
        return false;

    outFile.write(static_cast<const char*>(pLockedResource), dwResourceSize);
    outFile.close();

    return true;
}
```

## Testing

### Debug Build
1. Build in Debug configuration
2. Run DMATool
3. Navigate to Benchmark tab
4. Buttons should be enabled (PCILeech extracted from resources)
5. Try running a test

### Release Build
1. Build in Release configuration
2. Run DMATool.exe
3. Navigate to Benchmark tab
4. Click "Check Driver Status" (should extract from resources)
5. Try "Install FTDI Driver" (should extract and install)

## Expected Behavior

### First Run
- Extracts PCILeech files to `%TEMP%\DMATool_PCILeech\`
- Extracts FTDI drivers to `%TEMP%\DMATool_FT601_Driver\`
- Uses extracted files for all operations

### Subsequent Runs
- Checks if files already extracted
- Skips extraction if files exist
- Uses cached files from temp

## Files Modified

1. ? `src/resource.h` - Added resource IDs
2. ? `DMATool.rc` - Embedded resources
3. ? `dmafiles/pcileech/` - Copied PCILeech files
4. ?? `src/Backend/BenchmarkInterface.cpp` - TODO: Add resource extraction
5. ?? `src/Backend/BenchmarkInterface.h` - TODO: Add ExtractResourceToFile declaration
6. ?? `src/Backend/FT601DriverInterface.cpp` - TODO: Add resource extraction
7. ?? `src/Backend/FT601DriverInterface.h` - TODO: Add ExtractResourceToFile declaration

## Next Actions

Run the code modification script:
```powershell
# This will modify the .cpp files with resource extraction code
.\scripts\Implement-Resource-Extraction.ps1
```

Then rebuild the solution and test!

---
**Status**: Resources embedded, code modifications needed  
**Priority**: High  
**Estimated Time**: 15 minutes for code changes + rebuild
