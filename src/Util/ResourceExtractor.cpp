#include "ResourceExtractor.h"
#include <Windows.h>
#include <filesystem>

namespace DMATool::Util
{
    std::string ResourceExtractor::s_TempDir;

    std::string ResourceExtractor::GetTempDLLDirectory()
    {
        if (!s_TempDir.empty())
            return s_TempDir;

        char temp[MAX_PATH];
        GetTempPathA(MAX_PATH, temp);
        s_TempDir = std::string(temp) + "DMATool\\";
        std::filesystem::create_directories(s_TempDir);
        return s_TempDir;
    }

    std::string ResourceExtractor::ExtractDLL(int resourceId, const std::string& dllName)
    {
        std::string tempDir = GetTempDLLDirectory();
        std::string outputPath = tempDir + dllName;

        if (std::filesystem::exists(outputPath))
            return outputPath;

        if (ExtractResource(resourceId, outputPath))
            return outputPath;

        return "";
    }

    bool ResourceExtractor::ExtractResource(int resourceId, const std::string& outputPath)
    {
        HRSRC hResource = FindResourceA(NULL, MAKEINTRESOURCEA(resourceId), "BINARY");
        if (!hResource) return false;

        HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
        if (!hLoadedResource) return false;

        LPVOID pResourceData = LockResource(hLoadedResource);
        if (!pResourceData) return false;

        DWORD dwResourceSize = SizeofResource(NULL, hResource);
        if (dwResourceSize == 0) return false;

        HANDLE hFile = CreateFileA(outputPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        DWORD dwBytesWritten = 0;
        BOOL bSuccess = WriteFile(hFile, pResourceData, dwResourceSize, &dwBytesWritten, NULL);
        CloseHandle(hFile);

        return bSuccess && (dwBytesWritten == dwResourceSize);
    }

    void ResourceExtractor::Cleanup()
    {
        if (s_TempDir.empty()) return;
        try { std::filesystem::remove_all(s_TempDir); } catch (...) { }
    }
}
