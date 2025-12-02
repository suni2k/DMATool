#include "FT601DriverInterface.h"
#include "../Util/ResourceExtractor.h"
#include "../resource.h"
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <filesystem>

namespace DMATool::Backend
{
    FT601DriverInfo FT601DriverInterface::CheckDriver()
    {
        FT601DriverInfo info;
        std::string output;

        // Query all PnP devices for FTDI FT601
        std::string command = "Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_" + 
                            std::string(FT601_VID) + "&PID_" + std::string(FT601_PID) + "*'} | " +
                            "Select-Object FriendlyName, InstanceId, Status, DriverVersion | Format-List";

        if (ExecutePowerShell(command, output))
        {
            // Parse the output
            info = ParseDriverInfo(output);
            
            // Determine if it's the correct driver
            if (!info.deviceName.empty())
            {
                // Correct driver: "FT601 USB 3.0 Bridge Device" or "FTDI FT601 USB 3.0 Bridge Device"
                // Default/No driver: "FTDI SuperSpeed-FIFO Bridge" (base device name)
                if (info.deviceName.find("FT601 USB 3.0 Bridge Device") != std::string::npos ||
                    info.deviceName.find("USB 3.0 Bridge Device") != std::string::npos)
                {
                    info.isCorrectDriver = true;
                    info.installed = (info.installed && info.isCorrectDriver);
                }
                else if (info.deviceName.find("SuperSpeed-FIFO Bridge") != std::string::npos)
                {
                    // This is the default device name - no driver installed
                    info.isCorrectDriver = false;
                    info.installed = false;
                }
            }
        }

        return info;
    }

    bool FT601DriverInterface::InstallDriver()
    {
        std::cout << "[INFO] Installing FT601 driver..." << std::endl;
        
        std::string driverPath;
        
        // Try embedded resources first
        if (ExtractDriverFiles(driverPath))
        {
            std::cout << "[INFO] Using embedded driver files from temp" << std::endl;
        }
        else
        {
            // Fallback: Copy from external directory (same as CH347)
            std::cout << "[INFO] Embedded resources not found, using external driver files" << std::endl;
            
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
        
        std::cout << "[INFO] Using driver INF at: " << driverInfPath << std::endl;
        std::cout << "[INFO] Adding driver to Windows driver store..." << std::endl;

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
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output += buffer;
            std::cout << buffer;  // Echo output
        }

        int exitCode = _pclose(pipe);
        std::cout << "[DEBUG] pnputil exit code: " << exitCode << std::endl;
        
        bool success = (exitCode == 0);
        
        if (success)
        {
            std::cout << "[SUCCESS] FT601 driver added to Windows driver store" << std::endl;
            std::cout << "[INFO] Applying driver to device..." << std::endl;
            
            // Force Windows to rescan and apply the new driver
            std::string rescanCommand = "pnputil /scan-devices";
            std::cout << "[DEBUG] Rescanning devices..." << std::endl;
            system(rescanCommand.c_str());
            
            // Wait a moment for rescan
            Sleep(1000);
            
            // Now try to update the specific device
            std::string updateCommand = "powershell -Command \"$devices = Get-PnpDevice | Where-Object {$_.HardwareID -like '*VID_0403&PID_601F*'}; if ($devices) { foreach ($device in $devices) { try { Write-Output 'UPDATING'; pnputil /restart-device \\\"$($device.InstanceId)\\\" | Out-Null; Write-Output 'SUCCESS' } catch { Write-Output 'FAILED' } } } else { Write-Output 'NO_DEVICE' }\"";
            
            FILE* updatePipe = _popen(updateCommand.c_str(), "r");
            bool deviceUpdated = false;
            if (updatePipe)
            {
                while (fgets(buffer, sizeof(buffer), updatePipe) != nullptr)
                {
                    std::string line(buffer);
                    if (line.find("SUCCESS") != std::string::npos)
                    {
                        std::cout << "[SUCCESS] Device restarted with new driver" << std::endl;
                        deviceUpdated = true;
                    }
                    else if (line.find("FAILED") != std::string::npos || line.find("NO_DEVICE") != std::string::npos)
                    {
                        std::cout << "[INFO] Device restart not needed or failed" << std::endl;
                    }
                }
                _pclose(updatePipe);
            }
            
            if (!deviceUpdated)
            {
                std::cout << "[INFO] Driver installed - please unplug and replug the FT601 device" << std::endl;
                std::cout << "[INFO] Or restart the device in Device Manager to apply the driver" << std::endl;
            }
        }
        else
        {
            std::cerr << "[ERROR] Failed to add driver to Windows driver store" << std::endl;
        }
        
        // Cleanup temp files
        CleanupDriverFiles(driverPath);
        
        return success;
    }

