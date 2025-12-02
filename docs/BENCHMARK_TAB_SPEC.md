# DMA Benchmark Tab - Implementation Specification

## ?? Overview

This document outlines the implementation plan for the **Benchmark DMA** tab in DMATool, providing comprehensive DMA performance testing and validation capabilities.

---

## ?? UI Design

### Tab Layout

```
????????????????????????????????????????????????????????????????????
? DMA KINGS TOOL                                              [X]  ?
????????????????????????????????????????????????????????????????????
?  [DNA ID]  [Flash DMA]  [?? Benchmark DMA]                       ?
????????????????????????????????????????????????????????????????????
?                                                                   ?
?  ???????????????????????  ????????????????????????????????????  ?
?  ? Test Controls       ?  ? Real-Time Results                ?  ?
?  ?                     ?  ?                                  ?  ?
?  ? Quick Tests:        ?  ?  ?????????????????????????????? ?  ?
?  ? [? Quick Speed]    ?  ?  ? Current Test: Quick Speed  ? ?  ?
?  ? [?? Throughput]     ?  ?  ? Progress: [??????] 60%    ? ?  ?
?  ? [?? Mixed Test]     ?  ?  ? Time Remaining: 4s        ? ?  ?
?  ? [?? Stress Test]    ?  ?  ?????????????????????????????? ?  ?
?  ?                     ?  ?                                  ?  ?
?  ? Custom Test:        ?  ?  Performance Metrics:           ?  ?
?  ? Duration: [10]s     ?  ?  ????????????????????????????? ?  ?
?  ? Read Size: [1]bytes ?  ?  ? RPS          ?    649     ? ?  ?
?  ? [??? Custom Test]    ?  ?  ? Throughput   ? 145.3 MB/s ? ?  ?
?  ?                     ?  ?  ? Avg Latency  ? 1,541 탎   ? ?  ?
?  ? Advanced:           ?  ?  ? Min Latency  ? 1,049 탎   ? ?  ?
?  ? Algorithm: [Auto?]  ?  ?  ? Max Latency  ? 11,736 탎  ? ?  ?
?  ? Logging: [None?]    ?  ?  ????????????????????????????? ?  ?
?  ? [?? Settings]       ?  ?                                  ?  ?
?  ?                     ?  ?  Status: ? EXCELLENT            ?  ?
?  ? [? Stop Test]      ?  ?  ?? ?? ?? ?? ?? ?? ?? ?? ?? ??  ?  ?
?  ? [?? Export Report]  ?  ?  Performance Graph:             ?  ?
?  ?                     ?  ?  RPS? ????????????????????     ?  ?
?  ? [? Help]           ?  ?     ??????????????????????????  ?  ?
?  ???????????????????????  ?     0s                    10s   ?  ?
?                            ????????????????????????????????????  ?
?????????????????????????????????????????????????????????????????????
? Console Log                                                       ?
? ???????????????????????????????????????????????????????????????  ?
? ? [INFO] Starting Quick Speed Test...                         ?  ?
? ? [INFO] Enumerating memory ranges...                         ?  ?
? ? [SUCCESS] Added 6 memory ranges                             ?  ?
? ? [01/10s]: 616 RPS ?                                         ?  ?
? ? [02/10s]: 613 RPS ?                                         ?  ?
? ? [03/10s]: 632 RPS ?                                         ?  ?
? ? ...                                                          ?  ?
? ? [SUCCESS] Test completed: 649 RPS (GOOD)                    ?  ?
? ???????????????????????????????????????????????????????????????  ?
????????????????????????????????????????????????????????????????????
```

---

## ??? Architecture

### Class Structure

