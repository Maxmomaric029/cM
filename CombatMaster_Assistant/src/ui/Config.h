#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>

using json = nlohmann::json;

namespace Config {
    // --- Aimbot ---
    inline bool aimbot_enabled = false;
    inline float aimbot_fov = 383.0f;
    inline float aimbot_smooth_x = 3.0f;
    inline float aimbot_smooth_y = 3.0f;
    inline float aimbot_smooth = 5.0f; // Legacy single-axis (used if X/Y identical)
    inline bool aimbot_prediction = false;
    inline int aimbot_targeting = 0;     // 0 = Crosshair, 1 = Distance, 2 = Hybrid
    inline int aimbot_bone = 0;          // 0 = Head, 1 = Neck, 2 = Chest, 3 = Pelvis
    inline bool aimbot_vis_check = true;
    inline bool aimbot_follow_crouched = true;
    inline int aimbot_key = VK_RBUTTON;
    inline int aimbot_path_style = 0;    // 0 = Linear, 1 = Human
    inline bool aimbot_skip_spawn_protection = true;
    inline bool aimbot_team_check = true;
    inline bool aimbot_draw_fov = true;
    inline bool aimbot_target_tracer = false;
    inline bool aimbot_target_orb = false;

    // --- ESP ---
    inline bool esp_enabled = true;
    inline bool esp_boxes = true;
    inline bool esp_names = true;
    inline bool esp_health = true;
    inline bool esp_lines = false;       // Snaplines
    inline bool esp_tracers = false;
    inline bool esp_distance = true;
    inline bool esp_visibility_check = true;
    inline bool esp_spawn_protection_indicator = true;
    inline bool esp_show_team = false;
    inline float esp_max_distance = 200.0f;
    inline float esp_box_thickness = 2.25f;
    inline int esp_snapline_origin = 1;  // 0=Top, 1=Center, 2=Bottom
    inline int esp_tracer_origin = 2;    // 0=Top, 1=Center, 2=Bottom
    inline float esp_color_enemy[4] = { 0.7f, 0.2f, 1.0f, 1.0f };
    inline float esp_color_team[4] = { 0.4f, 1.0f, 1.0f, 1.0f };
    inline float esp_color_fov[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    inline float esp_color_target_orb[4] = { 0.7f, 0.2f, 1.0f, 0.8f };

    // --- Triggerbot ---
    inline bool triggerbot_enabled = false;
    inline int triggerbot_delay = 100;
    inline bool triggerbot_randomize = true;

    // --- Weapon / Exploits ---
    inline bool no_recoil = false;
    inline bool no_camera_shake = false;
    inline bool infinite_ammo = false;
    inline bool rapid_fire = false;
    inline bool infinite_lethals = false;

    // --- Movement (HOST) ---
    inline bool movement_speed = false;
    inline float movement_run_mult = 1.5f;
    inline float movement_sprint_mult = 1.5f;

    // --- Settings ---
    inline int menu_toggle_key = VK_INSERT;
    inline int unload_key = VK_DELETE;

    inline void Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return;

        json j;
        try { file >> j; } catch (...) { return; }

        if (j.contains("aimbot")) {
            auto& a = j["aimbot"];
            if (a.contains("enabled")) aimbot_enabled = a["enabled"];
            if (a.contains("fov")) aimbot_fov = a["fov"];
            if (a.contains("smooth")) aimbot_smooth = a["smooth"];
            if (a.contains("smooth_x")) aimbot_smooth_x = a["smooth_x"];
            if (a.contains("smooth_y")) aimbot_smooth_y = a["smooth_y"];
            if (a.contains("prediction")) aimbot_prediction = a["prediction"];
            if (a.contains("targeting")) aimbot_targeting = a["targeting"];
            if (a.contains("bone")) aimbot_bone = a["bone"];
            if (a.contains("vis_check")) aimbot_vis_check = a["vis_check"];
            if (a.contains("follow_crouched")) aimbot_follow_crouched = a["follow_crouched"];
            if (a.contains("key")) aimbot_key = a["key"];
            if (a.contains("path_style")) aimbot_path_style = a["path_style"];
            if (a.contains("skip_spawn_protection")) aimbot_skip_spawn_protection = a["skip_spawn_protection"];
            if (a.contains("team_check")) aimbot_team_check = a["team_check"];
            if (a.contains("draw_fov")) aimbot_draw_fov = a["draw_fov"];
            if (a.contains("target_tracer")) aimbot_target_tracer = a["target_tracer"];
            if (a.contains("target_orb")) aimbot_target_orb = a["target_orb"];
        }

        if (j.contains("esp")) {
            auto& e = j["esp"];
            if (e.contains("enabled")) esp_enabled = e["enabled"];
            if (e.contains("boxes")) esp_boxes = e["boxes"];
            if (e.contains("names")) esp_names = e["names"];
            if (e.contains("health")) esp_health = e["health"];
            if (e.contains("lines")) esp_lines = e["lines"];
            if (e.contains("tracers")) esp_tracers = e["tracers"];
            if (e.contains("distance")) esp_distance = e["distance"];
            if (e.contains("visibility_check")) esp_visibility_check = e["visibility_check"];
            if (e.contains("spawn_protection_indicator")) esp_spawn_protection_indicator = e["spawn_protection_indicator"];
            if (e.contains("show_team")) esp_show_team = e["show_team"];
            if (e.contains("max_distance")) esp_max_distance = e["max_distance"];
            if (e.contains("box_thickness")) esp_box_thickness = e["box_thickness"];
            if (e.contains("snapline_origin")) esp_snapline_origin = e["snapline_origin"];
            if (e.contains("tracer_origin")) esp_tracer_origin = e["tracer_origin"];

            if (e.contains("colors")) {
                auto& c = e["colors"];
                if (c.contains("enemy")) { auto arr = c["enemy"].get<std::vector<float>>(); if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_enemy); }
                if (c.contains("team")) { auto arr = c["team"].get<std::vector<float>>(); if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_team); }
                if (c.contains("fov")) { auto arr = c["fov"].get<std::vector<float>>(); if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_fov); }
                if (c.contains("target_orb")) { auto arr = c["target_orb"].get<std::vector<float>>(); if (arr.size() == 4) std::copy(arr.begin(), arr.end(), esp_color_target_orb); }
            }
        }

        if (j.contains("triggerbot")) {
            auto& t = j["triggerbot"];
            if (t.contains("enabled")) triggerbot_enabled = t["enabled"];
            if (t.contains("delay")) triggerbot_delay = t["delay"];
            if (t.contains("randomize")) triggerbot_randomize = t["randomize"];
        }

        if (j.contains("weapon")) {
            auto& w = j["weapon"];
            if (w.contains("no_recoil")) no_recoil = w["no_recoil"];
            if (w.contains("no_camera_shake")) no_camera_shake = w["no_camera_shake"];
            if (w.contains("infinite_ammo")) infinite_ammo = w["infinite_ammo"];
            if (w.contains("rapid_fire")) rapid_fire = w["rapid_fire"];
            if (w.contains("infinite_lethals")) infinite_lethals = w["infinite_lethals"];
        }

        if (j.contains("movement")) {
            auto& m = j["movement"];
            if (m.contains("speed")) movement_speed = m["speed"];
            if (m.contains("run_mult")) movement_run_mult = m["run_mult"];
            if (m.contains("sprint_mult")) movement_sprint_mult = m["sprint_mult"];
        }

        if (j.contains("settings")) {
            auto& s = j["settings"];
            if (s.contains("menu_toggle_key")) menu_toggle_key = s["menu_toggle_key"];
            if (s.contains("unload_key")) unload_key = s["unload_key"];
        }
    }

    inline void Save(const std::string& path) {
        json j;
        j["aimbot"]["enabled"] = aimbot_enabled;
        j["aimbot"]["fov"] = aimbot_fov;
        j["aimbot"]["smooth"] = aimbot_smooth;
        j["aimbot"]["smooth_x"] = aimbot_smooth_x;
        j["aimbot"]["smooth_y"] = aimbot_smooth_y;
        j["aimbot"]["prediction"] = aimbot_prediction;
        j["aimbot"]["targeting"] = aimbot_targeting;
        j["aimbot"]["bone"] = aimbot_bone;
        j["aimbot"]["vis_check"] = aimbot_vis_check;
        j["aimbot"]["follow_crouched"] = aimbot_follow_crouched;
        j["aimbot"]["key"] = aimbot_key;
        j["aimbot"]["path_style"] = aimbot_path_style;
        j["aimbot"]["skip_spawn_protection"] = aimbot_skip_spawn_protection;
        j["aimbot"]["team_check"] = aimbot_team_check;
        j["aimbot"]["draw_fov"] = aimbot_draw_fov;
        j["aimbot"]["target_tracer"] = aimbot_target_tracer;
        j["aimbot"]["target_orb"] = aimbot_target_orb;

        j["esp"]["enabled"] = esp_enabled;
        j["esp"]["boxes"] = esp_boxes;
        j["esp"]["names"] = esp_names;
        j["esp"]["health"] = esp_health;
        j["esp"]["lines"] = esp_lines;
        j["esp"]["tracers"] = esp_tracers;
        j["esp"]["distance"] = esp_distance;
        j["esp"]["visibility_check"] = esp_visibility_check;
        j["esp"]["spawn_protection_indicator"] = esp_spawn_protection_indicator;
        j["esp"]["show_team"] = esp_show_team;
        j["esp"]["max_distance"] = esp_max_distance;
        j["esp"]["box_thickness"] = esp_box_thickness;
        j["esp"]["snapline_origin"] = esp_snapline_origin;
        j["esp"]["tracer_origin"] = esp_tracer_origin;
        j["esp"]["colors"]["enemy"] = { esp_color_enemy[0], esp_color_enemy[1], esp_color_enemy[2], esp_color_enemy[3] };
        j["esp"]["colors"]["team"] = { esp_color_team[0], esp_color_team[1], esp_color_team[2], esp_color_team[3] };
        j["esp"]["colors"]["fov"] = { esp_color_fov[0], esp_color_fov[1], esp_color_fov[2], esp_color_fov[3] };
        j["esp"]["colors"]["target_orb"] = { esp_color_target_orb[0], esp_color_target_orb[1], esp_color_target_orb[2], esp_color_target_orb[3] };

        j["triggerbot"]["enabled"] = triggerbot_enabled;
        j["triggerbot"]["delay"] = triggerbot_delay;
        j["triggerbot"]["randomize"] = triggerbot_randomize;

        j["weapon"]["no_recoil"] = no_recoil;
        j["weapon"]["no_camera_shake"] = no_camera_shake;
        j["weapon"]["infinite_ammo"] = infinite_ammo;
        j["weapon"]["rapid_fire"] = rapid_fire;
        j["weapon"]["infinite_lethals"] = infinite_lethals;

        j["movement"]["speed"] = movement_speed;
        j["movement"]["run_mult"] = movement_run_mult;
        j["movement"]["sprint_mult"] = movement_sprint_mult;

        j["settings"]["menu_toggle_key"] = menu_toggle_key;
        j["settings"]["unload_key"] = unload_key;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
        }
    }
}
