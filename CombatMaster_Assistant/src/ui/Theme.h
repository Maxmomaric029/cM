#pragma once
#include <imgui.h>

namespace Theme {
    inline void Apply() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Dark gray background
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);
        
        // Red accents
        colors[ImGuiCol_Border] = ImVec4(0.55f, 0.00f, 0.00f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.10f, 0.10f, 1.00f);
        
        colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.20f, 0.20f, 1.00f);
        
        colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.20f, 0.20f, 1.00f);
        
        colors[ImGuiCol_Button] = ImVec4(0.30f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.15f, 0.15f, 1.00f);
        
        colors[ImGuiCol_Header] = ImVec4(0.35f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.20f, 0.20f, 1.00f);
        
        colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.50f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.08f, 0.08f, 1.00f);

        // Styling
        style.Alpha = 1.0f;
        style.WindowRounding = 8.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    }
}
