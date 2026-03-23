@echo off
setlocal
cd /d "%~dp0\.."

echo [1/3] Downloading dependencies...
if not exist "include\imgui" (
    git clone -b docking https://github.com/ocornut/imgui.git include/imgui
)
if not exist "include\nlohmann\json.hpp" (
    mkdir "include\nlohmann" 2>nul
    curl -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp -o include\nlohmann\json.hpp
)

echo [2/3] Configuring CMake...
mkdir build 2>nul
cd build
cmake .. -A x64

echo [3/3] Building Release...
cmake --build . --config Release

echo Build finished. Executable is in build/Release/
pause