    bool FT601DriverInterface::UninstallDriver()
    {
        std::string output;

        // Method 1: Try to find and delete the driver package
        std::string command = "pnputil /enum-drivers";
        
        if (!ExecutePowerShell(command, output))
        {
            return false;
        }

        // Look for FT601/FTD3XX driver
        std::regex oemRegex(R"(Published Name\s*:\s*(oem\d+\.inf).*?Original Name\s*:\s*FTD3XXWU\.inf)", std::regex::icase);
        std::smatch match;
        
        if (std::regex_search(output, match, oemRegex))
        {
            std::string oemInf = match[1].str();
            std::cout << "[INFO] Found driver package: " << oemInf << std::endl;
            
            // Uninstall and delete the driver package
            std::string uninstallCommand = "Start-Process pnputil.exe -ArgumentList '/delete-driver', '" + 
                                          oemInf + "', '/uninstall', '/force' -Verb RunAs -Wait -PassThru | Select-Object -ExpandProperty ExitCode";
            
            std::string uninstallOutput;
            if (ExecutePowerShell(uninstallCommand, uninstallOutput))
            {
                std::cout << "[SUCCESS] FT601 driver package deleted" << std::endl;
                
                // Method 2: Force device to re-enumerate (will show yellow triangle until driver reinstalled)
                std::string reenumCommand = "Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_0403&PID_601F*'} | " +
                                          std::string("ForEach-Object { pnputil /remove-device $_.InstanceId }");
                ExecutePowerShell(reenumCommand, output);
                
                return true;
            }
        }
        else
        {
            std::cout << "[WARNING] FT601 driver package not found in driver store" << std::endl;
        }

        return false;
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

        // Parse DriverVersion - fix: it was showing "FriendlyName : ..." instead of version
        std::regex versionRegex(R"(DriverVersion\s*:\s*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+))");
        if (std::regex_search(output, match, versionRegex))
        {
            info.version = match[1].str();
        }
        else
        {
            // Fallback: try to extract just numbers after DriverVersion
            std::regex versionFallbackRegex(R"(DriverVersion\s*:\s*(.+))");
            if (std::regex_search(output, match, versionFallbackRegex))
            {
                std::string versionLine = match[1].str();
                versionLine.erase(0, versionLine.find_first_not_of(" \t\r\n"));
                versionLine.erase(versionLine.find_last_not_of(" \t\r\n") + 1);
                
                // Only use if it looks like a version number
                if (versionLine.find_first_of("0123456789") == 0)
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
            std::cerr << "[ERROR] Failed to extract FT601 INF file" << std::endl;
            return false;
        }
        
        // Extract FT601 CAT file
        std::string catPath = outPath + "\\FTD3XXWU.cat";
        if (!ExtractResourceFile(IDR_FT601_CAT, catPath))
        {
            std::cerr << "[ERROR] Failed to extract FT601 CAT file" << std::endl;
            return false;
        }
        
        std::cout << "[INFO] FT601 driver files extracted to: " << outPath << std::endl;
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
