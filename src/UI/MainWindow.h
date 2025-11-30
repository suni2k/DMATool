#pragma once

#include <memory>
#include "../Backend/ProjectManager.h"

namespace DMATool::UI
{
    class MainWindow
    {
    public:
        MainWindow(Backend::ProjectManager* projectManager);
        ~MainWindow();

        void Render();

        void SwitchToDNAIDTab() { m_CurrentTab = 0; }

    private:
        void RenderStartupDialog();
        void RenderMainContent();
        void RenderTabs();
        
        Backend::ProjectManager* m_ProjectManager;
        bool m_ShowStartupDialog;
        float m_StartupDialogAlpha;
        
        int m_CurrentTab;
    };
}
