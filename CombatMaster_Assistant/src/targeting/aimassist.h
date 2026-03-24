#pragma once
#include "../game/Game.h"
#include "../ui/Config.h"
#include <windows.h>
#include <vector>
#include <optional>
#include "../visuals/WorldToScreen.h"
#include <algorithm>

namespace Aimbot {

    // Advanced Prediction & Physics Engine
    struct PredictionContext {
        float TimeToHit;
        FVector PredictedLocation;
        float GravityDrop;
        
        PredictionContext() : TimeToHit(0.0f), PredictedLocation(), GravityDrop(0.0f) {}
    };

    inline PredictionContext CalculatePrediction(const FVector& targetLoc, const FVector& targetVel, float distance, float projSpeed, float gravity = 980.0f) {
        PredictionContext ctx;
        if (projSpeed <= 1.0f) {
            ctx.PredictedLocation = targetLoc; // Instant hit
            return ctx;
        }

        ctx.TimeToHit = distance / projSpeed;
        
        // Linear velocity prediction
        ctx.PredictedLocation = targetLoc + (targetVel * ctx.TimeToHit);
        
        // Projectile drop simulation (1/2 * g * t^2)
        ctx.GravityDrop = 0.5f * gravity * (ctx.TimeToHit * ctx.TimeToHit);
        ctx.PredictedLocation.Z += ctx.GravityDrop; // Compensate by aiming higher
        
        return ctx;
    }

    // Cubic-Bezier like smooth curve math
    inline FRotator AdvancedSmooth(const FRotator& current, const FRotator& target, float factor, float dt = 0.016f) {
        if (factor <= 1.0f) return target;
        
        FRotator delta;
        delta.Pitch = target.Pitch - current.Pitch;
        delta.Yaw = target.Yaw - current.Yaw;
        
        // Normalize
        while (delta.Yaw > 180.0f) delta.Yaw -= 360.0f;
        while (delta.Yaw < -180.0f) delta.Yaw += 360.0f;
        
        // Dynamic easing based on delta magnitude (humanized response)
        float magnitude = std::sqrt(delta.Pitch*delta.Pitch + delta.Yaw*delta.Yaw);
        float dynamicFactor = factor;
        
        if (magnitude < 2.0f) {
            dynamicFactor *= 1.5f; // Slower when closer to target (micro-adjustments)
        } else if (magnitude > 15.0f) {
            dynamicFactor *= 0.8f; // Faster flick when far away
        }

        FRotator finalRot;
        finalRot.Pitch = current.Pitch + (delta.Pitch / dynamicFactor);
        finalRot.Yaw = current.Yaw + (delta.Yaw / dynamicFactor);
        finalRot.Roll = 0.0f;
        
        Math::ClampAngle(finalRot);
        return finalRot;
    }

    // Visibility and Priority Evaluation
    struct TargetResult {
        Entity targetEntity;
        float priorityScore; // Lower is better
    };

    inline std::optional<Entity> GetBestTarget(const std::vector<Entity>& players, const LocalPlayer& localPlayer, const Camera& camera, int screenWidth, int screenHeight) {
        std::vector<TargetResult> validTargets;
        float bestFov = Config::aimbot_fov;
        FVector cameraLoc = camera.GetLocation();
        ImVec2 center(screenWidth / 2.0f, screenHeight / 2.0f);
        int localTeam = localPlayer.GetTeamId();

        for (auto& player : players) {
            if (player.GetAddress() == localPlayer.GetAddress()) continue;
            if (player.GetHealth() <= 0) continue;
            if (player.GetTeamId() == localTeam && localTeam != 255) continue;

            FVector originalLoc = player.GetLocation();
            
            // Apply primitive velocity if prediction is enabled
            FVector currentLoc = originalLoc;
            if (Config::aimbot_prediction) {
                FVector vel = player.GetVelocity();
                float dist = originalLoc.Distance(cameraLoc);
                PredictionContext pred = CalculatePrediction(originalLoc, vel, dist, 50000.0f);
                currentLoc = pred.PredictedLocation;
            }

            FVector screenPos;
            if (Visuals::WorldToScreen(currentLoc, camera, screenWidth, screenHeight, screenPos)) {
                float distToCrosshair = std::sqrt(std::pow(screenPos.X - center.x, 2) + std::pow(screenPos.Y - center.y, 2));
                float fovMapping = distToCrosshair / (screenWidth / 90.0f); 
                
                if (fovMapping < bestFov) {
                    // Priority is a mix of distance to crosshair, real 3D distance, and absolute health
                    float realDist = originalLoc.Distance(cameraLoc) / 100.0f;
                    float hpWeight = player.GetHealth() / player.GetMaxHealth();
                    
                    // Complex priority score algorithm
                    float currentPriority = (fovMapping * 0.6f) + (realDist * 0.3f) + (hpWeight * 0.1f);
                    validTargets.push_back({player, currentPriority});
                }
            }
        }
        
        if (validTargets.empty()) return std::nullopt;
        
        // Sort by best score
        std::sort(validTargets.begin(), validTargets.end(), [](const TargetResult& a, const TargetResult& b) {
            return a.priorityScore < b.priorityScore;
        });

        return validTargets.front().targetEntity;
    }

    inline void AimAt(const FVector& target, const LocalPlayer& localPlayer, const Camera& camera) {
        FVector camLoc = camera.GetLocation();
        
        FRotator currentRot = localPlayer.GetControlRotation();
        FRotator targetRot = Math::CalcAngle(camLoc, target);
        
        Math::ClampAngle(targetRot);
        
        if (Config::aimbot_smooth > 1.0f) {
            targetRot = AdvancedSmooth(currentRot, targetRot, Config::aimbot_smooth);
        }
        
        localPlayer.SetControlRotation(targetRot);
    }
}
