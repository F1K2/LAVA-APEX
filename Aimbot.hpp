// game/Aimbot.hpp - VERSION DEBUG MAXIMALE
#pragma once
#include "../Structs.hpp"
#include "../Memory/Interface.hpp"
#include "../globals.hpp"
#include "Offsets.hpp"
#include "Config.hpp"
#include "../OS-ImGui/imgui/imgui.h"
#include <Windows.h>
#include <cmath>
#include <algorithm>

namespace Aimbot {
    // Offsets importants pour l'aimbot
    constexpr uint64_t OFF_VIEW_ANGLES = 0x384;        // m_localAngles
    constexpr uint64_t OFF_VELOCITY = 0x378;        // m_vecVelocity
    constexpr uint64_t OFF_LAST_VISIBLE_TIME = 0x1a54;       // lastVisibleTime
    constexpr uint64_t OFF_GLOBAL_VARS = 0x1bd38a0;    // GlobalVars structure
    constexpr uint64_t OFF_BLEEDOUT_STATE = 0x2830;       // m_bleedoutState

    // FIXED: Fonction correcte pour récupérer une entité par ID
    inline uint64_t GetEntityById(int index, uint64_t base) {
        if (index < 0 || index >= 100) return 0;

        try {
            // Lire la liste d'entités
            const uint64_t entityList = I::Read<uint64_t>(base + OFF_ENTITY_LIST);
            if (!entityList) return 0;

            // Lire l'entité à l'index (chaque entrée fait 0x20 bytes)
            const uint64_t entity = I::Read<uint64_t>(entityList + (static_cast<uint64_t>(index) * 0x20));

            return entity;
        }
        catch (...) {
            return 0;
        }
    }

    // Conversion de BoneId vers les IDs corrects
    inline int GetTargetBoneId(int targetBone) {
        switch (targetBone) {
        case 0: return BONE_HEAD;    // Head
        case 1: return BONE_NECK;    // Neck
        case 2: return BONE_CHEST;   // Chest (default)
        default: return BONE_CHEST;
        }
    }

    // Get bone position pour l'aimbot
    constexpr uint64_t OFF_STUDIO_HDR = 0xff0;
    constexpr uint64_t BONE_MATRIX_OFFSET = 0x80;

    inline Vector3 GetAimbotBonePosition(uint64_t entity, int boneId) {
        if (!entity) return Vector3();

        try {
            // 1. Lire studioHdr (pointeur)
            uint64_t studioHdr = I::Read<uint64_t>(entity + OFF_STUDIO_HDR);
            if (!studioHdr) {
                return I::Read<Vector3>(entity + OFF_VecAbsOrigin);
            }

            // 2. Aller à la vraie bone matrix
            uint64_t boneMatrix = studioHdr + BONE_MATRIX_OFFSET;
            if (!boneMatrix) {
                return I::Read<Vector3>(entity + OFF_VecAbsOrigin);
            }

            // 3. Lire la position de l'os
            uint64_t boneAddr = boneMatrix + (boneId * 0x30);

            return Vector3(
                I::Read<float>(boneAddr + 0x0C),
                I::Read<float>(boneAddr + 0x1C),
                I::Read<float>(boneAddr + 0x2C)
            );
        }
        catch (...) {
            return I::Read<Vector3>(entity + OFF_VecAbsOrigin);
        }
    }

    // Calculate angle between two vectors
    inline Vector2 CalcAngle(const Vector3& from, const Vector3& to) {
        Vector3 delta;
        delta.x = to.x - from.x;
        delta.y = to.y - from.y;
        delta.z = to.z - from.z;

        const float hyp = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        Vector2 angle;
        angle.x = std::atan2(-delta.z, hyp) * (180.0f / 3.14159265f);
        angle.y = std::atan2(delta.y, delta.x) * (180.0f / 3.14159265f);

        return angle;
    }

    // Normalize angle to [-180, 180]
    inline float NormalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    // Calculate FOV distance
    inline float GetFov(const Vector2& viewAngles, const Vector2& aimAngles) {
        Vector2 delta;
        delta.x = NormalizeAngle(aimAngles.x - viewAngles.x);
        delta.y = NormalizeAngle(aimAngles.y - viewAngles.y);

        return std::sqrt(delta.x * delta.x + delta.y * delta.y);
    }

    // Check if entity is visible
    inline bool IsVisible(uint64_t entity) {
        if (!cfg.aimbotVisibleOnly) return true;

        try {
            const float lastVisTime = I::Read<float>(entity + OFF_LAST_VISIBLE_TIME);
            const float currentTime = I::Read<float>(GameBase + OFF_GLOBAL_VARS);
            return (currentTime - lastVisTime) < 2.0f;
        }
        catch (...) {
            return true;
        }
    }

    // Check if entity is knocked
    inline bool IsKnocked(uint64_t entity) {
        try {
            const int bleedoutState = I::Read<int>(entity + OFF_BLEEDOUT_STATE);
            return bleedoutState > 0;
        }
        catch (...) {
            return false;
        }
    }

