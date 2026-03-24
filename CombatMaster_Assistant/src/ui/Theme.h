#pragma once
#include <imgui.h>

namespace Theme {
    inline void Apply() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Custom High-End Dark Red / Neon Theme
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.96f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.12f, 0.12f, 0.12f, 0.96f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.98f);
        colors[ImGuiCol_Border]                 = ImVec4(0.80f, 0.10f, 0.15f, 0.40f); 
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.80f, 0.10f, 0.15f, 0.10f); 
        colors[ImGuiCol_FrameBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.80f, 0.10f, 0.15f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.90f, 0.15f, 0.20f, 0.60f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.80f, 0.10f, 0.15f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.90f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.15f, 0.20f, 1.00f); 
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.80f, 0.10f, 0.15f, 0.80f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.90f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.80f, 0.10f, 0.15f, 0.80f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.90f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.80f, 0.10f, 0.15f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.90f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.80f, 0.10f, 0.15f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.90f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.80f, 0.10f, 0.15f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.90f, 0.15f, 0.20f, 0.95f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.80f, 0.10f, 0.15f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

        // Modern Shape Roundings & Spacing
        style.WindowRounding    = 8.0f;
        style.ChildRounding     = 6.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 6.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;
        
        style.WindowPadding     = ImVec2(16, 16);
        style.FramePadding      = ImVec2(8, 4);
        style.CellPadding       = ImVec2(6, 4);
        style.ItemSpacing       = ImVec2(8, 6);
        style.ItemInnerSpacing  = ImVec2(6, 6);
        style.ScrollbarSize     = 10.0f;
        style.GrabMinSize       = 12.0f;

        style.WindowBorderSize  = 1.0f;
        style.ChildBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;
        style.FrameBorderSize   = 0.0f;
        
        // Aesthetic rendering settings
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
    }
}
