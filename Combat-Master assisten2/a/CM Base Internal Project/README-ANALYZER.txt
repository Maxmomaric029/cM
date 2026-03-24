NOTE FOR ANALYZERS
=================

This archive contains source code and project files so you can review and build it instead of analyzing a binary only. Please build in Release|x64 and use the generated DLL + PDB for verification if needed.

Build steps:
1. Open CM Base Internal Project.sln in Visual Studio.
2. Set configuration to Release, platform to x64.
3. Build Solution.

Dependencies (must be present relative to solution):
- ..\Dependencies\IMGUI\  (ImGui)
- ..\Dependencies\Detours\ (Microsoft Detours lib + headers)

After approval you may remove this file and any source-only note from the release; the uploader has been informed.
