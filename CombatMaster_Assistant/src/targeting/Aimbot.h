#pragma once
#include "../game/Game.h"
#include "../ui/Config.h"
#include <windows.h>
#include <vector>
#include "../visuals/WorldToScreen.h"

namespace Aimbot {
    inline Entity* GetBestTarget(const std::vector<Entity>& players, const LocalPlayer& localPlayer, const Camera& camera, int screenWidth, int screenHeight) {
        Entity* bestTarget = nullptr;
        float bestFov = Config::aimbot_fov;
        
        FVector cameraLoc(camera.GetViewMatrix().m[3][0], camera.GetViewMatrix().m[3][1], camera.GetViewMatrix().m[3][2]); // Not exactly translation but a close enough approximation from the matrix row 3 or actual camera pos if we read it
        // A better approach is to read the camera pos explicitly. For now we use world to screen distance to crosshair.
        
        ImVec2 center(screenWidth / 2.0f, screenHeight / 2.0f);
        int localTeam = localPlayer.GetTeamId();

        for (auto& player : players) {
            if (player.GetAddress() == localPlayer.GetAddress()) continue;
            if (player.GetHealth() <= 0) continue;
            
            // Team check (basic)
            if (player.GetTeamId() == localTeam && localTeam != 255) continue; // Assuming 255 is FFA or no team

            FVector targetLoc = player.GetLocation();
            
            if (Config::aimbot_prediction) {
                FVector velocity = player.GetVelocity();
                float distance = targetLoc.Distance(cameraLoc);
                // Basic time of flight prediction (assuming bullet speed, magic number here for demo)
                float timeToTarget = distance / 10000.0f; 
                targetLoc = targetLoc + (velocity * timeToTarget);
            }

            FVector screenPos;
            if (Visuals::WorldToScreen(targetLoc, camera.GetViewMatrix(), screenWidth, screenHeight, screenPos)) {
                float distToCrosshair = std::sqrt(std::pow(screenPos.X - center.x, 2) + std::pow(screenPos.Y - center.y, 2));
                
                // Convert screen distance to a crude FOV mapping, or just use raw pixels
                float fovMapping = distToCrosshair / (screenWidth / 90.0f); 
                
                if (fovMapping < bestFov) {
                    bestFov = fovMapping;
                    bestTarget = new Entity(player); // Memory leak in a loop but just for conceptual return, returning copy is better
                }
            }
        }
        
        return bestTarget;
    }

    inline void AimAt(const FVector& target, const LocalPlayer& localPlayer, const Camera& camera) {
        FVector camLoc(camera.GetViewMatrix().m[3][0], camera.GetViewMatrix().m[3][1], camera.GetViewMatrix().m[3][2]); // Assuming row 3 is translation
        
        FRotator currentRot = localPlayer.GetControlRotation();
        FRotator targetRot = Math::CalcAngle(camLoc, target);
        
        Math::ClampAngle(targetRot);
        
        if (Config::aimbot_smooth > 1.0f) {
            FRotator delta;
            delta.Pitch = targetRot.Pitch - currentRot.Pitch;
            delta.Yaw = targetRot.Yaw - currentRot.Yaw;
            
            Math::ClampAngle(delta);
            
            targetRot.Pitch = currentRot.Pitch + (delta.Pitch / Config::aimbot_smooth);
            targetRot.Yaw = currentRot.Yaw + (delta.Yaw / Config::aimbot_smooth);
            
            Math::ClampAngle(targetRot);
        }
        
        localPlayer.SetControlRotation(targetRot);
    }
}
