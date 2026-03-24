#pragma once
#include "framework.h"

// IL2CPP Combat Master Entities

class CPlayerConnectData {
public:
    std::wstring NickName() {
        IL2CPP::String* str = *(IL2CPP::String**)((uintptr_t)this + Offsets::PlayerConnectData::NickName);
        return str ? str->ToString() : L"Unknown";
    }

    int GetTeamId() {
        return IL2CPP::CallRVA<int>(Offsets::rva::get_TeamId, this);
    }
};

class Camera {
public:
    Matrix4x4 GetViewMatrix() {
        uintptr_t cachedPtr = *(uintptr_t*)((uintptr_t)this + Offsets::Object::cachedPtr);
        if (!cachedPtr) return {};
        return *(Matrix4x4*)(cachedPtr + Offsets::viewMatrix);
    }
};

class CameraController {
public:
    Camera* GetCamera() {
        return *(Camera**)((uintptr_t)this + 0x98); // CM Base Internal Offset
    }
};

class CPlayer {
public:
    bool isRealPlayer() { return *(bool*)((uintptr_t)this + Offsets::PlayerRoot::isRealPlayer); }
    bool isVisible() { return *(bool*)((uintptr_t)this + Offsets::PlayerRoot::isVisible); }
    
    float GetHealth() { return *(float*)((uintptr_t)this + Offsets::PlayerHealth::currentHealth); }
    bool isCrouch() { return *(bool*)((uintptr_t)this + Offsets::PlayerMovement::isCrouch); }

    CPlayerConnectData* GetConnectData() {
        return IL2CPP::CallRVA<CPlayerConnectData*>(Offsets::rva::get_PlayerConnectData, this);
    }

    Vector3 GetRootPosition() {
        uintptr_t transformStruct = *(uintptr_t*)((uintptr_t)this + Offsets::Transform::transformData);
        if (!transformStruct) return {};
        return *(Vector3*)(transformStruct + Offsets::TransformData::rootPosition);
    }

    CameraController* GetCameraController() {
        return *(CameraController**)((uintptr_t)this + 0x48); // CM Base Internal Offset
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

    static int GetAllPlayersCount() {
        void* sf = GetStaticFields();
        if (!sf) return 0;
        IL2CPP::List<CPlayer*>* listObj = *(IL2CPP::List<CPlayer*>**)((uintptr_t)sf + Offsets::PlayerRoot::allPlayers);
        if (!listObj) return 0;
        return listObj->GetSize();
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
