#pragma once

#include <string>
#include <vector>
#include <cstdint>

// Hardware interface layer for DMA operations
// Integration planned with:
// - PCILeech: https://github.com/ufrisk/pcileech
// - LeechCore: https://github.com/ufrisk/LeechCore

namespace DMATool::Backend
{
    enum class DMADeviceType
    {
        None,
        PCILeech_FPGA,
        USB3380,
        Custom
    };

    struct DMADevice
    {
        DMADeviceType type;
        std::string name;
        std::string serialNumber;
        bool connected;
    };

    class DMAInterface
    {
    public:
        DMAInterface();
        ~DMAInterface();

        // Device management
        bool Initialize(DMADeviceType type);
        bool Connect();
        bool Disconnect();
        bool IsConnected() const { return m_Connected; }

        // Memory operations (placeholder signatures)
        bool ReadPhysicalMemory(uint64_t address, uint8_t* buffer, size_t size);
        bool WritePhysicalMemory(uint64_t address, const uint8_t* buffer, size_t size);
        
        // Device info
        DMADevice GetDeviceInfo() const { return m_Device; }
        uint64_t GetTotalPhysicalMemory() const { return m_TotalMemory; }

    private:
        DMADevice m_Device;
        bool m_Connected;
        uint64_t m_TotalMemory;
        
        // Future: LeechCore handle, device context, etc.
    };
}
