#pragma once
#include <Windows.h>
#include <string>

namespace DMATool::Util
{
    class ResourceExtractor
    {
    public:
        // Extract embedded DLL resource to temp directory
        static std::string ExtractDLL(int resourceId, const std::string& dllName);
        
        // Get temp directory for extracted DLLs
        static std::string GetTempDLLDirectory();
        
        // Clean up extracted DLLs on exit
        static void Cleanup();
        
    private:
        static bool ExtractResource(int resourceId, const std::string& outputPath);
        static std::string s_TempDir;
    };
}
