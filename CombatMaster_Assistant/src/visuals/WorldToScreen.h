#pragma once
#include "../targeting/Math.h"
#include "../game/Camera.h"

namespace Visuals {
    inline bool WorldToScreen(const FVector& worldLocation, const Camera& camera, int screenWidth, int screenHeight, FVector& outScreen) {
        FVector camLoc = camera.GetLocation();
        FRotator camRot = camera.GetRotation();
        float fov = camera.GetFOV();

        FVector axisX, axisY, axisZ;
        Math::GetAxes(camRot, axisX, axisY, axisZ);

        FVector delta = worldLocation - camLoc;
        FVector transformed(delta.DotProduct(axisY), delta.DotProduct(axisZ), delta.DotProduct(axisX));

        if (transformed.Z < 1.0f) {
            return false;
        }

        float fovRad = fov * (M_PI / 360.0f);
        outScreen.X = (screenWidth / 2.0f) + transformed.X * ((screenWidth / 2.0f) / std::tan(fovRad)) / transformed.Z;
        outScreen.Y = (screenHeight / 2.0f) - transformed.Y * ((screenWidth / 2.0f) / std::tan(fovRad)) / transformed.Z;
        outScreen.Z = transformed.Z;

        return true;
    }
}
