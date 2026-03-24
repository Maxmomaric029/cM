#pragma once
#include "Entity.h"
#include "../memory/Memory.h"

class LocalPlayer : public Entity {
private:
    uintptr_t playerController;

public:
    LocalPlayer(uintptr_t pawnAddr, uintptr_t controllerAddr) 
        : Entity(pawnAddr), playerController(controllerAddr) {}

    uintptr_t GetController() const {
        return playerController;
    }

    FRotator GetControlRotation() const {
        return Memory::Get().Read<FRotator>(playerController + Offsets::PLAYERCONTROLLER_CONTROLROTATION);
    }

    void SetControlRotation(const FRotator& rot) const {
        Memory::Get().Write<FRotator>(playerController + Offsets::PLAYERCONTROLLER_CONTROLROTATION, rot);
    }
    
    int GetAmmo() const {
        if (!IsValid()) return 0;
        uintptr_t currentWeapon = Memory::Get().Read<uintptr_t>(address + Offsets::PAWN_CURRENTWEAPON);
        if (!currentWeapon) return 0;
        
        return Memory::Get().Read<int>(currentWeapon + Offsets::WEAPON_AMMO);
    }
};
