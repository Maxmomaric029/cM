#pragma once
#include "Offsets.h"
#include "Types.h"
#include "Globals.h"
#include <windows.h>
#include <string>

template<typename T>
class List {
private:
    char pad_0x18[0x10];
    uint64_t* Data;
    int ListSize;
public:
    T* Get(int i) {
        if (!this || !Data || i < 0) return nullptr;
        int n = Size();
        if (i >= n) return nullptr;
        return reinterpret_cast<T*>(this->Data[4 + i]); // List starts at 0x20
    }
    int Size() {
        if (!this) return 0;
        int n = *(int*)((DWORD64)this + Offsets::List::listSize);
        return (n < 0 || n > 256) ? 0 : n;
    }
};

class Object {
public:
    uint64_t* GetCachedPtr() {
        return *(uint64_t**)((DWORD64)this + Offsets::Object::cachedPtr);
    }
};

class TransformData {
public:
    Vector3 GetRootPosition() {
        return *(Vector3*)((DWORD64)this + Offsets::TransformData::rootPosition);
    }
};

class Transform : public Object {
public:
    TransformData* GetTransformData() {
        uint64_t* cachedPtr = GetCachedPtr();
        if (!cachedPtr) return 0;
        return *(TransformData**)((DWORD64)cachedPtr + Offsets::Transform::transformData);
    }
    Vector3 get_PositionInjected() {
        auto transformData = GetTransformData();
        if (!transformData) return {};
        return transformData->GetRootPosition();
    }
};

class GameObject : public Object {};

class Camera : public GameObject {
public:
    ViewMatrix GetViewMatrix() {
        uint64_t* cachedPtr = GetCachedPtr();
        if (!cachedPtr) return {};
        return *(ViewMatrix*)((DWORD64)cachedPtr + Offsets::viewMatrix);
    }
    float GetFov() {
        uint64_t* cachedPtr = GetCachedPtr();
        if (!cachedPtr) return 0.f;
        typedef float(__fastcall *get_fov_t)(uint64_t);
        get_fov_t fn = (get_fov_t)(Globals::ProjectModule + Offsets::rva::get_fieldOfView);
        if (fn) return fn((uint64_t)*cachedPtr);
        return *(float*)((DWORD64)*cachedPtr + 0x170);
    }
};

class CameraController {
public:
    Camera* GetCamera() {
        return *(Camera**)((DWORD64)this + 0x98);
    }
};

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
};

class PlayerConnectData {
public:
    UnityString* GetNickName() {
        return *(UnityString**)((DWORD64)this + Offsets::PlayerConnectData::NickName);
    }
};

class NetPlayerData {
public:
    PlayerConnectData* GetPlayerConnectData() {
        if (!this) return nullptr;
        typedef PlayerConnectData*(*GetConnectDataFn)(NetPlayerData*);
        GetConnectDataFn getConnectData = (GetConnectDataFn)(Globals::ProjectModule + Offsets::rva::get_PlayerConnectData);
        return getConnectData(this);
    }
};

class PlayerHealth : public Object {
public:
    float GetHealthPercent() {
        int32_t currentHealthInt = *(int32_t*)((DWORD64)this + Offsets::PlayerHealth::currentHealth);
        return static_cast<float>(currentHealthInt);
    }
};

class PlayerMovement {
public:
    bool IsCrouch() {
        return *(bool*)((DWORD64)this + Offsets::PlayerMovement::isCrouch);
    }
};

class PlayerRoot : public GameObject {
private:
    static PlayerRoot* GetInstance() {
        if (!Globals::ProjectModule) return nullptr;
        uint64_t klass = *(uint64_t*)(Globals::ProjectModule + Offsets::playerRoot);
        if (!klass) return nullptr;
        return *(PlayerRoot**)(klass + Offsets::il2cppStaticField);
    }
public:
    static PlayerRoot* GetLocalPlayer() {
        PlayerRoot* inst = GetInstance();
        if (!inst) return nullptr;
        return *(PlayerRoot**)((DWORD64)inst + Offsets::PlayerRoot::localPlayer);
    }
    static List<PlayerRoot>* GetAllPlayers() {
        PlayerRoot* inst = GetInstance();
        if (!inst) return nullptr;
        return *(List<PlayerRoot>**)((DWORD64)inst + Offsets::PlayerRoot::allPlayers);
    }
    Vector3 GetNeckPosition() {
        Transform* neckTransform = *(Transform**)((DWORD64)this + 0x30);
        if (!neckTransform) return {};
        return neckTransform->get_PositionInjected();
    }
    Transform* GetNeckTransform() {
        return *(Transform**)((DWORD64)this + 0x30);
    }
    PlayerMovement* GetPlayerMovement() {
        return *(PlayerMovement**)((DWORD64)this + 0xB0);
    }
    int GetTeamId() {
        typedef int(__fastcall *t)(uintptr_t);
        static t fn = nullptr;
        if (!fn) fn = (t)(Globals::ProjectModule + Offsets::rva::get_TeamId);
        return fn ? fn((uintptr_t)this) : 0;
    }
    bool IsTeammate() {
        PlayerRoot* local = GetLocalPlayer();
        if (!local || local == this) return false;
        int myTeam = GetTeamId();
        int localTeam = local->GetTeamId();
        if (myTeam == 0) return false;
        return myTeam == localTeam;
    }
    bool IsRealPlayer() {
        return *(bool*)((DWORD64)this + Offsets::PlayerRoot::isRealPlayer);
    }
    bool IsVisible() {
        return *(bool*)((DWORD64)this + Offsets::PlayerRoot::isVisible);
    }
    PlayerHealth* GetPlayerHealth() {
        return *(PlayerHealth**)((DWORD64)this + 0xB8);
    }
    bool IsDead() {
        PlayerHealth* playerHealth = GetPlayerHealth();
        if (!playerHealth) return true;
        return playerHealth->GetHealthPercent() <= 0.0000f;
    }
    CameraController* GetCameraController() {
        return *(CameraController**)((DWORD64)this + 0x48);
    }
    NetPlayerData* GetNetPlayerData() {
        return *(NetPlayerData**)((DWORD64)this + Offsets::PlayerRoot::cachedPlayerData);
    }
    Vector3 GetRootPosition() {
        auto transform = this->GetNeckTransform();
        if (!transform) return {};
        auto transformData = transform->GetTransformData();
        if (!transformData) return {};
        return transformData->GetRootPosition();
    }
    Camera* GetCamera() {
        CameraController* cameraController = this->GetCameraController();
        if (!cameraController) return nullptr;
        Camera* camera = cameraController->GetCamera();
        if (!camera) return nullptr;
        return camera;
    }
};
