#pragma once
#include "Globals.h"
#include "Offsets.h"

// Host config: run/sprint speed. Only applied when in match and option enabled.
namespace MovementHooks {
    inline float oRun = 8.f, oSprint = 12.f;
    inline bool cached = false;

    inline void Restore() {
        if (!cached) return;
        uint64_t mod = Globals::GdiModule ? Globals::GdiModule : Globals::ProjectModule;
        if (!mod) { cached = false; return; }
        using get_Instance_t = void*(*)();
        auto getInstance = (get_Instance_t)(mod + Offsets::HostConfig::get_Instance);
        void* cfg = getInstance ? getInstance() : nullptr;
        if (!cfg) { cached = false; return; }
        uint64_t gdInfo = *(uint64_t*)((uintptr_t)cfg + Offsets::HostConfig::GdInfo);
        if (!gdInfo) { cached = false; return; }
        uint64_t ops = *(uint64_t*)(gdInfo + Offsets::HostConfig::Operators);
        if (!ops) { cached = false; return; }
        namespace Ops = Offsets::HostConfig::OperatorsSection;
        *(float*)(ops + Ops::RunSpeed) = oRun;
        *(float*)(ops + Ops::SprintSpeed) = oSprint;
        cached = false;
    }

    inline void ApplyIfNeeded() {
        if (!Globals::InMatch || !Globals::Menu::bMovementSpeed) return;
        uint64_t mod = Globals::GdiModule ? Globals::GdiModule : Globals::ProjectModule;
        if (!mod) return;
        using get_Instance_t = void*(*)();
        auto getInstance = (get_Instance_t)(mod + Offsets::HostConfig::get_Instance);
        void* cfg = getInstance ? getInstance() : nullptr;
        if (!cfg) return;
        uint64_t gdInfo = *(uint64_t*)((uintptr_t)cfg + Offsets::HostConfig::GdInfo);
        if (!gdInfo) return;
        uint64_t ops = *(uint64_t*)(gdInfo + Offsets::HostConfig::Operators);
        if (!ops) return;
        namespace Ops = Offsets::HostConfig::OperatorsSection;
        if (!cached) {
            oRun = *(float*)(ops + Ops::RunSpeed);
            oSprint = *(float*)(ops + Ops::SprintSpeed);
            cached = true;
        }
        *(float*)(ops + Ops::RunSpeed) = oRun * Globals::Menu::movementRunMult;
        *(float*)(ops + Ops::SprintSpeed) = oSprint * Globals::Menu::movementSprintMult;
    }
}
