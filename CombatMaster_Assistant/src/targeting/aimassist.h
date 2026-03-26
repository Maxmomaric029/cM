#pragma once
#include <windows.h>
#include <vector>
#include <cmath>
#include "../sdk/SDK.h"
#include "../ui/Config.h"
#include "../visuals/WorldToScreen.h"

// ─────────────────────────────────────────────────────────────────────────────
// Aimbot — External version
// All CPlayer instances are value objects that fetch data via RPM internally.
// ─────────────────────────────────────────────────────────────────────────────
namespace Aimbot {
    static const int kMaxAimMovePerFrame = 500;

    // Returns the best target (by value — CPlayer wraps a remote address)
    inline CPlayer GetBestTarget(Matrix4x4 viewMatrix, int screenWidth, int screenHeight) {
        CPlayer nonePlayer(0);
        float bestScore = 999999.0f;
        CPlayer bestPlayer(0);

        CPlayer localPlayer = CPlayerRoot::GetLocalPlayer();
        if (!localPlayer.Valid()) return nonePlayer;

        Vector3 localPos = localPlayer.GetRootPosition();

        int maxPlayers = CPlayerRoot::GetAllPlayersCount();
        if (maxPlayers <= 0) return nonePlayer;

        for (int i = 0; i < maxPlayers; i++) {
            CPlayer player = CPlayerRoot::GetPlayerAt(i);
            if (!player.Valid()) continue;
            if (player.GetAddress() == localPlayer.GetAddress()) continue;
            if (player.IsDead()) continue;
            if (Config::aimbot_team_check && player.IsTeammate(localPlayer)) continue;
            if (Config::aimbot_skip_spawn_protection && player.IsInvincible()) continue;
            if (Config::aimbot_vis_check && !player.isVisible()) continue;

            Vector3 targetPos = player.GetRootPosition();
            int bone = Config::aimbot_bone;
            if (Config::aimbot_follow_crouched && player.isCrouch()) bone = 2;

            if      (bone == 0) targetPos.y += 1.6f;   // Head
            else if (bone == 1) targetPos.y += 1.35f;  // Neck
            else if (bone == 2) targetPos.y += 1.1f;   // Chest
            else                targetPos.y += 0.8f;    // Pelvis

            Vector2 w2sPos;
            if (!Visuals::WorldToScreen(targetPos, &w2sPos, viewMatrix)) continue;

            float fovDist  = Vector2(w2sPos.x, w2sPos.y).Distance(Visuals::ScreenCenter);
            if (fovDist > Config::aimbot_fov) continue;

            float worldDist = Vector3(localPos.x, localPos.y, localPos.z).Distance(targetPos);

            float score = 0.f;
            if      (Config::aimbot_targeting == 0) score = fovDist;
            else if (Config::aimbot_targeting == 1) score = worldDist;
            else                                    score = fovDist + worldDist * 0.01f;

            if (score < bestScore) {
                bestScore  = score;
                bestPlayer = player;
            }
        }
        return bestPlayer;
    }

    inline void RunAimbot(CPlayer& targetPlayer, Matrix4x4 viewMatrix, float dt) {
        if (!targetPlayer.Valid()) return;

        Vector3 targetPos = targetPlayer.GetRootPosition();
        int bone = Config::aimbot_bone;
        if (Config::aimbot_follow_crouched && targetPlayer.isCrouch()) bone = 2;

        if      (bone == 0) targetPos.y += 1.6f;
        else if (bone == 1) targetPos.y += 1.35f;
        else if (bone == 2) targetPos.y += 1.1f;
        else                targetPos.y += 0.8f;

        Vector2 w2sPos;
        if (!Visuals::WorldToScreen(targetPos, &w2sPos, viewMatrix)) return;

        float dx = w2sPos.x - Visuals::ScreenCenter.x;
        float dy = w2sPos.y - Visuals::ScreenCenter.y;
        float distToTarget = std::sqrt(dx * dx + dy * dy);
        if (distToTarget < 2.0f) return;

        float smoothX = Config::aimbot_smooth_x >= 1.f ? Config::aimbot_smooth_x : 1.f;
        float smoothY = Config::aimbot_smooth_y >= 1.f ? Config::aimbot_smooth_y : 1.f;

        if (dt <= 0.f || dt > 0.05f) dt = 0.016f;
        if (dt < 0.001f) dt = 0.001f;

        float strengthX = 30.0f / smoothX;
        float strengthY = 30.0f / smoothY;

        float stepFactorX = 1.0f - std::exp(-strengthX * dt);
        float stepFactorY = 1.0f - std::exp(-strengthY * dt);

        if (Config::aimbot_path_style == 0) {
            if (stepFactorX > 0.92f) stepFactorX = 0.92f;
            if (stepFactorY > 0.92f) stepFactorY = 0.92f;
        } else {
            stepFactorX *= 1.06f; stepFactorY *= 1.06f;
            if (stepFactorX > 1.10f) stepFactorX = 1.10f;
            if (stepFactorY > 1.10f) stepFactorY = 1.10f;
        }

        int moveX = static_cast<int>(dx * stepFactorX);
        int moveY = static_cast<int>(dy * stepFactorY);

        if (moveX > kMaxAimMovePerFrame)  moveX =  kMaxAimMovePerFrame;
        if (moveX < -kMaxAimMovePerFrame) moveX = -kMaxAimMovePerFrame;
        if (moveY > kMaxAimMovePerFrame)  moveY =  kMaxAimMovePerFrame;
        if (moveY < -kMaxAimMovePerFrame) moveY = -kMaxAimMovePerFrame;

        if (moveX != 0 || moveY != 0)
            mouse_event(MOUSEEVENTF_MOVE, moveX, moveY, 0, 0);
    }
}
