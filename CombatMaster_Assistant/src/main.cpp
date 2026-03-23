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

    Logger::Log("Waiting for Combat Master...");
    while (!mem.Attach(L"CombatMaster.exe")) {
        Sleep(1000);
    }
    Logger::Log("Attached to Combat Master successfully!");

    // Start overlay and menu blocking the main thread
    renderer.Run();

#ifdef _DEBUG
    Console::Free();
#endif
    return 0;
}