    // Smooth angle interpolation
    inline Vector2 SmoothAngles(const Vector2& current, const Vector2& target, float smoothness) {
        if (smoothness < 1.0f) smoothness = 1.0f;

        Vector2 delta;
        delta.x = NormalizeAngle(target.x - current.x);
        delta.y = NormalizeAngle(target.y - current.y);

        Vector2 result;
        result.x = current.x + delta.x / smoothness;
        result.y = current.y + delta.y / smoothness;

        return result;
    }

    // ULTRA DEBUG: Find best target avec TOUS les détails
    inline uint64_t FindBestTarget(uint64_t localPlayer, const Vector3& localPos,
        const Vector2& viewAngles, float& outDistance) {

        printf("\n========================================\n");
        printf("AIMBOT SCAN START\n");
        printf("========================================\n");

        if (!localPlayer) {
            printf("ERROR: No local player!\n");
            return 0;
        }
        printf("LocalPlayer: 0x%llX\n", localPlayer);

        // Lire EntityList
        const uint64_t entityList = I::Read<uint64_t>(GameBase + OFF_ENTITY_LIST);
        printf("EntityList: 0x%llX\n", entityList);
        if (!entityList) {
            printf("ERROR: EntityList is NULL!\n");
            return 0;
        }

        const int localTeam = I::Read<int>(localPlayer + OFF_iTeamNum);
        printf("Local Team: %d\n", localTeam);
        printf("Local Pos: (%.1f, %.1f, %.1f)\n", localPos.x, localPos.y, localPos.z);
        printf("\nSettings:\n");
        printf("- FOV: %.0f°\n", cfg.aimbotFOV);
        printf("- Max Distance: %.0fm\n", cfg.maxDistance);
        printf("- Team Check: %s\n", cfg.aimbotTeamCheck ? "ON" : "OFF");
        printf("- Visibility Check: %s\n", cfg.aimbotVisibleOnly ? "ON" : "OFF");
        printf("- Target Bone: %d\n", cfg.targetBone);
        printf("\n");

        uint64_t bestTarget = 0;
        float bestFov = cfg.aimbotFOV;
        outDistance = 0.0f;

        int totalScanned = 0;
        int validEntities = 0;
        int sameTeam = 0;
        int dead = 0;
        int knocked = 0;
        int notVisible = 0;
        int tooFar = 0;
        int outsideFOV = 0;

        // Scanner TOUS les slots
        for (int i = 0; i < 100; i++) {
            const uint64_t entity = GetEntityById(i, GameBase);
            if (!entity || entity == 0) continue;

            totalScanned++;

            // Ne pas viser soi-même
            if (entity == localPlayer) {
                printf("Slot %d: SELF (skip)\n", i);
                continue;
            }

            // Team check
            int entityTeam = -1;
            try {
                entityTeam = I::Read<int>(entity + OFF_iTeamNum);
            }
            catch (...) {
                printf("Slot %d: FAILED to read team\n", i);
                continue;
            }

            if (cfg.aimbotTeamCheck && entityTeam == localTeam) {
                sameTeam++;
                continue;
            }

            // Alive check
            int lifeState = -1;
            try {
                lifeState = I::Read<int>(entity + OFF_lifeState);
            }
            catch (...) {
                printf("Slot %d: FAILED to read lifeState\n", i);
                continue;
            }

            if (lifeState != 0) {
                dead++;
                continue;
            }

            // Knocked check
            if (IsKnocked(entity)) {
                knocked++;
                continue;
            }

            validEntities++;

            // Get position
            Vector3 entityPos;
            try {
                entityPos = I::Read<Vector3>(entity + OFF_VecAbsOrigin);
            }
            catch (...) {
                printf("Slot %d: FAILED to read position\n", i);
                continue;
            }

            // Distance check
            const float distance = localPos.Distance(entityPos) / 39.37f;

            printf("Slot %d: VALID - Team=%d, Dist=%.1fm, Pos=(%.0f,%.0f,%.0f)\n",
                i, entityTeam, distance, entityPos.x, entityPos.y, entityPos.z);

            if (distance > cfg.maxDistance) {
                printf("  -> TOO FAR (max: %.0fm)\n", cfg.maxDistance);
                tooFar++;
                continue;
            }

            if (distance < 1.0f) {
                printf("  -> TOO CLOSE\n");
                continue;
            }

            // Visibility check
            if (!IsVisible(entity)) {
                printf("  -> NOT VISIBLE\n");
                notVisible++;
                continue;
            }

            // Get target bone
            const int targetBoneId = GetTargetBoneId(cfg.targetBone);
            const Vector3 bonePos = GetAimbotBonePosition(entity, targetBoneId);

            if (bonePos.x == 0.0f && bonePos.y == 0.0f && bonePos.z == 0.0f) {
                printf("  -> INVALID BONE POSITION\n");
                continue;
            }

            printf("  -> Bone Pos: (%.0f,%.0f,%.0f)\n", bonePos.x, bonePos.y, bonePos.z);

            // Calculate FOV
            const Vector2 aimAngles = CalcAngle(localPos, bonePos);
            const float fov = GetFov(viewAngles, aimAngles);

            printf("  -> FOV: %.2f° (max: %.0f°)\n", fov, cfg.aimbotFOV);

            if (fov > cfg.aimbotFOV) {
                printf("  -> OUTSIDE FOV\n");
                outsideFOV++;
                continue;
            }

            if (fov < bestFov) {
                bestFov = fov;
                bestTarget = entity;
                outDistance = distance;
                printf("  -> ★★★ NEW BEST TARGET! ★★★\n");
            }
        }

        printf("\n========================================\n");
        printf("SCAN RESULTS:\n");
        printf("========================================\n");
        printf("Total Scanned: %d\n", totalScanned);
        printf("Valid Entities: %d\n", validEntities);
        printf("Rejected:\n");
        printf("  - Same Team: %d\n", sameTeam);
        printf("  - Dead: %d\n", dead);
        printf("  - Knocked: %d\n", knocked);
        printf("  - Not Visible: %d\n", notVisible);
        printf("  - Too Far: %d\n", tooFar);
        printf("  - Outside FOV: %d\n", outsideFOV);
        printf("\n");

        if (bestTarget) {
            printf("✓ BEST TARGET FOUND!\n");
            printf("  - Distance: %.2fm\n", outDistance);
            printf("  - FOV: %.2f°\n", bestFov);
        }
        else {
            printf("✗ NO TARGET FOUND\n");
            printf("\nTROUBLESHOOTING:\n");
            if (totalScanned == 0) {
                printf("  ! No entities scanned - Check if you're in a game\n");
            }
            if (validEntities == 0 && totalScanned > 0) {
                printf("  ! All entities rejected - Try disabling filters:\n");
                printf("    - Set aimbotTeamCheck = false\n");
                printf("    - Set aimbotVisibleOnly = false\n");
            }
            if (tooFar > 0) {
                printf("  ! %d targets too far - Increase maxDistance\n", tooFar);
            }
            if (outsideFOV > 0) {
                printf("  ! %d targets outside FOV - Increase aimbotFOV\n", outsideFOV);
            }
        }
        printf("========================================\n\n");

        return bestTarget;
    }