```cpp
namespace DMATool::Backend
{
    // Benchmark test types
    enum class BenchmarkTestType
    {
        QuickSpeed,      // 10s speed test
        Throughput,      // 1GB throughput test
        MixedTest,       // Speed + Throughput
        StressTest,      // Extended duration test
        CustomSpeed      // User-configured test
    };

    // FPGA algorithms
    enum class FPGAAlgorithm
    {
        Auto = 0,
        AsyncNormal = 1,
        AsyncTiny = 2,
        OldNormal = 3,
        OldTiny = 4
    };

    // Test results structure
    struct BenchmarkResults
    {
        // Speed test results
        uint64_t totalReads = 0;
        double avgLatencyUs = 0.0;
        double minLatencyUs = 0.0;
        double maxLatencyUs = 0.0;
        double readsPerSecond = 0.0;

        // Throughput results
        double throughputMBps = 0.0;
        uint64_t bytesTransferred = 0;
        double durationSeconds = 0.0;

        // Status
        bool passed = false;
        std::string status; // "EXCELLENT", "GOOD", "ACCEPTABLE", "POOR"
        std::vector<std::string> warnings;
        std::vector<std::string> errors;

        // Timestamped data points for graphing
        std::vector<std::pair<double, double>> rpsHistory;  // (time, rps)
        std::vector<std::pair<double, double>> latencyHistory;  // (time, latency)
    };

    // Benchmark configuration
    struct BenchmarkConfig
    {
        BenchmarkTestType testType = BenchmarkTestType::QuickSpeed;
        uint32_t durationSeconds = 10;
        uint32_t readSizeBytes = 1;
        FPGAAlgorithm algorithm = FPGAAlgorithm::Auto;
        uint32_t loggingLevel = 0; // 0=none, 1=verbose, 2=very verbose
        
        // Custom memory range (optional)
        uint64_t memoryRangeStart = 0x1000;
        uint64_t memoryRangeEnd = 0x100000;
        bool useCustomMemoryRange = false;
    };

    // Main benchmark interface
    class BenchmarkInterface
    {
    public:
        // Start a benchmark test
        bool StartTest(const BenchmarkConfig& config,
                      std::function<void(const std::string&)> logCallback);

        // Stop running test
        void StopTest();

        // Check if test is running
        bool IsTestRunning() const { return m_IsRunning; }

        // Get current results (updated in real-time)
        BenchmarkResults GetCurrentResults() const;

        // Get final results after test completes
        BenchmarkResults GetFinalResults() const;

        // Export results to file
        bool ExportResults(const std::string& format); // "txt", "csv", "json"

    private:
        // PCILeech interface
        bool RunPCILeechBenchmark(const BenchmarkConfig& config);
        bool RunPCILeechTestMemRead(const BenchmarkConfig& config);
        bool RunPCILeechDump(const BenchmarkConfig& config);

        // Result parsing
        BenchmarkResults ParsePCILeechOutput(const std::string& output);
        void CalculatePerformanceStatus(BenchmarkResults& results);

        // State
        bool m_IsRunning = false;
        BenchmarkResults m_CurrentResults;
        BenchmarkResults m_FinalResults;
        BenchmarkConfig m_CurrentConfig;
    };
}

namespace DMATool::UI::Tabs
{
    class BenchmarkTab
    {
    public:
        static void Render();

    private:
        // Panel rendering
        static void RenderTestControlsPanel();
        static void RenderResultsPanel();
        static void RenderConsoleLog();

        // Test execution
        static void StartQuickSpeedTest();
        static void StartThroughputTest();
        static void StartMixedTest();
        static void StartStressTest();
        static void StartCustomTest();
        static void StopCurrentTest();

        // UI helpers
        static void RenderPerformanceGraph();
        static void RenderMetricsTable();
        static void RenderProgressBar(float progress);
        static ImVec4 GetStatusColor(const std::string& status);

        // Export
        static void ExportReport();

        // Static state
        static Backend::BenchmarkInterface s_Benchmark;
        static Backend::BenchmarkConfig s_Config;
        static Backend::BenchmarkResults s_Results;
        static std::vector<std::string> s_LogMessages;
        static bool s_IsTestRunning;
        static float s_TestProgress;
    };
}
```

---

## ?? PCILeech Integration

### Command Execution

```cpp
namespace DMATool::Backend
{
    class PCILeechExecutor
    {
    public:
        struct ExecutionResult
        {
            int exitCode = 0;
            std::string stdout;
            std::string stderr;
            bool success = false;
        };

        // Execute PCILeech command
        static ExecutionResult Execute(
            const std::string& command,
            const std::vector<std::string>& args,
            std::function<void(const std::string&)> outputCallback = nullptr
        );

        // Specific test commands
        static ExecutionResult RunBenchmark(FPGAAlgorithm algo);
        static ExecutionResult RunTestMemRead(uint64_t address, uint32_t iterations);
        static ExecutionResult RunDump(uint64_t start, uint64_t end);
        static ExecutionResult RunProbe();

    private:
        static std::string BuildCommandLine(
            const std::string& command,
            const std::vector<std::string>& args
        );

        static std::string FormatDeviceParam(FPGAAlgorithm algo);
    };
}
```

### Example Implementation:

