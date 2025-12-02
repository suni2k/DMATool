#pragma once
#include <Windows.h>
#include <string>
#include <functional>

namespace DMATool::Backend
{
    // LeechCore configuration struct (from leechcore.h)
    #define LC_CONFIG_VERSION 0xc0fd0002
    #define MAX_PATH 260

    typedef struct tdLC_CONFIG {
        DWORD dwVersion;
        DWORD dwPrintfVerbosity;
        CHAR szDevice[MAX_PATH];
        CHAR szRemote[MAX_PATH];
        void* pfn_printf_opt;
        uint64_t paMax;
        BOOL fVolatile;
        BOOL fWritable;
        BOOL fRemote;
        BOOL fRemoteDisableCompress;
        CHAR szDeviceName[MAX_PATH];
    } LC_CONFIG, *PLC_CONFIG;

    // LeechCore wrapper for high-performance DMA operations
    class LeechCoreWrapper
    {
    public:
        LeechCoreWrapper();
        ~LeechCoreWrapper();

        // Initialize LeechCore device
        bool Initialize();
        
        // Close device
        void Close();
        
        // Check if initialized
        bool IsInitialized() const { return m_hDevice != nullptr; }
        
        // Read 4KB of memory (one page)
        bool Read4KB(uint64_t address, uint8_t* buffer);
        
        // Read custom-sized chunk (for configurable tests)
        bool ReadCustomSize(uint64_t address, uint32_t sizeBytes, uint8_t* buffer);
        
        // Read larger chunk (for throughput testing)
        bool ReadChunk(uint64_t address, uint32_t sizeBytes, uint8_t* buffer);
        
        // Get error message
        std::string GetLastError() const { return m_LastError; }

    private:
        // LeechCore function types
        typedef void* (*PFN_LcCreate)(PLC_CONFIG pConfig);
        typedef void (*PFN_LcClose)(void* hLC);
        typedef int (*PFN_LcRead)(void* hLC, uint64_t pa, uint32_t cb, uint8_t* pb);
        
        HMODULE m_hLeechCore = nullptr;
        void* m_hDevice = nullptr;
        
        PFN_LcCreate m_pfnLcCreate = nullptr;
        PFN_LcClose m_pfnLcClose = nullptr;
        PFN_LcRead m_pfnLcRead = nullptr;
        
        std::string m_LastError;
        
        bool LoadDLL();
        bool GetFunctionPointers();
    };
}
