#pragma once

#include <string>
#include <cstdint>
#include <functional>

// OpenOCD interface for FPGA detection and DNA ID extraction
// Supports CH347 and RS232 JTAG adapters

namespace DMATool::Backend
{
    enum class ChipModel
    {
        Unknown,
        XC7A35T,    // Artix-7 35T
        XC7A50T,    // Artix-7 50T
        XC7A75T,    // Artix-7 75T
        XC7A100T    // Artix-7 100T
    };

    enum class AdapterType
    {
        Unknown,
        CH347,
        RS232
    };

    struct FPGAInfo
    {
        bool detected = false;
        ChipModel chipModel = ChipModel::Unknown;
        uint32_t idcode = 0;
        std::string manufacturer;
        std::string family;
        std::string partNumber;
        std::string logicCells;
        std::string dnaId;
        AdapterType adapterType = AdapterType::Unknown;
    };

    struct DriverInfo
    {
        bool installed = false;
        std::string version;
        std::string date;
        std::string provider;
        std::string deviceName;
        std::string vidPid;
        std::string service;  // Added for driver service type (FTDIBUS, WinUSB, etc.)
    };

    struct CardInfo
    {
        bool detected = false;
        AdapterType adapterType = AdapterType::Unknown;
        std::string cardTypeString;  // "35T", "75T", "100T"
        std::string vidPid;
        std::string instanceId;
        std::string deviceName;
    };

    class OpenOCDInterface
    {
    public:
        OpenOCDInterface();
        ~OpenOCDInterface();

        // Detection operations
        FPGAInfo DetectFPGA(std::function<void(const std::string&)> logCallback = nullptr);
        CardInfo DetectDMACard();
        DriverInfo CheckCH347Driver();
        DriverInfo CheckRS232Driver();
        
        // Driver management
        bool InstallCH347Driver();
        bool UninstallCH347Driver();
        bool InstallRS232Driver();
        bool UninstallRS232Driver();
        
        // Utility functions
        static std::string GetChipModelName(ChipModel model);
        static ChipModel IDCodeToChipModel(uint32_t idcode);
        static std::string FormatDNA(const std::string& dna);

    private:
        std::string m_OpenOCDPath;
        std::string m_ConfigPath;
        
        bool FindOpenOCD();
        bool ExtractEmbeddedResource(int resourceId, const std::string& outputPath);
        std::string GetTempDirectory();
        std::string ExecuteCommand(const std::string& command);
        FPGAInfo ParseOpenOCDOutput(const std::string& output);
        AdapterType DetectAdapterType();
    };
}
