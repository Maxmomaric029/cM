#pragma once
#include <windows.h>
#include <vector>
#include <cmath>
#include "../sdk/SDK.h"
#include "../ui/Config.h"
#include "../visuals/WorldToScreen.h"

// Expose ScreenCenter globally for WorldToScreen math
inline Vector2 ScreenCenter;

namespace Aimbot {

    // Converts Unity Vector3 to internal FVector to remain compatible with older struct passes if needed
    inline FVector ToFVector(const Vector3& v) {
        return FVector(v.x, v.y, v.z);
    }

    inline CPlayer* GetBestTarget(Matrix4x4 viewMatrix, int screenWidth, int screenHeight) {
        float bestScore = 999999.0f;
        CPlayer* bestPlayer = nullptr;

        CPlayer* localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer) return nullptr;

        Vector3 localPos = localPlayer->GetRootPosition();
        int localTeam = localPlayer->GetConnectData() ? localPlayer->GetConnectData()->GetTeamId() : 255;

        IL2CPP::Array<CPlayer*>* playersArray = CPlayerRoot::GetAllPlayersArray();
        int maxPlayers = CPlayerRoot::GetAllPlayersCount();
        if (!playersArray || maxPlayers == 0) return nullptr;

        for (int i = 0; i < maxPlayers; i++) {
            CPlayer* player = playersArray->vector[i];
            
            // Skip invalid or local player
            if (!player || player == localPlayer) continue;

            // Health and Team checks
            if (player->GetHealth() <= 0.0f) continue;
            
            int team = player->GetConnectData() ? player->GetConnectData()->GetTeamId() : 255;
            if (team == localTeam && localTeam != 0) continue; // 0 Usually means FFA or unassigned team

            // Visibility Check
            if (Config::aimbot_vis_check && !player->isVisible()) continue;

            // Target Bone Logic
            Vector3 targetPos = player->GetRootPosition();
            if (Config::aimbot_bone == 0) targetPos.y += 1.6f;      // Head
            else if (Config::aimbot_bone == 1) targetPos.y += 1.35f; // Neck
            else if (Config::aimbot_bone == 2) targetPos.y += 1.1f;  // Chest
            else targetPos.y += 0.8f;                                // Pelvis
            
            if (Config::aimbot_follow_crouched && player->isCrouch()) {
                targetPos.y -= 0.5f; // Adjust down if crouched
            }

            Vector2 w2sPos;
            if (Visuals::WorldToScreen(targetPos, &w2sPos, viewMatrix)) {
                // FOV Check
                float fovDist = Vector2(w2sPos.x, w2sPos.y).Distance(ScreenCenter);
                if (fovDist > Config::aimbot_fov) continue;

                float worldDist = Vector3(localPos.x, localPos.y, localPos.z).Distance(targetPos);

                // Targeting Style Algorithm
                float score = 0.f;
                if (Config::aimbot_targeting == 0)      score = fovDist; // Closest to Crosshair
                else if (Config::aimbot_targeting == 1) score = worldDist; // Closest Distance
                else                                    score = fovDist + worldDist * 0.01f; // Hybrid

                if (score < bestScore) {
                    bestScore = score;
                    bestPlayer = player;
                }
            }
        }
        
        return bestPlayer;
    }

    inline void RunAimbot(CPlayer* targetPlayer, Matrix4x4 viewMatrix, float dt) {
        if (!targetPlayer) return;

        Vector3 targetPos = targetPlayer->GetRootPosition();
        if (Config::aimbot_bone == 0) targetPos.y += 1.6f;
        else if (Config::aimbot_bone == 1) targetPos.y += 1.35f;
        else if (Config::aimbot_bone == 2) targetPos.y += 1.1f;
        else targetPos.y += 0.8f;
        
        if (Config::aimbot_follow_crouched && targetPlayer->isCrouch()) {
            targetPos.y -= 0.5f; 
        }

        Vector2 w2sPos;
        if (Visuals::WorldToScreen(targetPos, &w2sPos, viewMatrix)) {
            
            float dx = w2sPos.x - ScreenCenter.x;
            float dy = w2sPos.y - ScreenCenter.y;
            
            float distToTarget = std::sqrt(dx * dx + dy * dy);
            
            // Mouse event hardware fluid injection (Non-blocking)
            if (distToTarget >= 2.0f) {
                float smoothX = Config::aimbot_smooth >= 1.f ? Config::aimbot_smooth : 1.f;
                float smoothY = Config::aimbot_smooth >= 1.f ? Config::aimbot_smooth : 1.f;
                
                if (dt <= 0.f || dt > 0.05f) dt = 0.016f;
                
                float strengthX = 30.0f / smoothX;
                float strengthY = 30.0f / smoothY;
                
                float stepFactorX = 1.0f - std::exp(-strengthX * dt);
                float stepFactorY = 1.0f - std::exp(-strengthY * dt);
                
                // Humanization
                if (stepFactorX > 0.92f) stepFactorX = 0.92f;
                if (stepFactorY > 0.92f) stepFactorY = 0.92f;

                int moveX = static_cast<int>(dx * stepFactorX);
                int moveY = static_cast<int>(dy * stepFactorY);
                
                if (moveX != 0 || moveY != 0) {
                    // Maximum flick clamping 
                    if (moveX > 500) moveX = 500; else if (moveX < -500) moveX = -500;
                    if (moveY > 500) moveY = 500; else if (moveY < -500) moveY = -500;
                    
                    mouse_event(MOUSEEVENTF_MOVE, moveX, moveY, 0, 0);
                }
            }
        }
    }
}
