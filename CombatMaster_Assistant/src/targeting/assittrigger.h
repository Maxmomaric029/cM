#pragma once
#include "../ui/Config.h"
#include <windows.h>
#include <chrono>
#include <random>

namespace Triggerbot {
    inline std::chrono::steady_clock::time_point lastActionTime = std::chrono::steady_clock::now();
    inline bool isMouseDown = false;
    inline int nextDelay = 0;
    inline int currentHoldTime = 0;
    
    inline int GetRandomDelay(int baseDelay, bool randomize) {
        if (!randomize) return baseDelay;
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        int variance = static_cast<int>(baseDelay * 0.30f);
        if (variance == 0) variance = 10;
        
        std::uniform_int_distribution<> distr(baseDelay - variance, baseDelay + variance);
        return distr(gen);
    }

    // Now receives Menu status to prevent firing when the menu is open
    inline void Run(bool hasTargetInCrosshair, bool isMenuOpen) {
        if (!Config::triggerbot_enabled || isMenuOpen) {
            // Failsafe release if cheat disabled while shooting
            if (isMouseDown) {
                INPUT inputUp = { 0 };
                inputUp.type = INPUT_MOUSE;
                inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &inputUp, sizeof(INPUT));
                isMouseDown = false;
            }
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActionTime).count();
        
        if (isMouseDown) {
            // Currently holding the click down. Wait until hold time elapsed to release.
            if (elapsed >= currentHoldTime) {
                INPUT inputUp = { 0 };
                inputUp.type = INPUT_MOUSE;
                inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &inputUp, sizeof(INPUT));
                
                isMouseDown = false;
                lastActionTime = now;
                
                // Calculate logic for the NEXT shot delay once
                nextDelay = GetRandomDelay(Config::triggerbot_delay, Config::triggerbot_randomize);
            }
        } else {
            // Currently not shooting. Wait for target and nextDelay.
            if (hasTargetInCrosshair && elapsed >= nextDelay) {
                INPUT inputDown = { 0 };
                inputDown.type = INPUT_MOUSE;
                inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &inputDown, sizeof(INPUT));
                
                isMouseDown = true;
                lastActionTime = now;
                
                // Calculate hold time for THIS shot
                currentHoldTime = GetRandomDelay(25, Config::triggerbot_randomize); // 25ms base hold
            }
        }
    }
}
