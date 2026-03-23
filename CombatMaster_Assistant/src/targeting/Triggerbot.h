#pragma once
#include "../ui/Config.h"
#include <windows.h>
#include <chrono>

namespace Triggerbot {
    inline std::chrono::steady_clock::time_point lastShot = std::chrono::steady_clock::now();

    inline void Run(bool hasTargetInCrosshair) {
        if (!Config::triggerbot_enabled) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count();

        if (hasTargetInCrosshair && elapsed >= Config::triggerbot_delay) {
            // Simulate mouse click
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &input, sizeof(INPUT));

            Sleep(20); // Hold click briefly

            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(INPUT));

            lastShot = std::chrono::steady_clock::now();
        }
    }
}