    // Aim at target
    inline void AimAtTarget(uint64_t localPlayer, uint64_t target) {
        if (!localPlayer || !target) return;

        try {
            const Vector3 localPos = I::Read<Vector3>(localPlayer + OFF_VecAbsOrigin);
            const int targetBoneId = GetTargetBoneId(cfg.targetBone);
            const Vector3 targetPos = GetAimbotBonePosition(target, targetBoneId);

            if (targetPos.x == 0.0f && targetPos.y == 0.0f && targetPos.z == 0.0f) {
                return;
            }

            const Vector2 targetAngles = CalcAngle(localPos, targetPos);
            const uint64_t viewAnglesPtr = localPlayer + OFF_VIEW_ANGLES;

            Vector2 currentAngles;
            currentAngles.x = I::Read<float>(viewAnglesPtr);
            currentAngles.y = I::Read<float>(viewAnglesPtr + 0x4);

            Vector2 finalAngles = SmoothAngles(currentAngles, targetAngles, cfg.aimbotSmooth);
            finalAngles.x = std::clamp(finalAngles.x, -89.0f, 89.0f);
            finalAngles.y = NormalizeAngle(finalAngles.y);

            I::Write<float>(viewAnglesPtr, finalAngles.x);
            I::Write<float>(viewAnglesPtr + 0x4, finalAngles.y);
        }
        catch (...) {
            // Silent fail
        }
    }

    // Main aimbot update
    inline void Update() {
        if (!cfg.aimbotEnabled) return;

        // Check aim key
        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            return;
        }

        try {
            const uint64_t localPlayer = I::Read<uint64_t>(GameBase + OFF_LOCAL_PLAYER);
            if (!localPlayer) {
                printf("Aimbot: No local player\n");
                return;
            }

            const int lifeState = I::Read<int>(localPlayer + OFF_lifeState);
            if (lifeState != 0) return;

            const Vector3 localPos = I::Read<Vector3>(localPlayer + OFF_VecAbsOrigin);
            const uint64_t viewAnglesPtr = localPlayer + OFF_VIEW_ANGLES;

            Vector2 viewAngles;
            viewAngles.x = I::Read<float>(viewAnglesPtr);
            viewAngles.y = I::Read<float>(viewAnglesPtr + 0x4);

            float targetDistance = 0.0f;
            const uint64_t target = FindBestTarget(localPlayer, localPos, viewAngles, targetDistance);

            if (target) {
                AimAtTarget(localPlayer, target);
            }
        }
        catch (...) {
            printf("Aimbot: Exception caught\n");
        }
    }

    // Draw FOV circle
    inline void DrawFOVCircle() {
        if (!cfg.aimbotEnabled) return;

        const float screenCenterX = ScreenSize.x / 2.0f;
        const float screenCenterY = ScreenSize.y / 2.0f;
        const float radius = (cfg.aimbotFOV / 90.0f) * (ScreenSize.y / 4.0f);

        ImGui::GetForegroundDrawList()->AddCircle(
            ImVec2(screenCenterX, screenCenterY),
            radius,
            IM_COL32(255, 255, 255, 80),
            64,
            1.0f
        );
    }
}