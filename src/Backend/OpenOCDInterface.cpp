#include "OpenOCDInterface.h"
#include "../VMProtectConfig.h"  // VMProtect SDK integration
#include "../resource.h"
#include "../Util/ResourceExtractor.h"
#include "../DriverUpdateAPI.h"  // Windows driver update API
#include <filesystem>
#include <iostream>
#include <sstream>      // Added for std::stringstream, std::istringstream
#include <fstream>      // Added for std::ifstream, std::ofstream
#include <iomanip>      // Added for std::setfill, std::setw
#include <regex>
#include <Windows.h>

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
        // If file already exists, don't re-extract (optimization)
        if (std::filesystem::exists(outputPath))
        {
            return true;
        }

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
        // Extract embedded resources to temp directory
        std::string tempDir = GetTempDirectory();
        std::string openocdPath = tempDir + "openocd.exe";
        std::string openocd347Path = tempDir + "openocd-347.exe";
        std::string configPath = tempDir + "ch347.cfg";

        std::cout << "[INFO] Searching for OpenOCD executable..." << std::endl;
        std::cout << "[DEBUG] Attempting to extract resources to: " << tempDir << std::endl;

        // Extract both OpenOCD executables from resources
        bool extractedMain = ExtractEmbeddedResource(IDR_OPENOCD_EXE, openocdPath);
        bool extracted347 = ExtractEmbeddedResource(IDR_OPENOCD_347_EXE, openocd347Path);
        
        if (extractedMain || extracted347)
        {
            m_OpenOCDPath = openocdPath;           // FTDI/RS232 version
            m_OpenOCD347Path = openocd347Path;     // CH347 version
            std::cout << "[DEBUG] Extracted openocd.exe successfully" << std::endl;
            
            // Extract config files
            ExtractEmbeddedResource(IDR_CH347_CFG, configPath);
            m_ConfigPath = configPath;
            
            // Extract required DLLs
            bool dll1 = ExtractEmbeddedResource(IDR_LIBUSB_DLL, tempDir + "libusb-1.0.dll");
            bool dll2 = ExtractEmbeddedResource(IDR_LIBHIDAPI_DLL, tempDir + "libhidapi-0.dll");
            std::cout << "[DEBUG] Extracted DLLs: libusb=" << dll1 << ", libhidapi=" << dll2 << std::endl;
            
            // Create cpld subdirectory for OpenOCD scripts (OpenOCD expects scripts in cpld/ subdirectory)
            std::string cpldDir = tempDir + "cpld\\";
            std::filesystem::create_directories(cpldDir);
            
            // Extract Xilinx config files to cpld/ subdirectory so [find cpld/...] works
            bool cfg1 = ExtractEmbeddedResource(IDR_XILINX_DNA_347_CFG, cpldDir + "xilinx-dna-347.cfg");
            bool cfg2 = ExtractEmbeddedResource(IDR_XILINX_XC7_CFG, cpldDir + "xilinx-xc7.cfg");
            bool cfg3 = ExtractEmbeddedResource(IDR_JTAGSPI_CFG, cpldDir + "jtagspi.cfg");
            bool cfg4 = ExtractEmbeddedResource(IDR_XILINX_DNA_CFG, cpldDir + "xilinx-dna.cfg");
            std::cout << "[DEBUG] Extracted CFGs to cpld/: dna347=" << cfg1 << ", xc7=" << cfg2 << ", jtagspi=" << cfg3 << ", dna=" << cfg4 << std::endl;
            
            return true;
        }

        std::cout << "[ERROR] Failed to extract OpenOCD from embedded resources" << std::endl;
        std::cout << "[ERROR] The application may not have been built correctly" << std::endl;
        std::cout << "[ERROR] Please rebuild DMATool and ensure DMATool.rc includes all OpenOCD resources" << std::endl;
        
        return false;
    }

    std::string OpenOCDInterface::ExecuteCommand(const std::string& command)
    {
        VMPROTECT_MUTATE_BLOCK("JTAG_Execute");
        
        std::string result;

        // Output command to console for debugging
        std::cout << "[EXEC] Running command: " << command << std::endl;

        // Use CreateProcess instead of _popen for better process management
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hStdOutRead, hStdOutWrite;
        CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
        SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hStdOutWrite;
        si.hStdError = hStdOutWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        PROCESS_INFORMATION pi = { 0 };

        // Create mutable copy of command
        std::string cmdCopy = command;

        BOOL success = CreateProcessA(
            NULL,
            const_cast<char*>(cmdCopy.c_str()),
            NULL, NULL, TRUE,
            CREATE_NO_WINDOW,
            NULL, NULL,
            &si, &pi
        );

        CloseHandle(hStdOutWrite);

        if (!success)
        {
            std::cout << "[ERROR] Failed to create process" << std::endl;
            CloseHandle(hStdOutRead);
            return "ERROR: Failed to execute command";
        }

        // Read output in chunks
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result += buffer;
            // Also output to console in real-time
            std::cout << buffer;
        }

        // Wait for process to complete (with timeout)
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000); // 30 second timeout
        
        if (waitResult == WAIT_TIMEOUT)
        {
            std::cout << "[WARNING] Command timed out after 30 seconds, terminating..." << std::endl;
            TerminateProcess(pi.hProcess, 1);
        }

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        // Clean up process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hStdOutRead);

        std::cout << "[EXEC] Command completed with exit code: " << exitCode << std::endl;
        
        VMPROTECT_END_BLOCK();
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

        // Check for FTDI/RS232 - look for FTDI devices or Quad RS232 (WinUSB installed)
        // Also check by VID/PID (0403:6011) to be more reliable
        psCommand = R"(powershell -Command "$device = Get-PnpDevice | Where-Object {($_.FriendlyName -like '*FTDI*') -or ($_.FriendlyName -like '*Quad RS232*') -or ($_.InstanceId -like '*VID_0403&PID_6011*')} | Select-Object -First 1; if ($device) { Write-Output $device.FriendlyName }")";
        output = ExecuteCommand(psCommand);

        if (!output.empty() && output.find_first_not_of(" \t\n\r") != std::string::npos)
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
        case 0x3632: return ChipModel::XC7A75T;  // NOTE: Some 75T cards report 50T IDCODE (remarked/binned chips)
        case 0x362D: return ChipModel::XC7A75T;  // XC7A75T (official IDCODE)
        case 0x3631: return ChipModel::XC7A100T;
        default: return ChipModel::Unknown;
        }
    }

    std::string OpenOCDInterface::GetChipModelName(ChipModel model)
    {
        switch (model)
        {
        case ChipModel::XC7A35T: return "XC7A35T";
        case ChipModel::XC7A50T: return "XC7A50T";  // Note: Not used - 50T IDCODE maps to 75T
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
            case ChipModel::XC7A50T: info.logicCells = "52,160"; break;  // XC7A50T
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

    FPGAInfo OpenOCDInterface::DetectFPGA(AdapterType adapterType, std::function<void(const std::string&)> logCallback)
    {
        VMPROTECT_MUTATE_BLOCK("JTAG_Detect");
        
        FPGAInfo info;

        if (logCallback) logCallback("[INFO] Starting FPGA detection...");

        // Check if OpenOCD exists
        if (m_OpenOCDPath.empty() && !FindOpenOCD())
        {
            if (logCallback) logCallback("[ERROR] OpenOCD executable not found");
            return info;
        }

        if (logCallback) logCallback("[INFO] OpenOCD found at: " + m_OpenOCDPath);

        // Use provided adapter type, or detect if not provided
        AdapterType adapter = adapterType;
        if (adapter == AdapterType::Unknown)
        {
            adapter = DetectAdapterType();
            info.adapterType = adapter;
        }
        else
        {
            info.adapterType = adapter;
        }

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
        
        // Set OPENOCD_SCRIPTS environment variable so OpenOCD can find cpld/ subdirectory
        SetEnvironmentVariableA("OPENOCD_SCRIPTS", tempDir.c_str());
        if (logCallback) logCallback("[DEBUG] Set OPENOCD_SCRIPTS=" + tempDir);
        
        // Choose the correct OpenOCD binary based on adapter type
        std::string openocdBinary = (adapter == AdapterType::CH347) ? m_OpenOCD347Path : m_OpenOCDPath;
        
        // Build OpenOCD command - use cmd /c to properly handle quotes
        std::string command = "cmd /c \"\"" + openocdBinary + "\"";
        
        // Add interface configuration based on adapter type
        if (adapter == AdapterType::CH347)
        {
            // Use the CH347 DNA config which already includes adapter setup
            // xilinx-dna-347.cfg contains: adapter driver ch347, ch347 vid_pid, adapter speed, and sources xilinx-xc7.cfg
            command += " -c \"source [find cpld/xilinx-dna-347.cfg]\"";
            // Source xilinx-dna.cfg to get xc7_get_dna command
            command += " -c \"source [find cpld/xilinx-dna.cfg]\"";
        }
        else // FTDI/RS232
        {
            // For FTDI, we need to configure the adapter manually then source DNA config
            command += " -c \"interface ftdi\"";
            command += " -c \"ftdi_vid_pid 0x0403 0x6011\"";
            command += " -c \"ftdi_channel 0\"";
            command += " -c \"ftdi_layout_init 0x0098 0x008b\"";
            command += " -c \"reset_config none\"";
            command += " -c \"adapter_khz 10000\"";
            command += " -c \"source [find cpld/xilinx-xc7.cfg]\"";
            command += " -c \"source [find cpld/xilinx-dna.cfg]\"";
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

        VMPROTECT_END_BLOCK();
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

    DriverInfo OpenOCDInterface::CheckRS232Driver()
    {
        DriverInfo info;

        // PowerShell command to check for FTDI RS232 Interface 0 (VID_0403&PID_6011&MI_00)
        // Also gets child COM port device if FTDIBUS is installed
        std::string psCommand = R"(powershell -Command "
            $device = Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_0403&PID_6011&MI_00*'} | Select-Object -First 1
            if ($device) {
                $props = @{}
                $props['Status'] = $device.Status
                $props['FriendlyName'] = $device.FriendlyName
                try { $props['DriverVersion'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data } catch { $props['DriverVersion'] = 'Unknown' }
                try { $props['DriverDate'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverDate' -ErrorAction SilentlyContinue).Data } catch { $props['DriverDate'] = 'Unknown' }
                try { $props['DriverProvider'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverProvider' -ErrorAction SilentlyContinue).Data } catch { $props['DriverProvider'] = 'Unknown' }
                try { $props['HardwareIds'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_HardwareIds' -ErrorAction SilentlyContinue).Data[0] } catch { $props['HardwareIds'] = 'Unknown' }
                try { $props['Service'] = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_Service' -ErrorAction SilentlyContinue).Data } catch { $props['Service'] = 'Unknown' }
                
                Write-Output \"STATUS:$($props['Status'])\"
                Write-Output \"NAME:$($props['FriendlyName'])\"
                Write-Output \"VERSION:$($props['DriverVersion'])\"
                Write-Output \"DATE:$($props['DriverDate'])\"
                Write-Output \"PROVIDER:$($props['DriverProvider'])\"
                Write-Output \"HWID:$($props['HardwareIds'])\"
                Write-Output \"SERVICE:$($props['Service'])\"
                
                # If FTDIBUS, find child COM port device
                if ($props['Service'] -eq 'FTDIBUS') {
                    $comDevices = Get-PnpDevice | Where-Object {$_.FriendlyName -like '*Serial Port*COM*' -and $_.HardwareID -like '*VID_0403*PID_6011*'}
                    foreach ($com in $comDevices) {
                        $comProps = Get-PnpDeviceProperty -InstanceId $com.InstanceId
                        $parent = ($comProps | Where-Object {$_.KeyName -eq 'DEVPKEY_Device_Parent'}).Data
                        if ($parent -like '*MI_00*') {
                            Write-Output \"COMPORT:$($com.FriendlyName)\"
                            break
                        }
                    }
                }
            } else {
                Write-Output 'NOT_INSTALLED'
            }
        ")";

        std::string output = ExecuteCommand(psCommand);

        if (output.find("NOT_INSTALLED") != std::string::npos)
        {
            info.installed = false;
            return info;
        }

        std::string serviceType;
        std::string comPortName;
        bool deviceDetected = true;
        
        // Parse output
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.find("STATUS:") == 0)
            {
                std::string status = line.substr(7);
                deviceDetected = (status.find("OK") != std::string::npos);
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
            else if (line.find("SERVICE:") == 0)
            {
                serviceType = line.substr(8);
            }
            else if (line.find("COMPORT:") == 0)
            {
                comPortName = line.substr(8);
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
        
        // Trim whitespace from serviceType
        if (!serviceType.empty())
        {
            serviceType.erase(serviceType.find_last_not_of(" \n\r\t") + 1);
        }
        
        std::cout << "[DEBUG] Service Type: '" << serviceType << "' (length: " << serviceType.length() << ")" << std::endl;
        
        // Determine if this is the CORRECT driver for JTAG operations
        // FTDIBUS = Default Windows driver (WRONG - needs replacement with WinUSB)
        // WinUSB = Custom driver (CORRECT for JTAG with OpenOCD)
        if (serviceType == "FTDIBUS")
        {
            // Wrong driver - default FTDI driver, needs WinUSB
            // Show COM port name if available
            info.installed = false;  // Mark as not installed (wrong driver)
            
            // Trim whitespace from comPortName
            if (!comPortName.empty())
            {
                comPortName.erase(comPortName.find_last_not_of(" \n\r\t") + 1);
            }
            
            std::cout << "[DEBUG] COM Port Name: '" << comPortName << "' (length: " << comPortName.length() << ")" << std::endl;
            std::cout << "[DEBUG] Original Device Name: '" << info.deviceName << "'" << std::endl;
            
            if (!comPortName.empty())
            {
                info.deviceName = comPortName + "\n(FTDIBUS - Needs WinUSB)";
                std::cout << "[DEBUG] Updated Device Name: '" << info.deviceName << "'" << std::endl;
            }
            else
            {
                info.deviceName += "\n(FTDIBUS - Needs WinUSB)";
                std::cout << "[DEBUG] No COM port found, using parent device name" << std::endl;
            }
        }
        else if (serviceType == "WinUSB")
        {
            // Correct driver for JTAG operations
            info.installed = true;
            info.deviceName += " (WinUSB - JTAG Ready)";
        }
        else
        {
            // Unknown driver or no driver
            info.installed = false;
        }

        return info;
    }

    CardInfo OpenOCDInterface::DetectDMACard()
    {
        CardInfo info;

        // PowerShell command to detect DMA cards by VID/PID (driver-agnostic)
        // Checks for:
        // 1. VID_0403&PID_6011&MI_00 -> 35T (FTDI FT4232H Quad RS232)
        // 2. VID_1A86&PID_55DD or VID_1A86&PID_55DE -> 75T/100T (CH347)
        std::string psCommand = R"(powershell -Command "
            # Check for 35T (FTDI FT4232H Interface 0)
            $rs232Device = Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_0403&PID_6011&MI_00*'} | Select-Object -First 1
            
            # Check for 75T/100T (CH347)
            $ch347Device = Get-PnpDevice | Where-Object {
                $_.InstanceId -like '*VID_1A86&PID_55DD*' -or 
                $_.InstanceId -like '*VID_1A86&PID_55DE*'
            } | Select-Object -First 1
            
            if ($rs232Device) {
                $props = Get-PnpDeviceProperty -InstanceId $rs232Device.InstanceId
                $hwids = ($props | Where-Object {$_.KeyName -eq 'DEVPKEY_Device_HardwareIds'}).Data[0]
                Write-Output 'DETECTED:35T'
                Write-Output \"NAME:$($rs232Device.FriendlyName)\"
                Write-Output \"INSTANCE:$($rs232Device.InstanceId)\"
                Write-Output \"HWID:$hwids\"
            }
            elseif ($ch347Device) {
                $props = Get-PnpDeviceProperty -InstanceId $ch347Device.InstanceId
                $hwids = ($props | Where-Object {$_.KeyName -eq 'DEVPKEY_Device_HardwareIds'}).Data[0]
                Write-Output 'DETECTED:CH347'
                Write-Output \"NAME:$($ch347Device.FriendlyName)\"
                Write-Output \"INSTANCE:$($ch347Device.InstanceId)\"
                Write-Output \"HWID:$hwids\"
            }
            else {
                Write-Output 'NOT_DETECTED'
            }
        ")";

        std::string output = ExecuteCommand(psCommand);

        if (output.find("NOT_DETECTED") != std::string::npos)
        {
            info.detected = false;
            return info;
        }

        info.detected = true;

        // Parse output
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.find("DETECTED:35T") == 0)
            {
                info.adapterType = AdapterType::RS232;
                info.cardTypeString = "35T";
            }
            else if (line.find("DETECTED:CH347") == 0)
            {
                info.adapterType = AdapterType::CH347;
                // Will be refined to 75T or 100T after FPGA detection
                info.cardTypeString = "75T/100T";
            }
            else if (line.find("NAME:") == 0)
            {
                info.deviceName = line.substr(5);
            }
            else if (line.find("INSTANCE:") == 0)
            {
                info.instanceId = line.substr(9);
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
        
        // Extract driver files from embedded resources to temp directory
        std::cout << "[INFO] Extracting driver files from embedded resources..." << std::endl;
        
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
            
            // Extract all driver files from embedded resources
            std::cout << "[INFO] Extracting driver files to temp directory..." << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_INF, (destDriverDir / "CH341WDM.INF").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341WDM.INF" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341WDM.INF" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_SYS, (destDriverDir / "CH341WDM.SYS").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341WDM.SYS" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341WDM.SYS" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_M64_SYS, (destDriverDir / "CH341M64.SYS").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341M64.SYS" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341M64.SYS" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_W64_SYS, (destDriverDir / "CH341W64.SYS").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341W64.SYS" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341W64.SYS" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_CAT, (destDriverDir / "CH341WDM.CAT").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341WDM.CAT" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341WDM.CAT" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH347_DLL, (destDriverDir / "CH347DLL.DLL").string()))
            {
                std::cout << "[ERROR] Failed to extract CH347DLL.DLL" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH347DLL.DLL" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH347_DLL_A64, (destDriverDir / "CH347DLLA64.DLL").string()))
            {
                std::cout << "[ERROR] Failed to extract CH347DLLA64.DLL" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH347DLLA64.DLL" << std::endl;
            
            // Extract CH341 DLL files (required by INF file)
            if (!ExtractEmbeddedResource(IDR_CH341_DLL, (destDriverDir / "CH341DLL.DLL").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341DLL.DLL" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341DLL.DLL" << std::endl;
            
            if (!ExtractEmbeddedResource(IDR_CH341_DLL_A64, (destDriverDir / "CH341DLLA64.DLL").string()))
            {
                std::cout << "[ERROR] Failed to extract CH341DLLA64.DLL" << std::endl;
                return false;
            }
            std::cout << "[DEBUG] Extracted: CH341DLLA64.DLL" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to extract driver files: " << e.what() << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Driver files extracted successfully" << std::endl;
        
        // Use the extracted INF file
        std::filesystem::path infPath = destDriverDir / "CH341WDM.INF";
        std::cout << "[INFO] Using driver INF at: " << infPath.string() << std::endl;
        
        // Verify INF exists
        if (!std::filesystem::exists(infPath))
        {
            std::cout << "[ERROR] INF file not found after extraction: " << infPath.string() << std::endl;
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

    bool OpenOCDInterface::InstallRS232Driver()
    {
        std::cout << "[INFO] Installing RS232 (WinUSB) driver for FTDI FT4232H..." << std::endl;
        std::cout << "[INFO] This will replace the default FTDIBUS driver with WinUSB for JTAG operations" << std::endl;
        
        // Get temp directory
        std::string tempDirPath = GetTempDirectory();
        std::filesystem::path driverDir = std::filesystem::path(tempDirPath) / "drivers" / "rs232";
        
        try
        {
            // Remove old temp drivers if they exist
            if (std::filesystem::exists(driverDir))
            {
                std::filesystem::remove_all(driverDir);
            }
            
            // Create destination directory
            std::filesystem::create_directories(driverDir);
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to create driver directory: " << e.what() << std::endl;
            return false;
        }
        
        std::cout << "[INFO] Extracting WinUSB driver files from embedded resources..." << std::endl;
        
        // Extract Zadig signed INF and CAT files
        if (!ExtractEmbeddedResource(IDR_RS232_WINUSB_INF, (driverDir / "quad_rs232-hs_(interface_0).inf").string()))
        {
            std::cout << "[ERROR] Failed to extract quad_rs232-hs_(interface_0).inf" << std::endl;
            return false;
        }
        std::cout << "[DEBUG] Extracted: quad_rs232-hs_(interface_0).inf" << std::endl;

        if (!ExtractEmbeddedResource(IDR_RS232_WINUSB_CAT, (driverDir / "Quad_RS232-HS_(Interface_0).cat").string()))
        {
            std::cout << "[ERROR] Failed to extract Quad_RS232-HS_(Interface_0).cat" << std::endl;
            return false;
        }
        std::cout << "[DEBUG] Extracted: Quad_RS232-HS_(Interface_0).cat" << std::endl;

        // Extract coinstaller DLLs to amd64 subdirectory
        std::filesystem::path amd64Dir = driverDir / "amd64";
        std::filesystem::create_directories(amd64Dir);
        
        if (!ExtractEmbeddedResource(IDR_RS232_WINUSB_COINSTALLER, (amd64Dir / "WinUSBCoInstaller2.dll").string()))
        {
            std::cout << "[ERROR] Failed to extract WinUSBCoInstaller2.dll" << std::endl;
            return false;
        }
        std::cout << "[DEBUG] Extracted: amd64\\WinUSBCoInstaller2.dll" << std::endl;
        
        if (!ExtractEmbeddedResource(IDR_RS232_WDF_COINSTALLER, (amd64Dir / "WdfCoInstaller01011.dll").string()))
        {
            std::cout << "[ERROR] Failed to extract WdfCoInstaller01011.dll" << std::endl;
            return false;
        }
        std::cout << "[DEBUG] Extracted: amd64\\WdfCoInstaller01011.dll" << std::endl;
        
        std::cout << "[SUCCESS] Driver files extracted successfully" << std::endl;
        
        std::filesystem::path infPath = driverDir / "quad_rs232-hs_(interface_0).inf";
        std::cout << "[INFO] Using driver INF at: " << infPath.string() << std::endl;
        
        // Verify INF exists
        if (!std::filesystem::exists(infPath))
        {
            std::cout << "[ERROR] INF file was not extracted properly!" << std::endl;
            return false;
        }
        
        // ZADIG-LIKE SOLUTION: Force driver update without removing FTDIBUS
        // This preserves other interfaces (MI_01, MI_02, MI_03) which need FTDIBUS for COM ports
        
        // Step 1: Remove old WinUSB driver if it exists (to avoid conflicts)
        std::cout << "[INFO] Checking for existing WinUSB driver..." << std::endl;
        std::string tempScriptFile = GetTempDirectory() + "find_winusb.ps1";
        std::ofstream scriptOut(tempScriptFile);
        scriptOut << "$output = pnputil /enum-drivers\n";
        scriptOut << "$lines = $output -split \"`n\"\n";
        scriptOut << "for ($i = 0; $i -lt $lines.Count; $i++) {\n";
        scriptOut << "    if ($lines[$i] -match 'Original Name:\\s+.*quad.*rs232.*interface.*0.*\\.inf') {\n";
        scriptOut << "        for ($j = $i - 1; $j -ge 0; $j--) {\n";
        scriptOut << "            if ($lines[$j] -match 'Published Name:\\s+(oem\\d+\\.inf)') {\n";
        scriptOut << "                Write-Output $matches[1]\n";
        scriptOut << "                exit 0\n";
        scriptOut << "            }\n";
        scriptOut << "        }\n";
        scriptOut << "        break\n";
        scriptOut << "    }\n";
        scriptOut << "}\n";
        scriptOut << "Write-Output 'NOT_FOUND'\n";
        scriptOut.close();
        
        std::string findWinUSBCmd = "powershell -ExecutionPolicy Bypass -File \"" + tempScriptFile + "\"";
        std::string oldWinUSBInf = ExecuteCommand(findWinUSBCmd);
        DeleteFileA(tempScriptFile.c_str());
        
        oldWinUSBInf.erase(std::remove(oldWinUSBInf.begin(), oldWinUSBInf.end(), '\n'), oldWinUSBInf.end());
        oldWinUSBInf.erase(std::remove(oldWinUSBInf.begin(), oldWinUSBInf.end(), '\r'), oldWinUSBInf.end());
        
        if (oldWinUSBInf.find(".inf") != std::string::npos)
        {
            std::cout << "[INFO] Found old WinUSB driver: " << oldWinUSBInf << ", removing it..." << std::endl;
            std::string removeOldCmd = "pnputil.exe /delete-driver " + oldWinUSBInf + " /uninstall /force";
            std::string removeOldOutput = ExecuteCommand(removeOldCmd);
            std::cout << removeOldOutput << std::endl;
            Sleep(2000);
        }
        
        // Step 2: Add WinUSB driver to driver store
        std::cout << "[INFO] Adding WinUSB driver to Windows driver store..." << std::endl;
        std::string addCommand = "pnputil.exe /add-driver \"" + infPath.string() + "\" /install";
        std::cout << "[DEBUG] Running: " + addCommand << std::endl;
        
        std::string addOutput = ExecuteCommand(addCommand);
        std::cout << addOutput << std::endl;
        
        if (addOutput.find("Failed") != std::string::npos || addOutput.find("failed") != std::string::npos)
        {
            std::cout << "[ERROR] Failed to add WinUSB driver to store" << std::endl;
            std::cout << "[INFO] You may need to run DMATool as Administrator" << std::endl;
            return false;
        }
        
        // Step 3: Find and backup current FTDIBUS driver OEM number (for reference, but don't remove it)
        std::cout << "[INFO] Finding current FTDIBUS driver..." << std::endl;
        
        // Use temp script file to avoid escaping issues
        std::string tempScriptFile2 = GetTempDirectory() + "find_ftdibus.ps1";
        std::ofstream scriptOut2(tempScriptFile2);
        scriptOut2 << "$output = pnputil /enum-drivers\n";
        scriptOut2 << "$lines = $output -split \"`n\"\n";
        scriptOut2 << "for ($i = 0; $i -lt $lines.Count; $i++) {\n";
        scriptOut2 << "    if ($lines[$i] -match 'Original Name:\\s+ftdibus\\.inf') {\n";
        scriptOut2 << "        for ($j = $i - 1; $j -ge 0; $j--) {\n";
        scriptOut2 << "            if ($lines[$j] -match 'Published Name:\\s+(oem\\d+\\.inf)') {\n";
        scriptOut2 << "                Write-Output $matches[1]\n";
        scriptOut2 << "                exit 0\n";
        scriptOut2 << "            }\n";
        scriptOut2 << "        }\n";
        scriptOut2 << "        break\n";
        scriptOut2 << "    }\n";
        scriptOut2 << "}\n";
        scriptOut2 << "Write-Output 'NOT_FOUND'\n";
        scriptOut2.close();
        
        std::string findFTDICmd = "powershell -ExecutionPolicy Bypass -File \"" + tempScriptFile2 + "\"";
        std::string ftdiOemInf = ExecuteCommand(findFTDICmd);
        DeleteFileA(tempScriptFile2.c_str());
        
        ftdiOemInf.erase(std::remove(ftdiOemInf.begin(), ftdiOemInf.end(), '\n'), ftdiOemInf.end());
        ftdiOemInf.erase(std::remove(ftdiOemInf.begin(), ftdiOemInf.end(), '\r'), ftdiOemInf.end());
        
        if (ftdiOemInf.find(".inf") != std::string::npos)
        {
            std::cout << "[INFO] Found FTDIBUS driver: " << ftdiOemInf << std::endl;
        }
        else
        {
            std::cout << "[WARNING] Could not find FTDIBUS driver OEM number" << std::endl;
        }
        
        // Step 4: Force Interface 0 to use WinUSB using Windows API (Zadig method)
        // This method does NOT remove FTDIBUS from the driver store, so other interfaces keep working
        std::cout << "[INFO] Forcing Interface 0 to use WinUSB driver using Windows API..." << std::endl;
        
        // Use the hardware ID pattern that matches ONLY Interface 0
        std::wstring hardwareId = L"USB\\VID_0403&PID_6011&MI_00";
        std::wstring infPathW(infPath.wstring());
        
        bool updateResult = DriverUpdateAPI::UpdateDriverForDevice(hardwareId, infPathW);
        
        if (updateResult)
        {
            std::cout << "[SUCCESS] WinUSB driver forced onto Interface 0" << std::endl;
            std::cout << "[INFO] Other interfaces (MI_01, MI_02, MI_03) still use FTDIBUS for COM ports" << std::endl;
            Sleep(2000); // Wait for Windows to apply driver
        }
        else
        {
            std::cout << "[ERROR] Failed to force WinUSB driver onto Interface 0" << std::endl;
            std::cout << "[INFO] Falling back to manual method..." << std::endl;
            
            // Fallback to old method if Windows API fails
            std::string forceUpdateCmd = R"(powershell -Command ")"
                R"($device = Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_0403&PID_6011&MI_00*'} | Select-Object -First 1; )"
                R"(if ($device) { )"
                R"(pnputil /delete-driver )" + ftdiOemInf + R"( /uninstall /force; )"
                R"(Start-Sleep -Seconds 2; )"
                R"(pnputil /scan-devices; )"
                R"(Write-Output 'UPDATED' } else { Write-Output 'NO_DEVICE' }")";
            
            std::string updateOutput = ExecuteCommand(forceUpdateCmd);
            std::cout << updateOutput << std::endl;
            
            if (updateOutput.find("UPDATED") == std::string::npos)
            {
                std::cout << "[WARNING] Automatic driver update may have failed" << std::endl;
                std::cout << "[INFO] You may need to manually update the driver in Device Manager" << std::endl;
            }
        }
        
        // Step 5: Verify WinUSB driver is active
        std::cout << "[INFO] Verifying WinUSB driver installation..." << std::endl;
        Sleep(2000);
        
        auto driverInfo = CheckRS232Driver();
        
        bool isWinUSBInstalled = (driverInfo.service == "WinUSB") ||
                                  (driverInfo.deviceName.find("Quad RS232-HS") != std::string::npos) ||
                                  (driverInfo.provider.find("libwdi") != std::string::npos);
        
        if (isWinUSBInstalled)
        {
            std::cout << "[SUCCESS] WinUSB driver installed successfully!" << std::endl;
            std::cout << "[INFO] Device: " << driverInfo.deviceName << std::endl;
            std::cout << "[INFO] Service: " << driverInfo.service << std::endl;
            std::cout << "[INFO] Version: " << driverInfo.version << std::endl;
            return true;
        }
        else
        {
            std::cout << "[ERROR] WinUSB driver installation may have failed" << std::endl;
            std::cout << "[INFO] Current device: " << driverInfo.deviceName << std::endl;
            std::cout << "[INFO] Current service: " << driverInfo.service << std::endl;
            std::cout << "[INFO] Please check Device Manager or run 'Check Driver Status' again" << std::endl;
            return false;
        }
    }

    bool OpenOCDInterface::UninstallRS232Driver()
    {
        std::cout << "[INFO] Uninstalling RS232 (WinUSB) driver..." << std::endl;
        std::cout << "[INFO] This will restore the default FTDIBUS driver" << std::endl;
        
        // WORKING SOLUTION (from manual testing):
        // 1. Find and remove WinUSB driver (oem104.inf)
        // 2. Windows automatically falls back to FTDIBUS (Windows inbox driver)
        // Note: FTDIBUS doesn't need to be reinstalled - it's built into Windows!
        
        std::string tempDirPath = GetTempDirectory();
        
        // Step 1: Find WinUSB driver using PowerShell script
        std::cout << "[INFO] Finding WinUSB driver in driver store..." << std::endl;
        
        std::filesystem::path findScript = std::filesystem::path(tempDirPath) / "find_winusb.ps1";
        std::ofstream scriptFile(findScript);
        scriptFile << "$output = pnputil /enum-drivers\n";
        scriptFile << "$lines = $output -split \"`n\"\n";
        scriptFile << "for ($i = 0; $i -lt $lines.Count; $i++) {\n";
        scriptFile << "    if ($lines[$i] -match 'Original Name:\\s+quad_rs232-hs_\\(interface_0\\)\\.inf') {\n";
        scriptFile << "        for ($j = $i - 1; $j -ge 0; $j--) {\n";
        scriptFile << "            if ($lines[$j] -match 'Published Name:\\s+(oem\\d+\\.inf)') {\n";
        scriptFile << "                $matches[1]\n";
        scriptFile << "                break\n";
        scriptFile << "            }\n";
        scriptFile << "        }\n";
        scriptFile << "        break\n";
        scriptFile << "    }\n";
        scriptFile << "}\n";
        scriptFile.close();
        
        std::string findWinusbCmd = "powershell -ExecutionPolicy Bypass -File \"" + findScript.string() + "\"";
        std::string oemInf = ExecuteCommand(findWinusbCmd);
        oemInf.erase(std::remove(oemInf.begin(), oemInf.end(), '\n'), oemInf.end());
        oemInf.erase(std::remove(oemInf.begin(), oemInf.end(), '\r'), oemInf.end());
        
        if (oemInf.empty() || oemInf.find(".inf") == std::string::npos)
        {
            std::cout << "[WARNING] WinUSB driver not found in driver store" << std::endl;
            std::cout << "[INFO] It may have already been removed or FTDIBUS is already active" << std::endl;
            
            // Check if we're already on FTDIBUS
            auto currentDriver = CheckRS232Driver();
            if (currentDriver.service == "FTDIBUS")
            {
                std::cout << "[SUCCESS] FTDIBUS driver is already active!" << std::endl;
                return true;
            }
            return false;
        }
        
        std::cout << "[INFO] Found WinUSB driver: " << oemInf << std::endl;
        
        // Step 2: Remove WinUSB driver
        std::cout << "[INFO] Removing WinUSB driver from driver store..." << std::endl;
        std::string removeCmd = "pnputil.exe /delete-driver " + oemInf + " /force";
        std::cout << "[DEBUG] Running: " << removeCmd << std::endl;
        
        std::string output = ExecuteCommand(removeCmd);
        std::cout << output << std::endl;
        
        if (output.find("successfully") == std::string::npos && output.find("deleted") == std::string::npos)
        {
            std::cout << "[ERROR] Failed to remove WinUSB driver" << std::endl;
            std::cout << "[INFO] You may need to run DMATool as Administrator" << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] WinUSB driver removed from driver store" << std::endl;
        
        // Step 3: Remove the device completely so Windows reinstalls FTDIBUS
        std::cout << "[INFO] Removing device to trigger FTDIBUS reinstall..." << std::endl;
        
        // Get current device instance ID using PowerShell
        std::string psCmd = R"(powershell -Command "$device = Get-PnpDevice | Where-Object {$_.InstanceId -like '*VID_0403&PID_6011&MI_00*'} | Select-Object -First 1; if ($device) { $device.InstanceId }")";
        std::string instanceId = ExecuteCommand(psCmd);
        instanceId.erase(std::remove(instanceId.begin(), instanceId.end(), '\n'), instanceId.end());
        instanceId.erase(std::remove(instanceId.begin(), instanceId.end(), '\r'), instanceId.end());
        
        if (!instanceId.empty() && instanceId.find("VID_") != std::string::npos)
        {
            std::cout << "[INFO] Found device: " << instanceId << std::endl;
            
            // Remove device completely
            std::string removeDeviceCmd = "pnputil /remove-device \"" + instanceId + "\"";
            std::cout << "[DEBUG] Running: " << removeDeviceCmd << std::endl;
            
            std::string removeOutput = ExecuteCommand(removeDeviceCmd);
            std::cout << removeOutput << std::endl;
            
            if (removeOutput.find("successfully") != std::string::npos || 
                removeOutput.find("removed") != std::string::npos)
            {
                std::cout << "[SUCCESS] Device removed successfully" << std::endl;
            }
            else
            {
                std::cout << "[WARNING] Device removal had unexpected output" << std::endl;
            }
        }
        else
        {
            std::cout << "[WARNING] Could not find device instance ID" << std::endl;
        }
        
        // Step 4: Rescan for hardware - Windows will auto-install FTDIBUS
        std::cout << "[INFO] Scanning for hardware changes..." << std::endl;
        std::string rescanCmd = "pnputil /scan-devices";
        ExecuteCommand(rescanCmd);
        
        Sleep(3000); // Wait for Windows to apply FTDIBUS driver
        
        // Step 5: Verify FTDIBUS is now active
        std::cout << "[INFO] Verifying FTDIBUS driver restoration..." << std::endl;
        
        auto driverInfo = CheckRS232Driver();
        
        bool isFTDIBUSRestored = (driverInfo.service == "FTDIBUS") ||
                                  (driverInfo.deviceName.find("USB Serial Port") != std::string::npos) ||
                                  (driverInfo.deviceName.find("USB Serial Converter") != std::string::npos);
        
        if (isFTDIBUSRestored)
        {
            std::cout << "[SUCCESS] FTDIBUS driver restored successfully!" << std::endl;
            std::cout << "[INFO] Device: " << driverInfo.deviceName << std::endl;
            std::cout << "[INFO] Service: " << driverInfo.service << std::endl;
            return true;
        }
        else
        {
            std::cout << "[WARNING] Could not verify FTDIBUS driver restoration" << std::endl;
            std::cout << "[INFO] Current device: " << driverInfo.deviceName << std::endl;
            std::cout << "[INFO] Current service: " << driverInfo.service << std::endl;
            std::cout << "[INFO] The device may need a moment to reinitialize" << std::endl;
            std::cout << "[INFO] Try 'Check Driver Status' again in a few seconds" << std::endl;
            return false;
        }
    }
}
