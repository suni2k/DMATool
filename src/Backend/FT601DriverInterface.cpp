#include "FT601DriverInterface.h"
#include "../Util/ResourceExtractor.h"
#include "../resource.h"
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <algorithm>

namespace DMATool::Backend
{
    FT601DriverInfo FT601DriverInterface::CheckDriver()
    {
        FT601DriverInfo info;
        std::string output;

        // Query for FTDI FT601 device - search by device name to get the FIFO bridge, not USB composite
        // The USB composite device has VID/PID but uses Microsoft driver
        // The actual FTDI FIFO bridge device is what we want
        std::string command = 
            "$device = Get-PnpDevice | Where-Object {"
            "  ($_.FriendlyName -like '*FTDI*FIFO*' -or $_.FriendlyName -like '*SuperSpeed*FIFO*') -and "
            "  $_.InstanceId -like '*VID_" + std::string(FT601_VID) + "&PID_" + std::string(FT601_PID) + "*'"
            "} | Select-Object -First 1; "
            "if ($device) { "
            "  $props = @{}; "
            "  $props['FriendlyName'] = $device.FriendlyName; "
            "  $props['InstanceId'] = $device.InstanceId; "
            "  $props['Status'] = $device.Status; "
            // Get driver provider (e.g., "FTDI" for custom driver, "Microsoft" for default)
            "  $provider = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverProvider' -ErrorAction SilentlyContinue).Data; "
            "  if ($provider) { $props['Provider'] = $provider; } else { $props['Provider'] = 'Unknown'; } "
            // Get the driver version directly from device property (this is the INF version)
            "  $driverVer = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data; "
            "  if ($driverVer) { $props['DriverVersion'] = $driverVer; } else { $props['DriverVersion'] = ''; } "
            "  Write-Output \"FriendlyName: $($props['FriendlyName'])\"; "
            "  Write-Output \"InstanceId: $($props['InstanceId'])\"; "
            "  Write-Output \"Status: $($props['Status'])\"; "
            "  Write-Output \"Provider: $($props['Provider'])\"; "
            "  Write-Output \"DriverVersion: $($props['DriverVersion'])\" "
            "}";
        // Remove debug output - too verbose
        // std::cout << "====================" << std::endl;
        // std:: << command << std::endl;
        // std::cout << "====================" << std::endl;

        if (ExecutePowerShell(command, output))
        {
            // Parse the output
            info = ParseDriverInfo(output);
            
            // Driver detection logic:
            // 1. Device detected (has friendly name) = device is physically connected
            // 2. Check provider: "FTDI" = custom driver, "Microsoft" = default Windows driver
            // 3. No INF version or empty = using default Windows driver (no custom driver installed)
            // 4. Version present = custom FTDI driver installed
            //    - Version < 1.4.0.1 = out of date
            //    - Version >= 1.4.0.1 = correct version
            
            if (!info.deviceName.empty())
            {
                // Device is detected
                std::cout << "[DEBUG] Device detected: " << info.deviceName << std::endl;
                std::cout << "[DEBUG] Provider: " << info.provider << std::endl;
                std::cout << "[DEBUG] Version: " << info.version << std::endl;
                
                // Check provider - if Microsoft, then using default Windows driver
                if (info.provider == "Microsoft" || info.provider == "Unknown")
                {
                    // Using default Windows driver (winusb.sys)
                    std::cout << "[DEBUG] Using default Windows driver (no FTDI driver installed)" << std::endl;
                    info.installed = false;
                    info.isCorrectDriver = false;
                }
                else if (info.provider == "FTDI")
                {
                    // FTDI driver is installed - check version
                    if (info.version.empty() || info.version == "Unknown")
                    {
                        // Provider is FTDI but no version = corrupt installation?
                        std::cout << "[WARNING] FTDI provider detected but no version found" << std::endl;
                        info.installed = true;  // Installed but unknown version
                        info.isCorrectDriver = false;
                    }
                    else
                    {
                        // FTDI driver installed with version - check if it's correct
                        info.installed = true;
                        
                        std::cout << "[DEBUG] Comparing version " << info.version << " with 1.4.0.1" << std::endl;
                        int cmp = CompareVersion(info.version, "1.4.0.1");
                        
                        if (cmp < 0)
                        {
                            // Version is lower than 1.4.0.1 = out of date
                            std::cout << "[DEBUG] Driver version is OUT OF DATE (< 1.4.0.1)" << std::endl;
                            info.isCorrectDriver = false;
                        }
                        else
                        {
                            // Version is 1.4.0.1 or higher = correct
                            std::cout << "[DEBUG] Driver version is CORRECT (>= 1.4.0.1)" << std::endl;
                            info.isCorrectDriver = true;
                        }
                    }
                }
                else
                {
                    // Unknown provider - assume not FTDI driver
                    std::cout << "[DEBUG] Unknown provider: " << info.provider << std::endl;
                    info.installed = false;
                    info.isCorrectDriver = false;
                }
            }
            else
            {
                std::cout << "[DEBUG] No device detected" << std::endl;
            }
        }

        return info;
    }

