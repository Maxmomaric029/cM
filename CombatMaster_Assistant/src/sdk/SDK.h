#pragma once
#include "framework.h"

// IL2CPP Combat Master SDK - Fixed to match CM Base Internal reference

class UnityString {
public:
    char pad_0000[0x10];
    int length;
    wchar_t buffer[128];
    std::string ToString() {
        if (!this || length <= 0 || length > 128) return "Unknown";
        std::wstring ws(buffer, length);
        return std::string(ws.begin(), ws.end());
    }
    std::wstring ToWString() {
        if (!this || length <= 0 || length > 128) return L"Unknown";
        return std::wstring(buffer, length);
    }
};

class CPlayerConnectData {
public:
    UnityString* GetNickName() {
        return *(UnityString**)((uintptr_t)this + Offsets::PlayerConnectData::NickName);
    }
    std::wstring NickName() {
        UnityString* us = GetNickName();
        return us ? us->ToWString() : L"Unknown";
    }
    int GetTeamId() {
        return IL2CPP::CallRVA<int>(Offsets::rva::get_TeamId, this);
    }
};

class CNetPlayerData {
public:
    CPlayerConnectData* GetPlayerConnectData() {
        if (!this) return nullptr;
        return IL2CPP::CallRVA<CPlayerConnectData*>(Offsets::rva::get_PlayerConnectData, this);
    }
};

class Camera {
public:
    Matrix4x4 GetViewMatrix() {
        uintptr_t cachedPtr = *(uintptr_t*)((uintptr_t)this + Offsets::Object::cachedPtr);
        if (!cachedPtr) return {};
        return *(Matrix4x4*)(cachedPtr + Offsets::viewMatrix);
    }
    float GetFov() {
        uintptr_t cachedPtr = *(uintptr_t*)((uintptr_t)this + Offsets::Object::cachedPtr);
        if (!cachedPtr) return 0.f;
        typedef float(__fastcall* get_fov_t)(uintptr_t);
        get_fov_t fn = (get_fov_t)(Memory::Get().GetBaseAddress() + Offsets::rva::get_fieldOfView);
        if (fn) return fn(cachedPtr);
        return *(float*)(cachedPtr + 0x170);
    }
};

class CameraController {
public:
    Camera* GetCamera() {
        return *(Camera**)((uintptr_t)this + 0x98);
    }
};

class CPlayerHealth {
public:
    // Health is stored as int32_t, not float — critical fix from reference
    float GetHealthPercent() {
        int32_t currentHealthInt = *(int32_t*)((uintptr_t)this + Offsets::PlayerHealth::currentHealth);
        return static_cast<float>(currentHealthInt);
    }
    bool IsInvincible() {
        typedef bool(__fastcall* IsInvincible_t)(uintptr_t);
        static IsInvincible_t fn = nullptr;
        if (!fn) fn = (IsInvincible_t)(Memory::Get().GetBaseAddress() + Offsets::rva::get_IsInvincible);
        return fn ? fn((uintptr_t)this) : false;
    }
};

class CPlayerMovement {
public:
    bool IsCrouch() {
        return *(bool*)((uintptr_t)this + Offsets::PlayerMovement::isCrouch);
    }
};

class CTransformData {
public:
    Vector3 GetRootPosition() {
        return *(Vector3*)((uintptr_t)this + Offsets::TransformData::rootPosition);
    }
};

class CTransform {
public:
    uintptr_t GetCachedPtr() {
        return *(uintptr_t*)((uintptr_t)this + Offsets::Object::cachedPtr);
    }
    CTransformData* GetTransformData() {
        uintptr_t cachedPtr = GetCachedPtr();
        if (!cachedPtr) return nullptr;
        return *(CTransformData**)(cachedPtr + Offsets::Transform::transformData);
    }
    Vector3 GetPosition() {
        auto td = GetTransformData();
        if (!td) return {};
        return td->GetRootPosition();
    }
};

class CPlayer {
public:
    bool isRealPlayer() { return *(bool*)((uintptr_t)this + Offsets::PlayerRoot::isRealPlayer); }
    bool isVisible() { return *(bool*)((uintptr_t)this + Offsets::PlayerRoot::isVisible); }

    // --- Fixed: Health through PlayerHealth object at 0xB8, reading int32 ---
    CPlayerHealth* GetPlayerHealth() {
        return *(CPlayerHealth**)((uintptr_t)this + 0xB8);
    }
    float GetHealth() {
        CPlayerHealth* ph = GetPlayerHealth();
        if (!ph) return 0.f;
        return ph->GetHealthPercent();
    }
    bool IsDead() {
        return GetHealth() <= 0.f;
    }
    bool IsInvincible() {
        CPlayerHealth* ph = GetPlayerHealth();
        return ph ? ph->IsInvincible() : false;
    }

