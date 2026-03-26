#pragma once
#include <windows.h>
#include <imgui.h>

// Global flags for the external cheat
inline bool g_ExitRequested = false;

namespace DrawUtils {
    inline void DrawGlowText(ImDrawList* drawList, ImFont* font, float fontSize,
                             const ImVec2& pos, ImU32 color, const char* text,
                             float glowStrength = 1.0f) {
        if (!text || !drawList) return;
        ImU32 glowColor = IM_COL32(255, 0, 0, (int)(120 * glowStrength));
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (x == 0 && y == 0) continue;
                drawList->AddText(font, fontSize, ImVec2(pos.x + x, pos.y + y), glowColor, text);
            }
        }
        drawList->AddText(font, fontSize, pos, color, text);
    }
}