    bool FT601DriverInterface::InstallDriver(ProgressCallback progressCallback)
    {
        std::cout << "[INFO] Installing FTDI driver..." << std::endl;
        if (progressCallback) progressCallback("Initializing driver installation...");
        
        std::string driverPath;
        
        // Extract driver files from embedded resources - NO FALLBACK
        std::cout << "[INFO] Extracting driver files from embedded resources..." << std::endl;
        if (progressCallback) progressCallback("Extracting driver files...");
        
        if (!ExtractDriverFiles(driverPath))
        {
            std::cerr << "[ERROR] Failed to extract FTDI driver files from embedded resources" << std::endl;
            if (progressCallback) progressCallback("Failed to extract driver files!");
            return false;
        }
        
        std::cout << "[INFO] Using embedded driver files from temp" << std::endl;

        std::string driverInfPath = driverPath + "\\FTD3XXWU.Inf";
        
        // Verify INF file exists
        if (!std::filesystem::exists(driverInfPath))
        {
            std::cerr << "[ERROR] Driver INF file not found: " << driverInfPath << std::endl;
            CleanupDriverFiles(driverPath);
            return false;
        }
        
        std::cout << "[INFO] Installing driver to Windows driver store..." << std::endl;
        std::cout << "[DEBUG] Driver INF path: " << driverInfPath << std::endl;
        if (progressCallback) progressCallback("Adding driver to Windows driver store...");

        // Use pnputil to install driver
        std::string command = "pnputil.exe /add-driver \"" + driverInfPath + "\" /install";
        std::cout << "[DEBUG] Running: " << command << std::endl;
        
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe)
        {
            std::cerr << "[ERROR] Failed to execute pnputil" << std::endl;
            CleanupDriverFiles(driverPath);
            return false;
        }

        // Read pnputil output
        char buffer[256];
        std::string output;
        bool shownPnpUtilityMessage = false;  // Track if we've shown the patience message
        
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output += buffer;
            std::cout << buffer;  // Echo output
            
