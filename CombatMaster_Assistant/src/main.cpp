#include <windows.h>
#include <iostream>
#include "utils/Console.h"
#include "utils/Logger.h"
#include "memory/Memory.h"
#include "visuals/Renderer.h"
#include "ui/Config.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    Console::Alloc();
    Logger::Log("Nexus Overlay - Initializing Console");

    Config::Load("config.json");

    Logger::Log("Nexus Overlay starting. Auto-attaching to Combat Master.");

    Renderer renderer;
    renderer.Run();

    Console::Free();
    return 0;
}
