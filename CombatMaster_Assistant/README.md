# Combat Master Assistant

A modern, external assistance tool for Combat Master (Unreal Engine 4.27), featuring an ImGui overlay for ESP, Aimbot, and Triggerbot via memory reading. 

## Features
- **Visuals (ESP)**: Enemy outlines, names, health bars, distance, and reference lines. (Colors customizable)
- **Aimbot**: Field of View based closest enemy targeting, smoothing, predicting.
- **Triggerbot**: Auto-fire when locking on an enemy.
- **UI**: Modern ImGui theme and persistence through config.json.

## Building Instructions
1. Requires **Visual Studio 2022** and **CMake** installed.
2. Run `scripts/build.bat`. This will automatically:
   - Clone `imgui` to `include/imgui`.
   - Download `nlohmann/json` to `include/nlohmann`.
   - Setup and build the x64 Release project files using CMake.
3. Once completed, your executable will be located in `build/Release/CombatMaster_Assistant.exe`.

## Usage
Run the game, set it to Borderless or Windowed mode. Run the executable as Administrator. 
Press **INSERT** to toggle the menu.