```cpp
BenchmarkInterface::ExecutionResult BenchmarkInterface::RunBenchmark(FPGAAlgorithm algo)
{
    std::vector<std::string> args;
    args.push_back("benchmark");

    if (algo != FPGAAlgorithm::Auto)
    {
        args.push_back("-device");
        args.push_back(FormatDeviceParam(algo));
    }

    return Execute("pcileech.exe", args, [](const std::string& line) {
        // Parse real-time output
        AddLog(line);
    });
}

std::string BenchmarkInterface::FormatDeviceParam(FPGAAlgorithm algo)
{
    switch (algo)
    {
    case FPGAAlgorithm::AsyncNormal: return "fpga://algo=1";
    case FPGAAlgorithm::AsyncTiny:   return "fpga://algo=2";
    case FPGAAlgorithm::OldNormal:   return "fpga://algo=3";
    case FPGAAlgorithm::OldTiny:     return "fpga://algo=4";
    case FPGAAlgorithm::Auto:
    default:                         return "fpga";
    }
}
```

---

## ?? Output Parsing

### PCILeech Benchmark Output Format:

```
Memory Benchmark:
  Read Performance:
    1MB  :  150.2 MB/s
    10MB :  145.8 MB/s
    100MB:  142.3 MB/s

  Write Performance:
    1MB  :  120.5 MB/s
    10MB :  118.2 MB/s
    100MB:  115.7 MB/s
```

### Parsing Implementation:

```cpp
BenchmarkResults BenchmarkInterface::ParsePCILeechOutput(const std::string& output)
{
    BenchmarkResults results;

    // Parse throughput values
    std::regex mbpsRegex(R"((\d+)MB\s*:\s*(\d+\.\d+)\s*MB/s)");
    std::smatch match;

    std::string::const_iterator searchStart(output.cbegin());
    while (std::regex_search(searchStart, output.cend(), match, mbpsRegex))
    {
        double mbps = std::stod(match[2].str());
        results.throughputMBps = std::max(results.throughputMBps, mbps);
        searchStart = match.suffix().first;
    }

    // Calculate status
    CalculatePerformanceStatus(results);

    return results;
}

void BenchmarkInterface::CalculatePerformanceStatus(BenchmarkResults& results)
{
    // Throughput classification (USB3/FT601)
    if (results.throughputMBps > 150.0)
        results.status = "EXCELLENT";
    else if (results.throughputMBps > 120.0)
        results.status = "GOOD";
    else if (results.throughputMBps > 80.0)
        results.status = "ACCEPTABLE";
    else
        results.status = "POOR";

    // RPS classification
    if (results.readsPerSecond > 700)
        results.status = "EXCELLENT";
    else if (results.readsPerSecond > 600)
        results.status = results.status == "EXCELLENT" ? results.status : "GOOD";
    else if (results.readsPerSecond > 400)
        results.status = (results.status == "EXCELLENT" || results.status == "GOOD") 
                         ? results.status : "ACCEPTABLE";
    else
        results.status = "POOR";

    results.passed = (results.status != "POOR");
}
```

---

## ?? UI Components

### 1. Performance Graph (ImPlot)

```cpp
void BenchmarkTab::RenderPerformanceGraph()
{
    if (ImPlot::BeginPlot("RPS Over Time", ImVec2(-1, 200)))
    {
        ImPlot::SetupAxes("Time (s)", "RPS", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        
        std::vector<double> timePoints;
        std::vector<double> rpsValues;
        
        for (const auto& [time, rps] : s_Results.rpsHistory)
        {
            timePoints.push_back(time);
            rpsValues.push_back(rps);
        }
        
        ImPlot::PlotLine("RPS", timePoints.data(), rpsValues.data(), timePoints.size());
        
        ImPlot::EndPlot();
    }
}
```

### 2. Metrics Table

```cpp
void BenchmarkTab::RenderMetricsTable()
{
    ImGui::BeginTable("Metrics", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
    
    ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 150);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("RPS");
    ImGui::TableNextColumn();
    ImGui::TextColored(GetStatusColor(s_Results.status), "%.0f", s_Results.readsPerSecond);
    
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Throughput");
    ImGui::TableNextColumn();
    ImGui::TextColored(GetStatusColor(s_Results.status), "%.2f MB/s", s_Results.throughputMBps);
    
    // ... more rows
    
    ImGui::EndTable();
}
```

### 3. Progress Bar with Status

```cpp
void BenchmarkTab::RenderProgressBar(float progress)
{
    ImVec2 progressSize(-1, 30);
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::Info);
    ImGui::ProgressBar(progress, progressSize, "");
    ImGui::PopStyleColor();
    
    // Overlay text
    ImGui::SameLine(0, -progressSize.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::Text("%.0f%% Complete", progress * 100.0f);
}
```

