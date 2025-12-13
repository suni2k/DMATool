#include "FlashInterface.h"
#include "OpenOCDInterface.h"
#include "../VMProtectConfig.h"  // VMProtect SDK integration
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "../resource.h"
#include "../Util/TempDirectoryManager.h"
#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")

namespace fs = std::filesystem;

namespace DMATool::Backend
{
    FlashInterface::FlashInterface()
    {
        // STANDALONE MODE: Extract everything from embedded resources to temp
        // This ensures DMATool.exe works as a single file with no external dependencies
        
        // Register cleanup on first use
        Util::TempDirectoryManager::GetInstance().RegisterCleanup();
        
        std::string tempDir = Util::TempDirectoryManager::GetInstance().GetTempDirectory();
        
        std::cout << "[INFO] Flash tab initializing (standalone mode)..." << std::endl;
        std::cout << "[INFO] Temp directory: " << tempDir << std::endl;
        
        // Set paths to temp
        m_openocdPath = tempDir + "openocd.exe";
        m_openocdScriptsPath = tempDir;
        m_bscanBasePath = tempDir + "bscan\\";
        
        // Extract OpenOCD and dependencies (shared with JTAG Port tab)
        // These are extracted by OpenOCDInterface, but we verify they exist
        if (!fs::exists(m_openocdPath)) {
            std::cout << "[INFO] OpenOCD not in temp, extracting from resources..." << std::endl;
            ExtractEmbeddedResource(IDR_OPENOCD_EXE, m_openocdPath);
            ExtractEmbeddedResource(IDR_LIBUSB_DLL, tempDir + "libusb-1.0.dll");
            ExtractEmbeddedResource(IDR_LIBHIDAPI_DLL, tempDir + "libhidapi-0.dll");
            
            // Create cpld/ subdirectory for OpenOCD to find config files
            std::string cpldDir = tempDir + "cpld\\";
            fs::create_directories(cpldDir);
            
            // Extract to cpld/ subdirectory for source [find ...] to work
            ExtractEmbeddedResource(IDR_XILINX_XC7_CFG, cpldDir + "xilinx-xc7.cfg");
            ExtractEmbeddedResource(IDR_JTAGSPI_CFG, cpldDir + "jtagspi.cfg");
            
            std::cout << "[INFO] Created cpld/ subdirectory for OpenOCD scripts" << std::endl;
        } else {
            std::cout << "[INFO] OpenOCD already in temp (shared with JTAG Port tab)" << std::endl;
        }
        
        // Always extract BSCAN bitstreams (unique to Flash tab)
        fs::create_directories(m_bscanBasePath);
        ExtractBSCANBitstreams();
        
        // Set OPENOCD_SCRIPTS environment variable so OpenOCD can find cpld/ subdirectory
        SetEnvironmentVariableA("OPENOCD_SCRIPTS", m_openocdScriptsPath.c_str());
        std::cout << "[DEBUG] Set OPENOCD_SCRIPTS=" << m_openocdScriptsPath << std::endl;
        
        std::cout << "[DEBUG] OpenOCD path: " << m_openocdPath << std::endl;
        std::cout << "[DEBUG] BSCAN path: " << m_bscanBasePath << std::endl;
        std::cout << "[SUCCESS] Flash interface initialized (standalone mode)" << std::endl;
    }

    FlashInterface::~FlashInterface()
    {
    }

    std::vector<std::pair<FPGAChipModel, std::string>> FlashInterface::GetSupportedChipModels()
    {
        return {
            { FPGAChipModel::XC7A35T, "XC7A35T (Artix-7 35T)" },
            { FPGAChipModel::XC7A50T, "XC7A50T (Artix-7 50T)" },
            { FPGAChipModel::XC7A75T, "XC7A75T (Artix-7 75T)" },
            { FPGAChipModel::XC7A100T, "XC7A100T (Artix-7 100T)" },
            { FPGAChipModel::XC7A200T, "XC7A200T (Artix-7 200T)" },
            { FPGAChipModel::XC7K70T, "XC7K70T (Kintex-7 70T)" },
            { FPGAChipModel::XC7K160T, "XC7K160T (Kintex-7 160T)" },
            { FPGAChipModel::XC7K325T, "XC7K325T (Kintex-7 325T)" },
            { FPGAChipModel::XC7K410T, "XC7K410T (Kintex-7 410T)" },
            { FPGAChipModel::XC6SLX9, "XC6SLX9 (Spartan-6 9K)" },
            { FPGAChipModel::XC6SLX45, "XC6SLX45 (Spartan-6 45K)" },
            { FPGAChipModel::XC6SLX75, "XC6SLX75 (Spartan-6 75K)" }
        };
    }

