#pragma once
#include "../memory/Memory.h"
#include "../memory/Offsets.h"
#include "../targeting/Math.h"
#include <string>

class Entity {
protected:
    uintptr_t address;

public:
    Entity(uintptr_t addr);
    
    uintptr_t GetAddress() const;
    bool IsValid() const;

    float GetHealth() const;
    float GetMaxHealth() const;
    int GetTeamId() const;
    std::wstring GetPlayerName() const;
    FVector GetLocation() const;
    FVector GetVelocity() const;
};
