#pragma once

#include <string>
#include <vector>
#include <functional>
#include <Windows.h>

namespace DMATool::Backend
{
    // Adapter types for flash programming
    // Determines which OpenOCD configuration and scripts to use
    enum class FlashAdapterType
    {
        Unknown = 0,
        CH347,      // USB-JTAG (CH347) adapter for 35T/75T/100T
        RS232       // Quad-RS232 (FTDI) adapter for 35T only
    };

    // Supported FPGA chip models for flash programming
    // Limited to DMA Kings supported cards only
    enum class FPGAChipModel
    {
        Unknown = 0,
        // Artix-7 Series (DMA Kings Cards)
        XC7A35T_RS232,   // 35T via RS232/FTDI adapter (Screamer)
        XC7A35T,         // 35T via CH347 adapter
        XC7A50T,         // Legacy support (not commonly used)
        XC7A75T,         // Very common (CH347)
        XC7A100T,        // Very common (CH347)
        XC7A200T,
        // Kintex-7 Series
        XC7K70T,
        XC7K160T,
        XC7K325T,
        XC7K355T,
        XC7K410T,
        XC7K420T,
        XC7K480T,
        // Virtex-7 Series
        XC7V585T,
        XC7V2000T,
        XC7VX330T,
        XC7VX415T,
        XC7VX485T,
        XC7VX550T,
        XC7VX690T,
        XC7VX980T,
        XC7VX1140T,
        XC7VH580T,
        XC7VH870T,
        // Spartan-6 Series (Older)
        XC6SLX9,
        XC6SLX16,
        XC6SLX25,
        XC6SLX45,
        XC6SLX45T,
        XC6SLX75,
        XC6SLX75T,
        XC6SLX100,
        XC6SLX150
    };

    // Flash device information
    // Contains detected FPGA and adapter details
    struct FlashDeviceInfo
    {
        bool detected = false;
        std::string manufacturer;
        std::string deviceId;
        std::string model;          // e.g., "W25Q32"
        uint64_t capacity = 0;      // In bytes
        uint32_t sectorSize = 0;    // In bytes
        uint32_t pageSize = 0;      // In bytes
        FPGAChipModel fpgaModel = FPGAChipModel::Unknown;
        std::string fpgaModelString;
        FlashAdapterType adapterType = FlashAdapterType::Unknown;  // Detected adapter type (CH347 or RS232)
    };
    };

    // Flash operation result
    struct FlashOperationResult
    {
        bool success = false;
        std::string message;
        uint64_t bytesProcessed = 0;
        double durationSeconds = 0.0;
    };

    // Progress callback signature
    // Parameters: current bytes, total bytes, message
    using FlashProgressCallback = std::function<void(uint64_t, uint64_t, const std::string&)>;

    class FlashInterface
    {
    public:
        FlashInterface();
        ~FlashInterface();

        // Device detection
        FlashDeviceInfo DetectFlashDevice(FlashProgressCallback progressCallback = nullptr);
        
        // Get list of supported chip models (for dropdown)
        static std::vector<std::pair<FPGAChipModel, std::string>> GetSupportedChipModels();
        static std::string ChipModelToString(FPGAChipModel model);
        static FPGAChipModel StringToChipModel(const std::string& modelStr);
        
        // Check if BSCAN bitstream exists for this chip
        bool HasBSCANBitstream(FPGAChipModel model);
        std::string GetBSCANBitstreamPath(FPGAChipModel model);

        // Flash operations
        FlashOperationResult ProgramFirmware(
            const std::string& firmwarePath,
            FPGAChipModel chipModel,
            bool verifyAfter = true,
            bool backupBefore = false,
            FlashProgressCallback progressCallback = nullptr
        );

        FlashOperationResult VerifyFirmware(
            const std::string& firmwarePath,
            FPGAChipModel chipModel,
            FlashProgressCallback progressCallback = nullptr
        );

        FlashOperationResult ReadFlash(
            const std::string& outputPath,
            FPGAChipModel chipModel,
            uint64_t size,
            FlashProgressCallback progressCallback = nullptr
        );

        FlashOperationResult EraseChip(
            FPGAChipModel chipModel,
            FlashProgressCallback progressCallback = nullptr
        );

        // Utility functions
        bool IsOpenOCDAvailable();
        bool IsCH347Connected();
        std::string GetOpenOCDVersion();

    private:
        // Paths
        std::string m_openocdPath;
        std::string m_openocdScriptsPath;
        std::string m_bscanBasePath;
        
        // Helper methods
        std::string CreateOpenOCDConfig(FPGAChipModel model, int clockSpeed = 10000000);
        bool ExecuteOpenOCDCommand(
            const std::string& configPath,
            const std::vector<std::string>& commands,
            std::string& output,
            std::string& error,
            FlashProgressCallback progressCallback = nullptr
        );
        FlashDeviceInfo ParseFlashInfo(const std::string& openocdOutput);
        
        // Resource extraction (private implementation)
        bool ExtractEmbeddedResource(int resourceId, const std::string& outputPath);
        void ExtractBSCANBitstreams();
        
        // SHA256 hash calculation
        std::string CalculateSHA256(const std::string& filePath);
    };
}
