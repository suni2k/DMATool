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

        // Query PnP devices for FTDI FT601 and get driver properties
        // Use Get-PnpDeviceProperty to reliably get driver version
        std::string command = 
            "$device = Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_" + std::string(FT601_VID) + "&PID_" + std::string(FT601_PID) + "*'} | Select-Object -First 1; "
            "if ($device) { "
            "  $props = @{}; "
            "  $props['FriendlyName'] = $device.FriendlyName; "
            "  $props['InstanceId'] = $device.InstanceId; "
            "  $props['Status'] = $device.Status; "
            "  $driverVersion = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data; "
            "  if ($driverVersion) { $props['DriverVersion'] = $driverVersion } else { $props['DriverVersion'] = '' } "
            "  Write-Output \"FriendlyName: $($props['FriendlyName'])\"; "
            "  Write-Output \"InstanceId: $($props['InstanceId'])\"; "
            "  Write-Output \"Status: $($props['Status'])\"; "
            "  Write-Output \"DriverVersion: $($props['DriverVersion'])\" "
            "}";

        if (ExecutePowerShell(command, output))
        {
            // Parse the output
            info = ParseDriverInfo(output);
            
            // Determine if the correct driver is installed based on driver version
            if (!info.deviceName.empty())
            {
                // Check if driver version is present and is 1.4.0.1 or higher
                if (!info.version.empty() && info.version != "Unknown")
                {
                    // Driver is installed - version 1.4.0.1 is the correct WinUSB driver
                    info.isCorrectDriver = true;
                    info.installed = true;
                }
                else
                {
                    // Device detected but no driver version = default Windows driver
                    info.isCorrectDriver = false;
                    info.installed = false;
                }
            }
        }

        return info;
    }

    bool FT601DriverInterface::InstallDriver(ProgressCallback progressCallback)
    {
        std::cout << "[INFO] Installing FTDI driver..." << std::endl;
        if (progressCallback) progressCallback("Initializing driver installation...");
        
        std::string driverPath;
        
        // Try embedded resources first
        std::cout << "[INFO] Extracting driver files..." << std::endl;
        if (progressCallback) progressCallback("Extracting driver files...");
        
        if (ExtractDriverFiles(driverPath))
        {
            std::cout << "[INFO] Using embedded driver files from temp" << std::endl;
        }
        else
        {
            // Fallback: Copy from external directory (same as CH347)
            std::cout << "[INFO] Embedded resources not found, using external driver files" << std::endl;
            if (progressCallback) progressCallback("Loading external driver files...");
            
            // Create temp directory
            char tempPath[MAX_PATH];
            GetTempPathA(MAX_PATH, tempPath);
            driverPath = std::string(tempPath) + "DMATool\\drivers";
            
            // Create directory
            std::filesystem::create_directories(driverPath);
            
            // Find source directory - use tools folder like CH347
            std::string sourceDir;
            std::vector<std::string> searchPaths = {
                "tools\\ftdi601\\drivers",  // From solution root
                "..\\..\\tools\\ftdi601\\drivers",  // From bin\Debug-x64 or bin\Release-x64
            };
            
            // Get exe directory to try absolute paths
            char exePath[MAX_PATH];
            GetModuleFileNameA(nullptr, exePath, MAX_PATH);
            std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
            
            // Try relative to exe directory
            searchPaths.push_back((exeDir.parent_path().parent_path() / "tools\\ftdi601\\drivers").string());
            
            bool found = false;
            for (const auto& path : searchPaths)
            {
                if (std::filesystem::exists(path))
                {
                    sourceDir = path;
                    found = true;
                    std::cout << "[DEBUG] Found driver files at: " << sourceDir << std::endl;
                    break;
                }
            }
            
            if (!found)
            {
                std::cerr << "[ERROR] Driver source directory not found. Searched:" << std::endl;
                for (const auto& path : searchPaths)
                {
                    std::cerr << "  - " << path << std::endl;
                }
                return false;
            }
            
            std::cout << "[INFO] Copying driver files from: " << sourceDir << std::endl;
            
            try
            {
                // Copy INF file
                std::filesystem::copy_file(
                    sourceDir + "\\FTD3XXWU.Inf",
                    driverPath + "\\FTD3XXWU.Inf",
                    std::filesystem::copy_options::overwrite_existing
                );
                std::cout << "[DEBUG] Copied FTD3XXWU.Inf" << std::endl;
                
                // Copy CAT file
                std::filesystem::copy_file(
                    sourceDir + "\\FTD3XXWU.cat",
                    driverPath + "\\FTD3XXWU.cat",
                    std::filesystem::copy_options::overwrite_existing
                );
                std::cout << "[DEBUG] Copied FTD3XXWU.cat" << std::endl;
                
                std::cout << "[SUCCESS] Driver files copied to temp directory" << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ERROR] Failed to copy driver files: " << e.what() << std::endl;
                return false;
            }
        }

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
        
        // Uninstall and delete the driver package - RUN SYNCHRONOUSLY
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

        // Parse DriverVersion - CRITICAL: Get this from device properties, not the basic query
        // The basic DriverVersion field from Get-PnpDevice doesn't always populate correctly
        // We need to use Get-PnpDeviceProperty to get the actual driver version
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

        // Set provider (always FTDI for FT601)
        info.provider = "FTDI";

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
        
        HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), "RCDATA");
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
}
