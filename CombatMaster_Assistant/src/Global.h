#pragma once
#include <windows.h>
#include <imgui.h>

// Global flags and handles for the cheat
inline bool g_UnloadRequested = false;
inline HMODULE g_hInjectModule = nullptr;

namespace DrawUtils {
    inline void DrawGlowText(ImDrawList* drawList, ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float glowStrength = 1.0f) {
        if (!text || !drawList) return;
        ImU32 glowColor = IM_COL32(255, 0, 0, (int)(120 * glowStrength));
        
        // Draw glow (multi-pass shadow)
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (x == 0 && y == 0) continue;
                drawList->AddText(font, fontSize, ImVec2(pos.x + x, pos.y + y), glowColor, text);
            }
        }
        
        // Main text
        drawList->AddText(font, fontSize, pos, color, text);
    }
}
