# CM Base Internal Project

A minimal internal (DLL) base for Combat Master with ESP, aimbot, and extra options.

## Build requirements

- Visual Studio 2017 or later (v141/v142/v143 toolset) with **Desktop development with C++** and **Windows 10 SDK**
- Solution opened from: `CM Base Internal Project.sln`

## Dependencies

- **ImGui** – expected at `$(SolutionDir)..\Dependencies\IMGUI\` (imgui.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp, imgui_impl_win32.cpp, imgui_impl_dx11.cpp, and headers)
- **Detours** – expected at `$(SolutionDir)..\Dependencies\Detours\` (include + detours.lib)

Ensure those paths exist relative to the solution before building.

## How to build

1. Open `CM Base Internal Project.sln` in Visual Studio.
2. Select configuration **Release** and platform **x64**.
3. Build Solution (Ctrl+Shift+B).

Output:

- `CM Base Internal Project\CM Base Internal Project\x64\Release\CM Base Internal Project.dll`
- `CM Base Internal Project\CM Base Internal Project\x64\Release\CM Base Internal Project.pdb` (include this with your release for analyzers)

## Release checklist (e.g. UCDownloads)

- Upload **Release** builds (x64), not Debug.
- Include the **PDB** (`CM Base Internal Project.pdb`) alongside the DLL.
- Do not pack, obfuscate, or password-protect the DLL/archive.
- If you bundle an injector, prefer linking to an already approved UCDownloads injector instead of bundling your own.
- Source code must be uploaded to the site (e.g. UCDownloads); do not link to external repos (e.g. GitHub) as the primary source.

## .NET

This project is **native C++**. The “disable local copy for system DLLs” guideline applies only to .NET projects and is not relevant here.
