#pragma once

#include <string>
#include <functional>

namespace DMATool::Backend
{
    struct FT601DriverInfo
    {
        bool installed = false;
        std::string deviceName;
        std::string version;
        std::string provider;
        std::string vidPid;
        std::string location;
        bool isCorrectDriver = false;  // True if "FT601 USB 3.0 Bridge Device", false if "SuperSpeed-FIFO Bridge"
    };

    class FT601DriverInterface
    {
    public:
        FT601DriverInterface() = default;
        ~FT601DriverInterface() = default;

        // Check if FT601 driver is installed
        FT601DriverInfo CheckDriver();

        // Install FT601 driver from embedded resources
        bool InstallDriver();

        // Uninstall FT601 driver
        bool UninstallDriver();

    private:
        // FT601 VID/PID
        static constexpr const char* FT601_VID = "0403";
        static constexpr const char* FT601_PID = "601F";
        
        // Execute PowerShell command
        bool ExecutePowerShell(const std::string& command, std::string& output);
        
        // Parse driver info from pnputil output
        FT601DriverInfo ParseDriverInfo(const std::string& output);
        
        // Extract driver files from embedded resources
        bool ExtractDriverFiles(std::string& outPath);
        
        // Extract a single resource file
        bool ExtractResourceFile(int resourceId, const std::string& outputPath);
        
        // Clean up extracted driver files
        void CleanupDriverFiles(const std::string& path);
    };
}
