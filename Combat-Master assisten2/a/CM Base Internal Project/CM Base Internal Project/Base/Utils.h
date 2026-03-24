#pragma once
#include "Globals.h"
#include "Sdk.h"
#include "Offsets.h"
#include <vector>
#include <string>


inline bool WorldToScreen(const Vector3& position, Vector2* outPos, ViewMatrix viewMatrix) {
  float m00 = viewMatrix.matrix[0][0], m01 = viewMatrix.matrix[0][1],
        m02 = viewMatrix.matrix[0][2], m03 = viewMatrix.matrix[0][3];
  float m10 = viewMatrix.matrix[1][0], m11 = viewMatrix.matrix[1][1],
        m12 = viewMatrix.matrix[1][2], m13 = viewMatrix.matrix[1][3];
  float m20 = viewMatrix.matrix[2][0], m21 = viewMatrix.matrix[2][1],
        m22 = viewMatrix.matrix[2][2], m23 = viewMatrix.matrix[2][3];
  float m30 = viewMatrix.matrix[3][0], m31 = viewMatrix.matrix[3][1],
        m32 = viewMatrix.matrix[3][2], m33 = viewMatrix.matrix[3][3];

  float w = m03 * position.x + m13 * position.y + m23 * position.z + m33;

  if (w < 1.f)
    return false;

  float invW = 1.0f / w;

  float x = m00 * position.x + m10 * position.y + m20 * position.z + m30;
  float y = m01 * position.x + m11 * position.y + m21 * position.z + m31;

  float screenX = (Screen::ScreenCenter.x) * (1.f + x * invW);
  float screenY = (Screen::ScreenCenter.y) * (1.f - y * invW);

  outPos->x = screenX;
  outPos->y = screenY;

  return true;
}

inline std::vector<PlayerRoot*> GetEntities(bool includeDead = false, bool includeTeammates = true) {
  std::vector<PlayerRoot*> out;

  auto localPlayer = PlayerRoot::GetLocalPlayer();
  if (!localPlayer)
    return {};

  auto Players = PlayerRoot::GetAllPlayers();
  if (!Players)
    return {};

  int numPlayers = Players->Size();
  if (numPlayers < 0 || numPlayers > 256)
    numPlayers = 0;

  for (int i = 0; i < numPlayers; i++) {
    auto entity = Players->Get(i);
    if (!entity)
      continue;

    if (entity == localPlayer)
      continue;

    if (!includeTeammates && entity->IsTeammate())
      continue;

    if (!includeDead && entity->IsDead())
      continue;

    out.push_back(entity);
  }

  return out;
}

inline void GetEntitiesTo(std::vector<PlayerRoot*>& out, bool includeDead,
                          bool includeTeammates) {
  out.clear();
  auto localPlayer = PlayerRoot::GetLocalPlayer();
  if (!localPlayer)
    return;
  auto Players = PlayerRoot::GetAllPlayers();
  if (!Players)
    return;
  int numPlayers = Players->Size();
  if (numPlayers < 0 || numPlayers > 256)
    numPlayers = 0;
  out.reserve((size_t)numPlayers);
  for (int i = 0; i < numPlayers; i++) {
    auto entity = Players->Get(i);
    if (!entity || entity == localPlayer)
      continue;
    if (!includeTeammates && entity->IsTeammate())
      continue;
    if (!includeDead && entity->IsDead())
      continue;
    out.push_back(entity);
  }
}

inline Vector3 GetTargetBonePosition(PlayerRoot* player, int boneIdx) {
  Vector3 pos = player->GetRootPosition();
  if (boneIdx == 0)
    pos.y += 1.6f;
  else if (boneIdx == 1)
    pos.y += 1.35f;
  else if (boneIdx == 2)
    pos.y += 1.1f;
  else if (boneIdx == 3)
    pos.y += 0.8f;
  return pos;
}

inline PlayerRoot* ClosestInFOV(float fov, bool bVisCheck,
                        const std::vector<PlayerRoot*>& entities,
                        int boneOverride = -1) {
  auto localPlayer = PlayerRoot::GetLocalPlayer();
  if (!localPlayer)
    return nullptr;
  auto localCamera = localPlayer->GetCamera();
  if (!localCamera)
    return nullptr;
  PlayerRoot* bestPlayer = nullptr;
  float bestScore = 999999.f;
  for (auto& player : entities) {
    if (player == localPlayer) continue;

    if (Menu::bSkipSpawnProtection) {
      PlayerHealth* ph = player->GetPlayerHealth();
      if (ph) {
        typedef bool(__fastcall *IsInvincible_t)(uint64_t);
        static IsInvincible_t isInvFn = nullptr;
        if (!isInvFn)
          isInvFn = (IsInvincible_t)(Globals::ProjectModule + Offsets::rva::get_IsInvincible);
        if (isInvFn && isInvFn((uint64_t)ph))
          continue;
      }
    }

    if (bVisCheck && !player->IsVisible())
      continue;

    int targetBoneIdx = (boneOverride >= 0) ? boneOverride : Menu::aimbotBone;

    Vector3 targetPos = player->GetRootPosition();
    if (targetBoneIdx == 0)
      targetPos.y += 1.6f;
    else if (targetBoneIdx == 1)
      targetPos.y += 1.35f;
    else if (targetBoneIdx == 2)
      targetPos.y += 1.1f;
    else if (targetBoneIdx == 3)
      targetPos.y += 0.8f;

    Vector2 w2sPos;
    bool w2sValid = WorldToScreen(targetPos, &w2sPos, localCamera->GetViewMatrix());

    float fovDist = 9999.f;
    if (!w2sValid)
      continue;
    fovDist = Vector2::Distance(w2sPos, Screen::ScreenCenter);
    if (fovDist > fov)
      continue;

    float worldDist = Vector3::Distance(localPlayer->GetRootPosition(),
                                        player->GetRootPosition());

    float score = 0.f;
    if (Menu::aimbotTargeting == 0)
      score = fovDist;
    else if (Menu::aimbotTargeting == 1)
      score = worldDist;
    else if (Menu::aimbotTargeting == 2)
      score = fovDist + worldDist * 0.01f;

    if (score < bestScore) {
      bestPlayer = player;
      bestScore = score;
    }
  }

  return bestPlayer;
}

inline PlayerRoot* ClosestInFOV(float fov, bool bVisCheck = false,
                               int boneOverride = -1) {
  auto players = GetEntities(false, !Menu::bTeamCheck);
  return ClosestInFOV(fov, bVisCheck, players, boneOverride);
}
