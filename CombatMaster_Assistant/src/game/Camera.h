#pragma once
#include "../memory/Memory.h"
#include "../memory/Offsets.h"
#include "../targeting/Math.h"

struct FCameraCacheEntry {
    FVector Location;
    FRotator Rotation;
    float FOV;
};

class Camera {
private:
    uintptr_t cameraManager;
    FCameraCacheEntry cache;

public:
    Camera(uintptr_t managerAddr) : cameraManager(managerAddr) {
        if (managerAddr) {
            // Attempt standard +0x10 offset for FMinimalViewInfo inside FCameraCacheEntry
            cache.Location = Memory::Get().Read<FVector>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX + 0x10);
            cache.Rotation = Memory::Get().Read<FRotator>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX + 0x1C);
            cache.FOV = Memory::Get().Read<float>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX + 0x28);
            
            // Fallback if Offsets::CAMERAMANAGER_VIEWMATRIX already points directly to Location
            if (cache.FOV <= 0.0f || cache.FOV > 180.f) {
                cache.Location = Memory::Get().Read<FVector>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX);
                cache.Rotation = Memory::Get().Read<FRotator>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX + 0xC);
                cache.FOV = Memory::Get().Read<float>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX + 0x18);
            }
        }
    }

    bool IsValid() const {
        return cameraManager != 0 && cache.FOV > 0.0f && cache.FOV < 180.0f;
    }

    FVector GetLocation() const { return cache.Location; }
    FRotator GetRotation() const { return cache.Rotation; }
    float GetFOV() const { return cache.FOV > 0.0f ? cache.FOV : 90.0f; }
};

