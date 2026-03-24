#pragma once
#include "../ui/Config.h"
#include <windows.h>
#include <chrono>
#include <random>
#include <thread>

namespace Triggerbot {
    inline std::chrono::steady_clock::time_point lastShot = std::chrono::steady_clock::now();
    
    // RNG for humanization
    inline int GetRandomDelay(int baseDelay, bool randomize) {
        if (!randomize) return baseDelay;
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        // Apply a +/- 30% variance to the base delay
        int variance = static_cast<int>(baseDelay * 0.30f);
        if (variance == 0) variance = 10;
        std::uniform_int_distribution<> distr(baseDelay - variance, baseDelay + variance);
        
        return distr(gen);
    }

    inline void Run(bool hasTargetInCrosshair) {
        if (!Config::triggerbot_enabled) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count();
        
        // Calculate true delay incorporating humanization
        int actualDelay = GetRandomDelay(Config::triggerbot_delay, Config::triggerbot_randomize);

        if (hasTargetInCrosshair && elapsed >= actualDelay) {
            
            // Validate window focus to prevent misfires in other apps
            HWND foreground = GetForegroundWindow();
            DWORD processId;
            GetWindowThreadProcessId(foreground, &processId);
            if (processId != Memory::Get().GetProcessId()) return;

            // Prepare Down Event
            INPUT inputDown = { 0 };
            inputDown.type = INPUT_MOUSE;
            inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &inputDown, sizeof(INPUT));

            // Humanized click-hold latency
            int holdTime = GetRandomDelay(25, Config::triggerbot_randomize); // 25ms base hold
            std::this_thread::sleep_for(std::chrono::milliseconds(holdTime));

            // Prepare Up Event
            INPUT inputUp = { 0 };
            inputUp.type = INPUT_MOUSE;
            inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &inputUp, sizeof(INPUT));

            lastShot = std::chrono::steady_clock::now();
        }
    }
}
