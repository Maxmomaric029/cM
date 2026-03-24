#include "Entity.h"

Entity::Entity(uintptr_t addr) : address(addr) {
}

uintptr_t Entity::GetAddress() const {
    return address;
}

bool Entity::IsValid() const {
    // Validate address bounds roughly and check if pointer is somewhat sane
    return address > 0x10000 && address < 0x7FFFFFFFFFFF;
}

float Entity::GetHealth() const {
    if (!IsValid()) return 0.0f;
    return Memory::Get().Read<float>(address + Offsets::ACTOR_HEALTH);
}

float Entity::GetMaxHealth() const {
    if (!IsValid()) return 100.0f;
    float maxHealth = Memory::Get().Read<float>(address + Offsets::ACTOR_HEALTH_MAX);
    return maxHealth > 0.0f ? maxHealth : 100.0f; // Prevent div by 0 just in case
}

int Entity::GetTeamId() const {
    if (!IsValid()) return 255;
    return Memory::Get().Read<int>(address + Offsets::ACTOR_TEAMID);
}

std::wstring Entity::GetPlayerName() const {
    if (!IsValid()) return L"Unknown";
    
    uintptr_t playerState = Memory::Get().Read<uintptr_t>(address + Offsets::CONTROLLER_PLAYERSTATE);
    if (!playerState) return L"Unknown";
    
    std::wstring name = Memory::Get().ReadFString(playerState + Offsets::PLAYERSTATE_PLAYERNAME);
    if (name.empty()) return L"Unknown";
    
    return name;
}

FVector Entity::GetLocation() const {
    if (!IsValid()) return FVector();
    
    uintptr_t rootComponent = Memory::Get().Read<uintptr_t>(address + Offsets::ACTOR_ROOTCOMPONENT);
    if (!rootComponent) return FVector();
    
    return Memory::Get().Read<FVector>(rootComponent + Offsets::COMPONENT_WORLDLOCATION);
}

FVector Entity::GetVelocity() const {
    if (!IsValid()) return FVector();
    
    return Memory::Get().Read<FVector>(address + Offsets::PAWN_VELOCITY);
}
