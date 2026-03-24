#pragma once
#include <imgui.h>
#include "Config.h"

namespace Menu {
    inline bool bShowMenu = true;

    inline void Draw() {
        if (!bShowMenu) return;

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
        ImGui::Begin("Nexus Overlay", &bShowMenu, ImGuiWindowFlags_NoCollapse);

        if (ImGui::BeginTabBar("Features")) {
            if (ImGui::BeginTabItem("Aimbot")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable Aimbot", &Config::aimbot_enabled);
                ImGui::Separator();
                ImGui::SliderFloat("FOV", &Config::aimbot_fov, 1.0f, 180.0f, "%.1f");
                ImGui::SliderFloat("Smoothing", &Config::aimbot_smooth, 1.0f, 20.0f, "%.1f");
                ImGui::Checkbox("Enable Prediction", &Config::aimbot_prediction);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable ESP", &Config::esp_enabled);
                ImGui::Separator();
                
                ImGui::Columns(2, nullptr, false);
                ImGui::Checkbox("Boxes", &Config::esp_boxes);
                ImGui::Checkbox("Names", &Config::esp_names);
                ImGui::Checkbox("Health", &Config::esp_health);
                
                ImGui::NextColumn();
                ImGui::Checkbox("Snap Lines", &Config::esp_lines);
                ImGui::Checkbox("Distance", &Config::esp_distance);
                ImGui::SliderFloat("Max Distance", &Config::esp_max_distance, 10.0f, 1000.0f, "%.0f");
                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Triggerbot")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable Triggerbot", &Config::triggerbot_enabled);
                ImGui::Separator();
                ImGui::SliderInt("Delay (ms)", &Config::triggerbot_delay, 0, 1000);
                ImGui::Checkbox("Humanize Delay", &Config::triggerbot_randomize);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Colors")) {
                ImGui::Spacing();
                ImGui::ColorEdit4("Enemy Color", Config::esp_color_enemy);
                ImGui::ColorEdit4("Team Color", Config::esp_color_team);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings")) {
                ImGui::Spacing();
                if (ImGui::Button("Save Configuration", ImVec2(-1, 30))) {
                    Config::Save("config.json");
                }
                ImGui::Spacing();
                if (ImGui::Button("Load Configuration", ImVec2(-1, 30))) {
                    Config::Load("config.json");
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
