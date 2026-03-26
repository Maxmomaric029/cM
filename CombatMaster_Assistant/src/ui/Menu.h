#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <string>
#include "Config.h"
#include "Icons.h"
#include "../Global.h"

namespace Menu {
    inline bool bShowMenu = true;
    inline float alphaAnim = 1.0f;
    inline int currentTab = 0;

    static const char* GetKeyName(int vkCode) {
        static char buf[32];
        if (vkCode == VK_LBUTTON) return "Mouse 1";
        if (vkCode == VK_RBUTTON) return "Mouse 2";
        if (vkCode == VK_MBUTTON) return "Mouse 3";
        UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
        if (GetKeyNameTextA((LONG)scanCode << 16, buf, 32) == 0) return "?";
        return buf;
    }

    static void HotkeyButton(const char* label, int* currentKey) {
        ImGui::Text("%s", label); ImGui::SameLine();
        static int* activeKeybind = nullptr;
        char btnLabel[64];
        if (activeKeybind == currentKey) {
            snprintf(btnLabel, sizeof(btnLabel), "[ Press key ]##%s", label);
            if (ImGui::Button(btnLabel, ImVec2(120, 0))) activeKeybind = nullptr;
            for (int i = 1; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i != VK_ESCAPE) *currentKey = i;
                    activeKeybind = nullptr; return;
                }
            }
        } else {
            snprintf(btnLabel, sizeof(btnLabel), "[ %s ]##%s", GetKeyName(*currentKey), label);
            if (ImGui::Button(btnLabel, ImVec2(120, 0))) activeKeybind = currentKey;
        }
    }

    static void SidebarTab(const char* icon, int index) {
        bool active = (currentTab == index);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.06f, 0.07f, 1.00f));
        }
        if (ImGui::Button(icon, ImVec2(50, 50))) currentTab = index;
        ImGui::PopStyleColor(2);
    }

    inline void Draw(ImTextureID logoTexture) {
        float targetAlpha = bShowMenu ? 1.0f : 0.0f;
        alphaAnim += (targetAlpha - alphaAnim) * 0.1f;
        if (alphaAnim < 0.01f) return;

        ImGui::SetNextWindowSize(ImVec2(600, 450));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alphaAnim);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Nexus External V1", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::BeginChild("##Sidebar", ImVec2(60, 450), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Spacing(); ImGui::SetCursorPosX(5);
        SidebarTab(ICON_MD_VISIBILITY, 0); ImGui::Spacing();
        SidebarTab(ICON_MD_GPS_FIXED, 1); ImGui::Spacing();
        SidebarTab(ICON_MD_PEOPLE, 2); ImGui::Spacing();
        SidebarTab(ICON_MD_EXTENSION, 3); ImGui::Spacing();
        SidebarTab(ICON_MD_PALETTE, 4); ImGui::Spacing();
        SidebarTab(ICON_MD_SAVE, 5); ImGui::Spacing();
        SidebarTab(ICON_MD_SETTINGS, 6);
        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("##Header", ImVec2(540, 60), false);
        
        // --- High-End Glow Logo ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        p.x += 20; p.y += 15;
        DrawUtils::DrawGlowText(dl, ImGui::GetFont(), 24.0f, p, IM_COL32(255, 0, 0, 255), "NEXUS", 1.2f);
        p.x += 80;
        DrawUtils::DrawGlowText(dl, ImGui::GetFont(), 24.0f, p, IM_COL32(255, 255, 255, 255), "EXTERNAL", 0.5f);
        
        ImGui::SetCursorPosX(420); ImGui::SetCursorPosY(20);
        ImGui::TextDisabled("V1.0.0-EXT");
        ImGui::SetCursorPosY(55); ImGui::Separator();
        ImGui::EndChild();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::BeginChild("##Content", ImVec2(520, 370), false);
        ImGui::Spacing();

        if (currentTab == 0) { // ESP
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "VISUALS");
            ImGui::Checkbox("Master Enable", &Config::esp_enabled);
            ImGui::Columns(2, nullptr, false);
            ImGui::Checkbox("Boxes", &Config::esp_boxes);
            ImGui::Checkbox("Names", &Config::esp_names);
            ImGui::Checkbox("Distance", &Config::esp_distance);
            ImGui::Checkbox("Health", &Config::esp_health);
            ImGui::NextColumn();
            ImGui::Checkbox("Lines", &Config::esp_lines);
            ImGui::Checkbox("Tracers", &Config::esp_tracers);
            ImGui::Checkbox("Invincible", &Config::esp_spawn_protection_indicator);
            ImGui::Checkbox("Vis Only", &Config::esp_visibility_check);
            ImGui::Columns(1);
            ImGui::SliderFloat("FOV", &Config::aimbot_fov, 10.f, 800.f, "%.0f px");
        }
        else if (currentTab == 1) { // AIM
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "AIM ASSIST");
            ImGui::Checkbox("Aimbot", &Config::aimbot_enabled);
            ImGui::SliderFloat("Smooth X", &Config::aimbot_smooth_x, 1.f, 30.f);
            ImGui::SliderFloat("Smooth Y", &Config::aimbot_smooth_y, 1.f, 30.f);
            HotkeyButton("Aimbot Key", &Config::aimbot_key);
        }
        else if (currentTab == 3) { // EXTRA
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MISC");
            ImGui::TextDisabled("(Weapon/Movement hacks require internal mode)");
        }
        else if (currentTab == 5) { // CONFIG
            if (ImGui::Button("SAVE SETTINGS", ImVec2(-1, 40))) Config::Save("nexus_config.json");
            if (ImGui::Button("LOAD SETTINGS", ImVec2(-1, 40))) Config::Load("nexus_config.json");
        }
        else if (currentTab == 6) { // SETTINGS
            HotkeyButton("Menu Toggle", &Config::menu_toggle_key);
            if (ImGui::Button("EXIT NEXUS", ImVec2(-1, 40))) g_ExitRequested = true;
        }

        ImGui::EndChild(); ImGui::EndGroup();
        ImGui::End(); ImGui::PopStyleVar(2);
    }
}
