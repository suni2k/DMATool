#pragma once

namespace DMATool::UI::Tabs
{
    class DataPortTab
    {
    public:
        static void Render();
        
    private:
        static void RenderDMAConfigPanel(float height);
        static void RenderMemoryOperationsPanel(float height);
        static void RenderMemoryViewPanel(float height);
    };
}
