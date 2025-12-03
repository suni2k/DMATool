#include "LeechCoreWrapper.h"
#include "../Util/ResourceExtractor.h"
#include "../resource.h"
#include <filesystem>
#include <iostream>

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
        strcpy_s(config.szDevice, "fpga://algo=0");  // FPGA with algorithm 0 (auto-detect)
        config.szRemote[0] = 0;  // Not remote
        config.pfn_printf_opt = nullptr;
        config.paMax = 0;  // Auto-detect max address

        // Create device with config
        m_hDevice = m_pfnLcCreate(&config);
        if (!m_hDevice)
        {
            // Try alternative device string
            std::cout << "[WARNING] Failed with fpga://algo=0, trying 'fpga'" << std::endl;
            strcpy_s(config.szDevice, "fpga");
            m_hDevice = m_pfnLcCreate(&config);
            
            if (!m_hDevice)
            {
                m_LastError = "Failed to create LeechCore device.\n";
                m_LastError += "Possible causes:\n";
                m_LastError += "  1. DMA hardware not connected\n";
                m_LastError += "  2. FTDI WinUSB driver not installed (version 1.4.0.1 required)\n";
                m_LastError += "  3. Device in use by another application\n";
                m_LastError += "  4. Run as Administrator\n";
                m_LastError += "  5. Device may be in UPDATE mode instead of DATA mode\n";
                m_LastError += "\nTroubleshooting:\n";
                m_LastError += "  - Check Data Port tab and install FTDI driver if needed\n";
                m_LastError += "  - Disconnect and reconnect the DMA device\n";
                m_LastError += "  - Ensure device is in DATA mode (not UPDATE mode)";
                FreeLibrary(m_hLeechCore);
                m_hLeechCore = nullptr;
                return false;
            }
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
        // ONLY use temp directory - no fallbacks to external DLLs
        // All DLLs are extracted by GetPCILeechPath() to the temp directory
        
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string pcileechTempDir = std::string(tempPath) + "DMATool_PCILeech\\";
        std::string leechcorePath = pcileechTempDir + "leechcore.dll";
        
        // Try to load from temp directory (where PCILeech extracts all DLLs)
        m_hLeechCore = LoadLibraryA(leechcorePath.c_str());
        if (m_hLeechCore)
        {
            m_LastError = "Loaded from: " + leechcorePath;
            return true;
        }

        // If not found, DLLs haven't been extracted yet or extraction failed
        m_LastError = "Failed to load leechcore.dll from: " + leechcorePath + "\n";
        m_LastError += "Ensure PCILeech resources have been extracted.\n";
        m_LastError += "This happens automatically when accessing the Benchmark tab.";
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
