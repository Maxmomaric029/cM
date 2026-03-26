#pragma once
#include <imgui.h>
#include <cmath>
#include "Config.h"

namespace Menu {
    inline bool bShowMenu = true;
    inline float alphaAnim = 1.0f;

    // Hotkey button helper (from reference)
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
            ImGui::Button(btnLabel);
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
            if (ImGui::Button(btnLabel)) activeKeybind = currentKey;
        }
    }

    inline void Draw(ImTextureID logoTexture) {
        float targetAlpha = bShowMenu ? 1.0f : 0.0f;
        alphaAnim += (targetAlpha - alphaAnim) * 0.1f;
        if (alphaAnim < 0.01f) return;

        ImGui::SetNextWindowSize(ImVec2(520, 480), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.96f * alphaAnim);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alphaAnim);

        ImGui::Begin("Nexus Internal", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        // Title
        float windowWidth = ImGui::GetWindowSize().x;
        const char* title = "NEXUS INTERNAL";
        float textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.90f, 0.15f, 0.20f, 1.00f), "%s", title);
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("Features", ImGuiTabBarFlags_None)) {

            // ==================== ESP TAB ====================
            if (ImGui::BeginTabItem("ESP")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable ESP", &Config::esp_enabled);
                ImGui::Separator();

                ImGui::Checkbox("Boxes", &Config::esp_boxes);
                if (Config::esp_boxes)
                    ImGui::SliderFloat("Box thickness", &Config::esp_box_thickness, 0.5f, 6.f, "%.1f");
                ImGui::Checkbox("Tracers", &Config::esp_tracers);
                if (Config::esp_tracers) {
                    const char* tracerOrigins[] = { "Top", "Center", "Bottom" };
                    ImGui::Combo("Tracer origin", &Config::esp_tracer_origin, tracerOrigins, 3);
                }
                ImGui::Checkbox("Show teammates", &Config::esp_show_team);
                ImGui::Checkbox("Names", &Config::esp_names);
                ImGui::Checkbox("Distance", &Config::esp_distance);
                ImGui::Checkbox("Health bar", &Config::esp_health);
                ImGui::Checkbox("Snaplines", &Config::esp_lines);
                if (Config::esp_lines) {
                    const char* snapOrigins[] = { "Top", "Center", "Bottom" };
                    ImGui::Combo("Snapline origin", &Config::esp_snapline_origin, snapOrigins, 3);
                }
                ImGui::Checkbox("Spawn protection indicator", &Config::esp_spawn_protection_indicator);
                ImGui::Checkbox("Visibility check", &Config::esp_visibility_check);
                ImGui::SliderFloat("Max distance", &Config::esp_max_distance, 10.f, 1000.f, "%.0fm");

                ImGui::EndTabItem();
            }

            // ==================== AIM TAB ====================
            if (ImGui::BeginTabItem("Aim")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable Aimbot", &Config::aimbot_enabled);
                ImGui::Checkbox("Draw FOV", &Config::aimbot_draw_fov);
                ImGui::SliderFloat("FOV radius", &Config::aimbot_fov, 1.f, 850.f, "%.0f");
                ImGui::Checkbox("Visibility check", &Config::aimbot_vis_check);
                ImGui::Separator();

                ImGui::SliderFloat("Smoothing X", &Config::aimbot_smooth_x, 1.f, 20.f, "%.1f");
                ImGui::SliderFloat("Smoothing Y", &Config::aimbot_smooth_y, 1.f, 20.f, "%.1f");

                const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
                ImGui::Combo("Target bone", &Config::aimbot_bone, bones, 4);

                const char* targeting[] = { "Closest to crosshair", "Closest distance", "Both" };
                ImGui::Combo("Target method", &Config::aimbot_targeting, targeting, 3);

                const char* pathStyles[] = { "Linear", "Human" };
                ImGui::Combo("Path style", &Config::aimbot_path_style, pathStyles, 2);

                ImGui::Separator();
                ImGui::Checkbox("Skip spawn protection", &Config::aimbot_skip_spawn_protection);
                ImGui::Checkbox("Team check", &Config::aimbot_team_check);
                ImGui::Checkbox("Follow crouched (chest)", &Config::aimbot_follow_crouched);
                ImGui::Checkbox("Target tracer", &Config::aimbot_target_tracer);
                ImGui::Checkbox("Target orb", &Config::aimbot_target_orb);
                HotkeyButton("Aimbot key", &Config::aimbot_key);

                ImGui::EndTabItem();
            }

            // ==================== TRIGGERBOT TAB ====================
            if (ImGui::BeginTabItem("Trigger")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable Triggerbot", &Config::triggerbot_enabled);
                ImGui::SliderInt("Delay (ms)", &Config::triggerbot_delay, 0, 1000);
                ImGui::Checkbox("Randomize / Humanize", &Config::triggerbot_randomize);
                ImGui::EndTabItem();
            }

            // ==================== EXTRA TAB ====================
            if (ImGui::BeginTabItem("Extra")) {
                ImGui::Spacing();
                ImGui::Checkbox("No recoil", &Config::no_recoil);
                ImGui::Checkbox("No camera shake", &Config::no_camera_shake);
                ImGui::Separator();

                ImGui::Checkbox("Infinite ammo", &Config::infinite_ammo);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");

                ImGui::Checkbox("Rapid fire", &Config::rapid_fire);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");

                ImGui::Checkbox("Infinite lethals", &Config::infinite_lethals);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");

                ImGui::Separator();
                ImGui::Checkbox("Movement speed", &Config::movement_speed);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "[HOST]");
                if (Config::movement_speed) {
                    ImGui::SliderFloat("Run mult", &Config::movement_run_mult, 0.5f, 5.f, "%.2f");
                    ImGui::SliderFloat("Sprint mult", &Config::movement_sprint_mult, 0.5f, 5.f, "%.2f");
                }

                ImGui::EndTabItem();
            }

            // ==================== COLORS TAB ====================
            if (ImGui::BeginTabItem("Colors")) {
                ImGui::Spacing();
                ImGui::ColorEdit4("Enemy box", Config::esp_color_enemy, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Team box", Config::esp_color_team, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("FOV circle", Config::esp_color_fov, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Target orb", Config::esp_color_target_orb, ImGuiColorEditFlags_NoInputs);
                ImGui::EndTabItem();
            }

            // ==================== CONFIG TAB ====================
            if (ImGui::BeginTabItem("Config")) {
                ImGui::Spacing();
                ImGui::Text("Save or load settings.");
                if (ImGui::Button("Save config", ImVec2(180, 0))) Config::Save("nexus_config.json");
                ImGui::SameLine();
                if (ImGui::Button("Load config", ImVec2(180, 0))) Config::Load("nexus_config.json");
                ImGui::Spacing();
                ImGui::TextDisabled("File: nexus_config.json");
                ImGui::EndTabItem();
            }

            // ==================== SETTINGS TAB ====================
            if (ImGui::BeginTabItem("Settings")) {
                ImGui::Spacing();
                HotkeyButton("Menu toggle", &Config::menu_toggle_key);
                HotkeyButton("Unload key", &Config::unload_key);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}
