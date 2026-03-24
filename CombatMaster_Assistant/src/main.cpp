#include "utils/Console.h"
#include "utils/Logger.h"
#include "memory/Memory.h"
#include "visuals/Renderer.h"
#include "ui/Config.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#ifdef _DEBUG
    Console::Alloc();
    Logger::Log("Nexus Overlay - Debug Mode");
#endif

    Config::Load("config.json");

    Logger::Log("Nexus Overlay starting. Auto-attaching to Combat Master.");

    Renderer renderer;
    renderer.Run();

#ifdef _DEBUG
    Console::Free();
#endif
    return 0;
}
