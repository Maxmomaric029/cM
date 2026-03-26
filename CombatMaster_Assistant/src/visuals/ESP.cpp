#include "ESP.h"
#include <cmath>
#include <algorithm>

namespace ESP {
    void DrawBox(float x, float y, float w, float h, ImU32 color, float thickness) {
        if (!ImGui::GetCurrentContext()) return;
        if (x < -10000 || y < -10000 || x > 20000 || y > 20000 || w <= 0 || h <= 0) return;
        
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        
        // Very thin black outline for high contrast
        drawList->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), ImGui::GetColorU32(ImVec4(0,0,0,0.6f)), 0.0f, 0, 1.0f);
        drawList->AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), ImGui::GetColorU32(ImVec4(0,0,0,0.6f)), 0.0f, 0, 1.0f);
        
        // Main Box
        drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
    }
    
    void DrawCornerBox(float x, float y, float w, float h, ImU32 color, float thickness) {
        if (!ImGui::GetCurrentContext() || x < -10000 || y < -10000 || x > 20000 || y > 20000 || w <= 0 || h <= 0) return;
        
        float line_w = w / 4.0f;
        float line_h = h / 4.0f;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImU32 shadow = ImGui::GetColorU32(ImVec4(0,0,0,0.5f));
        
        auto drawCorner = [&](float x1, float y1, float x2, float y2, ImU32 col, float thick) {
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y1), col, thick);
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x1, y2), col, thick);
        };

        // Shadows
        drawCorner(x - 1, y - 1, x + line_w, y + line_h, shadow, thickness + 1.0f);
        drawCorner(x + w + 1, y - 1, x + w - line_w, y + line_h, shadow, thickness + 1.0f);
        drawCorner(x - 1, y + h + 1, x + line_w, y + h - line_h, shadow, thickness + 1.0f);
        drawCorner(x + w + 1, y + h + 1, x + w - line_w, y + h - line_h, shadow, thickness + 1.0f);

        // Core
        drawCorner(x, y, x + line_w, y + line_h, color, thickness);
        drawCorner(x + w, y, x + w - line_w, y + line_h, color, thickness);
        drawCorner(x, y + h, x + line_w, y + h - line_h, color, thickness);
        drawCorner(x + w, y + h, x + w - line_w, y + h - line_h, color, thickness);
    }
    
    void DrawLine(float x1, float y1, float x2, float y2, ImU32 color, float thickness) {
        if (!ImGui::GetCurrentContext()) return;
        if (std::isnan(x1) || std::isnan(y1) || std::isnan(x2) || std::isnan(y2)) return;
        
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
    }

    void DrawTextCentered(const std::string& text, float x, float y, ImU32 color) {
        if (!ImGui::GetCurrentContext() || text.empty()) return;
        if (std::isnan(x) || std::isnan(y)) return;

        ImVec2 size = ImGui::CalcTextSize(text.c_str());
        
        // Text Shadow
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x - size.x / 2 + 1, y + 1), ImGui::GetColorU32(ImVec4(0,0,0,0.8f)), text.c_str());
        // Main Text (Oswald from fallback/default)
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x - size.x / 2, y), color, text.c_str());
    }

    void DrawHealthBar(float x, float y, float w, float h, float health, float maxHealth) {
        if (!ImGui::GetCurrentContext()) return;
        if (w <= 0 || h <= 0 || maxHealth <= 0 || std::isnan(x) || std::isnan(y)) return;
        
        float healthPct = std::clamp(health / maxHealth, 0.0f, 1.0f);
        ImU32 col = ImGui::GetColorU32(ImVec4(1.0f - healthPct, healthPct, 0.0f, 1.0f));
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        
        // Background (Opaque Black)
        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::GetColorU32(ImVec4(0, 0, 0, 1.0f)));
        
        // Health bar foreground
        float healthHeight = h * healthPct;
        drawList->AddRectFilled(ImVec2(x, y + h - healthHeight), ImVec2(x + w, y + h), col);
        
        // Border
        drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::GetColorU32(ImVec4(0, 0, 0, 1.0f)), 0.0f, 0, 1.0f);
    }
}
