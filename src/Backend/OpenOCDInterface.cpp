#include "OpenOCDInterface.h"
#include <windows.h>
#include <filesystem>
#include <sstream>
#include <regex>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "../resource.h"

namespace DMATool::Backend
{
    OpenOCDInterface::OpenOCDInterface()
    {
        FindOpenOCD();
    }

    OpenOCDInterface::~OpenOCDInterface()
    {
    }

    bool OpenOCDInterface::ExtractEmbeddedResource(int resourceId, const std::string& outputPath)
    {
        // Find the resource
        HRSRC hResource = FindResourceA(NULL, MAKEINTRESOURCEA(resourceId), MAKEINTRESOURCEA(RT_RCDATA));
        if (!hResource)
            return false;

        // Load the resource
        HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
        if (!hLoadedResource)
            return false;

        // Lock the resource to get a pointer to the data
        LPVOID pResourceData = LockResource(hLoadedResource);
        if (!pResourceData)
            return false;

        // Get the size of the resource
        DWORD resourceSize = SizeofResource(NULL, hResource);
        if (resourceSize == 0)
            return false;

        // Create directory if it doesn't exist
        std::filesystem::path filePath(outputPath);
        std::filesystem::create_directories(filePath.parent_path());

        // Write to file
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile)
            return false;

        outFile.write(static_cast<const char*>(pResourceData), resourceSize);
        outFile.close();

