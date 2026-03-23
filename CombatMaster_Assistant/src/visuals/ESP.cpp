#include "ESP.h"
#include <cmath>

namespace ESP {
    void DrawBox(float x, float y, float w, float h, ImU32 color, float thickness) {
        if (!ImGui::GetCurrentContext()) return;
        
        // Validation for reasonable coordinates to prevent ImGui assertion failures
        if (x < -10000 || y < -10000 || x > 20000 || y > 20000 || w <= 0 || h <= 0) return;
        
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
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
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x - size.x / 2, y), color, text.c_str());
    }

    void DrawHealthBar(float x, float y, float w, float h, float health, float maxHealth) {
        if (!ImGui::GetCurrentContext()) return;
        
        // Validation
        if (w <= 0 || h <= 0 || maxHealth <= 0 || std::isnan(x) || std::isnan(y)) return;
        
        float healthPct = health / maxHealth;
        // Clamp percentage
        if (healthPct > 1.0f) healthPct = 1.0f;
        if (healthPct < 0.0f) healthPct = 0.0f;

        // Color interpolation based on health level
        float r = (healthPct > 0.5f) ? (1.0f - 2.0f * (healthPct - 0.5f)) : 1.0f;
        float g = (healthPct > 0.5f) ? 1.0f : (2.0f * healthPct);
        
        ImU32 col = ImGui::GetColorU32(ImVec4(r, g, 0.0f, 1.0f));
        
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        
        // Background shadow
        drawList->AddRectFilled(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 0.8f)));
        
        // Health bar foreground
        float healthHeight = h * healthPct;
        drawList->AddRectFilled(ImVec2(x, y + h - healthHeight), ImVec2(x + w, y + h), col);
    }
}
