#pragma once
#include <windows.h>
#include <cstdint>
#include "../memory/Offsets.h"
#include "../memory/Memory.h"
#include "../ui/Config.h"

// Host config speed modifications (run/sprint multipliers)
// Uses GDI module like reference (falls back to Project.dll)
namespace MovementHooks {
    inline float oRun = 8.f, oSprint = 12.f;
    inline bool cached = false;

    inline void Restore() {
        if (!cached) return;
        uintptr_t mod = Memory::Get().GetGdiModule();
        if (!mod) { cached = false; return; }

        using get_Instance_t = void*(*)();
        auto getInstance = (get_Instance_t)(mod + Offsets::HostConfig::get_Instance);
        void* cfg = nullptr;
        __try { cfg = getInstance ? getInstance() : nullptr; } __except(EXCEPTION_EXECUTE_HANDLER) { cached = false; return; }
        if (!cfg) { cached = false; return; }

        __try {
            uint64_t gdInfo = *(uint64_t*)((uintptr_t)cfg + Offsets::HostConfig::GdInfo);
            if (!gdInfo) { cached = false; return; }
            uint64_t ops = *(uint64_t*)(gdInfo + Offsets::HostConfig::Operators);
            if (!ops) { cached = false; return; }

            *(float*)(ops + Offsets::HostConfig::OperatorsSection::RunSpeed) = oRun;
            *(float*)(ops + Offsets::HostConfig::OperatorsSection::SprintSpeed) = oSprint;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        cached = false;
    }

    inline void ApplyIfNeeded(bool inMatch) {
        if (!inMatch || !Config::movement_speed) return;

        uintptr_t mod = Memory::Get().GetGdiModule();
        if (!mod) return;

        __try {
            using get_Instance_t = void*(*)();
            auto getInstance = (get_Instance_t)(mod + Offsets::HostConfig::get_Instance);
            void* cfg = getInstance ? getInstance() : nullptr;
            if (!cfg) return;

            uint64_t gdInfo = *(uint64_t*)((uintptr_t)cfg + Offsets::HostConfig::GdInfo);
            if (!gdInfo) return;
            uint64_t ops = *(uint64_t*)(gdInfo + Offsets::HostConfig::Operators);
            if (!ops) return;

            if (!cached) {
                oRun = *(float*)(ops + Offsets::HostConfig::OperatorsSection::RunSpeed);
                oSprint = *(float*)(ops + Offsets::HostConfig::OperatorsSection::SprintSpeed);
                cached = true;
            }

            *(float*)(ops + Offsets::HostConfig::OperatorsSection::RunSpeed) = oRun * Config::movement_run_mult;
            *(float*)(ops + Offsets::HostConfig::OperatorsSection::SprintSpeed) = oSprint * Config::movement_sprint_mult;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}
