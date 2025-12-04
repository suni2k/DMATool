#pragma once

#include "LeechCoreWrapper.h"
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <thread>

namespace DMATool::Backend
{
    // Test types
    enum class BenchmarkTestType
    {
        QuickTest = 0,      // Quick 10s speed test
        Throughput = 1,     // Throughput test
        StressTest = 2,     // Long duration stress test
        CustomTest = 3      // User-configured
    };

    // Test results
    struct BenchmarkResults
    {
        // Speed metrics (RPS-based like the other tool)
        uint64_t totalReads = 0;
        double readsPerSecond = 0.0;       // Main metric for Quick Speed Test
        double avgLatencyUs = 0.0;
        double minLatencyUs = 0.0;
        double maxLatencyUs = 0.0;

        // Throughput metrics
        double throughputMBps = 0.0;
        uint64_t bytesTransferred = 0;
        double durationSeconds = 0.0;

        // Per-second progress (for real-time display)
        std::vector<std::pair<int, int>> rpsPerSecond;  // (second, rps)

        // Status
        bool success = false;
        std::string rating; // "ELITE", "AMAZING", "GOOD", "WARNING", "LOW"
        std::vector<std::string> messages;

        // Memory ranges enumerated
        std::vector<std::string> memoryRanges;

        // Progress (0.0 to 1.0)
        float progress = 0.0;
    };

    // Test configuration
    struct BenchmarkConfig
    {
        BenchmarkTestType testType = BenchmarkTestType::QuickTest;
        uint32_t durationSeconds = 10;
        uint32_t testSizeMB = 16;
        std::string memoryAddress = "0x1000";
        uint32_t customReadSizeBytes = 4096;  // For Custom Test - configurable read size
    };

    class BenchmarkInterface
    {
    public:
        BenchmarkInterface();
        ~BenchmarkInterface();

        // Test execution
        bool StartTest(const BenchmarkConfig& config, 
                      std::function<void(const std::string&)> logCallback);
        void StopTest();
        void ForceCleanup();  // Force cleanup of all resources (close LeechCore device)
        bool IsTestRunning() const { return m_IsRunning; }

        // Results
        BenchmarkResults GetCurrentResults() const { return m_CurrentResults; }
        
        // PCILeech paths
        static std::string GetPCILeechPath();
        static bool IsPCILeechAvailable();

    private:
        // Test execution
        bool RunQuickTest(const BenchmarkConfig& config);
        bool RunQuickTestLeechCore(const BenchmarkConfig& config);  // New: Real-time test
        bool RunThroughputTest(const BenchmarkConfig& config);
        
        // PCILeech command execution
        bool ExecutePCILeechCommand(const std::string& args, std::string& output);
        
        // Resource extraction
        static bool ExtractResourceToFile(int resourceId, const std::string& outputPath);
        
        // Result parsing
        void ParseQuickTestOutput(const std::string& output);
        void CalculateRating();
        
        // Logging
        void AddLog(const std::string& message);
        
        // State
        bool m_IsRunning = false;
        std::thread m_TestThread;
        BenchmarkConfig m_CurrentConfig;
        BenchmarkResults m_CurrentResults;
        std::function<void(const std::string&)> m_LogCallback;
    };
}
