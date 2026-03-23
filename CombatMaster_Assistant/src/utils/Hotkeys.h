#pragma once
#include <windows.h>
#include <unordered_map>

namespace Hotkeys {
    inline bool IsPressed(int vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
    }

    inline bool WasKeyPressed(int vk) {
        static std::unordered_map<int, bool> state;
        bool curr = IsPressed(vk);
        if (curr && !state[vk]) {
            state[vk] = true;
            return true;
        } else if (!curr) {
            state[vk] = false;
        }
        return false;
    }
}