---

## ?? File Export

### Export Formats

#### 1. **Text Report (`.txt`)**

```
???????????????????????????????????????????
 DMA BENCHMARK REPORT
???????????????????????????????????????????

Test Date: 2025-01-15 14:32:15
Test Type: Quick Speed Test
Duration: 10 seconds

FPGA Information:
- Model: Xilinx Artix-7 XC7A100T
- DNA ID: 00542417dc636678
- Adapter: CH347 (USB3/FT601)

Configuration:
- Algorithm: Auto
- Read Size: 1 byte
- Logging: None

Results:
?????????????????????????????????????????
Performance Metrics:
  Total Reads: 6,494
  Reads Per Second (RPS): 649 ? GOOD
  Avg. Latency: 1,541 탎
  Min. Latency: 1,049 탎
  Max. Latency: 11,736 탎

Throughput: N/A (Speed test only)

Status: ? PASS (GOOD)
?????????????????????????????????????????

Per-Second Breakdown:
[01/10s]: 616 RPS
[02/10s]: 613 RPS
[03/10s]: 632 RPS
...
[10/10s]: 640 RPS

???????????????????????????????????????????
Generated by DMATool v1.0
```

#### 2. **CSV Export (`.csv`)**

```csv
Test Type,Date,Time,Duration (s),FPGA Model,DNA ID,Algorithm,Read Size,Total Reads,RPS,Avg Latency (탎),Min Latency (탎),Max Latency (탎),Throughput (MB/s),Status
Quick Speed Test,2025-01-15,14:32:15,10,XC7A100T,00542417dc636678,Auto,1,6494,649,1541,1049,11736,N/A,GOOD
```

#### 3. **JSON Export (`.json`)**

```json
{
  "testInfo": {
    "type": "QuickSpeedTest",
    "date": "2025-01-15",
    "time": "14:32:15",
    "duration": 10
  },
  "hardware": {
    "fpgaModel": "XC7A100T",
    "dnaId": "00542417dc636678",
    "adapter": "CH347",
    "connection": "USB3/FT601"
  },
  "configuration": {
    "algorithm": "Auto",
    "readSize": 1,
    "loggingLevel": 0
  },
  "results": {
    "totalReads": 6494,
    "readsPerSecond": 649,
    "avgLatency": 1541,
    "minLatency": 1049,
    "maxLatency": 11736,
    "throughput": null,
    "status": "GOOD",
    "passed": true
  },
  "history": [
    {"second": 1, "rps": 616},
    {"second": 2, "rps": 613},
    ...
  ]
}
```

---

## ?? Test State Machine

```
???????????????
?    IDLE     ?
???????????????
       ? [Start Test]
       ?
???????????????
? INITIALIZING????????
???????????????      ? [Error]
       ?             ?
       ? [Success]   ?
       ?             ?
???????????????  ??????????
?   RUNNING   ?  ? ERROR  ?
???????????????  ??????????
       ?
       ??? [Update] ??> Update UI every 100ms
       ?
       ??? [Stop] ????> STOPPED
       ?
       ??? [Complete] ?> COMPLETED
                          ?
                          ??> [Export]
                          ??> [IDLE]
```

---

## ?? Implementation Checklist

### Phase 1: Basic Infrastructure
- [ ] Create `BenchmarkInterface` class
- [ ] Implement PCILeech command execution
- [ ] Add output parsing logic
- [ ] Test with basic benchmark command

### Phase 2: UI Development
- [ ] Design tab layout
- [ ] Implement test control buttons
- [ ] Add results display panel
- [ ] Integrate console log

### Phase 3: Test Types
- [ ] Quick Speed Test
- [ ] Throughput Test
- [ ] Mixed Test
- [ ] Stress Test
- [ ] Custom Test

### Phase 4: Advanced Features
- [ ] Real-time graphing (ImPlot)
- [ ] Performance classification
- [ ] Algorithm selection
- [ ] Memory range configuration

### Phase 5: Export & Reporting
- [ ] Text report export
- [ ] CSV export
- [ ] JSON export
- [ ] PDF generation (optional)

### Phase 6: Polish
- [ ] Error handling
- [ ] Progress indicators
- [ ] Help/tooltips
- [ ] Settings persistence

---

**Next Step:** Begin Phase 1 - Create `BenchmarkInterface` class and PCILeech integration.

**Estimated Timeline:** 2-3 weeks for complete implementation.
