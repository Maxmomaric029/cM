#pragma once
#include <imgui.h>
#include <cmath>
#include "Config.h"

namespace Menu {
    inline bool bShowMenu = true;
    inline float alphaAnim = 1.0f; // For fade-in/fade-out animations

    inline void Draw(ImTextureID logoTexture) {
        // Animation Logic: Smooth Fade in and Fade out
        float targetAlpha = bShowMenu ? 1.0f : 0.0f;
        alphaAnim += (targetAlpha - alphaAnim) * 0.1f; // Lerp
        
        if (alphaAnim < 0.01f) return; // Completely hidden

        ImGui::SetNextWindowSize(ImVec2(650, 500), ImGuiCond_FirstUseEver);
        
        // Apply alpha to window
        ImGui::SetNextWindowBgAlpha(0.96f * alphaAnim);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alphaAnim);
        
        ImGui::Begin("Nexus Overlay", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        // 1. Logo Rendering with pulsing animation
        if (logoTexture) {
            float pulse = (std::sin(ImGui::GetTime() * 3.0f) + 1.0f) * 0.5f; // 0 to 1
            float imgSize = 64.0f + (pulse * 4.0f); // Slight beat
            
            // Center the image
            float availWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availWidth - imgSize) * 0.5f);
            
            ImGui::Image(logoTexture, ImVec2(imgSize, imgSize));
            ImGui::Spacing();
        }

        // Title text centered if no logo or just as complementary
        float windowWidth = ImGui::GetWindowSize().x;
        const char* title = "NEXUS OVERLAY";
        float textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.90f, 0.15f, 0.20f, 1.00f), "%s", title);
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("Features", ImGuiTabBarFlags_NoTooltip | ImGuiTabBarFlags_FittingPolicyScroll)) {
            if (ImGui::BeginTabItem("Asistente de Puntería")) {
                ImGui::Spacing();
                ImGui::Text("Ajustes de Aimbot Avanzado");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox("Habilitar Aimbot", &Config::aimbot_enabled);
                ImGui::Spacing();
                ImGui::SliderFloat("Campo de Visión (FOV)", &Config::aimbot_fov, 1.0f, 180.0f, "%.1f px");
                ImGui::SliderFloat("Suavizado Bezier", &Config::aimbot_smooth, 1.0f, 20.0f, "%.1f");
                ImGui::Checkbox("Predicción Balística", &Config::aimbot_prediction);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuales")) {
                ImGui::Spacing();
                ImGui::Text("Ajustes de Percepción Adicional");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox("Habilitar ESP", &Config::esp_enabled);
                ImGui::Spacing();
                
                ImGui::Columns(2, nullptr, false);
                ImGui::Checkbox("Cajas (Boxes)", &Config::esp_boxes);
                ImGui::Checkbox("Nombres", &Config::esp_names);
                ImGui::Checkbox("Barra de Vida", &Config::esp_health);
                
                ImGui::NextColumn();
                ImGui::Checkbox("Líneas (Snaplines)", &Config::esp_lines);
                ImGui::Checkbox("Distancia", &Config::esp_distance);
                ImGui::SliderFloat("Límite (Max Dist)", &Config::esp_max_distance, 10.0f, 1000.0f, "%.0fm");
                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Disparo Automático")) {
                ImGui::Spacing();
                ImGui::Text("Ajustes de Triggerbot Humanizado");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox("Habilitar Triggerbot", &Config::triggerbot_enabled);
                ImGui::Spacing();
                ImGui::SliderInt("Retraso Base (ms)", &Config::triggerbot_delay, 0, 1000);
                ImGui::Checkbox("Aleatorizar / Humanizar Latencia", &Config::triggerbot_randomize);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Configuración")) {
                ImGui::Spacing();
                ImGui::Text("Opciones del Sistema");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text("Colores Visuales:");
                ImGui::ColorEdit4("Color Enemigo", Config::esp_color_enemy, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Color Aliado", Config::esp_color_team, ImGuiColorEditFlags_NoInputs);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                // Animated Buttons using PushStyleColor for Hover interpolation natively 
                if (ImGui::Button(u8"💾 Guardar Configuración", ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
                    Config::Save("config.json");
                }
                ImGui::Spacing();
                if (ImGui::Button(u8"📂 Cargar Configuración", ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
                    Config::Load("config.json");
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}
