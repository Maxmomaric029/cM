#include "utils/Console.h"
#include "utils/Logger.h"
#include "memory/Memory.h"
#include "visuals/Renderer.h"
#include "ui/Config.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#ifdef _DEBUG
    Console::Alloc();
    Logger::Log("Combat Master Assistant - Debug Mode");
#endif

    Config::Load("config.json");

    Logger::Log("Starting Overlay. Will auto-attach to Combat Master when opening.");

    Renderer renderer;
    renderer.Run();

#ifdef _DEBUG
    Console::Free();
#endif
    return 0;
}
