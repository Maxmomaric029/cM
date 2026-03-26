#pragma once
#include "framework.h"

// ─────────────────────────────────────────────────────────────────────────────
// Combat Master External SDK
// All reads go through Memory::Get().Read<T>(address)
// NO IL2CPP::CallRVA — function calls on the game process are not possible externally.
// ─────────────────────────────────────────────────────────────────────────────

// Helper macro for brevity
#define RPM(addr, T)   (Memory::Get().Read<T>(addr))
#define RPTR(addr)     (Memory::Get().Read<uintptr_t>(addr))

class UnityString {
public:
    // Remote read: given the pointer to the UnityString object in game memory,
    // read its length + wchar buffer.
    static std::string ReadFrom(uintptr_t ptr) {
        if (!ptr) return "Unknown";
        int length = RPM(ptr + 0x10, int);
        if (length <= 0 || length > 128) return "Unknown";
        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; i++) {
            wchar_t ch = RPM(ptr + 0x14 + i * sizeof(wchar_t), wchar_t);
            result += static_cast<char>(ch);
        }
        return result;
    }
};

class CPlayerConnectData {
    uintptr_t self;
public:
    explicit CPlayerConnectData(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    std::string GetNickName() {
        // NickName is a UnityString* at self + Offsets::PlayerConnectData::NickName
        uintptr_t usPtr = RPTR(self + Offsets::PlayerConnectData::NickName);
        return UnityString::ReadFrom(usPtr);
    }

    int GetTeamId() {
        // Read team id via known offset instead of RVA call
        // TeamId is typically stored at a fixed offset — if unknown, return 0
        return RPM(self + 0x20, int);
    }
};

class CNetPlayerData {
    uintptr_t self;
public:
    explicit CNetPlayerData(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    CPlayerConnectData GetPlayerConnectData() {
        // Follow pointer chain via RPM
        uintptr_t cdAddr = RPTR(self + 0x18);
        return CPlayerConnectData(cdAddr);
    }
};

class Camera {
    uintptr_t self;
public:
    explicit Camera(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    Matrix4x4 GetViewMatrix() {
        uintptr_t cachedPtr = RPTR(self + Offsets::Object::cachedPtr);
        if (!cachedPtr) return {};
        return RPM(cachedPtr + Offsets::viewMatrix, Matrix4x4);
    }
    float GetFov() {
        uintptr_t cachedPtr = RPTR(self + Offsets::Object::cachedPtr);
        if (!cachedPtr) return 0.f;
        return RPM(cachedPtr + 0x170, float);
    }
};

class CameraController {
    uintptr_t self;
public:
    explicit CameraController(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    Camera GetCamera() {
        return Camera(RPTR(self + 0x98));
    }
};

class CPlayerHealth {
    uintptr_t self;
public:
    explicit CPlayerHealth(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    float GetHealthPercent() {
        int32_t hp = RPM(self + Offsets::PlayerHealth::currentHealth, int32_t);
        return static_cast<float>(hp);
    }
    bool IsInvincible() {
        // Read field at known offset (no RVA call available externally)
        return RPM(self + 0xD8, bool);
    }
};

class CPlayerMovement {
    uintptr_t self;
public:
    explicit CPlayerMovement(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }
    bool IsCrouch() { return RPM(self + Offsets::PlayerMovement::isCrouch, bool); }
};

class CTransformData {
    uintptr_t self;
public:
    explicit CTransformData(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }
    Vector3 GetRootPosition() { return RPM(self + Offsets::TransformData::rootPosition, Vector3); }
};

class CTransform {
    uintptr_t self;
public:
    explicit CTransform(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }

    uintptr_t GetCachedPtr() { return RPTR(self + Offsets::Object::cachedPtr); }

    CTransformData GetTransformData() {
        uintptr_t cachedPtr = GetCachedPtr();
        if (!cachedPtr) return CTransformData(0);
        return CTransformData(RPTR(cachedPtr + Offsets::Transform::transformData));
    }
    Vector3 GetPosition() {
        auto td = GetTransformData();
        if (!td.Valid()) return {};
        return td.GetRootPosition();
    }
};

class CPlayer {
    uintptr_t self;
public:
    explicit CPlayer(uintptr_t addr) : self(addr) {}
    bool Valid() const { return self != 0; }
    uintptr_t GetAddress() const { return self; }

