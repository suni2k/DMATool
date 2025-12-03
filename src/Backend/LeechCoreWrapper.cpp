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
        strcpy_s(config.szDevice, "fpga");  // Use simple "fpga" device string
        config.szRemote[0] = 0;  // Not remote
        config.pfn_printf_opt = nullptr;
        config.paMax = 0;  // Auto-detect max address

        std::cout << "[INFO] Attempting to create LeechCore device with config: " << config.szDevice << std::endl;

        // Create device with config
        m_hDevice = m_pfnLcCreate(&config);
        if (!m_hDevice)
        {
            m_LastError = "Failed to create LeechCore device.\n";
            m_LastError += "Device config used: " + std::string(config.szDevice) + "\n";
            m_LastError += "\nPossible causes:\n";
            m_LastError += "  1. DMA hardware not connected or not recognized\n";
            m_LastError += "  2. FTDI driver installed but device not in correct mode\n";
            m_LastError += "  3. Bitstream not loaded or wrong bitstream\n";
            m_LastError += "  4. Device in use by another application\n";
            m_LastError += "  5. USB port doesn't have enough power\n";
            m_LastError += "  6. Run as Administrator required\n";
            m_LastError += "\nTroubleshooting:\n";
            m_LastError += "  - Check Device Manager: Device should show as 'FTDI SuperSpeed-FIFO Bridge'\n";
            m_LastError += "  - If it shows as 'USB Composite Device', driver may need reinstall\n";
            m_LastError += "  - Try unplugging and replugging the device\n";
            m_LastError += "  - Make sure device is connected to a USB 3.0 port (blue port)\n";
            m_LastError += "  - Check if device has any LED indicators (should be stable, not blinking)\n";
            m_LastError += "  - Verify no other DMA tools are running (PCILeech, Arbiter, etc.)\n";
            m_LastError += "  - Try running DMATool as Administrator";
            
            std::cout << "[ERROR] " << m_LastError << std::endl;
            
            FreeLibrary(m_hLeechCore);
            m_hLeechCore = nullptr;
            return false;
        }

        m_LastError = "Success - Device: " + std::string(config.szDeviceName);
        std::cout << "[SUCCESS] LeechCore device created: " << config.szDeviceName << std::endl;
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
        // Load LeechCore DLLs from temp directory (extracted by BenchmarkInterface)
        
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string pcileechDir = std::string(tempPath) + "DMATool_PCILeech\\";
        std::string leechcorePath = pcileechDir + "leechcore.dll";
        std::string ftd3xxPath = pcileechDir + "FTD3XX.dll";
        
        // Check if required DLLs exist
        if (!std::filesystem::exists(leechcorePath))
        {
            m_LastError = "leechcore.dll not found at: " + leechcorePath + "\n";
            m_LastError += "Resources not extracted yet.\n";
            m_LastError += "Access Benchmark tab first to trigger extraction.";
            std::cout << "[ERROR] " << m_LastError << std::endl;
            return false;
        }
        
        if (!std::filesystem::exists(ftd3xxPath))
        {
            m_LastError = "FTD3XX.dll not found at: " + ftd3xxPath;
            std::cout << "[ERROR] " << m_LastError << std::endl;
            return false;
        }
        
        // Load FTD3XX.dll first (dependency of leechcore.dll)
        HMODULE hFtd3xx = LoadLibraryA(ftd3xxPath.c_str());
        if (!hFtd3xx)
        {
            DWORD dwError = ::GetLastError();
            m_LastError = "Failed to load FTD3XX.dll (error " + std::to_string(dwError) + ")";
            std::cout << "[ERROR] " << m_LastError << std::endl;
            return false;
        }
        
        std::cout << "[INFO] FTD3XX.dll loaded successfully" << std::endl;
        
        // Now load leechcore.dll
        m_hLeechCore = LoadLibraryA(leechcorePath.c_str());
        if (m_hLeechCore)
        {
            m_LastError = "Loaded from: " + leechcorePath;
            std::cout << "[INFO] leechcore.dll loaded successfully" << std::endl;
            return true;
        }

        // Failed to load leechcore.dll
        DWORD dwError = ::GetLastError();
        m_LastError = "Failed to load leechcore.dll (error " + std::to_string(dwError) + ")";
        std::cout << "[ERROR] " << m_LastError << std::endl;
        
        // Clean up FTD3XX.dll if leechcore.dll failed to load
        FreeLibrary(hFtd3xx);
        
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
