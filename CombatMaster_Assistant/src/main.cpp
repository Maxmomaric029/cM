#include <windows.h>
#include <thread>
#include "utils/Logger.h"
#include "utils/Console.h"
#include "memory/Memory.h"
#include "ui/Config.h"
#include "overlay/Overlay.h"
#include "Global.h"

int main() {
    // Allocate debug console
    Console::Alloc();
    Logger::Log("=== Nexus External - Starting ===");

    // 1. Wait for the game process
    const char* GAME_PROCESS = "CombatMaster-Win64-Shipping.exe";
    if (!Memory::Get().WaitForProcess(GAME_PROCESS, 60000)) {
        Logger::Error("FATAL: Could not find game process! Make sure Combat Master is running.");
        Console::Free();
        return 1;
    }

    // 2. Resolve in-game module bases (Project.dll, GameAssembly.dll)
    if (!Memory::Get().InitModules()) {
        Logger::Error("FATAL: Could not resolve game modules.");
        Console::Free();
        return 1;
    }

    // 3. Load config
    Config::Load("nexus_config.json");
    Logger::Log("Config loaded.");

    // 4. Start overlay (creates transparent window + ImGui, blocks until exit)
    Logger::Log("Starting overlay...");
    Overlay::Get().Init();

    // 5. Save config on exit
    Config::Save("nexus_config.json");
    Logger::Log("Nexus External — Unloaded cleanly.");
    Console::Free();
    return 0;
}
