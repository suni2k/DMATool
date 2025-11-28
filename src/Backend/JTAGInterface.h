#pragma once

#include <string>
#include <vector>
#include <cstdint>

// JTAG interface layer for flash programming and device control
// Integration planned with:
// - CH347: https://github.com/WCHSoftGroup/ch347
// - PCILeech-FPGA programming: https://github.com/ufrisk/pcileech-fpga

namespace DMATool::Backend
{
    enum class JTAGAdapterType
    {
        None,
        CH347_USB,
        Custom
    };

    struct JTAGDevice
    {
        uint32_t idcode;
        std::string manufacturer;
        std::string partNumber;
        uint32_t irLength;
    };

    class JTAGInterface
    {
    public:
        JTAGInterface();
        ~JTAGInterface();

        // Adapter management
        bool Initialize(JTAGAdapterType type);
        bool Connect();
        bool Disconnect();
        bool IsConnected() const { return m_Connected; }

        // JTAG operations
        bool DetectChain();
        std::vector<JTAGDevice> GetDetectedDevices() const { return m_Devices; }
        
        bool ReadIDCode(uint32_t& idcode);
        bool SetTCKFrequency(uint32_t frequencyHz);
        
        // Flash operations (for FPGA firmware)
        bool DetectFlash();
        bool ReadFlash(uint32_t address, uint8_t* buffer, size_t size);
        bool WriteFlash(uint32_t address, const uint8_t* buffer, size_t size);
        bool EraseFlash();
        bool VerifyFlash(const uint8_t* buffer, size_t size);

    private:
        JTAGAdapterType m_AdapterType;
        bool m_Connected;
        uint32_t m_TCKFrequency;
        std::vector<JTAGDevice> m_Devices;
        
        // Future: CH347 device handle, JTAG state machine, etc.
    };
}