    std::string FlashInterface::ChipModelToString(FPGAChipModel model)
    {
        switch (model)
        {
        case FPGAChipModel::XC7A35T: return "xc7a35t";
        case FPGAChipModel::XC7A50T: return "xc7a50t";
        case FPGAChipModel::XC7A75T: return "xc7a75t";
        case FPGAChipModel::XC7A100T: return "xc7a100t";
        case FPGAChipModel::XC7A200T: return "xc7a200t";
        case FPGAChipModel::XC7K70T: return "xc7k70t";
        case FPGAChipModel::XC7K160T: return "xc7k160t";
        case FPGAChipModel::XC7K325T: return "xc7k325t";
        case FPGAChipModel::XC7K410T: return "xc7k410t";
        case FPGAChipModel::XC6SLX9: return "xc6slx9";
        case FPGAChipModel::XC6SLX45: return "xc6slx45";
        case FPGAChipModel::XC6SLX75: return "xc6slx75";
        default: return "unknown";
        }
    }

    FPGAChipModel FlashInterface::StringToChipModel(const std::string& modelStr)
    {
        std::string lower = modelStr;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("7a35t") != std::string::npos) return FPGAChipModel::XC7A35T;
        if (lower.find("7a50t") != std::string::npos) return FPGAChipModel::XC7A50T;
        if (lower.find("7a75t") != std::string::npos) return FPGAChipModel::XC7A75T;
        if (lower.find("7a100t") != std::string::npos) return FPGAChipModel::XC7A100T;
        if (lower.find("7a200t") != std::string::npos) return FPGAChipModel::XC7A200T;
        if (lower.find("7k70t") != std::string::npos) return FPGAChipModel::XC7K70T;
        if (lower.find("7k160t") != std::string::npos) return FPGAChipModel::XC7K160T;
        if (lower.find("7k325t") != std::string::npos) return FPGAChipModel::XC7K325T;
        if (lower.find("7k410t") != std::string::npos) return FPGAChipModel::XC7K410T;
        return FPGAChipModel::Unknown;
    }

    std::string FlashInterface::GetBSCANBitstreamPath(FPGAChipModel model)
    {
        // Use the member variable m_bscanBasePath which was set in constructor
        std::string filename = "bscan_spi_" + ChipModelToString(model) + ".bit";
        return m_bscanBasePath + "\\" + filename;
    }

    bool FlashInterface::HasBSCANBitstream(FPGAChipModel model)
    {
        if (model == FPGAChipModel::Unknown) return false;
        std::string path = GetBSCANBitstreamPath(model);
        return fs::exists(path);
    }

    bool FlashInterface::IsOpenOCDAvailable()
    {
        return fs::exists(m_openocdPath);
    }

    std::string FlashInterface::GetOpenOCDVersion()
    {
        if (!IsOpenOCDAvailable()) return "Not Found";
        return "0.11.0+dev (CH347 Edition)";
    }

    // SIMPLIFIED: Just detect FPGA chip model, no flash access needed!
    FlashDeviceInfo FlashInterface::DetectFlashDevice(FlashProgressCallback progressCallback)
    {
        FlashDeviceInfo info;

        if (progressCallback)
            progressCallback(0, 100, "Starting FPGA detection...");

        // Use OpenOCDInterface to detect FPGA (same as JTAG Port tab!)
        OpenOCDInterface openocd;
        
        if (progressCallback)
            progressCallback(20, 100, "Detecting FPGA via JTAG...");

        // Detect FPGA using the proven working method (no DNA needed for flash!)
        FPGAInfo fpgaInfo = openocd.DetectFPGA(AdapterType::Unknown, [&](const std::string& msg) {
            std::cout << msg << std::endl;
        });

        if (!fpgaInfo.detected)
        {
            if (progressCallback)
                progressCallback(0, 100, "Failed to detect FPGA");
            return info;
        }

        // We detected the FPGA! Fill in the flash info
        info.detected = true;
        info.fpgaModelString = fpgaInfo.partNumber;  // e.g., "XC7A75T"
        
        // Convert from ChipModel (JTAG) to FPGAChipModel (Flash)
        switch (fpgaInfo.chipModel)
        {
        case ChipModel::XC7A35T: 
            info.fpgaModel = FPGAChipModel::XC7A35T;
            info.manufacturer = "Xilinx";
            info.model = "Artix-7 35T";
            break;
        case ChipModel::XC7A50T:
            info.fpgaModel = FPGAChipModel::XC7A50T;
            info.manufacturer = "Xilinx";
            info.model = "Artix-7 50T";
            break;
        case ChipModel::XC7A75T:
            info.fpgaModel = FPGAChipModel::XC7A75T;
            info.manufacturer = "Xilinx";
            info.model = "Artix-7 75T";
            break;
        case ChipModel::XC7A100T:
            info.fpgaModel = FPGAChipModel::XC7A100T;
            info.manufacturer = "Xilinx";
            info.model = "Artix-7 100T";
            break;
        default:
            info.fpgaModel = FPGAChipModel::Unknown;
            info.manufacturer = "Unknown";
            info.model = "Unknown Chip";
            break;
        }
        
        // Flash capacity will be detected during actual flash operation
        info.capacity = 0;  // Not needed for chip detection
        info.sectorSize = 65536;  // Standard 64KB for Xilinx 7-series
        info.pageSize = 256;  // Standard 256B

        if (progressCallback)
            progressCallback(100, 100, "FPGA detected: " + info.fpgaModelString);

        return info;
    }

    std::string FlashInterface::CreateOpenOCDConfig(FPGAChipModel model, int clockSpeed)
    {
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string configPath = std::string(tempPath) + "flash_temp_" + std::to_string(GetTickCount64()) + ".cfg";

        std::ofstream config(configPath);
        config << "# Temporary OpenOCD Flash Configuration\n";
        config << "adapter driver ch347\n";
        config << "ch347 vid_pid 0x1a86 0x55dd\n";
        config << "transport select jtag\n";
        config << "adapter speed " << clockSpeed << "\n";
        config << "source [find cpld/xilinx-xc7.cfg]\n";
        config << "source [find cpld/jtagspi.cfg]\n";
        config.close();

        return configPath;
    }

    bool FlashInterface::ExecuteOpenOCDCommand(
        const std::string& configPath,
        const std::vector<std::string>& commands,
        std::string& output,
        std::string& error,
        FlashProgressCallback progressCallback)
    {
        std::string cmdLine = "\"" + m_openocdPath + "\" -f \"" + configPath + "\"";
        
        for (const auto& cmd : commands)
        {
            cmdLine += " -c \"" + cmd + "\"";
        }

        std::cout << "[DEBUG] OpenOCD command: " << cmdLine << std::endl;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hStdOutRead, hStdOutWrite;
        HANDLE hStdErrRead, hStdErrWrite;

        CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
        CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0);
        SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hStdOutWrite;
        si.hStdError = hStdErrWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        PROCESS_INFORMATION pi = { 0 };

        fs::path openocdDir = fs::path(m_openocdPath).parent_path();

        BOOL success = CreateProcessA(
            NULL,
            const_cast<char*>(cmdLine.c_str()),
            NULL, NULL, TRUE,
            CREATE_NO_WINDOW,
            NULL,
            openocdDir.string().c_str(),
            &si, &pi
        );

        CloseHandle(hStdOutWrite);
        CloseHandle(hStdErrWrite);

        if (!success)
        {
            std::cout << "[ERROR] Failed to launch OpenOCD process" << std::endl;
            CloseHandle(hStdOutRead);
            CloseHandle(hStdErrRead);
            return false;
        }

        std::cout << "[DEBUG] OpenOCD process launched successfully" << std::endl;

        // Read output in real-time using non-blocking approach
        char buffer[4096];
        DWORD bytesRead;
        DWORD bytesAvail;
        
        std::string outputBuffer;  // Accumulate stdout line-by-line
        std::string errorBuffer;   // Accumulate stderr line-by-line
        
        // Track when sectors complete to start periodic writing updates
        bool sectorsCompleted = false;
        auto lastProgressUpdate = std::chrono::steady_clock::now();

        while (true)
        {
            // Check if process is still running
            DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);  // Wait 100ms
            
            // Send periodic "Writing progress" updates during the writing phase (every 2 seconds)
            auto now = std::chrono::steady_clock::now();
            double timeSinceLastUpdate = std::chrono::duration<double>(now - lastProgressUpdate).count();
            
            if (sectorsCompleted && timeSinceLastUpdate >= 2.0)
            {
                if (progressCallback)
                    progressCallback(0, 0, "Writing progress");
                lastProgressUpdate = now;
            }
            
            // Try to read stdout
            PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, NULL);
            if (bytesAvail > 0)
            {
                if (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    output += buffer;
                    outputBuffer += buffer;
                    std::cout << "[OPENOCD] " << buffer;  // Real-time output
                    
                    // Parse complete lines for progress updates
                    size_t newlinePos;
                    while ((newlinePos = outputBuffer.find('\n')) != std::string::npos)
                    {
                        std::string line = outputBuffer.substr(0, newlinePos);
                        outputBuffer = outputBuffer.substr(newlinePos + 1);
                        
                        // Parse progress from this line
                        if (progressCallback)
                        {
                            // Check for sector progress: "Info : sector 15 took 232 ms"
                            std::regex sectorRegex(R"(sector (\d+) took)");
                            std::smatch match;
                            if (std::regex_search(line, match, sectorRegex))
                            {
                                uint64_t sectorNum = std::stoull(match[1].str());
                                progressCallback(sectorNum, 0, "Sector " + std::to_string(sectorNum) + " complete");
                                
                                // Check if this is the last sector to enable writing updates
                                if (line.find("sector") != std::string::npos)
                                {
                                    sectorsCompleted = true;
                                    lastProgressUpdate = std::chrono::steady_clock::now();
                                }
                            }
                            // Check for file read: "read 2099688 bytes from file"
                            else if (line.find("read") != std::string::npos && line.find("bytes from file") != std::string::npos)
                            {
                                progressCallback(0, 0, "Firmware loaded");
                            }
                            // Check for verification: "contents match"
                            else if (line.find("contents match") != std::string::npos)
                            {
                                progressCallback(0, 0, "Verification passed");
                            }
                            // Check for verification failure: "contents differ"
                            else if (line.find("contents differ") != std::string::npos)
                            {
                                progressCallback(0, 0, "Verification failed");
                            }
                        }
                    }
                }
            }

            // Try to read stderr - ALSO PARSE FOR PROGRESS UPDATES!
            PeekNamedPipe(hStdErrRead, NULL, 0, NULL, &bytesAvail, NULL);
            if (bytesAvail > 0)
            {
                if (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    error += buffer;
                    errorBuffer += buffer;
                    std::cout << "[OPENOCD-ERR] " << buffer;  // Real-time error output
                    
                    // CRITICAL FIX: Parse stderr for progress too!
                    // OpenOCD sends sector progress to stderr, not stdout
                    size_t newlinePos;
                    while ((newlinePos = errorBuffer.find('\n')) != std::string::npos)
                    {
                        std::string line = errorBuffer.substr(0, newlinePos);
                        errorBuffer = errorBuffer.substr(newlinePos + 1);
                        
                        // Parse progress from stderr line
                        if (progressCallback)
                        {
                            // Check for sector progress: "Info : sector 15 took 232 ms"
                            std::regex sectorRegex(R"(sector (\d+) took)");
                            std::smatch match;
                            if (std::regex_search(line, match, sectorRegex))
                            {
                                uint64_t sectorNum = std::stoull(match[1].str());
                                progressCallback(sectorNum, 0, "Sector " + std::to_string(sectorNum) + " complete");
                                
                                // Mark that sectors are being processed
                                sectorsCompleted = true;
                                lastProgressUpdate = std::chrono::steady_clock::now();
                            }
                            // Check for file read: "read 2099688 bytes from file"
                            else if (line.find("read") != std::string::npos && line.find("bytes from file") != std::string::npos)
                            {
                                progressCallback(0, 0, "Firmware loaded");
                                sectorsCompleted = false;  // Writing phase complete
                            }
                            // Check for file write: "wrote 2099688 bytes to file"
                            else if (line.find("wrote") != std::string::npos && line.find("bytes to file") != std::string::npos)
                            {
                                progressCallback(0, 0, "Flash read complete");
                            }
                            // Check for verification: "contents match"
                            else if (line.find("contents match") != std::string::npos)
                            {
                                progressCallback(0, 0, "Verification passed");
                            }
                            // Check for verification failure: "contents differ"
                            else if (line.find("contents differ") != std::string::npos)
                            {
                                progressCallback(0, 0, "Verification failed");
                            }
                        }
                    }
                }
            }

            // If process exited, read any remaining data and break
            if (waitResult == WAIT_OBJECT_0)
            {
                // Read any remaining stdout
                while (PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, NULL) && bytesAvail > 0)
                {
                    if (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';
                        output += buffer;
                        std::cout << "[OPENOCD] " << buffer;
                    }
                    else
                        break;
                }

                // Read any remaining stderr
                while (PeekNamedPipe(hStdErrRead, NULL, 0, NULL, &bytesAvail, NULL) && bytesAvail > 0)
                {
                    if (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';
                        error += buffer;
                        std::cout << "[OPENOCD-ERR] " << buffer;
                    }
                    else
                        break;
                }

                break;
            }
        }

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        std::cout << "[DEBUG] OpenOCD process exited with code: " << exitCode << std::endl;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hStdOutRead);
        CloseHandle(hStdErrRead);

        return (exitCode == 0);
    }

    FlashDeviceInfo FlashInterface::ParseFlashInfo(const std::string& openocdOutput)
    {
        FlashDeviceInfo info;

        std::regex flashRegex(R"(Found flash device '([^']+)' \(ID (0x[0-9A-Fa-f]+)\))");
        std::regex sizeRegex(R"(flash size:\s*(\d+)\s*([KMG]?)i?B)");

        std::smatch match;
        if (std::regex_search(openocdOutput, match, flashRegex))
        {
            info.detected = true;
            info.model = match[1].str();
            info.deviceId = match[2].str();

            if (info.model.find("win") != std::string::npos || 
                info.model.find("w25") != std::string::npos)
            {
                info.manufacturer = "Winbond";
            }
            else if (info.model.find("mx") != std::string::npos)
            {
                info.manufacturer = "Macronix";
            }
            else
            {
                info.manufacturer = "Unknown";
            }
        }

        if (std::regex_search(openocdOutput, match, sizeRegex))
        {
            uint64_t size = std::stoull(match[1].str());
            std::string unit = match[2].str();

            if (unit == "K") size *= 1024;
            else if (unit == "M") size *= 1024 * 1024;
            else if (unit == "G") size *= 1024 * 1024 * 1024;

            info.capacity = size;
        }

        info.sectorSize = 65536;
        info.pageSize = 256;

        return info;
    }

    FlashOperationResult FlashInterface::ProgramFirmware(
        const std::string& firmwarePath,
        FPGAChipModel chipModel,
        bool verifyAfter,
        bool backupBefore,
        FlashProgressCallback progressCallback)
    {
        VMPROTECT_MUTATE_BLOCK("FlashProgram");
        
        FlashOperationResult result;

        if (!fs::exists(firmwarePath))
        {
            result.message = "Firmware file not found: " + firmwarePath;
            VMPROTECT_END_BLOCK();
            return result;
        }

        if (!HasBSCANBitstream(chipModel))
        {
            std::string bscanPath = GetBSCANBitstreamPath(chipModel);
            result.message = "BSCAN bitstream not found!\n";
            result.message += "Expected: " + bscanPath + "\n";
            result.message += "Please ensure OpenOCD files are in dmafiles\\CH347FPGATool\\OpenOCD_CH347\\";
            return result;
        }

        auto startTime = std::chrono::steady_clock::now();

        if (progressCallback)
            progressCallback(0, 100, "Preparing to flash firmware...");

        std::cout << "[INFO] Creating OpenOCD config for " << ChipModelToString(chipModel) << std::endl;
        
        std::string configPath = CreateOpenOCDConfig(chipModel);
        std::string bscanPath = GetBSCANBitstreamPath(chipModel);
        std::string firmwarePathUnix = firmwarePath;
        std::replace(bscanPath.begin(), bscanPath.end(), '\\', '/');
        std::replace(firmwarePathUnix.begin(), firmwarePathUnix.end(), '\\', '/');

        std::cout << "[INFO] BSCAN bitstream: " << bscanPath << std::endl;
        std::cout << "[INFO] Firmware file: " << firmwarePathUnix << std::endl;

        // Calculate total sectors for progress tracking
        uint64_t firmwareSize = fs::file_size(firmwarePath);
        uint64_t totalSectors = (firmwareSize + 65535) / 65536;  // Round up (64KB sectors)
        
        std::cout << "[INFO] Firmware size: " << firmwareSize << " bytes (" << totalSectors << " sectors)" << std::endl;
        
        // Estimate writing time based on typical speed (~200 KiB/s)
        double estimatedWriteSeconds = (firmwareSize / 1024.0) / 200.0;  // ~200 KiB/s typical
        std::cout << "[INFO] Estimated write time: " << estimatedWriteSeconds << " seconds" << std::endl;

        // CRITICAL FIX: Do NOT run xc7_program between jtagspi_program and verify!
        // The xc7_program command reloads the FPGA which invalidates the flash state
        std::vector<std::string> cmds = {
            "init",
            "jtagspi_init 0 \"" + bscanPath + "\"",
            "jtagspi_program \"" + firmwarePathUnix + "\" 0x0"
            // DO NOT add xc7_program here - it corrupts verification!
        };

        // Only add verification if requested
        if (verifyAfter)
        {
            cmds.push_back("flash verify_bank 0 \"" + firmwarePathUnix + "\"");
        }

        // Reload FPGA configuration at the very end (after verification)
        cmds.push_back("xc7_program xc7.tap");
        cmds.push_back("shutdown");

        if (progressCallback)
            progressCallback(10, 100, "Initializing flash...");

        std::cout << "[INFO] Executing OpenOCD flash programming..." << std::endl;
        
        std::string output, error;
        
        // Track phase transitions
        bool sectorsComplete = false;
        bool writingComplete = false;
        auto sectorsStartTime = std::chrono::steady_clock::now();
        auto writingStartTime = std::chrono::steady_clock::now();
        uint64_t lastLoggedElapsedSeconds = 0;  // Track last logged elapsed time
        
        // Create a progress callback that handles different phases
        auto sectorProgressCallback = [&](uint64_t sectorNum, uint64_t unused, const std::string& msg) {
            if (msg.find("Sector") != std::string::npos && msg.find("complete") != std::string::npos)
            {
                // PHASE 1: Erasing sectors (10% - 40%)
                // Each sector takes ~230ms, total ~8 seconds for 33 sectors
                float sectorProgress = ((float)(sectorNum + 1) / (float)totalSectors) * 30.0f;  // 0-30%
                float totalProgress = 10.0f + sectorProgress;  // 10-40%
                
                std::string progressMsg = "Erasing sector " + std::to_string(sectorNum + 1) + "/" + std::to_string(totalSectors);
                
                if (progressCallback)
                    progressCallback((uint64_t)totalProgress, 100, progressMsg);
                    
                // Mark when sectors are complete
                if (sectorNum + 1 == totalSectors)
                {
                    sectorsComplete = true;
                    writingStartTime = std::chrono::steady_clock::now();
                    lastLoggedElapsedSeconds = 0;
                }
            }
            else if (msg.find("Firmware loaded") != std::string::npos || msg.find("read") != std::string::npos)
            {
                // Firmware loaded - sectors complete, writing started
                if (!sectorsComplete)
                {
                    sectorsComplete = true;
                    writingStartTime = std::chrono::steady_clock::now();
                    lastLoggedElapsedSeconds = 0;
                }
            }
            else if (msg.find("Writing progress") != std::string::npos)
            {
                // PHASE 2: Writing flash (40% - 90%)
                // This takes most of the time (~120-130 seconds for 2MB)
                if (sectorsComplete && !writingComplete)
                {
                    auto now = std::chrono::steady_clock::now();
                    double elapsedWrite = std::chrono::duration<double>(now - writingStartTime).count();
                    
                    // Progress from 40% to 90% based on ACTUAL elapsed time (not estimated)
                    // Cap at 90% after 130 seconds
                    double ratio = elapsedWrite / 130.0;  // 130 seconds to reach 90%
                    if (ratio > 1.0) ratio = 1.0;  // Cap at 90%
                    double writeProgress = ratio * 50.0;  // 0-50%
                    float totalProgress = 40.0f + (float)writeProgress;  // 40-90%
                    
                    // Log elapsed time every 10 seconds (not every 2 seconds to avoid spam)
                    uint64_t currentElapsedSeconds = (uint64_t)elapsedWrite;
                    if (currentElapsedSeconds >= lastLoggedElapsedSeconds + 10)
                    {
                        std::string progressMsg = "Writing flash contents... (elapsed: " + std::to_string(currentElapsedSeconds) + "s)";
                        if (progressCallback)
                            progressCallback((uint64_t)totalProgress, 100, progressMsg);
                        lastLoggedElapsedSeconds = currentElapsedSeconds;
                    }
                }
            }
            else if (msg.find("Verification passed") != std::string::npos)
            {
                // Mark writing as complete
                writingComplete = true;
                
                // PHASE 3: Verification (90% - 95%)
                if (progressCallback)
                    progressCallback(95, 100, "Verification passed!");
            }
            else if (msg.find("Verification failed") != std::string::npos)
            {
                writingComplete = true;
                if (progressCallback)
                    progressCallback(90, 100, "Verification failed!");
            }
        };
        
        result.success = ExecuteOpenOCDCommand(configPath, cmds, output, error, sectorProgressCallback);

        std::cout << "[INFO] OpenOCD execution complete" << std::endl;
        
        if (fs::exists(configPath))
            fs::remove(configPath);

        auto endTime = std::chrono::steady_clock::now();
        result.durationSeconds = std::chrono::duration<double>(endTime - startTime).count();

        if (result.success)
        {
            // CRITICAL FIX: Search both stdout AND stderr for byte count and verification
            // OpenOCD outputs to stderr!
            std::string combinedOutput = output + error;
            
            std::regex bytesRegex(R"(read (\d+) bytes from file)");
            std::smatch match;
            if (std::regex_search(combinedOutput, match, bytesRegex))
            {
                result.bytesProcessed = std::stoull(match[1].str());
            }

            // Check if verification passed (search both stdout and stderr!)
            bool verifyPassed = (combinedOutput.find("contents match") != std::string::npos);
            bool verifyFailed = (combinedOutput.find("contents differ") != std::string::npos);
            
            if (verifyAfter)
            {
                if (verifyPassed)
                {
                    result.message = "Flash programming and verification completed successfully";
                }
                else if (verifyFailed)
                {
                    result.success = false;
                    result.message = "Flash programming completed but verification FAILED!\n";
                    result.message += "The flash contents do not match the firmware file.\n";
                    result.message += "This may indicate a programming error or flash corruption.";
                }
                else
                {
                    result.message = "Flash programming completed (verification status unknown)";
                }
            }
            else
            {
                result.message = "Flash programming completed successfully (not verified)";
            }
        }
        else
        {
            std::cout << "[ERROR] Flash programming failed!" << std::endl;
            std::cout << "[ERROR] Output: " << output << std::endl;
            std::cout << "[ERROR] Error: " << error << std::endl;
            
            result.message = "Flash programming failed";
            
            // Combine output and error for detailed message
            if (!error.empty())
                result.message += "\n\nError Output:\n" + error;
            if (!output.empty())
                result.message += "\n\nOpenOCD Output:\n" + output;
        }

        VMPROTECT_END_BLOCK();
        return result;
    }

    FlashOperationResult FlashInterface::VerifyFirmware(
        const std::string& firmwarePath,
        FPGAChipModel chipModel,
        FlashProgressCallback progressCallback)
    {
        VMPROTECT_MUTATE_BLOCK("FlashVerify");
        
        FlashOperationResult result;
        
        if (!fs::exists(firmwarePath))
        {
            result.message = "Firmware file not found: " + firmwarePath;
            return result;
        }

        if (!HasBSCANBitstream(chipModel))
        {
            result.message = "BSCAN bitstream not available for " + ChipModelToString(chipModel);
            return result;
        }

        auto startTime = std::chrono::steady_clock::now();

        if (progressCallback)
            progressCallback(0, 100, "Preparing to verify firmware...");

        uint64_t firmwareSize = fs::file_size(firmwarePath);
        result.bytesProcessed = firmwareSize;
        
        std::cout << "[INFO] Firmware size: " << firmwareSize << " bytes (" 
                  << (firmwareSize / 1024 / 1024) << " MB)" << std::endl;

        if (progressCallback)
            progressCallback(10, 100, "Reading flash contents...");

        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string readbackPath = std::string(tempPath) + "flash_verify_" + std::to_string(GetTickCount64()) + ".bin";

        std::string configPath = CreateOpenOCDConfig(chipModel);
        std::string bscanPath = GetBSCANBitstreamPath(chipModel);
        std::string readbackPathUnix = readbackPath;
        std::replace(bscanPath.begin(), bscanPath.end(), '\\', '/');
        std::replace(readbackPathUnix.begin(), readbackPathUnix.end(), '\\', '/');

        std::vector<std::string> cmds = {
            "init",
            "jtagspi_init 0 \"" + bscanPath + "\"",
            "flash read_bank 0 \"" + readbackPathUnix + "\" 0x0 " + std::to_string(firmwareSize),
            "shutdown"
        };

        if (progressCallback)
            progressCallback(30, 100, "Reading " + std::to_string(firmwareSize / 1024 / 1024) + " MB from flash...");

        std::string output, error;
        
        // Track read progress with callback and extract speed info
        std::string flashReadSpeed;
        auto verifyStartTime = std::chrono::steady_clock::now();
        
        auto readProgressCallback = [&](uint64_t unused1, uint64_t unused2, const std::string& msg) {
            if (msg.find("Flash read complete") != std::string::npos)
            {
                if (progressCallback)
                    progressCallback(65, 100, "Flash read complete");
            }
        };
        
        bool readSuccess = ExecuteOpenOCDCommand(configPath, cmds, output, error, readProgressCallback);

        if (fs::exists(configPath))
            fs::remove(configPath);

        if (!readSuccess)
        {
            result.message = "Failed to read flash: " + error;
            if (fs::exists(readbackPath))
                fs::remove(readbackPath);
            return result;
        }
        
        // Extract speed from OpenOCD output
        std::regex speedRegex(R"((\d+\.\d+)\s+KiB/s)");
        std::smatch speedMatch;
        std::string combinedOutput = output + error;
        if (std::regex_search(combinedOutput, speedMatch, speedRegex))
        {
            flashReadSpeed = speedMatch[1].str() + " KiB/s";
        }
        
        // Verify readback file exists and has correct size
        if (!fs::exists(readbackPath))
        {
            result.message = "Flash readback file was not created";
            return result;
        }
        
        uint64_t readbackSize = fs::file_size(readbackPath);
        std::cout << "[INFO] Readback file size: " << readbackSize << " bytes" << std::endl;
        
        if (readbackSize != firmwareSize)
        {
            result.message = "Flash readback size mismatch! Expected " 
                + std::to_string(firmwareSize) + " bytes, got " 
                + std::to_string(readbackSize) + " bytes";
            if (fs::exists(readbackPath))
                fs::remove(readbackPath);
            return result;
        }

        // Spread SHA256 computation over remaining time (7 seconds total verification)
        // Progress from 70% to 90% during SHA256 calculation
        
        if (progressCallback)
            progressCallback(70, 100, "Computing SHA256 hashes...");

        // Calculate SHA256 hash for original file
        std::string originalHash = CalculateSHA256(firmwarePath);
        
        // Calculate SHA256 hash for readback file
        std::string readbackHash = CalculateSHA256(readbackPath);
        
        if (progressCallback)
            progressCallback(85, 100, "Comparing hashes...");

        bool filesMatch = (originalHash == readbackHash && !originalHash.empty());

        // Clean up readback file
        if (fs::exists(readbackPath))
            fs::remove(readbackPath);

        auto endTime = std::chrono::steady_clock::now();
        result.durationSeconds = std::chrono::duration<double>(endTime - startTime).count();
        
        std::cout << "[INFO] Verification completed in " << result.durationSeconds << " seconds" << std::endl;
        std::cout << "[INFO] Original SHA256: " << originalHash << std::endl;
        std::cout << "[INFO] Readback SHA256: " << readbackHash << std::endl;

        if (filesMatch)
        {
            result.success = true;
            result.message = "Verification passed! Flash contents match firmware file exactly.\n";
            result.message += "Original SHA256: " + originalHash + "\n";
            result.message += "Readback SHA256: " + readbackHash + "\n";
            result.message += "Speed: " + flashReadSpeed;
            
            if (progressCallback)
                progressCallback(100, 100, "Verification complete - MATCH!");
        }
        else
        {
            result.success = false;
            result.message = "Verification failed! Flash contents do NOT match firmware file.\n";
            result.message += "Original SHA256: " + originalHash + "\n";
            result.message += "Readback SHA256: " + readbackHash;
            
            if (progressCallback)
                progressCallback(0, 100, "Verification failed - MISMATCH!");
        }

        VMPROTECT_END_BLOCK();
        return result;
    }

    FlashOperationResult FlashInterface::ReadFlash(
        const std::string& outputPath,
        FPGAChipModel chipModel,
        uint64_t size,
        FlashProgressCallback progressCallback)
    {
        FlashOperationResult result;
        return result;
    }

    FlashOperationResult FlashInterface::EraseChip(
        FPGAChipModel chipModel,
        FlashProgressCallback progressCallback)
    {
        FlashOperationResult result;
        return result;
    }

    bool FlashInterface::ExtractEmbeddedResource(int resourceId, const std::string& outputPath)
    {
        std::cout << "[DEBUG] Extracting resource " << resourceId << " to: " << outputPath << std::endl;
        
        HRSRC hResource = FindResourceA(NULL, MAKEINTRESOURCEA(resourceId), MAKEINTRESOURCEA(RT_RCDATA));
        if (!hResource) {
            std::cout << "[ERROR] Resource not found: " << resourceId << std::endl;
            return false;
        }

        HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
        if (!hLoadedResource) {
            std::cout << "[ERROR] Failed to load resource: " << resourceId << std::endl;
            return false;
        }

        LPVOID pResourceData = LockResource(hLoadedResource);
        if (!pResourceData) {
            std::cout << "[ERROR] Failed to lock resource: " << resourceId << std::endl;
            return false;
        }

        DWORD resourceSize = SizeofResource(NULL, hResource);
        if (resourceSize == 0) {
            std::cout << "[ERROR] Resource has zero size: " << resourceId << std::endl;
            return false;
        }

        std::cout << "[DEBUG] Resource size: " << resourceSize << " bytes" << std::endl;

        std::filesystem::path filePath(outputPath);
        std::filesystem::create_directories(filePath.parent_path());

        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) {
            std::cout << "[ERROR] Failed to create output file: " << outputPath << std::endl;
            return false;
        }

        outFile.write(static_cast<const char*>(pResourceData), resourceSize);
        outFile.close();
        
        // Verify file was written correctly
        if (!std::filesystem::exists(outputPath)) {
            std::cout << "[ERROR] File was not created: " << outputPath << std::endl;
            return false;
        }
        
        auto writtenSize = std::filesystem::file_size(outputPath);
        if (writtenSize != resourceSize) {
            std::cout << "[ERROR] File size mismatch! Expected: " << resourceSize << ", Got: " << writtenSize << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Extracted: " << outputPath << " (" << resourceSize << " bytes)" << std::endl;
        return true;
    }

    void FlashInterface::ExtractBSCANBitstreams()
    {
        std::cout << "[INFO] Extracting BSCAN bitstreams to temp folder..." << std::endl;
        
        // Map of chip models to resource IDs
        std::vector<std::pair<FPGAChipModel, int>> bscanResources = {
            { FPGAChipModel::XC7A35T, IDR_BSCAN_XC7A35T },
            { FPGAChipModel::XC7A50T, IDR_BSCAN_XC7A50T },
            { FPGAChipModel::XC7A75T, IDR_BSCAN_XC7A75T },
            { FPGAChipModel::XC7A100T, IDR_BSCAN_XC7A100T },
            { FPGAChipModel::XC7A200T, IDR_BSCAN_XC7A200T }
        };
        
        int extracted = 0;
        for (const auto& [model, resourceId] : bscanResources)
        {
            std::string filename = "bscan_spi_" + ChipModelToString(model) + ".bit";
            std::string outputPath = m_bscanBasePath + filename;
            
            if (ExtractEmbeddedResource(resourceId, outputPath))
            {
                std::cout << "[INFO] Extracted: " << filename << std::endl;
                extracted++;
            }
            else
            {
                std::cout << "[WARNING] Failed to extract: " << filename << std::endl;
            }
        }
        
        std::cout << "[INFO] Extracted " << extracted << " BSCAN bitstream(s)" << std::endl;
    }

    std::string FlashInterface::CalculateSHA256(const std::string& filePath)
    {
        VMPROTECT_MUTATE_BLOCK("SHA256Hash");
        
        std::cout << "[DEBUG] Calculating SHA256 for: " << filePath << std::endl;
        
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            std::cout << "[ERROR] Failed to open file for SHA256: " << filePath << std::endl;
            return "";
        }

        // Initialize Windows Crypto API
        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        
        if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        {
            std::cout << "[ERROR] CryptAcquireContext failed" << std::endl;
            return "";
        }

        if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
        {
            std::cout << "[ERROR] CryptCreateHash failed" << std::endl;
            CryptReleaseContext(hProv, 0);
            return "";
        }

        // Read file and hash in chunks
        const size_t bufferSize = 8192;
        char buffer[bufferSize];
        
        while (file.read(buffer, bufferSize) || file.gcount() > 0)
        {
            DWORD bytesRead = static_cast<DWORD>(file.gcount());
            if (!CryptHashData(hHash, reinterpret_cast<BYTE*>(buffer), bytesRead, 0))
            {
                std::cout << "[ERROR] CryptHashData failed" << std::endl;
                CryptDestroyHash(hHash);
                CryptReleaseContext(hProv, 0);
                return "";
            }
        }
        
        file.close();

        // Get hash value
        BYTE hashBytes[32]; // SHA-256 = 32 bytes
        DWORD hashLen = 32;
        
        if (!CryptGetHashParam(hHash, HP_HASHVAL, hashBytes, &hashLen, 0))
        {
            std::cout << "[ERROR] CryptGetHashParam failed" << std::endl;
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return "";
        }

        // Convert to hex string
        std::stringstream ss;
        for (DWORD i = 0; i < hashLen; i++)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hashBytes[i]);
        }
        
        std::string hashString = ss.str();
        
        // Convert to uppercase for consistency with PowerShell
        std::transform(hashString.begin(), hashString.end(), hashString.begin(), ::toupper);

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        
        std::cout << "[DEBUG] SHA256: " << hashString << std::endl;
        
        VMPROTECT_END_BLOCK();
        return hashString;
    }
}
