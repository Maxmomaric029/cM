#pragma once
#include "../targeting/Math.h"

namespace Visuals {
    inline bool WorldToScreen(const FVector& worldLocation, const FMatrix& viewMatrix, int screenWidth, int screenHeight, FVector& outScreen) {
        FVector vAxisX(viewMatrix.m[0][0], viewMatrix.m[1][0], viewMatrix.m[2][0]);
        FVector vAxisY(viewMatrix.m[0][1], viewMatrix.m[1][1], viewMatrix.m[2][1]);
        FVector vAxisZ(viewMatrix.m[0][2], viewMatrix.m[1][2], viewMatrix.m[2][2]);
        FVector vAxisW(viewMatrix.m[0][3], viewMatrix.m[1][3], viewMatrix.m[2][3]);
        
        float w = worldLocation.X * viewMatrix.m[0][3] + worldLocation.Y * viewMatrix.m[1][3] + worldLocation.Z * viewMatrix.m[2][3] + viewMatrix.m[3][3];
        
        if (w < 0.01f) {
            return false;
        }
        
        float x = worldLocation.X * viewMatrix.m[0][0] + worldLocation.Y * viewMatrix.m[1][0] + worldLocation.Z * viewMatrix.m[2][0] + viewMatrix.m[3][0];
        float y = worldLocation.X * viewMatrix.m[0][1] + worldLocation.Y * viewMatrix.m[1][1] + worldLocation.Z * viewMatrix.m[2][1] + viewMatrix.m[3][1];
        
        outScreen.X = (screenWidth / 2.0f) + (x / w) * (screenWidth / 2.0f);
        outScreen.Y = (screenHeight / 2.0f) - (y / w) * (screenHeight / 2.0f);
        outScreen.Z = w;
        
        return true;
    }
}
