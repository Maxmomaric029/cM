#pragma once
#include "../memory/Memory.h"
#include "../memory/Offsets.h"
#include "../targeting/Math.h"

class Camera {
private:
    uintptr_t cameraManager;

public:
    Camera(uintptr_t managerAddr) : cameraManager(managerAddr) {}

    bool IsValid() const {
        return cameraManager != 0;
    }

    FMatrix GetViewMatrix() const {
        return Memory::Get().Read<FMatrix>(cameraManager + Offsets::CAMERAMANAGER_VIEWMATRIX);
    }
};
