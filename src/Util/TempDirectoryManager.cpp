#include "TempDirectoryManager.h"
#include <windows.h>
#include <iostream>

namespace DMATool::Util
{
    // Static instance
    static TempDirectoryManager* g_instance = nullptr;

    // Cleanup callback for atexit
    static void CleanupOnExit()
    {
        if (g_instance && g_instance->IsCleanupEnabled())
        {
            std::cout << "[INFO] Cleaning up temporary files on exit..." << std::endl;
            g_instance->Cleanup();
        }
    }

    TempDirectoryManager& TempDirectoryManager::GetInstance()
    {
        if (!g_instance)
        {
            g_instance = new TempDirectoryManager();
        }
        return *g_instance;
    }

    TempDirectoryManager::TempDirectoryManager()
    {
        // Get Windows temp directory
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        m_tempDirectory = std::string(tempPath) + "DMATool\\";
        
        // Create the directory if it doesn't exist
        std::filesystem::create_directories(m_tempDirectory);
        
        std::cout << "[DEBUG] TempDirectoryManager initialized: " << m_tempDirectory << std::endl;
    }

    TempDirectoryManager::~TempDirectoryManager()
    {
        // Cleanup is called via atexit, not here
        // This destructor runs too late (after static destruction)
    }

    std::string TempDirectoryManager::GetTempDirectory()
    {
        // Ensure directory exists
        std::filesystem::create_directories(m_tempDirectory);
        return m_tempDirectory;
    }

    void TempDirectoryManager::RegisterCleanup()
    {
        if (!m_cleanupRegistered)
        {
            std::atexit(CleanupOnExit);
            m_cleanupRegistered = true;
            std::cout << "[INFO] Registered temp directory cleanup on exit" << std::endl;
        }
    }

    void TempDirectoryManager::Cleanup()
    {
        try
        {
            if (std::filesystem::exists(m_tempDirectory))
            {
                std::cout << "[INFO] Removing temp directory: " << m_tempDirectory << std::endl;
                
                // Remove all files and subdirectories
                std::filesystem::remove_all(m_tempDirectory);
                
                std::cout << "[SUCCESS] Temp directory cleaned up successfully" << std::endl;
            }
            else
            {
                std::cout << "[DEBUG] Temp directory doesn't exist, nothing to clean up" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to clean up temp directory: " << e.what() << std::endl;
        }
    }
}
