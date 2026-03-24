#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>

using json = nlohmann::json;

namespace Config {
    inline bool aimbot_enabled = false;
    inline float aimbot_fov = 15.0f;
    inline float aimbot_smooth = 5.0f;
    inline bool aimbot_prediction = false;
    inline int aimbot_targeting = 0; // 0 = Crosshair, 1 = Distance, 2 = Hybrid
    inline int aimbot_bone = 0; // 0 = Head, 1 = Neck, 2 = Chest, 3 = Pelvis
    inline bool aimbot_vis_check = false;
    inline bool aimbot_follow_crouched = true;

    inline bool esp_enabled = true;
    inline bool esp_boxes = true;
    inline bool esp_names = true;
    inline bool esp_health = true;
    inline bool esp_lines = false;
    inline bool esp_distance = true;
    inline float esp_max_distance = 200.0f;
    inline float esp_color_enemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    inline float esp_color_team[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

    inline bool triggerbot_enabled = false;
    inline int triggerbot_delay = 100;
    inline bool triggerbot_randomize = true;

    inline void Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return;

        json j;
        file >> j;

        if (j.contains("aimbot")) {
            auto& a = j["aimbot"];
            if (a.contains("enabled")) aimbot_enabled = a["enabled"];
            if (a.contains("fov")) aimbot_fov = a["fov"];
            if (a.contains("smooth")) aimbot_smooth = a["smooth"];
            if (a.contains("prediction")) aimbot_prediction = a["prediction"];
        }

        if (j.contains("esp")) {
            auto& e = j["esp"];
            if (e.contains("enabled")) esp_enabled = e["enabled"];
            if (e.contains("boxes")) esp_boxes = e["boxes"];
            if (e.contains("names")) esp_names = e["names"];
            if (e.contains("health")) esp_health = e["health"];
            if (e.contains("lines")) esp_lines = e["lines"];
            if (e.contains("distance")) esp_distance = e["distance"];
            if (e.contains("max_distance")) esp_max_distance = e["max_distance"];
            
            if (e.contains("colors")) {
                auto& c = e["colors"];
                if (c.contains("enemy")) {
                    auto arr = c["enemy"].get<std::vector<float>>();
                    if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_enemy);
                }
                if (c.contains("team")) {
                    auto arr = c["team"].get<std::vector<float>>();
                    if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_team);
                }
            }
        }

        if (j.contains("triggerbot")) {
            auto& t = j["triggerbot"];
            if (t.contains("enabled")) triggerbot_enabled = t["enabled"];
            if (t.contains("delay")) triggerbot_delay = t["delay"];
            if (t.contains("randomize")) triggerbot_randomize = t["randomize"];
        }
    }

    inline void Save(const std::string& path) {
        json j;
        j["aimbot"]["enabled"] = aimbot_enabled;
        j["aimbot"]["fov"] = aimbot_fov;
        j["aimbot"]["smooth"] = aimbot_smooth;
        j["aimbot"]["prediction"] = aimbot_prediction;

        j["esp"]["enabled"] = esp_enabled;
        j["esp"]["boxes"] = esp_boxes;
        j["esp"]["names"] = esp_names;
        j["esp"]["health"] = esp_health;
        j["esp"]["lines"] = esp_lines;
        j["esp"]["distance"] = esp_distance;
        j["esp"]["max_distance"] = esp_max_distance;
        j["esp"]["colors"]["enemy"] = { esp_color_enemy[0], esp_color_enemy[1], esp_color_enemy[2], esp_color_enemy[3] };
        j["esp"]["colors"]["team"] = { esp_color_team[0], esp_color_team[1], esp_color_team[2], esp_color_team[3] };

        j["triggerbot"]["enabled"] = triggerbot_enabled;
        j["triggerbot"]["delay"] = triggerbot_delay;
        j["triggerbot"]["randomize"] = triggerbot_randomize;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
        }
    }
}
