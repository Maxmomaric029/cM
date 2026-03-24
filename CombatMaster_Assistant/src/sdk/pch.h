#pragma once

// C++ RunTime Header Files
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <optional>

// Core Utilities
#include "../memory/Memory.h"
#include "../utils/Logger.h"
#include "../memory/Offsets.h"

// Unity Math Types
struct Vector3 { 
    float x, y, z; 
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    float Distance(const Vector3& v) const {
        return std::sqrt(std::pow(v.x - x, 2) + std::pow(v.y - y, 2) + std::pow(v.z - z, 2));
    }
};

struct Vector2 { 
    float x, y; 
    
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    float Distance(const Vector2& v) const {
        return std::sqrt(std::pow(v.x - x, 2) + std::pow(v.y - y, 2));
    }
};

struct Matrix4x4 { 
    float m[4][4]; 
};