    // --- Fixed: isCrouch through PlayerMovement at 0xB0 ---
    CPlayerMovement* GetPlayerMovement() {
        return *(CPlayerMovement**)((uintptr_t)this + 0xB0);
    }
    bool isCrouch() {
        CPlayerMovement* pm = GetPlayerMovement();
        return pm ? pm->IsCrouch() : false;
    }

    // --- Fixed: GetRootPosition through neck transform at 0x30 ---
    CTransform* GetNeckTransform() {
        return *(CTransform**)((uintptr_t)this + 0x30);
    }
    Vector3 GetRootPosition() {
        CTransform* transform = GetNeckTransform();
        if (!transform) return {};
        auto td = transform->GetTransformData();
        if (!td) return {};
        return td->GetRootPosition();
    }
    Vector3 GetNeckPosition() {
        CTransform* transform = GetNeckTransform();
        if (!transform) return {};
        return transform->GetPosition();
    }

    // --- Net player data chain (reference pattern) ---
    CNetPlayerData* GetNetPlayerData() {
        return *(CNetPlayerData**)((uintptr_t)this + Offsets::PlayerRoot::cachedPlayerData);
    }
    CPlayerConnectData* GetConnectData() {
        CNetPlayerData* nd = GetNetPlayerData();
        if (!nd) return nullptr;
        return nd->GetPlayerConnectData();
    }

    int GetTeamId() {
        typedef int(__fastcall* t)(uintptr_t);
        static t fn = nullptr;
        if (!fn) fn = (t)(Memory::Get().GetBaseAddress() + Offsets::rva::get_TeamId);
        return fn ? fn((uintptr_t)this) : 0;
    }
    bool IsTeammate(CPlayer* localPlayer) {
        if (!localPlayer || localPlayer == this) return false;
        int myTeam = GetTeamId();
        int localTeam = localPlayer->GetTeamId();
        if (myTeam == 0) return false; // FFA or unassigned
        return myTeam == localTeam;
    }

    CameraController* GetCameraController() {
        return *(CameraController**)((uintptr_t)this + 0x48);
    }
    Camera* GetCamera() {
        CameraController* controller = GetCameraController();
        if (!controller) return nullptr;
        return controller->GetCamera();
    }
};

class CPlayerRoot {
public:
    static void* GetStaticFields() {
        uintptr_t klass = *(uintptr_t*)(Memory::Get().GetBaseAddress() + Offsets::playerRoot);
        if (!klass) return nullptr;
        return *(void**)(klass + Offsets::il2cppStaticField);
    }

    static CPlayer* GetLocalPlayer() {
        void* sf = GetStaticFields();
        if (!sf) return nullptr;
        return *(CPlayer**)((uintptr_t)sf + Offsets::PlayerRoot::localPlayer);
    }

    static CPlayer* GetSpectatorPlayer(CPlayer* localPlayer) {
        if (!localPlayer) return nullptr;
        return *(CPlayer**)((uintptr_t)localPlayer + Offsets::PlayerRoot::currentSpectatorPlayer);
    }

    static int GetAllPlayersCount() {
        void* sf = GetStaticFields();
        if (!sf) return 0;
        IL2CPP::List<CPlayer*>* listObj = *(IL2CPP::List<CPlayer*>**)((uintptr_t)sf + Offsets::PlayerRoot::allPlayers);
        if (!listObj) return 0;
        int n = listObj->GetSize();
        return (n < 0 || n > 256) ? 0 : n;
    }

    static IL2CPP::Array<CPlayer*>* GetAllPlayersArray() {
        void* sf = GetStaticFields();
        if (!sf) return nullptr;
        IL2CPP::List<CPlayer*>* listObj = *(IL2CPP::List<CPlayer*>**)((uintptr_t)sf + Offsets::PlayerRoot::allPlayers);
        if (!listObj) return nullptr;
        return listObj->GetItems();
    }
};

class CHostConfig {
public:
    static CHostConfig* GetInstance() {
        return IL2CPP::CallRVA<CHostConfig*>(Offsets::HostConfig::get_Instance);
    }
};

class CWeapon {
public:
    int GetUnChargedAmmoLeft() {
        return IL2CPP::CallRVA<int>(Offsets::weapon_rva::get_UnChargedAmmoLeft, this);
    }
    int GetChargedAmmoLeft() {
        return IL2CPP::CallRVA<int>(Offsets::weapon_rva::get_ChargedAmmoLeft, this);
    }
    float GetAdsPercent() {
        return IL2CPP::CallRVA<float>(Offsets::weapon_rva::get_AdsPercent, this);
    }
    bool IsUseCooldown() {
        return IL2CPP::CallRVA<bool>(Offsets::ShootWeapon_rva::get_IsUseCooldown, this);
    }
    void Use() {
        IL2CPP::CallRVA<void>(Offsets::weapon_rva::Use, this);
    }
};