        return true;
    }

    std::string OpenOCDInterface::GetTempDirectory()
    {
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        
        // Create DMATool subdirectory in temp
        std::string dmaToolTemp = std::string(tempPath) + "DMATool\\";
        std::filesystem::create_directories(dmaToolTemp);
        
        return dmaToolTemp;
    }

    bool OpenOCDInterface::FindOpenOCD()
    {
        // First, try to extract embedded resources to temp directory
        std::string tempDir = GetTempDirectory();
        std::string openocdPath = tempDir + "openocd.exe";
        std::string configPath = tempDir + "ch347.cfg";

        std::cout << "[DEBUG] Attempting to extract resources to: " << tempDir << std::endl;

        // Extract OpenOCD executable from resources
        if (ExtractEmbeddedResource(IDR_OPENOCD_EXE, openocdPath))
        {
            m_OpenOCDPath = openocdPath;
            std::cout << "[DEBUG] Extracted openocd.exe successfully" << std::endl;
            
            // Extract config files
            ExtractEmbeddedResource(IDR_CH347_CFG, configPath);
            m_ConfigPath = configPath;
            
            // Extract required DLLs
            bool dll1 = ExtractEmbeddedResource(IDR_LIBUSB_DLL, tempDir + "libusb-1.0.dll");
            bool dll2 = ExtractEmbeddedResource(IDR_LIBHIDAPI_DLL, tempDir + "libhidapi-0.dll");
            std::cout << "[DEBUG] Extracted DLLs: libusb=" << dll1 << ", libhidapi=" << dll2 << std::endl;
            
            // Extract Xilinx DNA extraction scripts
            bool cfg1 = ExtractEmbeddedResource(IDR_XILINX_DNA_347_CFG, tempDir + "xilinx-dna-347.cfg");
            bool cfg2 = ExtractEmbeddedResource(IDR_XILINX_XC7_CFG, tempDir + "xilinx-xc7.cfg");
            bool cfg3 = ExtractEmbeddedResource(IDR_JTAGSPI_CFG, tempDir + "jtagspi.cfg");
            bool cfg4 = ExtractEmbeddedResource(IDR_XILINX_DNA_CFG, tempDir + "xilinx-dna.cfg");
            std::cout << "[DEBUG] Extracted CFGs: dna347=" << cfg1 << ", xc7=" << cfg2 << ", jtagspi=" << cfg3 << ", dna=" << cfg4 << std::endl;
            
            return true;
        }

        std::cout << "[DEBUG] Failed to extract embedded resources, trying fallback paths" << std::endl;

        // Fallback: Check multiple possible locations for OpenOCD
        std::vector<std::string> possiblePaths = {
            "openocd.exe",
            "tools\\openocd.exe",
            "dmafiles\\ch347\\OpenOCD_CH347\\bin\\openocd.exe",
            "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\openocd.exe",
            "tools\\openocd\\openocd-347.exe",
            "tools\\openocd\\openocd.exe"
        };

        for (const auto& path : possiblePaths)
        {
            if (std::filesystem::exists(path))
            {
                m_OpenOCDPath = path;
                
                // Try to find corresponding config file
                std::vector<std::string> configPaths = {
                    "ch347.cfg",
                    "tools\\ch347.cfg",
                    "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\ch347.cfg",
                };
                
                for (const auto& cfgPath : configPaths)
                {
                    if (std::filesystem::exists(cfgPath))
                    {
                        m_ConfigPath = cfgPath;
                        break;
                    }
                }
                
                return true;
            }
        }

        return false;
    }

    std::string OpenOCDInterface::ExecuteCommand(const std::string& command)
    {
        std::array<char, 128> buffer;
        std::string result;

        // Output command to console for debugging
        std::cout << "[EXEC] Running command: " << command << std::endl;

        // Create a pipe to capture output
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe)
        {
            std::cout << "[ERROR] Failed to create pipe for command" << std::endl;
            return "ERROR: Failed to execute command";
        }

        // Read output
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            result += buffer.data();
            // Also output to console in real-time
            std::cout << buffer.data();
        }

        int exitCode = _pclose(pipe);
        std::cout << "[EXEC] Command completed with exit code: " << exitCode << std::endl;
        
        return result;
    }

    AdapterType OpenOCDInterface::DetectAdapterType()
    {
        // Query USB devices for CH347
        std::string psCommand = R"(powershell -Command "Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'} | Select-Object -First 1 | Select-Object -ExpandProperty FriendlyName")";
        std::string output = ExecuteCommand(psCommand);

        if (output.find("CH347") != std::string::npos)
        {
            return AdapterType::CH347;
        }

        // Check for FTDI (RS232)
        psCommand = R"(powershell -Command "Get-PnpDevice | Where-Object {$_.FriendlyName -like '*FTDI*'} | Select-Object -First 1 | Select-Object -ExpandProperty FriendlyName")";
        output = ExecuteCommand(psCommand);

        if (output.find("FTDI") != std::string::npos)
        {
            return AdapterType::RS232;
        }

        return AdapterType::Unknown;
    }

    ChipModel OpenOCDInterface::IDCodeToChipModel(uint32_t idcode)
    {
        // Extract part number from IDCODE (bits 27:12)
        uint32_t partNumber = (idcode >> 12) & 0xFFFF;

        switch (partNumber)
        {
        case 0x3622: return ChipModel::XC7A35T;
        case 0x362D: return ChipModel::XC7A75T;
        case 0x3631: return ChipModel::XC7A100T;
        default: return ChipModel::Unknown;
        }
    }

    std::string OpenOCDInterface::GetChipModelName(ChipModel model)
    {
        switch (model)
        {
        case ChipModel::XC7A35T: return "XC7A35T";
        case ChipModel::XC7A75T: return "XC7A75T";
        case ChipModel::XC7A100T: return "XC7A100T";
        default: return "Unknown";
        }
    }

    std::string OpenOCDInterface::FormatDNA(const std::string& dna)
    {
        // Remove "0x" prefix if present
        std::string formatted = dna;
        if (formatted.find("0x") == 0 || formatted.find("0X") == 0)
        {
            formatted = formatted.substr(2);
        }
        return formatted;
    }

    FPGAInfo OpenOCDInterface::ParseOpenOCDOutput(const std::string& output)
    {
        FPGAInfo info;

        // Parse IDCODE: "JTAG tap: xc7.tap tap/device found: 0x13631093"
        std::regex idcodeRegex(R"(tap/device found:\s*0x([0-9A-Fa-f]+))");
        std::smatch match;

        if (std::regex_search(output, match, idcodeRegex))
        {
            info.detected = true;
            std::string idcodeStr = match[1].str();
            info.idcode = std::stoul(idcodeStr, nullptr, 16);
            info.chipModel = IDCodeToChipModel(info.idcode);
            info.partNumber = GetChipModelName(info.chipModel);
            info.manufacturer = "Xilinx";
            info.family = "Artix-7";

            // Set logic cells based on chip model
            switch (info.chipModel)
            {
            case ChipModel::XC7A35T: info.logicCells = "33,280"; break;
            case ChipModel::XC7A75T: info.logicCells = "75,520"; break;
            case ChipModel::XC7A100T: info.logicCells = "101,440"; break;
            default: info.logicCells = "Unknown"; break;
            }
        }

        // Parse DNA - try multiple formats
        
        // Format 1: "DNA = 000111...100 (0x003ccd8c77d04854)" - from xilinx-dna.cfg scripts
        std::regex dnaRegex1(R"(DNA\s*=\s*[01]+\s*\(0x([0-9A-Fa-f]+)\))");
        if (std::regex_search(output, match, dnaRegex1))
        {
            info.dnaId = FormatDNA(match[1].str());
        }
        
        // Format 2: Raw hex output from drscan - look for 16-digit hex after the last drscan
        // Example: "0000000013631093" or just a standalone hex value
        if (info.dnaId.empty())
        {
            // Look for 16-digit hex number (64-bit DNA value)
            std::regex dnaRegex2(R"(\b([0-9a-fA-F]{16})\b)");
            std::sregex_iterator iter(output.begin(), output.end(), dnaRegex2);
            std::sregex_iterator end;
            
            // Find all 16-digit hex values
            std::vector<std::string> hexValues;
            while (iter != end)
            {
                hexValues.push_back((*iter)[1].str());
                ++iter;
            }
            
            // The DNA is typically the last 16-digit hex value before shutdown
            if (!hexValues.empty())
            {
                // Try to find one that's not all zeros or all IDCODEs
                for (auto it = hexValues.rbegin(); it != hexValues.rend(); ++it)
                {
                    std::string hexVal = *it;
                    // Check if it's not just an IDCODE (IDCODEs typically have pattern like 13631093)
                    if (hexVal != "0000000000000000" && hexVal.length() == 16)
                    {
                        // This might be the DNA - verify it's different from IDCODE patterns
                        uint64_t val = std::stoull(hexVal, nullptr, 16);
                        if (val != 0)
                        {
                            info.dnaId = FormatDNA(hexVal);
                            break;
                        }
                    }
                }
            }
        }
        
        // Format 3: "DR Data: 00542417dc636678" format from working tool
        if (info.dnaId.empty())
        {
            std::regex dnaRegex3(R"(DR Data:\s*([0-9A-Fa-f]+))");
            if (std::regex_search(output, match, dnaRegex3))
            {
                std::string hexVal = match[1].str();
                if (hexVal.length() >= 14) // At least 57 bits (14+ hex digits)
                {
                    info.dnaId = FormatDNA(hexVal);
                }
            }
        }

        return info;
    }

    FPGAInfo OpenOCDInterface::DetectFPGA(std::function<void(const std::string&)> logCallback)
    {
        FPGAInfo info;

        if (logCallback) logCallback("[INFO] Starting FPGA detection...");

        // Check if OpenOCD exists
        if (m_OpenOCDPath.empty() && !FindOpenOCD())
        {
            if (logCallback) logCallback("[ERROR] OpenOCD executable not found");
            return info;
        }

        if (logCallback) logCallback("[INFO] OpenOCD found at: " + m_OpenOCDPath);

        // Detect adapter type
        AdapterType adapter = DetectAdapterType();
        info.adapterType = adapter;

        if (adapter == AdapterType::Unknown)
        {
            if (logCallback) logCallback("[ERROR] No JTAG adapter detected (CH347 or FTDI)");
            return info;
        }

        if (logCallback)
        {
            std::string adapterName = (adapter == AdapterType::CH347) ? "CH347" : "RS232/FTDI";
            logCallback("[INFO] Detected adapter: " + adapterName);
        }

        // Get temp directory where cfg files are extracted
        std::string tempDir = GetTempDirectory();
        
        // Replace backslashes with forward slashes for OpenOCD (it's Unix-style)
        std::string tempDirUnix = tempDir;
        std::replace(tempDirUnix.begin(), tempDirUnix.end(), '\\', '/');
        
        // Build OpenOCD command - use cmd /c to properly handle quotes
        std::string command = "cmd /c \"\"" + m_OpenOCDPath + "\"";
        
        // Add interface configuration inline - matching working config files
        if (adapter == AdapterType::CH347)
        {
            // From init_347_75t.cfg (working)
            command += " -c \"adapter driver ch347\"";
            command += " -c \"ch347 vid_pid 0x1a86 0x55dd\"";
            command += " -c \"adapter speed 10000\"";
            command += " -c \"source {" + tempDirUnix + "xilinx-dna-347.cfg}\"";
            command += " -c \"source {" + tempDirUnix + "xilinx-xc7.cfg}\"";
            command += " -c \"source {" + tempDirUnix + "jtagspi.cfg}\"";
        }
        else // FTDI/RS232
        {
            // From init_232_35t.cfg (working)
            command += " -c \"interface ftdi\"";
            command += " -c \"ftdi_vid_pid 0x0403 0x6011\"";
            command += " -c \"ftdi_channel 0\"";
            command += " -c \"ftdi_layout_init 0x0098 0x008b\"";
            command += " -c \"reset_config none\"";
            command += " -c \"adapter_khz 10000\"";
            command += " -c \"source {" + tempDirUnix + "xilinx-dna.cfg}\"";
            command += " -c \"source {" + tempDirUnix + "xilinx-xc7.cfg}\"";
            command += " -c \"source {" + tempDirUnix + "jtagspi.cfg}\"";
        }
        
        // Add init and DNA extraction procedure (matching working bat files)
        command += " -c \"init\"";
        command += " -c \"set dna [xc7_get_dna xc7.tap]\"";
        command += " -c \"xilinx_print_dna $dna\"";
        command += " -c \"shutdown\"";
        command += " 2>&1\"";

        if (logCallback) logCallback("[INFO] Executing OpenOCD...");

        // Execute OpenOCD
        std::string output = ExecuteCommand(command);

        if (logCallback) 
        {
            logCallback("[DEBUG] OpenOCD output received");
            // Log the actual output for debugging
            logCallback("[DEBUG] OpenOCD raw output:");
            std::istringstream stream(output);
            std::string line;
            int lineCount = 0;
            while (std::getline(stream, line) && lineCount < 40) // Show more lines for debugging
            {
                logCallback("[DEBUG]   " + line);
                lineCount++;
            }
        }

        // Parse output
        info = ParseOpenOCDOutput(output);
        info.adapterType = adapter;

        if (info.detected)
        {
            if (logCallback)
            {
                logCallback("[SUCCESS] FPGA detected: " + info.partNumber);
                
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << info.idcode;
                logCallback("[INFO] IDCODE: " + ss.str());
                
                if (!info.dnaId.empty())
                {
                    logCallback("[INFO] DNA ID: " + info.dnaId);
                }
                else
                {
                    logCallback("[WARNING] DNA ID not extracted - check debug output for DNA = line");
                }
            }
        }
        else
        {
            if (logCallback) 
            {
                logCallback("[ERROR] Failed to detect FPGA");
                logCallback("[INFO] Possible issues:");
                logCallback("[INFO]   - JTAG cable not connected to DMA card");
                logCallback("[INFO]   - JTAG Drivers not installed");
                logCallback("[INFO]   - DMA card not powered on");
                logCallback("[INFO]   - Wrong JTAG pins (check TDI, TDO, TCK, TMS, GND)");
                logCallback("[INFO]   - Try running DMATool as Administrator");
            }
        }

        return info;
    }

    DriverInfo OpenOCDInterface::CheckCH347Driver()
    {
        DriverInfo info;

        // PowerShell command to check for CH347 driver
        std::string psCommand = R"(powershell -Command "$device = Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'} | Select-Object -First 1; if ($device) { $props = @{}; $props['Status'] = $device.Status; $props['FriendlyName'] = $device.FriendlyName; try { $props['DriverVersion'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data } catch { $props['DriverVersion'] = 'Unknown' }; try { $props['DriverDate'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverDate' -ErrorAction SilentlyContinue).Data } catch { $props['DriverDate'] = 'Unknown' }; try { $props['DriverProvider'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverProvider' -ErrorAction SilentlyContinue).Data } catch { $props['DriverProvider'] = 'Unknown' }; try { $props['HardwareIds'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_HardwareIds' -ErrorAction SilentlyContinue).Data[0] } catch { $props['HardwareIds'] = 'Unknown' }; Write-Output \"STATUS:$($props['Status'])\"; Write-Output \"NAME:$($props['FriendlyName'])\"; Write-Output \"VERSION:$($props['DriverVersion'])\"; Write-Output \"DATE:$($props['DriverDate'])\"; Write-Output \"PROVIDER:$($props['DriverProvider'])\"; Write-Output \"HWID:$($props['HardwareIds'])\" } else { Write-Output 'NOT_INSTALLED' }")";

        std::string output = ExecuteCommand(psCommand);

        if (output.find("NOT_INSTALLED") != std::string::npos)
        {
            info.installed = false;
            return info;
        }

        info.installed = true;

        // Parse output
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.find("STATUS:") == 0)
            {
                std::string status = line.substr(7);
                info.installed = (status.find("OK") != std::string::npos);
            }
            else if (line.find("NAME:") == 0)
            {
                info.deviceName = line.substr(5);
            }
            else if (line.find("VERSION:") == 0)
            {
                info.version = line.substr(8);
            }
            else if (line.find("DATE:") == 0)
            {
                info.date = line.substr(5);
            }
            else if (line.find("PROVIDER:") == 0)
            {
                info.provider = line.substr(9);
            }
            else if (line.find("HWID:") == 0)
            {
                std::string hwid = line.substr(5);
                // Extract VID/PID from hardware ID
                std::regex vidPidRegex(R"(VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4}))");
                std::smatch match;
                if (std::regex_search(hwid, match, vidPidRegex))
                {
                    info.vidPid = match[1].str() + ":" + match[2].str();
                }
            }
        }

        return info;
    }

    bool OpenOCDInterface::InstallCH347Driver()
    {
        std::cout << "[INFO] Installing CH347 driver..." << std::endl;
        
        // Check if we need to uninstall existing WCH driver first
        std::cout << "[INFO] Checking for existing WCH drivers..." << std::endl;
        
        DriverInfo currentDriver = CheckCH347Driver();
        
        if (currentDriver.installed && currentDriver.provider.find("wch") != std::string::npos)
        {
            std::cout << "[WARNING] Found existing WCH driver (possibly wrong version)" << std::endl;
            std::cout << "[INFO] Current driver: " << currentDriver.deviceName << std::endl;
            std::cout << "[INFO] Version: " << currentDriver.version << std::endl;
            
            // Check if it's the wrong driver (not HighSpeed-JTAG)
            if (currentDriver.deviceName.find("HighSpeed-JTAG") == std::string::npos)
            {
                std::cout << "[INFO] Uninstalling old driver first..." << std::endl;
                
                // Uninstall the old driver
                if (UninstallCH347Driver())
                {
                    std::cout << "[SUCCESS] Old driver uninstalled successfully" << std::endl;
                    std::cout << "[INFO] Waiting for device to be ready..." << std::endl;
                    Sleep(2000);  // Wait for Windows to detect the device again
                }
                else
                {
                    std::cout << "[ERROR] Failed to uninstall old driver" << std::endl;
                    std::cout << "[INFO] Continuing with installation attempt..." << std::endl;
                }
            }
            else
            {
                std::cout << "[INFO] Correct driver already installed!" << std::endl;
                return true;
            }
        }
        
        // Find the driver folder in the source directory
        std::cout << "[INFO] Looking for driver files..." << std::endl;
        
        // Get executable directory
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
        
        std::cout << "[DEBUG] Executable directory: " << exeDir.string() << std::endl;
        
        // Look for drivers in multiple locations relative to exe
        std::vector<std::filesystem::path> searchPaths = {
            exeDir / "tools" / "ch347" / "drivers",
            exeDir / ".." / "tools" / "ch347" / "drivers",  // One level up (for Debug/Release builds)
            exeDir / ".." / ".." / "tools" / "ch347" / "drivers",  // Two levels up
            std::filesystem::path("C:/Users/suni/source/repos/DMATool/tools/ch347/drivers")  // Absolute fallback for development
        };
        
        std::filesystem::path sourceDriverDir;
        for (const auto& path : searchPaths)
        {
            if (std::filesystem::exists(path / "CH341WDM.INF"))
            {
                sourceDriverDir = std::filesystem::canonical(path);
                std::cout << "[INFO] Found drivers at: " << sourceDriverDir.string() << std::endl;
                break;
            }
        }
        
        if (sourceDriverDir.empty())
        {
            std::cout << "[ERROR] Could not find driver files in any search location" << std::endl;
            std::cout << "[INFO] Searched locations:" << std::endl;
            for (const auto& path : searchPaths)
            {
                std::cout << "[INFO]   - " << path.string() << std::endl;
            }
            std::cout << "[INFO] Opening browser to download driver manually..." << std::endl;
            ShellExecuteA(nullptr, "open", "https://www.wch.cn/downloads/CH347SER_ZIP.html", nullptr, nullptr, SW_SHOWNORMAL);
            return false;
        }
        
        // Copy entire driver folder to temp directory
        std::string tempDir = GetTempDirectory();
        std::filesystem::path destDriverDir = std::filesystem::path(tempDir) / "drivers";
        
        try
        {
            // Remove old temp drivers if they exist
            if (std::filesystem::exists(destDriverDir))
            {
                std::filesystem::remove_all(destDriverDir);
            }
            
            // Create destination directory
            std::filesystem::create_directories(destDriverDir);
            
            // Copy all files from source to destination
            std::cout << "[INFO] Copying driver files to temp directory..." << std::endl;
            for (const auto& entry : std::filesystem::directory_iterator(sourceDriverDir))
            {
                if (entry.is_regular_file())
                {
                    std::filesystem::path destFile = destDriverDir / entry.path().filename();
                    std::filesystem::copy_file(entry.path(), destFile, std::filesystem::copy_options::overwrite_existing);
                    std::cout << "[DEBUG] Copied: " << entry.path().filename().string() << std::endl;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to copy driver files: " << e.what() << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Driver files copied successfully" << std::endl;
        
        // Use the copied INF file
        std::filesystem::path infPath = destDriverDir / "CH341WDM.INF";
        std::cout << "[INFO] Using driver INF at: " << infPath.string() << std::endl;
        
        // Verify INF exists
        if (!std::filesystem::exists(infPath))
        {
            std::cout << "[ERROR] INF file not found after copy: " << infPath.string() << std::endl;
            return false;
        }
        
        // Step 1: Add driver to Windows driver store using pnputil
        std::cout << "[INFO] Adding driver to Windows driver store..." << std::endl;
        
        // Build command parameters - MUST persist for entire ShellExecuteExA call
        std::string params = "/add-driver \"" + infPath.string() + "\" /install";
        
        // Execute with admin privileges
        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
        sei.lpVerb = "runas";  // Request admin
        sei.lpFile = "pnputil.exe";
        sei.lpParameters = params.c_str();  // Use c_str() from persistent string
        sei.nShow = SW_HIDE;
        
        std::cout << "[DEBUG] Running: pnputil.exe " << params << std::endl;
        
        if (!ShellExecuteExA(&sei))
        {
            DWORD error = GetLastError();
            std::cout << "[ERROR] Failed to run pnputil (error: " << error << ")" << std::endl;
            if (error == ERROR_CANCELLED)
            {
                std::cout << "[INFO] User cancelled UAC prompt" << std::endl;
            }
            return false;
        }
        
        // Wait for installation to complete
        if (sei.hProcess)
        {
            DWORD waitResult = WaitForSingleObject(sei.hProcess, 30000); // 30 second timeout
            
            if (waitResult == WAIT_TIMEOUT)
            {
                std::cout << "[WARNING] Driver installation timed out" << std::endl;
                TerminateProcess(sei.hProcess, 1);
                CloseHandle(sei.hProcess);
                return false;
            }
            
            DWORD exitCode = 0;
            GetExitCodeProcess(sei.hProcess, &exitCode);
            CloseHandle(sei.hProcess);
            
            std::cout << "[DEBUG] pnputil exit code: " << exitCode << std::endl;
            
            if (exitCode == 0)
            {
                std::cout << "[SUCCESS] Driver added to Windows driver store" << std::endl;
            }
            else if (exitCode == 259)
            {
                // ERROR_DRIVER_STORE_ADD_FAILED or driver already exists
                std::cout << "[WARNING] pnputil returned exit code: " << exitCode << std::endl;
                std::cout << "[INFO] Driver may already be in store or installation failed" << std::endl;
                std::cout << "[INFO] Continuing to try device update..." << std::endl;
            }
            else
            {
                std::cout << "[ERROR] pnputil failed with exit code: " << exitCode << std::endl;
                std::cout << "[INFO] This may mean the driver is incompatible or already installed" << std::endl;
                std::cout << "[INFO] Trying to continue anyway..." << std::endl;
            }
        }
        
        // Step 2: Wait for device to be detected
        std::cout << "[INFO] Waiting for Windows to detect CH347 device..." << std::endl;
        Sleep(2000);
        
        // Step 3: Force Windows to update the device driver
        std::cout << "[INFO] Updating device with new driver..." << std::endl;
        
        std::string psCommand = R"(powershell -Command "$devices = Get-PnpDevice | Where-Object {$_.HardwareID -like '*VID_1A86&PID_55DD*' -or $_.HardwareID -like '*VID_1A86&PID_55DE*'}; if ($devices) { foreach ($device in $devices) { try { Write-Output 'UPDATING_DEVICE'; Update-PnpDevice -InstanceId $device.InstanceId -ErrorAction Stop; Write-Output 'UPDATE_SUCCESS' } catch { Write-Output 'UPDATE_FAILED' } } } else { Write-Output 'NO_DEVICE' }")";
        
        std::string output = ExecuteCommand(psCommand);
        
        if (output.find("NO_DEVICE") != std::string::npos)
        {
            std::cout << "[ERROR] CH347 device not detected" << std::endl;
            std::cout << "[INFO] Please make sure the CH347 adapter is plugged in" << std::endl;
            return false;
        }
        else if (output.find("UPDATE_SUCCESS") != std::string::npos)
        {
            std::cout << "[SUCCESS] Device driver updated successfully" << std::endl;
        }
        else if (output.find("UPDATE_FAILED") != std::string::npos)
        {
            std::cout << "[WARNING] Driver update failed - device may need manual driver installation" << std::endl;
            std::cout << "[INFO] Try: Device Manager → Right-click device → Update driver" << std::endl;
            std::cout << "[INFO] Point to: " << destDriverDir.string() << std::endl;
        }
        
        // Step 4: Verify the new driver
        std::cout << "[INFO] Verifying driver installation..." << std::endl;
        Sleep(2000);
        
        DriverInfo newDriver = CheckCH347Driver();
        
        if (newDriver.installed)
        {
            std::cout << "[SUCCESS] Driver installation complete!" << std::endl;
            std::cout << "[INFO] Device: " << newDriver.deviceName << std::endl;
            std::cout << "[INFO] Version: " << newDriver.version << std::endl;
            std::cout << "[INFO] Provider: " << newDriver.provider << std::endl;
            
            if (newDriver.deviceName.find("HighSpeed-JTAG") != std::string::npos)
            {
                std::cout << "[SUCCESS] Correct CH347 JTAG driver is now installed!" << std::endl;
                return true;
            }
            else
            {
                std::cout << "[WARNING] Driver installed but may not be the correct version" << std::endl;
                std::cout << "[INFO] Expected: USB HighSpeed-JTAG/I2C... CH347T" << std::endl;
                std::cout << "[INFO] Got: " << newDriver.deviceName << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "[ERROR] Driver installation verification failed" << std::endl;
            std::cout << "[INFO] The driver files were extracted but Windows couldn't apply them" << std::endl;
            std::cout << "[INFO] You may need to manually install using Device Manager:" << std::endl;
            std::cout << "[INFO] 1. Open Device Manager" << std::endl;
            std::cout << "[INFO] 2. Find the CH347 device (may show as Unknown)" << std::endl;
            std::cout << "[INFO] 3. Right-click → Update driver → Browse" << std::endl;
            std::cout << "[INFO] 4. Point to: " << destDriverDir.string() << std::endl;
            return false;
        }
    }

    bool OpenOCDInterface::UninstallCH347Driver()
    {
        std::cout << "[INFO] Uninstalling CH347 driver..." << std::endl;
        
        // Step 1: Find all WCH-related driver packages using pnputil
        std::cout << "[INFO] Finding WCH driver packages..." << std::endl;
        
        // Write output to temp file to avoid buffering issues
        std::string tempFile = GetTempDirectory() + "wch_drivers.txt";
        
        // PowerShell script to find WCH drivers and output to file
        std::string findCommand = "powershell -Command \"$output = pnputil /enum-drivers; $lines = $output -split '`n'; $currentInf = ''; $isWch = $false; foreach ($line in $lines) { $line = $line.Trim(); if ($line -match 'Published [Nn]ame\\s*:\\s*(.+\\.inf)') { $currentInf = $matches[1].Trim(); $isWch = $false } elseif ($line -match '[Pp]rovider.*wch') { $isWch = $true; if ($currentInf) { Write-Output $currentInf } } }\" > \"" + tempFile + "\"";
        
        // Execute command
        std::cout << "[DEBUG] Running: " << findCommand << std::endl;
        int result = system(findCommand.c_str());
        std::cout << "[DEBUG] Command exit code: " << result << std::endl;
        
        // Small delay to ensure file is written
        Sleep(500);
        
        // Read the temp file
        std::vector<std::string> driverInfs;
        std::ifstream inFile(tempFile);
        
        if (!inFile.is_open())
        {
            std::cout << "[ERROR] Failed to read temp file: " << tempFile << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(inFile, line))
        {
            // Trim whitespace and newlines
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            // Check if it's an INF file
            if (!line.empty() && line.find(".inf") != std::string::npos)
            {
                driverInfs.push_back(line);
                std::cout << "[INFO] Found WCH driver package: " << line << std::endl;
            }
        }
        
        inFile.close();
        
        // Clean up temp file
        DeleteFileA(tempFile.c_str());
        
        if (driverInfs.empty())
        {
            std::cout << "[WARNING] No WCH driver packages found" << std::endl;
            std::cout << "[ERROR] Failed to uninstall driver. Administrator privileges required." << std::endl;
            return false;
        }
        
        // Step 2: Uninstall each driver package with elevated privileges
        bool anySuccess = false;
        for (const auto& infName : driverInfs)
        {
            std::cout << "[INFO] Uninstalling driver package: " << infName << std::endl;
            
            // Use ShellExecute with runas to get admin privileges
            SHELLEXECUTEINFOA uninstallSei = { sizeof(uninstallSei) };
            uninstallSei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
            uninstallSei.lpVerb = "runas";
            uninstallSei.lpFile = "pnputil.exe";
            
            std::string params = "/delete-driver " + infName + " /uninstall /force";
            uninstallSei.lpParameters = params.c_str();
            uninstallSei.nShow = SW_HIDE;
            
            if (ShellExecuteExA(&uninstallSei))
            {
                // Wait for the process to complete
                if (uninstallSei.hProcess)
                {
                    DWORD waitResult = WaitForSingleObject(uninstallSei.hProcess, 30000); // 30 second timeout
                    
                    if (waitResult == WAIT_TIMEOUT)
                    {
                        std::cout << "[WARNING] Uninstall operation timed out for: " << infName << std::endl;
                        TerminateProcess(uninstallSei.hProcess, 1);
                        CloseHandle(uninstallSei.hProcess);
                        continue;
                    }
                    
                    DWORD exitCode = 0;
                    GetExitCodeProcess(uninstallSei.hProcess, &exitCode);
                    CloseHandle(uninstallSei.hProcess);
                    
                    if (exitCode == 0)
                    {
                        std::cout << "[SUCCESS] Uninstalled: " << infName << std::endl;
                        anySuccess = true;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to uninstall: " << infName << " (exit code: " << exitCode << ")" << std::endl;
                    }
                }
            }
            else
            {
                DWORD error = GetLastError();
                std::cout << "[ERROR] Failed to launch pnputil for: " << infName << " (error code: " << error << ")" << std::endl;
                if (error == ERROR_CANCELLED)
                {
                    std::cout << "[INFO] User cancelled UAC prompt" << std::endl;
                }
            }
        }
        
        if (!anySuccess)
        {
            std::cout << "[ERROR] Failed to uninstall driver. Administrator privileges required." << std::endl;
        }
        
        return anySuccess;
    }
}
