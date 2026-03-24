#pragma once
#include <cmath>

struct ViewMatrix {
    float matrix[4][4];
};

class Vector2 {
public:
    float x, y;
    Vector2() : x(), y() {}
    Vector2(float x_, float y_) : x(x_), y(y_) {}
    Vector2 operator+(const Vector2& a) const { return Vector2(x + a.x, y + a.y); }
    Vector2 operator-(const Vector2& a) const { return Vector2(x - a.x, y - a.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return s != 0.f ? Vector2(x / s, y / s) : Vector2(0, 0); }
    float Length() const { return std::sqrt(x * x + y * y); }
    static float Distance(const Vector2& a, const Vector2& b) { return (a - b).Length(); }
};

class Vector3 {
public:
    float x, y, z;
    Vector3() : x(), y(), z() {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Vector3 operator+(const Vector3& a) const { return Vector3(x + a.x, y + a.y, z + a.z); }
    Vector3 operator-(const Vector3& a) const { return Vector3(x - a.x, y - a.y, z - a.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    static float Distance(const Vector3& a, const Vector3& b) { return (a - b).Length(); }
};
