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
        
        // Try static offset first
        uintptr_t staticAddr = base + Offsets::GWORLD_OFFSET;
        uintptr_t uWorldPtr = mem.Read<uintptr_t>(staticAddr);
        if (uWorldPtr) return uWorldPtr;

        // Pattern Scan Fallback for UE4.27 GWorld
        // Pattern: 48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74 06
        uintptr_t sig = mem.FindPattern(base, 0x8000000, "\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x88\x00\x00\x00\x00\x48\x85\xC9\x74\x06", "xxx????xxx????xxxxx");
        if (sig) {
            uint32_t offset = mem.Read<uint32_t>(sig + 3);
            uintptr_t gworld_ptr_addr = sig + offset + 7;
            return mem.Read<uintptr_t>(gworld_ptr_addr);
        }

        // Another common UE4 pattern
        // 48 8B 1D ? ? ? ? 48 85 DB 74 3B 41 B0 01
        sig = mem.FindPattern(base, 0x8000000, "\x48\x8B\x1D\x00\x00\x00\x00\x48\x85\xDB\x74\x3B\x41\xB0\x01", "xxx????xxxxxxxx");
        if (sig) {
            uint32_t offset = mem.Read<uint32_t>(sig + 3);
            uintptr_t gworld_ptr_addr = sig + offset + 7;
            return mem.Read<uintptr_t>(gworld_ptr_addr);
        }

        return 0;
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
