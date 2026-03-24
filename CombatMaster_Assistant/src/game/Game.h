#pragma once
#include "LocalPlayer.h"
#include "Camera.h"
#include "Entity.h"
#include "../utils/Logger.h"
#include <vector>
#include <chrono>

class Game {
private:
    uintptr_t uWorld = 0;
    uintptr_t gameInstance = 0;
    uintptr_t localPlayerPtr = 0;
    uintptr_t playerController = 0;

    std::vector<Entity> cachedPlayers;
    std::chrono::steady_clock::time_point lastCacheTime;

    uintptr_t FindGWorld() {
        auto& mem = Memory::Get();
        uintptr_t base = mem.GetBaseAddress();
        
        // Trust static offsets as requested.
        // No pattern scanning fallback here, because scanning 128MB per frame causes massive lag if UWorld is 0 in the menu.
        uintptr_t staticAddr = base + Offsets::GWORLD_OFFSET;
        return mem.Read<uintptr_t>(staticAddr);
    }

public:
    Game() : lastCacheTime(std::chrono::steady_clock::now()) {}

    bool Update() {
        auto& mem = Memory::Get();
        
        uWorld = FindGWorld();
        if (!uWorld) return false;

        gameInstance = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_OWNINGGAMEINSTANCE);
        if (!gameInstance) return false;

        uintptr_t localPlayersArray = mem.Read<uintptr_t>(gameInstance + Offsets::GAMEINSTANCE_LOCALPLAYERS);
        if (!localPlayersArray) return false;

        localPlayerPtr = mem.Read<uintptr_t>(localPlayersArray);
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

    std::vector<Entity> GetPlayers() {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheTime).count() < 200 && !cachedPlayers.empty()) {
            return cachedPlayers;
        }

        std::vector<Entity> players;
        auto& mem = Memory::Get();
        
        if (!uWorld) return cachedPlayers;
        
        uintptr_t persistentLevel = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_PERSISTENTLEVEL);
        if (!persistentLevel) return cachedPlayers;
        
        uintptr_t actorsArray = mem.Read<uintptr_t>(persistentLevel + Offsets::ULEVEL_ACTORS);
        int actorCount = mem.Read<int>(persistentLevel + Offsets::ULEVEL_ACTORCOUNT);
        
        if (!actorsArray || actorCount <= 0 || actorCount > 10000) return cachedPlayers;
        
        std::vector<uintptr_t> actorPointers = mem.ReadArray<uintptr_t>(actorsArray, actorCount);
        for (uintptr_t actor : actorPointers) {
            if (!actor) continue;
            uintptr_t rootComp = mem.Read<uintptr_t>(actor + Offsets::ACTOR_ROOTCOMPONENT);
            if (rootComp) players.push_back(Entity(actor));
        }
        
        cachedPlayers = players;
        lastCacheTime = now;
        return players;
    }
};
