#pragma once
#include "../sdk/SDK.h"

namespace Visuals {

    inline Vector2 ScreenCenter;

    // Unity IL2CPP ViewMatrix WorldToScreen (No more UE4 FVector/FRotator math)
    inline bool WorldToScreen(const Vector3& position, Vector2* outPos, Matrix4x4 viewMatrix) {
        float m00 = viewMatrix.m[0][0], m01 = viewMatrix.m[0][1], m02 = viewMatrix.m[0][2], m03 = viewMatrix.m[0][3];
        float m10 = viewMatrix.m[1][0], m11 = viewMatrix.m[1][1], m12 = viewMatrix.m[1][2], m13 = viewMatrix.m[1][3];
        float m20 = viewMatrix.m[2][0], m21 = viewMatrix.m[2][1], m22 = viewMatrix.m[2][2], m23 = viewMatrix.m[2][3];
        float m30 = viewMatrix.m[3][0], m31 = viewMatrix.m[3][1], m32 = viewMatrix.m[3][2], m33 = viewMatrix.m[3][3];

        // Ensure w is greater than 1.f to prevent drawing behind the camera
        float w = m03 * position.x + m13 * position.y + m23 * position.z + m33;
        if (w < 1.f)
            return false;

        float invW = 1.0f / w;

        float x = m00 * position.x + m10 * position.y + m20 * position.z + m30;
        float y = m01 * position.x + m11 * position.y + m21 * position.z + m31;


        float screenX = (ScreenCenter.x) * (1.f + x * invW);
        float screenY = (ScreenCenter.y) * (1.f - y * invW);

        outPos->x = screenX;
        outPos->y = screenY;

        return true;
    }

}
