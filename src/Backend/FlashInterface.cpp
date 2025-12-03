#include "FlashInterface.h"
#include "OpenOCDInterface.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <algorithm>
#include <iostream>
#include "../resource.h"
#include "../Util/TempDirectoryManager.h"

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
        FPGAInfo fpgaInfo = openocd.DetectFPGA([&](const std::string& msg) {
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
        std::string& error)
    {
        std::string cmdLine = "\"" + m_openocdPath + "\" -f \"" + configPath + "\"";
        
        for (const auto& cmd : commands)
        {
            cmdLine += " -c \"" + cmd + "\"";
        }

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
            CloseHandle(hStdOutRead);
            CloseHandle(hStdErrRead);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;

        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        while (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            error += buffer;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

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
        FlashOperationResult result;

        if (!fs::exists(firmwarePath))
        {
            result.message = "Firmware file not found: " + firmwarePath;
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

        std::string configPath = CreateOpenOCDConfig(chipModel);
        std::string bscanPath = GetBSCANBitstreamPath(chipModel);
        std::string firmwarePathUnix = firmwarePath;
        std::replace(bscanPath.begin(), bscanPath.end(), '\\', '/');
        std::replace(firmwarePathUnix.begin(), firmwarePathUnix.end(), '\\', '/');

        std::vector<std::string> cmds = {
            "init",
            "jtagspi_init 0 \"" + bscanPath + "\"",
            "jtagspi_program \"" + firmwarePathUnix + "\" 0x0",
            "xc7_program xc7.tap"
        };

        if (verifyAfter)
        {
            cmds.push_back("flash verify_bank 0 \"" + firmwarePathUnix + "\"");
        }

        cmds.push_back("shutdown");

        if (progressCallback)
            progressCallback(10, 100, "Programming flash...");

        std::string output, error;
        result.success = ExecuteOpenOCDCommand(configPath, cmds, output, error);

        if (fs::exists(configPath))
            fs::remove(configPath);

        auto endTime = std::chrono::steady_clock::now();
        result.durationSeconds = std::chrono::duration<double>(endTime - startTime).count();

        if (result.success)
        {
            std::regex bytesRegex(R"(read (\d+) bytes from file)");
            std::smatch match;
            if (std::regex_search(output, match, bytesRegex))
            {
                result.bytesProcessed = std::stoull(match[1].str());
            }

            result.message = "Flash programming completed successfully";
            if (progressCallback)
                progressCallback(100, 100, result.message);
        }
        else
        {
            result.message = "Flash programming failed: " + error;
            if (progressCallback)
                progressCallback(0, 100, result.message);
        }

        return result;
    }

    FlashOperationResult FlashInterface::VerifyFirmware(
        const std::string& firmwarePath,
        FPGAChipModel chipModel,
        FlashProgressCallback progressCallback)
    {
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
        bool readSuccess = ExecuteOpenOCDCommand(configPath, cmds, output, error);

        if (fs::exists(configPath))
            fs::remove(configPath);

        if (!readSuccess)
        {
            result.message = "Failed to read flash: " + error;
            if (fs::exists(readbackPath))
                fs::remove(readbackPath);
            return result;
        }

        if (progressCallback)
            progressCallback(70, 100, "Comparing files...");

        std::ifstream originalFile(firmwarePath, std::ios::binary);
        std::ifstream readbackFile(readbackPath, std::ios::binary);

        if (!originalFile.is_open() || !readbackFile.is_open())
        {
            result.message = "Failed to open files for comparison";
            if (fs::exists(readbackPath))
                fs::remove(readbackPath);
            return result;
        }

        bool filesMatch = true;
        uint64_t bytesCompared = 0;
        const size_t bufferSize = 4096;
        char buffer1[bufferSize];
        char buffer2[bufferSize];

        while (originalFile.read(buffer1, bufferSize) && readbackFile.read(buffer2, bufferSize))
        {
            size_t bytesRead = originalFile.gcount();
            
            if (std::memcmp(buffer1, buffer2, bytesRead) != 0)
            {
                filesMatch = false;
                break;
            }

            bytesCompared += bytesRead;

            if (progressCallback && firmwareSize > 0)
            {
                float percent = 70.0f + (30.0f * (float)bytesCompared / (float)firmwareSize);
                progressCallback((uint64_t)percent, 100, "Comparing: " + std::to_string(bytesCompared / 1024 / 1024) + " MB...");
            }
        }

        if (originalFile.gcount() != readbackFile.gcount())
        {
            filesMatch = false;
        }

        originalFile.close();
        readbackFile.close();

        if (fs::exists(readbackPath))
            fs::remove(readbackPath);

        auto endTime = std::chrono::steady_clock::now();
        result.durationSeconds = std::chrono::duration<double>(endTime - startTime).count();

        if (filesMatch)
        {
            result.success = true;
            result.message = "Verification passed! Flash contents match firmware file exactly.";
            if (progressCallback)
                progressCallback(100, 100, "Verification complete - MATCH!");
        }
        else
        {
            result.success = false;
            result.message = "Verification failed! Flash contents do NOT match firmware file.";
            if (progressCallback)
                progressCallback(0, 100, "Verification failed - MISMATCH!");
        }

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
}
