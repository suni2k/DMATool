#pragma once

namespace DMATool::UI::Tabs
{
    class JTAGPortTab
    {
    public:
        static void Render();
        
    private:
        static void RenderConnectionPanel();
        static void RenderControlPanel();
        static void RenderStatusPanel();
    };
}
