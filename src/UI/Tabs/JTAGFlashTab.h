#pragma once

namespace DMATool::UI::Tabs
{
    class JTAGFlashTab
    {
    public:
        static void Render();
        
    private:
        static void RenderFlashInfoPanel();
        static void RenderFlashOperationsPanel();
        static void RenderProgressPanel();
    };
}