            // Update progress with pnputil output if callback is provided
            if (progressCallback)
            {
                std::string line = buffer;
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                if (!line.empty())
                {
                    progressCallback(line);
                    
                    // If we see "Microsoft PnP Utility", add a helpful message
                    if (!shownPnpUtilityMessage && 
                        (line.find("Microsoft PnP Utility") != std::string::npos ||
                         line.find("PnP") != std::string::npos))
                    {
                        shownPnpUtilityMessage = true;
                        // Give a brief pause so the user can see both messages
                        Sleep(100);
                        progressCallback("This may take a moment, please wait...");
                    }
                }
            }
        }

        int exitCode = _pclose(pipe);
        std::cout << "[DEBUG] pnputil exit code: " << exitCode << std::endl;
        
        bool success = (exitCode == 0);
        
        if (success)
        {
            std::cout << "[SUCCESS] FTDI driver installed successfully" << std::endl;
            std::cout << "[INFO] Driver will be applied automatically" << std::endl;
            std::cout << "[INFO] Windows is refreshing device list in background..." << std::endl;
            if (progressCallback) progressCallback("Driver installed! Applying changes...");
            
            // Start background device rescan (non-blocking)
            // Use START command to launch in separate process that doesn't wait
            std::string bgCommand = "start /B cmd /c \"pnputil /scan-devices >nul 2>&1\"";
            system(bgCommand.c_str());
        }
        else
        {
            std::cerr << "[ERROR] Failed to add driver to Windows driver store" << std::endl;
        }
        
        // Cleanup temp files
        CleanupDriverFiles(driverPath);
        
        return success;
    }

    bool FT601DriverInterface::UninstallDriver(ProgressCallback progressCallback)
    {
        std::string output;

        // Method 1: Try to find and delete the driver package
        std::cout << "[INFO] Searching for FTDI driver package..." << std::endl;
        if (progressCallback) progressCallback("Searching for FTDI driver package...");
        
        std::string command = "pnputil /enum-drivers";
        
        FILE* enumPipe = _popen(command.c_str(), "r");
        if (!enumPipe)
        {
            std::cerr << "[ERROR] Failed to enumerate drivers" << std::endl;
            return false;
        }
        
        // Read enum output
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), enumPipe) != nullptr)
        {
            output += buffer;
        }
        _pclose(enumPipe);

        // Parse the output line by line to find the FTDI driver
        // Look for the OEM inf that corresponds to ftd3xxwu.inf
        std::istringstream iss(output);
        std::string line;
        std::string oemInf;
        bool foundDriver = false;
        std::string publishedName;
        
        while (std::getline(iss, line))
        {
            // Look for "Published Name:" line
            if (line.find("Published Name") != std::string::npos || 
                line.find("Published name") != std::string::npos)
            {
                size_t colonPos = line.find(":");
                if (colonPos != std::string::npos)
                {
                    publishedName = line.substr(colonPos + 1);
                    // Trim whitespace
                    publishedName.erase(0, publishedName.find_first_not_of(" \t\r\n"));
                    publishedName.erase(publishedName.find_last_not_of(" \t\r\n") + 1);
                }
            }
            // Look for "Original Name:" line with ftd3xxwu.inf
            else if ((line.find("Original Name") != std::string::npos || 
                      line.find("Original name") != std::string::npos) && 
                     !publishedName.empty())
            {
                // Check if this is the ftd3xxwu.inf driver
                std::string lowerLine = line;
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
                
                if (lowerLine.find("ftd3xxwu.inf") != std::string::npos)
                {
                    oemInf = publishedName;
                    foundDriver = true;
                    std::cout << "[INFO] Found driver package: " << oemInf << std::endl;
                    std::cout << "[DEBUG] Original name: ftd3xxwu.inf" << std::endl;
                    if (progressCallback) progressCallback("Found driver: " + oemInf);
                    break;
                }
                // Reset published name after checking
                publishedName.clear();
            }
        }
        
        if (!foundDriver || oemInf.empty())
        {
            std::cout << "[WARNING] FTDI driver package not found in driver store" << std::endl;
            std::cout << "[DEBUG] Searched for 'ftd3xxwu.inf' (case-insensitive)" << std::endl;
            std::cout << "[INFO] The driver may not be installed, or was already removed" << std::endl;
            return false;
        }
        
        // Uninstall and delete the driver package - RUN SYNCHRONously
        std::cout << "[INFO] Uninstalling driver package: " << oemInf << std::endl;
        if (progressCallback) progressCallback("Removing driver from devices...");
        
        // Run pnputil synchronously to properly verify the result
        std::string uninstallCommand = "pnputil.exe /delete-driver " + oemInf + " /uninstall /force";
        std::cout << "[DEBUG] Running: " << uninstallCommand << std::endl;
        
        FILE* pipe = _popen(uninstallCommand.c_str(), "r");
        if (!pipe)
        {
            std::cerr << "[ERROR] Failed to execute pnputil" << std::endl;
            return false;
        }

        // Read pnputil output LINE BY LINE to show progress
        char uninstallBuffer[256];
        std::string uninstallOutput;
        int lineCount = 0;
        bool shownPnpUtilityMessage = false;  // Track if we've shown the patience message
        
        while (fgets(uninstallBuffer, sizeof(uninstallBuffer), pipe) != nullptr)
        {
            uninstallOutput += uninstallBuffer;
            std::cout << uninstallBuffer;  // Echo output
            
            // Update progress with pnputil output if callback is provided
            if (progressCallback)
            {
                std::string outputLine = uninstallBuffer;
                outputLine.erase(0, outputLine.find_first_not_of(" \t\r\n"));
                outputLine.erase(outputLine.find_last_not_of(" \t\r\n") + 1);
                
                if (!outputLine.empty())
                {
                    progressCallback(outputLine);
                    
                    // If we see "Microsoft PnP Utility", add a helpful message
                    if (!shownPnpUtilityMessage && 
                        (outputLine.find("Microsoft PnP Utility") != std::string::npos ||
                         outputLine.find("PnP") != std::string::npos))
                    {
                        shownPnpUtilityMessage = true;
                        // Give a brief pause so the user can see both messages
                        Sleep(100);
                        progressCallback("This may take a few minutes, please be patient...");
                    }
                }
                else
                {
                    // pnputil doesn't output much, so show timer-based progress
                    lineCount++;
                    if (lineCount % 2 == 0)
                    {
                        progressCallback("Removing driver from devices...");
                    }
                }
            }
        }

        int exitCode = _pclose(pipe);
        std::cout << "[DEBUG] pnputil exit code: " << exitCode << std::endl;
        
        bool success = (exitCode == 0);
        
        if (success)
        {
            std::cout << "[SUCCESS] FTDI driver package deleted successfully: " << oemInf << std::endl;
            std::cout << "[INFO] Driver uninstalled successfully" << std::endl;
            std::cout << "[INFO] Windows is refreshing device list in background..." << std::endl;
            if (progressCallback) progressCallback("Driver removed! Refreshing devices...");
            
            // Start background device rescan (non-blocking)
            std::string bgCommand = "start /B cmd /c \"pnputil /scan-devices >nul 2>&1\"";
            system(bgCommand.c_str());
            
            return true;
        }
        else
        {
            std::cerr << "[ERROR] Failed to delete driver package: " << oemInf << std::endl;
            std::cerr << "[DEBUG] pnputil output: " << uninstallOutput << std::endl;
            return false;
        }
    }

    bool FT601DriverInterface::ExecutePowerShell(const std::string& command, std::string& output)
    {
        std::string fullCommand = "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"" + command + "\"";
        
        FILE* pipe = _popen(fullCommand.c_str(), "r");
        if (!pipe)
        {
            std::cerr << "[ERROR] Failed to execute PowerShell command" << std::endl;
            return false;
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output += buffer;
        }

        int result = _pclose(pipe);
        return (result == 0);
    }

    FT601DriverInfo FT601DriverInterface::ParseDriverInfo(const std::string& output)
    {
        FT601DriverInfo info;

        // Parse FriendlyName
        std::regex friendlyNameRegex(R"(FriendlyName\s*:\s*(.+))");
        std::smatch match;
        if (std::regex_search(output, match, friendlyNameRegex))
        {
            info.deviceName = match[1].str();
            // Trim whitespace
            info.deviceName.erase(0, info.deviceName.find_first_not_of(" \t\r\n"));
            info.deviceName.erase(info.deviceName.find_last_not_of(" \t\r\n") + 1);
        }

        // Parse Status
        std::regex statusRegex(R"(Status\s*:\s*(\w+))");
        if (std::regex_search(output, match, statusRegex))
        {
            std::string status = match[1].str();
            info.installed = (status == "OK");
        }

        // Parse Provider (CRITICAL: Determines if FTDI driver or Microsoft default driver)
        std::regex providerRegex(R"(Provider\s*:\s*(.+))");
        if (std::regex_search(output, match, providerRegex))
        {
            info.provider = match[1].str();
            // Trim whitespace
            info.provider.erase(0, info.provider.find_first_not_of(" \t\r\n"));
            info.provider.erase(info.provider.find_last_not_of(" \t\r\n") + 1);
        }
        else
        {
            info.provider = "Unknown";
        }

        // Parse DriverVersion from INF file (provider version, NOT OS driver version)
        std::regex versionRegex(R"(DriverVersion\s*:\s*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+))");
        if (std::regex_search(output, match, versionRegex))
        {
            info.version = match[1].str();
        }
        else
        {
            // Fallback: try to extract any version-like pattern
            std::regex versionFallbackRegex(R"(DriverVersion\s*:\s*(.+))");
            if (std::regex_search(output, match, versionFallbackRegex))
            {
                std::string versionLine = match[1].str();
                versionLine.erase(0, versionLine.find_first_not_of(" \t\r\n"));
                versionLine.erase(versionLine.find_last_not_of(" \t\r\n") + 1);
                
                // Only use if it looks like a version number or is empty
                if (versionLine.empty() || versionLine.find_first_of("0123456789") == 0)
                {
                    info.version = versionLine;
                }
            }
        }

        // Parse InstanceId for VID/PID
        std::regex instanceRegex(R"(InstanceId\s*:\s*(.+))");
        if (std::regex_search(output, match, instanceRegex))
        {
            std::string instanceId = match[1].str();
            
            // Extract VID/PID
            std::regex vidPidRegex(R"(VID_([0-9A-F]{4})&PID_([0-9A-F]{4}))");
            if (std::regex_search(instanceId, match, vidPidRegex))
            {
                info.vidPid = "VID_" + match[1].str() + " / PID_" + match[2].str();
            }
            
            info.location = instanceId;
        }

        return info;
    }

    bool FT601DriverInterface::ExtractDriverFiles(std::string& outPath)
    {
        // Create temp directory for driver files
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        
        outPath = std::string(tempPath) + "DMATool_FT601_Driver";
        
        // Create directory if it doesn't exist
        std::filesystem::create_directories(outPath);
        
        // Extract FT601 INF file
        std::string infPath = outPath + "\\FTD3XXWU.inf";
        if (!ExtractResourceFile(IDR_FT601_INF, infPath))
        {
            std::cerr << "[ERROR] Failed to extract FTDI INF file" << std::endl;
            return false;
        }
        
        // Extract FT601 CAT file
        std::string catPath = outPath + "\\FTD3XXWU.cat";
        if (!ExtractResourceFile(IDR_FT601_CAT, catPath))
        {
            std::cerr << "[ERROR] Failed to extract FTDI CAT file" << std::endl;
            return false;
        }
        
        std::cout << "[INFO] FTDI driver files extracted to: " << outPath << std::endl;
        return true;
    }

    bool FT601DriverInterface::ExtractResourceFile(int resourceId, const std::string& outputPath)
    {
        // Get handle to current module (exe file)
        HMODULE hModule = GetModuleHandleA(nullptr);
        if (!hModule)
        {
            std::cerr << "[ERROR] Failed to get module handle" << std::endl;
            return false;
        }
        
        HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), MAKEINTRESOURCEA(RT_RCDATA));
        if (!hRes)
        {
            std::cerr << "[ERROR] Resource not found: " << resourceId << std::endl;
            return false;
        }

        HGLOBAL hData = LoadResource(hModule, hRes);
        if (!hData)
        {
            std::cerr << "[ERROR] Failed to load resource: " << resourceId << std::endl;
            return false;
        }

        DWORD dataSize = SizeofResource(hModule, hRes);
        const void* pData = LockResource(hData);

        if (!pData)
        {
            std::cerr << "[ERROR] Failed to lock resource: " << resourceId << std::endl;
            return false;
        }

        // Write to file
        HANDLE hFile = CreateFileA(outputPath.c_str(), GENERIC_WRITE, 0, nullptr, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cerr << "[ERROR] Failed to create file: " << outputPath << std::endl;
            return false;
        }

        DWORD bytesWritten;
        bool success = WriteFile(hFile, pData, dataSize, &bytesWritten, nullptr) && 
                      (bytesWritten == dataSize);

        CloseHandle(hFile);

        if (!success)
        {
            std::cerr << "[ERROR] Failed to write file: " << outputPath << std::endl;
            return false;
        }

        return true;
    }

    void FT601DriverInterface::CleanupDriverFiles(const std::string& path)
    {
        try
        {
            if (std::filesystem::exists(path))
            {
                std::filesystem::remove_all(path);
                std::cout << "[INFO] Cleaned up temporary driver files" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[WARNING] Failed to cleanup driver files: " << e.what() << std::endl;
        }
    }

    int FT601DriverInterface::CompareVersion(const std::string& v1, const std::string& v2)
    {
        // Parse version strings like "1.4.0.1" and compare
        // Returns: -1 if v1 < v2, 0 if equal, 1 if v1 > v2
        
        auto parseVersion = [](const std::string& ver) -> std::vector<int> {
            std::vector<int> parts;
            std::string current;
            for (char c : ver)
            {
                if (c == '.')
                {
                    if (!current.empty())
                    {
                        parts.push_back(std::stoi(current));
                        current.clear();
                    }
                }
                else if (std::isdigit(c))
                {
                    current += c;
                }
            }
            if (!current.empty())
                parts.push_back(std::stoi(current));
            return parts;
        };
        
        std::vector<int> parts1 = parseVersion(v1);
        std::vector<int> parts2 = parseVersion(v2);
        
        // Compare each part
        size_t maxLen = (parts1.size() > parts2.size()) ? parts1.size() : parts2.size();
        for (size_t i = 0; i < maxLen; ++i)
        {
            int p1 = (i < parts1.size()) ? parts1[i] : 0;
            int p2 = (i < parts2.size()) ? parts2[i] : 0;
            
            if (p1 < p2) return -1;
            if (p1 > p2) return 1;
        }
        
        return 0;  // Equal
    }
}
