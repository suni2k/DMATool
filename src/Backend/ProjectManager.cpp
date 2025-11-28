#include "ProjectManager.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace DMATool::Backend
{
    ProjectManager::ProjectManager()
        : m_ActiveProject(nullptr)
    {
    }

    ProjectManager::~ProjectManager()
    {
    }

    bool ProjectManager::CreateNewProject(const std::string& name)
    {
        m_ActiveProject = std::make_unique<Project>();
        m_ActiveProject->name = name;
        m_ActiveProject->path = "";
        
        // Get current date/time
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);
        
        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
        
        m_ActiveProject->createdDate = oss.str();
        m_ActiveProject->lastModified = oss.str();
        
        // Default settings
        m_ActiveProject->settings.lastUsedDevice = "";
        m_ActiveProject->settings.targetSystem = "Unknown";
        m_ActiveProject->settings.autoConnect = false;
        
        return true;
    }

    bool ProjectManager::LoadProject(const std::string& path)
    {
        // Placeholder: Would load from JSON/XML file
        m_ActiveProject = std::make_unique<Project>();
        m_ActiveProject->name = "Loaded Project";
        m_ActiveProject->path = path;
        
        return true;
    }

    bool ProjectManager::SaveProject()
    {
        if (!m_ActiveProject)
        {
            return false;
        }
        
        // Placeholder: Would save to JSON/XML file
        
        return true;
    }

    bool ProjectManager::CloseProject()
    {
        m_ActiveProject.reset();
        return true;
    }

    std::string ProjectManager::GetProjectName() const
    {
        if (m_ActiveProject)
        {
            return m_ActiveProject->name;
        }
        return "No Project";
    }
}
