#pragma once
#include <string>
#include <imgui.h>

namespace ESP {
    void DrawBox(float x, float y, float w, float h, ImU32 color, float thickness);
    void DrawCornerBox(float x, float y, float w, float h, ImU32 color, float thickness);
    void DrawLine(float x1, float y1, float x2, float y2, ImU32 color, float thickness);
    void DrawTextCentered(const std::string& text, float x, float y, ImU32 color);
    void DrawHealthBar(float x, float y, float w, float h, float health, float maxHealth);
}
