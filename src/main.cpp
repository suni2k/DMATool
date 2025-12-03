// DMATool - Modern DMA Hardware Interface Tool
// 
// Hardware Integration References:
// - pcileech: https://github.com/ufrisk/pcileech (DMA attack framework)
// - pcileech-fpga: https://github.com/ufrisk/pcileech-fpga (FPGA firmware)
// - LeechCore: https://github.com/ufrisk/LeechCore (Physical memory acquisition library)
// - MemProcFS: https://github.com/ufrisk/MemProcFS (Memory process file system)
// - MemProcFS-plugins: https://github.com/ufrisk/MemProcFS-plugins (Plugin system)
// - ch347: https://github.com/WCHSoftGroup/ch347 (USB-JTAG/SPI bridge)
//
// Planned Integration Areas:
// - JTAG operations via CH347 adapter
// - DMA operations via PCILeech framework
// - Memory access via LeechCore
// - Flash programming for FPGA firmware

#include "Application.h"
#include <memory>
#include <Windows.h>

// Entry point for Windows subsystem (no console)
#ifdef NDEBUG  // Release build
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    auto app = std::make_unique<DMATool::Application>("DMATool", 1400, 900);
    
    if (!app->Initialize())
    {
        return -1;
    }
    
    app->Run();
    app->Shutdown();
    
    return 0;
}
#else  // Debug build (console)
int main(int argc, char** argv)
{
    auto app = std::make_unique<DMATool::Application>("DMATool", 1400, 900);
    
    if (!app->Initialize())
    {
        return -1;
    }
    
    app->Run();
    app->Shutdown();
    
    return 0;
}
#endif
