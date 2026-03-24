#pragma once
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct FVector {
    float X, Y, Z;
    
    FVector() : X(0), Y(0), Z(0) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    
    FVector operator+(const FVector& v) const { return FVector(X + v.X, Y + v.Y, Z + v.Z); }
    FVector operator-(const FVector& v) const { return FVector(X - v.X, Y - v.Y, Z - v.Z); }
    FVector operator*(float f) const { return FVector(X * f, Y * f, Z * f); }
    FVector operator/(float f) const { return FVector(X / f, Y / f, Z / f); }
    
    float Size() const { return std::sqrt(X * X + Y * Y + Z * Z); }
    float Size2D() const { return std::sqrt(X * X + Y * Y); }
    
    float Distance(const FVector& v) const { return (*this - v).Size(); }
    float DotProduct(const FVector& v) const { return X * v.X + Y * v.Y + Z * v.Z; }
    
    void Normalize() {
        float size = Size();
        if (size > 0.f) {
            X /= size;
            Y /= size;
            Z /= size;
        }
    }
};

struct FRotator {
    float Pitch, Yaw, Roll;
    FRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FRotator(float p, float y, float r) : Pitch(p), Yaw(y), Roll(r) {}
};

struct FMatrix {
    float m[4][4];
};

struct FTransform {
    FVector Translation;
    FVector Scale3D;
};

namespace Math {
    inline void GetAxes(const FRotator& rot, FVector& x, FVector& y, FVector& z) {
        float cp = std::cos(rot.Pitch * (M_PI / 180.0f));
        float sp = std::sin(rot.Pitch * (M_PI / 180.0f));
        float cy = std::cos(rot.Yaw * (M_PI / 180.0f));
        float sy = std::sin(rot.Yaw * (M_PI / 180.0f));
        float cr = std::cos(rot.Roll * (M_PI / 180.0f));
        float sr = std::sin(rot.Roll * (M_PI / 180.0f));

        x.X = cp * cy;
        x.Y = cp * sy;
        x.Z = sp;

        y.X = sr * sp * cy - cr * sy;
        y.Y = sr * sp * sy + cr * cy;
        y.Z = -sr * cp;

        z.X = -(cr * sp * cy + sr * sy);
        z.Y = cy * sr - cr * sp * sy;
        z.Z = cr * cp;
    }

    inline FRotator CalcAngle(const FVector& from, const FVector& to) {
        FVector delta = to - from;
        float distance = delta.Size();
        if (distance < 1.0f) return FRotator();
        
        FRotator rot;
        rot.Pitch = -std::asin(delta.Z / distance) * (180.0f / M_PI);
        rot.Yaw = std::atan2(delta.Y, delta.X) * (180.0f / M_PI);
        rot.Roll = 0.f;
        
        return rot;
    }
    
    inline void ClampAngle(FRotator& rot) {
        if (rot.Pitch > 89.0f) rot.Pitch = 89.0f;
        if (rot.Pitch < -89.0f) rot.Pitch = -89.0f;
        
        while (rot.Yaw > 180.0f) rot.Yaw -= 360.0f;
        while (rot.Yaw < -180.0f) rot.Yaw += 360.0f;
        
        rot.Roll = 0.0f;
    }
}
