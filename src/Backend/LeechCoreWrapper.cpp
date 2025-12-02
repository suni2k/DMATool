#include "LeechCoreWrapper.h"
#include "../Util/ResourceExtractor.h"
#include "../resource.h"
#include <filesystem>

namespace DMATool::Backend
{
    LeechCoreWrapper::LeechCoreWrapper()
    {
    }

    LeechCoreWrapper::~LeechCoreWrapper()
    {
        Close();
    }

    bool LeechCoreWrapper::Initialize()
    {
        if (m_hDevice)
        {
            m_LastError = "Already initialized";
            return true;
        }

        // Load DLL
        if (!LoadDLL())
        {
            return false;
        }

        // Get function pointers
        if (!GetFunctionPointers())
        {
            return false;
        }

        // Create LeechCore config for FPGA device
        LC_CONFIG config = { 0 };
        config.dwVersion = LC_CONFIG_VERSION;
        config.dwPrintfVerbosity = 0;  // No debug output
        strcpy_s(config.szDevice, "fpga");  // Auto-detect FPGA
        config.szRemote[0] = 0;  // Not remote
        config.pfn_printf_opt = nullptr;
        config.paMax = 0;  // Auto-detect max address

        // Create device with config
        m_hDevice = m_pfnLcCreate(&config);
        if (!m_hDevice)
        {
            m_LastError = "Failed to create LeechCore device.\n";
            m_LastError += "Possible causes:\n";
            m_LastError += "  1. DMA hardware not connected\n";
            m_LastError += "  2. FTDI drivers not installed\n";
            m_LastError += "  3. Device in use by another application\n";
            m_LastError += "  4. Run as Administrator\n";
            m_LastError += "\nDevice config used: " + std::string(config.szDevice);
            FreeLibrary(m_hLeechCore);
            m_hLeechCore = nullptr;
            return false;
        }

        m_LastError = "Success - Device: " + std::string(config.szDeviceName);
        return true;
    }

    void LeechCoreWrapper::Close()
    {
        if (m_hDevice && m_pfnLcClose)
        {
            m_pfnLcClose(m_hDevice);
            m_hDevice = nullptr;
        }

        if (m_hLeechCore)
        {
            FreeLibrary(m_hLeechCore);
            m_hLeechCore = nullptr;
        }
    }

    bool LeechCoreWrapper::Read4KB(uint64_t address, uint8_t* buffer)
    {
        if (!m_hDevice || !m_pfnLcRead)
        {
            m_LastError = "Not initialized";
            return false;
        }

        // Read 4096 bytes (one page)
        int result = m_pfnLcRead(m_hDevice, address, 4096, buffer);
        if (!result)
        {
            m_LastError = "Read failed";
            return false;
        }

        return true;
    }

    bool LeechCoreWrapper::ReadCustomSize(uint64_t address, uint32_t sizeBytes, uint8_t* buffer)
    {
        if (!m_hDevice || !m_pfnLcRead)
        {
            m_LastError = "Not initialized";
            return false;
        }

        // Read custom-sized block (for configurable Custom Test)
        int result = m_pfnLcRead(m_hDevice, address, sizeBytes, buffer);
        if (!result)
        {
            m_LastError = "Read failed";
            return false;
        }

        return true;
    }

    bool LeechCoreWrapper::ReadChunk(uint64_t address, uint32_t sizeBytes, uint8_t* buffer)
    {
        if (!m_hDevice || !m_pfnLcRead)
        {
            m_LastError = "Not initialized";
            return false;
        }

        // Read larger chunk in one call (much faster than multiple 4KB reads)
        int result = m_pfnLcRead(m_hDevice, address, sizeBytes, buffer);
        if (!result)
        {
            m_LastError = "Read failed";
            return false;
        }

        return true;
    }

    bool LeechCoreWrapper::LoadDLL()
    {
        // First, try to extract and load embedded DLLs
        std::string tempDir = Util::ResourceExtractor::GetTempDLLDirectory();
        
        // Extract all required DLLs from resources
        struct DLLResource {
            int resourceId;
            std::string filename;
        };
        
        DLLResource dlls[] = {
            { IDR_LEECHCORE_DLL, "leechcore.dll" },
            { IDR_FTD3XX_DLL, "FTD3XX.dll" },
            { IDR_FTD3XXWU_DLL, "FTD3XXWU.dll" },
            { IDR_LEECHCORE_DEVICE_HVSAVED, "leechcore_device_hvsavedstate.dll" },
            { IDR_LEECHCORE_DEVICE_RAWTCP, "leechcore_device_rawtcp.dll" },
            { IDR_LEECHCORE_DRIVER, "leechcore_driver.dll" }
        };
        
        // Extract all DLLs
        bool allExtracted = true;
        for (const auto& dll : dlls)
        {
            std::string extracted = Util::ResourceExtractor::ExtractDLL(dll.resourceId, dll.filename);
            if (extracted.empty())
            {
                // If extraction fails, it's okay - we'll fall back to external DLLs
                allExtracted = false;
            }
        }
        
        // Try multiple locations
        std::vector<std::string> searchPaths;
        
        if (allExtracted)
        {
            // Prioritize extracted DLLs
            searchPaths.push_back(tempDir + "leechcore.dll");
        }
        
        // Fallback to external DLLs
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string exeDir = exePath;
        exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));
        
        searchPaths.push_back(exeDir + "\\vendor\\leechcore\\leechcore.dll");
        searchPaths.push_back("C:\\Tools\\PCILeech\\leechcore.dll");
        searchPaths.push_back(exeDir + "\\leechcore.dll");

        for (const auto& path : searchPaths)
        {
            m_hLeechCore = LoadLibraryA(path.c_str());
            if (m_hLeechCore)
            {
                m_LastError = "Loaded from: " + path;
                return true;
            }
        }

        m_LastError = "Failed to load leechcore.dll from any location. Tried:\n";
        for (const auto& path : searchPaths)
        {
            m_LastError += "  - " + path + "\n";
        }
        m_LastError += "Install PCILeech to C:\\Tools\\PCILeech\\ or copy DLLs to exe directory.";
        return false;
    }

    bool LeechCoreWrapper::GetFunctionPointers()
    {
        m_pfnLcCreate = (PFN_LcCreate)GetProcAddress(m_hLeechCore, "LcCreate");
        m_pfnLcClose = (PFN_LcClose)GetProcAddress(m_hLeechCore, "LcClose");
        m_pfnLcRead = (PFN_LcRead)GetProcAddress(m_hLeechCore, "LcRead");

        if (!m_pfnLcCreate || !m_pfnLcClose || !m_pfnLcRead)
        {
            m_LastError = "Failed to get LeechCore function pointers";
            return false;
        }

        return true;
    }
}
