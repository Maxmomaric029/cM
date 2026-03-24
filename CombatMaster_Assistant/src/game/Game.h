#pragma once
#include "LocalPlayer.h"
#include "Camera.h"
#include "../utils/Logger.h"
#include <vector>
#include <chrono>

class Game {
private:
    uintptr_t uWorld = 0;
    uintptr_t gameInstance = 0;
    uintptr_t localPlayerPtr = 0;
    uintptr_t playerController = 0;

    // Entity cache to prevent massive FPS drops
    std::vector<Entity> cachedPlayers;
    std::chrono::steady_clock::time_point lastCacheTime;

public:
    Game() : lastCacheTime(std::chrono::steady_clock::now()) {}

    bool Update() {
        auto& mem = Memory::Get();
        uWorld = mem.Read<uintptr_t>(mem.GetBaseAddress() + Offsets::GWORLD_OFFSET);
        if (!uWorld) {
            static bool loggedW = false; if (!loggedW) { Logger::Error("[Game::Update] Failed to read UWorld. Is GWORLD_OFFSET (" + std::to_string(Offsets::GWORLD_OFFSET) + ") correct?"); loggedW = true; }
            return false;
        }

        gameInstance = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_OWNINGGAMEINSTANCE);
        if (!gameInstance) {
            static bool loggedI = false; if (!loggedI) { Logger::Error("[Game::Update] Failed to read GameInstance at UWorld + " + std::to_string(Offsets::UWORLD_OWNINGGAMEINSTANCE)); loggedI = true; }
            return false;
        }

        uintptr_t localPlayersArray = mem.Read<uintptr_t>(gameInstance + Offsets::GAMEINSTANCE_LOCALPLAYERS);
        if (!localPlayersArray) {
            static bool loggedL = false; if (!loggedL) { Logger::Error("[Game::Update] Failed to read LocalPlayersArray"); loggedL = true; }
            return false;
        }

        localPlayerPtr = mem.Read<uintptr_t>(localPlayersArray); // First local player
        if (!localPlayerPtr) {
            static bool loggedP = false; if (!loggedP) { Logger::Error("[Game::Update] Failed to read LocalPlayer[0]"); loggedP = true; }
            return false;
        }

        playerController = mem.Read<uintptr_t>(localPlayerPtr + Offsets::LOCALPLAYER_PLAYERCONTROLLER);
        if (!playerController) {
            static bool loggedC = false; if (!loggedC) { Logger::Error("[Game::Update] Failed to read PlayerController"); loggedC = true; }
            return false;
        }

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
        // Cache players every 500ms to save CPU cycles from reading ULevel array every frame
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheTime).count() < 200 && !cachedPlayers.empty()) {
            return cachedPlayers;
        }

        std::vector<Entity> players;
        auto& mem = Memory::Get();
        
        if (!uWorld) return cachedPlayers;
        
        uintptr_t persistentLevel = mem.Read<uintptr_t>(uWorld + Offsets::UWORLD_PERSISTENTLEVEL);
        if (!persistentLevel) {
            static bool loggedPL = false; if (!loggedPL) { Logger::Error("[Game::GetPlayers] Failed to read PersistentLevel"); loggedPL = true; }
            return cachedPlayers;
        }
        
        uintptr_t actorsArray = mem.Read<uintptr_t>(persistentLevel + Offsets::ULEVEL_ACTORS);
        int actorCount = mem.Read<int>(persistentLevel + Offsets::ULEVEL_ACTORCOUNT);
        
        static bool debugged = false;
        if (!debugged && actorsArray) {
            Logger::Log("Successfully dumped UWorld! Actor Count: " + std::to_string(actorCount));
            debugged = true;
        }

        if (!actorsArray) {
            static bool loggedA = false; if (!loggedA) { Logger::Error("[Game::GetPlayers] Failed to read ActorsArray"); loggedA = true; }
            return cachedPlayers;
        }

        if (actorCount <= 0 || actorCount > 10000) {
            static bool loggedC = false; if (!loggedC) { Logger::Error("[Game::GetPlayers] Invalid ActorCount: " + std::to_string(actorCount) + " (Max: 10000). Check ULEVEL_ACTORCOUNT offset."); loggedC = true; }
            return cachedPlayers;
        }
        
        // Massive performance improvement: Read entire pointer array at once instead of individual RPMs
        std::vector<uintptr_t> actorPointers = mem.ReadArray<uintptr_t>(actorsArray, actorCount);
        
        for (uintptr_t actor : actorPointers) {
            if (!actor) continue;
            
            // Basic sanity check to ensure it's a character or has root component
            uintptr_t rootComp = mem.Read<uintptr_t>(actor + Offsets::ACTOR_ROOTCOMPONENT);
            if (rootComp) {
                players.push_back(Entity(actor));
            }
        }
        
        cachedPlayers = players;
        lastCacheTime = now;
        
        return players;
    }
};
