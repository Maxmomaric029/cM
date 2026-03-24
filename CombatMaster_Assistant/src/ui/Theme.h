#pragma once
#include <imgui.h>

namespace Theme {
    inline void Apply() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Nexus Cyberpunk Premium Theme
        colors[ImGuiCol_Text]                   = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.96f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.98f, 0.93f, 0.30f); // Cyan subtle border
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.52f, 0.50f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.19f, 0.52f, 0.50f, 0.67f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.11f, 1.00f, 0.94f, 1.00f); // Bright Cyber Cyan
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.11f, 1.00f, 0.94f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.08f, 0.80f, 0.75f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.12f, 0.98f, 0.93f, 0.31f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.12f, 0.98f, 0.93f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.12f, 0.98f, 0.93f, 1.00f);
        colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.12f, 0.98f, 0.93f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.14f, 0.31f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);

        // Advanced smooth styling
        style.Alpha = 1.0f;
        style.WindowRounding = 10.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ChildRounding = 6.0f;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.ItemSpacing = ImVec2(10, 8);
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    }
}
