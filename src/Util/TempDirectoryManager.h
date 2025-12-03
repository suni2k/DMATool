#pragma once
#include <string>
#include <filesystem>

namespace DMATool::Util
{
    /// <summary>
    /// Manages temporary directory cleanup for DMATool
    /// Automatically cleans up temp files when the application exits
    /// </summary>
    class TempDirectoryManager
    {
    public:
        /// <summary>
        /// Get the singleton instance
        /// </summary>
        static TempDirectoryManager& GetInstance();

        /// <summary>
        /// Get the temp directory path for DMATool
        /// Creates it if it doesn't exist
        /// </summary>
        std::string GetTempDirectory();

        /// <summary>
        /// Register for cleanup on application exit
        /// </summary>
        void RegisterCleanup();

        /// <summary>
        /// Clean up all temp files/directories
        /// </summary>
        void Cleanup();

        /// <summary>
        /// Get whether cleanup is enabled
        /// </summary>
        bool IsCleanupEnabled() const { return m_cleanupEnabled; }

        /// <summary>
        /// Set whether cleanup is enabled
        /// </summary>
        void SetCleanupEnabled(bool enabled) { m_cleanupEnabled = enabled; }

    private:
        TempDirectoryManager();
        ~TempDirectoryManager();

        // Prevent copying
        TempDirectoryManager(const TempDirectoryManager&) = delete;
        TempDirectoryManager& operator=(const TempDirectoryManager&) = delete;

        std::string m_tempDirectory;
        bool m_cleanupEnabled = true;
        bool m_cleanupRegistered = false;
    };
}
