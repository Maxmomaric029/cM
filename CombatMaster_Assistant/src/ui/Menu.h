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

    // Hotkey button helper
    static const char* GetKeyName(int vkCode) {
        static char buf[32];
        if (vkCode == VK_LBUTTON) return "Mouse 1";
        if (vkCode == VK_RBUTTON) return "Mouse 2";
        if (vkCode == VK_MBUTTON) return "Mouse 3";
        if (vkCode == VK_XBUTTON1) return "Mouse 4";
        if (vkCode == VK_XBUTTON2) return "Mouse 5";
        UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
        if (GetKeyNameTextA((LONG)scanCode << 16, buf, 32) == 0) return "?";
        return buf;
    }

    static void HotkeyButton(const char* label, int* currentKey) {
        ImGui::Text("%s", label);
        ImGui::SameLine();
        static int* activeKeybind = nullptr;
        char btnLabel[64];
        if (activeKeybind == currentKey) {
            snprintf(btnLabel, sizeof(btnLabel), "[ Press key ]##%s", label);
            ImGui::Button(btnLabel, ImVec2(120, 0));
            for (int i = 1; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i != VK_ESCAPE) *currentKey = i;
                    while (GetAsyncKeyState(i) & 0x8000) {}
                    activeKeybind = nullptr;
                    return;
                }
            }
        } else {
            snprintf(btnLabel, sizeof(btnLabel), "[ %s ]##%s", GetKeyName(*currentKey), label);
            if (ImGui::Button(btnLabel, ImVec2(120, 0))) activeKeybind = currentKey;
        }
    }

    // Sidebar Tab Helper
    static void SidebarTab(const char* icon, int index) {
        bool active = (currentTab == index);
        ImGuiContext& g = *GImGui;
        ImGuiStyle& style = g.Style;
        
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.06f, 0.07f, 1.00f));
        }

        ImGui::PushFont(g.IO.Fonts->Fonts[0]); // Ensure we use main font for icons if merged
        if (ImGui::Button(icon, ImVec2(50, 50))) {
            currentTab = index;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(2);

        if (ImGui::IsItemHovered() && !active) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }

    inline void Draw(ImTextureID logoTexture) {
        float targetAlpha = bShowMenu ? 1.0f : 0.0f;
        alphaAnim += (targetAlpha - alphaAnim) * 0.1f;
        if (alphaAnim < 0.01f) return;

        ImGui::SetNextWindowSize(ImVec2(600, 450));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alphaAnim);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        
        ImGui::Begin("Nexus Internal V2", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        // Sidebar
        ImGui::BeginChild("##Sidebar", ImVec2(60, 450), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Spacing();
        ImGui::SetCursorPosX(5);
        
        SidebarTab(ICON_MD_VISIBILITY, 0); ImGui::Spacing();
        SidebarTab(ICON_MD_GPS_FIXED, 1); ImGui::Spacing();
        SidebarTab(ICON_MD_PEOPLE, 2); ImGui::Spacing();
        SidebarTab(ICON_MD_EXTENSION, 3); ImGui::Spacing();
        SidebarTab(ICON_MD_PALETTE, 4); ImGui::Spacing();
        SidebarTab(ICON_MD_SAVE, 5); ImGui::Spacing();
        SidebarTab(ICON_MD_SETTINGS, 6);

        ImGui::EndChild();

        ImGui::SameLine();

        // Main Content Area
        ImGui::BeginGroup();
        
        // Header
        ImGui::BeginChild("##Header", ImVec2(540, 60), false);
        ImGui::SetCursorPos(ImVec2(20, 15));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "NEXUS"); ImGui::SameLine(); 
        ImGui::Text("INTERNAL"); ImGui::SameLine();
        ImGui::SetCursorPosX(420);
        ImGui::TextDisabled("V2.0.0-BS");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::EndChild();

        // Content
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::BeginChild("##Content", ImVec2(520, 370), false);
        ImGui::Spacing();

        if (currentTab == 0) { // ESP
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "VISUALS");
            ImGui::Checkbox("Master Enable", &Config::esp_enabled);
            ImGui::Columns(2, nullptr, false);
            ImGui::Checkbox("Player Boxes", &Config::esp_boxes);
            ImGui::Checkbox("Player Names", &Config::esp_names);
            ImGui::Checkbox("Player Distance", &Config::esp_distance);
            ImGui::Checkbox("Health Bars", &Config::esp_health);
            ImGui::NextColumn();
            ImGui::Checkbox("Snaplines", &Config::esp_lines);
            if (Config::esp_lines) ImGui::Combo("##SnapOrigin", &Config::esp_snapline_origin, "Top\0Center\0Bottom\0");
            ImGui::Checkbox("Tracers", &Config::esp_tracers);
            ImGui::Checkbox("Invincible Check", &Config::esp_spawn_protection_indicator);
            ImGui::Checkbox("Visibility Only", &Config::esp_visibility_check);
            ImGui::Columns(1);
            ImGui::SliderFloat("Max Distance", &Config::esp_max_distance, 50.f, 1000.f, "%.0f m");
            ImGui::SliderFloat("Box Thickness", &Config::esp_box_thickness, 1.0f, 5.0f);
        }
        else if (currentTab == 1) { // AIM
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "AIM ASSIST");
            ImGui::Checkbox("Aimbot Enable", &Config::aimbot_enabled);
            ImGui::Checkbox("Show FOV", &Config::aimbot_draw_fov);
            ImGui::SliderFloat("Field of View", &Config::aimbot_fov, 10.f, 800.f, "%.0f px");
            ImGui::Separator();
            ImGui::Columns(2, nullptr, false);
            ImGui::SliderFloat("Smooth X", &Config::aimbot_smooth_x, 1.f, 30.f);
            ImGui::SliderFloat("Smooth Y", &Config::aimbot_smooth_y, 1.f, 30.f);
            ImGui::NextColumn();
            ImGui::Combo("Target Bone", &Config::aimbot_bone, "Head\0Neck\0Chest\0Pelvis\0");
            ImGui::Combo("Path Style", &Config::aimbot_path_style, "Linear\0Human\0");
            ImGui::Columns(1);
            ImGui::Checkbox("Skip Invisible", &Config::aimbot_vis_check);
            ImGui::Checkbox("Skip Protected", &Config::aimbot_skip_spawn_protection);
            HotkeyButton("Aimbot Key", &Config::aimbot_key);
        }
        else if (currentTab == 2) { // TRIGGER
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "TRIGGERBOT");
            ImGui::Checkbox("Triggerbot Enable", &Config::triggerbot_enabled);
            ImGui::SliderInt("Shot Delay", &Config::triggerbot_delay, 0, 500, "%d ms");
            ImGui::Checkbox("Humanized Timing", &Config::triggerbot_randomize);
        }
        else if (currentTab == 3) { // EXTRA
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WEAPON & MOVEMENT");
            ImGui::Checkbox("No Recoil", &Config::no_recoil);
            ImGui::Checkbox("No Camera Shake", &Config::no_camera_shake);
            ImGui::Separator();
            ImGui::TextDisabled("Host-dependent Features:");
            ImGui::Checkbox("Infinite Ammo", &Config::infinite_ammo);
            ImGui::Checkbox("Rapid Fire", &Config::rapid_fire);
            ImGui::Checkbox("Infinite Lethals", &Config::infinite_lethals);
            ImGui::Separator();
            ImGui::Checkbox("Movement Speed", &Config::movement_speed);
            if (Config::movement_speed) {
                ImGui::SliderFloat("Run Speed", &Config::movement_run_mult, 1.0f, 5.0f);
                ImGui::SliderFloat("Sprint Speed", &Config::movement_sprint_mult, 1.0f, 5.0f);
            }
        }
        else if (currentTab == 4) { // COLORS
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "APPEARANCE");
            ImGui::ColorEdit4("Enemy Visible", Config::esp_color_enemy, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Team Color", Config::esp_color_team, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("FOV Circle", Config::esp_color_fov, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Target Orb", Config::esp_color_target_orb, ImGuiColorEditFlags_NoInputs);
        }
        else if (currentTab == 5) { // CONFIG
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "SETTINGS PERSISTENCE");
            if (ImGui::Button("Save nexus_config.json", ImVec2(-1, 40))) Config::Save("nexus_config.json");
            ImGui::Spacing();
            if (ImGui::Button("Load nexus_config.json", ImVec2(-1, 40))) Config::Load("nexus_config.json");
        }
        else if (currentTab == 6) { // SETTINGS
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "GLOBAL SETTINGS");
            HotkeyButton("Menu Toggle", &Config::menu_toggle_key);
            HotkeyButton("Emergency Unload", &Config::unload_key);
            ImGui::Separator();
            if (ImGui::Button("UNLOAD DLL NOW", ImVec2(-1, 40))) {
                // Trigger unload logic in renderer
                g_UnloadRequested = true;
            }
        }

        ImGui::EndChild();
        ImGui::EndGroup();

        ImGui::End();
        ImGui::PopStyleVar(2);
    }
}