    bool isRealPlayer() { return RPM(self + Offsets::PlayerRoot::isRealPlayer, bool); }
    bool isVisible()    { return RPM(self + Offsets::PlayerRoot::isVisible,    bool); }

    CPlayerHealth GetPlayerHealth() {
        return CPlayerHealth(RPTR(self + 0xB8));
    }
    float GetHealth() {
        auto ph = GetPlayerHealth();
        return ph.Valid() ? ph.GetHealthPercent() : 0.f;
    }
    bool IsDead()       { return GetHealth() <= 0.f; }
    bool IsInvincible() { auto ph = GetPlayerHealth(); return ph.Valid() && ph.IsInvincible(); }

    CPlayerMovement GetPlayerMovement() {
        return CPlayerMovement(RPTR(self + 0xB0));
    }
    bool isCrouch() {
        auto pm = GetPlayerMovement();
        return pm.Valid() ? pm.IsCrouch() : false;
    }

    CTransform GetNeckTransform() { return CTransform(RPTR(self + 0x30)); }
    Vector3 GetRootPosition() {
        auto t = GetNeckTransform();
        if (!t.Valid()) return {};
        auto td = t.GetTransformData();
        return td.Valid() ? td.GetRootPosition() : Vector3{};
    }
    Vector3 GetNeckPosition() {
        auto t = GetNeckTransform();
        return t.Valid() ? t.GetPosition() : Vector3{};
    }

    CNetPlayerData GetNetPlayerData() {
        return CNetPlayerData(RPTR(self + Offsets::PlayerRoot::cachedPlayerData));
    }
    CPlayerConnectData GetConnectData() {
        return GetNetPlayerData().GetPlayerConnectData();
    }

    int GetTeamId() {
        // Read directly from field rather than via RVA call
        return RPM(self + 0x158, int);
    }
    bool IsTeammate(CPlayer& localPlayer) {
        if (!localPlayer.Valid() || self == localPlayer.GetAddress()) return false;
        int myTeam    = GetTeamId();
        int localTeam = localPlayer.GetTeamId();
        if (myTeam == 0) return false;
        return myTeam == localTeam;
    }

    CameraController GetCameraController() {
        return CameraController(RPTR(self + 0x48));
    }
    Camera GetCamera() { return GetCameraController().GetCamera(); }
};

// ─── Static player root helpers ───────────────────────────────────────────────
namespace CPlayerRoot {
    inline uintptr_t GetStaticFieldsAddr() {
        uintptr_t klass = RPTR(Memory::Get().GetBaseAddress() + Offsets::playerRoot);
        if (!klass) return 0;
        return RPTR(klass + Offsets::il2cppStaticField);
    }

    inline CPlayer GetLocalPlayer() {
        uintptr_t sf = GetStaticFieldsAddr();
        if (!sf) return CPlayer(0);
        return CPlayer(RPTR(sf + Offsets::PlayerRoot::localPlayer));
    }

    inline CPlayer GetSpectatorPlayer(CPlayer& localPlayer) {
        if (!localPlayer.Valid()) return CPlayer(0);
        return CPlayer(RPTR(localPlayer.GetAddress() + Offsets::PlayerRoot::currentSpectatorPlayer));
    }

    inline int GetAllPlayersCount() {
        uintptr_t sf = GetStaticFieldsAddr();
        if (!sf) return 0;
        uintptr_t listPtr = RPTR(sf + Offsets::PlayerRoot::allPlayers);
        if (!listPtr) return 0;
        int n = IL2CPP::List<uintptr_t>::GetSize(listPtr);
        return (n < 0 || n > 256) ? 0 : n;
    }

    // Returns the pointer to the IL2CPP items array (not dereferenced — callers use GetElement)
    inline uintptr_t GetAllPlayersListPtr() {
        uintptr_t sf = GetStaticFieldsAddr();
        if (!sf) return 0;
        return RPTR(sf + Offsets::PlayerRoot::allPlayers);
    }

    inline CPlayer GetPlayerAt(int index) {
        uintptr_t listPtr = GetAllPlayersListPtr();
        uintptr_t itemsPtr = IL2CPP::List<uintptr_t>::GetItemsPtr(listPtr);
        uintptr_t playerAddr = IL2CPP::Array<uintptr_t>::GetElement(itemsPtr, index);
        return CPlayer(playerAddr);
    }
}
