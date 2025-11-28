#pragma once

#include <string>
#include <memory>

namespace DMATool::Backend
{
    // Project data structure
    struct Project
    {
        std::string name;
        std::string path;
        std::string createdDate;
        std::string lastModified;
        
        // Project-specific settings
        struct Settings
        {
            std::string lastUsedDevice;
            std::string targetSystem;
            bool autoConnect;
        } settings;
    };

    class ProjectManager
    {
    public:
        ProjectManager();
        ~ProjectManager();

        bool CreateNewProject(const std::string& name);
        bool LoadProject(const std::string& path);
        bool SaveProject();
        bool CloseProject();

        bool HasActiveProject() const { return m_ActiveProject != nullptr; }
        std::string GetProjectName() const;
        Project* GetActiveProject() { return m_ActiveProject.get(); }

    private:
        std::unique_ptr<Project> m_ActiveProject;
    };
}
