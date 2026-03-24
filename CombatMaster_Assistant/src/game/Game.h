#pragma once
#include "LocalPlayer.h"
#include "Camera.h"
#include "../utils/Logger.h"
#include <vector>

class Game {
private:
    uintptr_t uWorld = 0;
    uintptr_t gameInstance = 0;
    uintptr_t localPlayerPtr = 0;
    uintptr_t playerController = 0;

public:
    bool Update() {
        auto& mem = Memory::Get();
        uintptr_t gWorldPtr = mem.Read<uintptr_t>(mem.GetBaseAddress() + Offsets::GWORLD_OFFSET);
        if (!gWorldPtr) return false;

        uWorld = mem.Read<uintptr_t>(gWorldPtr);
        if (!uWorld) return false;

        gameInstance = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_OWNINGGAMEINSTANCE);
        if (!gameInstance) return false;

        uintptr_t localPlayersArray = mem.Read<uintptr_t>(gameInstance + Offsets::GAMEINSTANCE_LOCALPLAYERS);
        if (!localPlayersArray) return false;

        localPlayerPtr = mem.Read<uintptr_t>(localPlayersArray); // First local player
        if (!localPlayerPtr) return false;

        playerController = mem.Read<uintptr_t>(localPlayerPtr + Offsets::LOCALPLAYER_PLAYERCONTROLLER);
        if (!playerController) return false;

        return true;
    }

    LocalPlayer GetLocalPlayer() const {
        if (!playerController) return LocalPlayer(0, 0);
        
        uintptr_t pawn = Memory::Get().Read<uintptr_t>(playerController + Offsets::PLAYERCONTROLLER_PAWN);
        return LocalPlayer(pawn, playerController);
    }

    Camera GetCamera() const {
        if (!playerController) return Camera(0);
        
        uintptr_t cameraManager = Memory::Get().Read<uintptr_t>(playerController + Offsets::PLAYERCONTROLLER_CAMERAMANAGER);
        return Camera(cameraManager);
    }

    std::vector<Entity> GetPlayers() const {
        std::vector<Entity> players;
        auto& mem = Memory::Get();
        
        if (!uWorld) return players;
        
        uintptr_t persistentLevel = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_PERSISTENTLEVEL);
        if (!persistentLevel) return players;
        
        uintptr_t actorsArray = mem.Read<uintptr_t>(persistentLevel + Offsets::ULEVEL_ACTORS);
        int actorCount = mem.Read<int>(persistentLevel + Offsets::ULEVEL_ACTORCOUNT);
        
        if (!actorsArray || actorCount <= 0 || actorCount > 10000) return players;
        
        for (int i = 0; i < actorCount; i++) {
            uintptr_t actor = mem.Read<uintptr_t>(actorsArray + (i * 8));
            if (!actor) continue;
            
            // Basic sanity check, could check vtable or specific components to ensure it's a character
            uintptr_t rootComp = mem.Read<uintptr_t>(actor + Offsets::ACTOR_ROOTCOMPONENT);
            if (rootComp) {
                players.push_back(Entity(actor));
            }
        }
        
        return players;
    }
};
