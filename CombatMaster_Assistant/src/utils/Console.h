#pragma once
#include <iostream>
#include <windows.h>

namespace Console {
    inline void Alloc() {
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
        SetConsoleTitleA("Combat Master Assistant - Debug Console");
    }

    inline void Free() {
        fclose(stdout);
        fclose(stderr);
        fclose(stdin);
        FreeConsole();
    }
}
