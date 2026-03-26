#pragma once
#include <windows.h>

// Global flags and handles for the cheat
inline bool g_UnloadRequested = false;
inline HMODULE g_hInjectModule = nullptr;
