#include "render.hpp"
#include "../Entity.hpp"
#include "ESP.hpp"
#include "Config.hpp"
#include "Offsets.hpp"
#include "../globals.hpp"
#include <string>
#include <unordered_map>
#include <algorithm>

// Variables globales
uint64_t GameBase = 0;
HWND hwnd = nullptr;
Vector2 ScreenSize = { 1920.0f, 1080.0f };
bool isRunning = true;

class PlayerManager {
private:
    static constexpr float NAME_CACHE_TIMEOUT = 5.0f;
    static constexpr size_t MAX_NAME_CACHE = 200;

    struct NameCacheEntry {
        std::string name;
        float timestamp;
    };

    std::unordered_map<uint64_t, NameCacheEntry> nameCache;
    float lastCleanupTime = 0.0f;

public:
    static PlayerManager& Get() {
        static PlayerManager instance;
        return instance;
    }

    std::string GetPlayerName(uint64_t entity) {
        if (!entity) return "Unknown";

        const float currentTime = GetTickCount64() / 1000.0f;

        if (currentTime - lastCleanupTime > 10.0f) {
            CleanupOldEntries(currentTime);
            lastCleanupTime = currentTime;
        }

        auto it = nameCache.find(entity);
        if (it != nameCache.end() && (currentTime - it->second.timestamp) < NAME_CACHE_TIMEOUT) {
            return it->second.name;
        }

        std::string name = ReadPlayerNameFromMemory(entity);
        nameCache[entity] = { name, currentTime };

        if (nameCache.size() > MAX_NAME_CACHE) {
            nameCache.erase(nameCache.begin());
        }

        return name;
    }

    bool IsValidPlayer(uint64_t entity, uint64_t localPlayer, int localTeam) {
        if (!entity || !localPlayer || entity == localPlayer) {
            return false;
        }

        const int entityTeam = I::Read<int>(entity + OFF_iTeamNum);
        const int lifeState = I::Read<int>(entity + OFF_lifeState);

        if (!cfg.espTeamCheck && entityTeam == localTeam) {
            return false;
        }

        return lifeState == 0;
    }

    float CalculateDistance(const Vector3& pos1, const Vector3& pos2) {
        return pos1.Distance(pos2) / 39.37f;
    }

private:
    std::string ReadPlayerNameFromMemory(uint64_t entity) {
        try {
            const int nameIndex = I::Read<int>(entity + OFF_NameIndex);

            if (nameIndex < 1 || nameIndex > 10000) {
                return "Player";
            }

            const uint64_t nameList = GameBase + OFF_NAME_LIST;
            const uint64_t nameAddress = I::Read<uint64_t>(nameList + (static_cast<uint64_t>(nameIndex) * 0x10));

            if (!nameAddress) {
                return "Player";
            }

            char nameBuffer[128] = { 0 };
            I::ReadBytes(nameAddress, reinterpret_cast<unsigned char*>(nameBuffer), sizeof(nameBuffer) - 1);

            return SanitizeString(nameBuffer);
        }
        catch (...) {
            return "Error";
        }
    }

    std::string SanitizeString(const char* input) {
        std::string result;
        for (int i = 0; i < 128 && input[i] != '\0'; i++) {
            if (input[i] >= 32 && input[i] <= 126) {
                result += input[i];
            }
            else {
                break;
            }
        }
        return result.empty() ? "Player" : result;
    }

    void CleanupOldEntries(float currentTime) {
        for (auto it = nameCache.begin(); it != nameCache.end(); ) {
            if (currentTime - it->second.timestamp > NAME_CACHE_TIMEOUT) {
                it = nameCache.erase(it);
            }
            else {
                ++it;
            }
        }
    }
};

// Get bone position in 3D world space
Vector3 GetBonePosition(uint64_t entity, int boneId) {
    const uint64_t boneMatrix = I::Read<uint64_t>(entity + OFF_BoneMatrix);
    if (!boneMatrix) return Vector3();

    const uint64_t boneAddress = boneMatrix + (boneId * 0x30);

    Vector3 bonePos;
    bonePos.x = I::Read<float>(boneAddress + 0x0C);
    bonePos.y = I::Read<float>(boneAddress + 0x1C);
    bonePos.z = I::Read<float>(boneAddress + 0x2C);

    return bonePos;
}

void RenderESP() {
    if (!cfg.enableESP) return;

    const uint64_t viewRender = I::Read<uint64_t>(GameBase + OFF_VIEW_RENDER);
    if (!viewRender) return;

    const uint64_t viewMatrixPtr = I::Read<uint64_t>(viewRender + OFF_VIEW_MATRIX);
    if (!viewMatrixPtr) return;

    const Matrix viewMatrix = I::Read<Matrix>(viewMatrixPtr);
    if (!viewMatrix.IsValid()) return;

    const uint64_t localPlayer = I::Read<uint64_t>(GameBase + OFF_LOCAL_PLAYER);
    if (!localPlayer) return;

    const int localTeam = I::Read<int>(localPlayer + OFF_iTeamNum);
    const Vector3 localPos = cfg.espDistance ?
        I::Read<Vector3>(localPlayer + OFF_VecAbsOrigin) : Vector3();

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    auto& playerManager = PlayerManager::Get();

    int renderedPlayers = 0;

    for (int i = 0; i < cfg.maxPlayers && renderedPlayers < cfg.maxPlayers; i++) {
        const uint64_t entity = GetEntityById(i, GameBase);
        if (!entity) continue;

        if (!playerManager.IsValidPlayer(entity, localPlayer, localTeam)) {
            continue;
        }

        const Vector3 entityPos = I::Read<Vector3>(entity + OFF_VecAbsOrigin);
        const Vector3 headPos = entityPos + Vector3(0, 0, 70.0f);

        const Vector3 screenPos = WorldToScreen(entityPos, viewMatrix, ScreenSize);
        const Vector3 screenHeadPos = WorldToScreen(headPos, viewMatrix, ScreenSize);

        if (screenPos.z <= 0.0f || screenHeadPos.z <= 0.0f) {
            continue;
        }

        if (cfg.distanceLimit && cfg.espDistance) {
            const float distance = playerManager.CalculateDistance(entityPos, localPos);
            if (distance > cfg.espMaxDistance) {
                continue;
            }
        }

        const float boxHeight = screenPos.y - screenHeadPos.y;
        if (boxHeight <= 0.0f) continue;

        const float boxWidth = boxHeight * 0.4f;
        const float centerX = screenHeadPos.x;

        RenderPlayerESP(drawList, entity, screenPos, screenHeadPos, boxWidth, boxHeight,
            centerX, localPos, viewMatrix);

        renderedPlayers++;
    }
}

void RenderPlayerESP(ImDrawList* drawList, uint64_t entity, const Vector3& screenPos,
    const Vector3& screenHeadPos, float boxWidth, float boxHeight,
    float centerX, const Vector3& localPos, const Matrix& viewMatrix) {

    auto& playerManager = PlayerManager::Get();
    const float alpha = cfg.espAlpha;

    // Apply alpha to color
    ImColor boxColor = ImGui::ColorConvertFloat4ToU32(cfg.boxColor);
    boxColor.Value.w *= alpha;

    // Snaplines (dessiner en premier pour qu'elles soient en arrière-plan)
    if (cfg.snaplines) {
        float startY;
        switch (cfg.snaplinePosition) {
        case 0: // Bottom
            startY = ScreenSize.y;
            break;
        case 1: // Center
            startY = ScreenSize.y / 2.0f;
            break;
        case 2: // Top
            startY = 0.0f;
            break;
        default:
            startY = ScreenSize.y / 2.0f;
        }

        ImColor snapColor = ImGui::ColorConvertFloat4ToU32(cfg.snaplineColor);
        snapColor.Value.w *= alpha;

        ESP::DrawSnapline(ScreenSize.x / 2.0f, startY, centerX, screenPos.y,
            snapColor, cfg.snaplineThickness);
    }

    // Skeleton
    if (cfg.showSkeleton) {
        ImVec2 bones[16];
        int boneIds[] = {
            BONE_HEAD, BONE_NECK, BONE_CHEST, BONE_PELVIS,
            BONE_L_SHOULDER, BONE_L_ELBOW, BONE_L_HAND,
            BONE_R_SHOULDER, BONE_R_ELBOW, BONE_R_HAND,
            BONE_L_HIP, BONE_L_KNEE, BONE_L_FOOT,
            BONE_R_HIP, BONE_R_KNEE, BONE_R_FOOT
        };

        bool validSkeleton = true;
        for (int i = 0; i < 16; i++) {
            Vector3 bonePos = GetBonePosition(entity, boneIds[i]);
            Vector3 screenBone = WorldToScreen(bonePos, viewMatrix, ScreenSize);

            if (screenBone.z <= 0.0f) {
                validSkeleton = false;
                break;
            }

            bones[i] = ImVec2(screenBone.x, screenBone.y);
        }

        if (validSkeleton) {
            ImColor skelColor = ImGui::ColorConvertFloat4ToU32(cfg.skeletonColor);
            skelColor.Value.w *= alpha;
            ESP::DrawSkeleton(drawList, bones, 16, skelColor, cfg.skeletonThickness);
        }
    }

    // Box (selon le type choisi)
    if (cfg.showBoxes) {
        const int x = static_cast<int>(screenHeadPos.x - (boxWidth / 2));
        const int y = static_cast<int>(screenHeadPos.y);
        const int w = static_cast<int>(boxWidth);
        const int h = static_cast<int>(boxHeight);

        switch (cfg.boxType) {
        case 0: // Cornered
            ESP::DrawCorneredBox(x, y, w, h, boxColor, cfg.boxThickness);
            break;
        case 1: // Full
            ESP::DrawFullBox(x, y, w, h, boxColor, cfg.boxThickness);
            break;
        case 2: // Filled
            ESP::DrawFilledBox(x, y, w, h, boxColor, cfg.boxThickness);
            break;
        case 3: // 3D
            ESP::Draw3DBox(x, y, w, h, boxColor, cfg.boxThickness);
            break;
        default:
            ESP::DrawCorneredBox(x, y, w, h, boxColor, cfg.boxThickness);
        }
    }

    // Head dot (plus discret)
    if (cfg.headdot) {
        ImColor dotColor = ImColor(255, 255, 255);
        dotColor.Value.w *= alpha * 0.8f;
        ESP::DrawFilledCircle(screenHeadPos.x, screenHeadPos.y, dotColor, 2.0f);
    }

    // Name
    if (cfg.espName) {
        const std::string playerName = playerManager.GetPlayerName(entity);
        const ImVec2 textSize = ImGui::CalcTextSize(playerName.c_str());
        const float textX = centerX - (textSize.x / 2.0f);
        ESP::DrawPlayerName(drawList, textX, screenHeadPos.y - 15.0f, playerName.c_str(), alpha);
    }

    // Health and shield bars
    RenderBars(drawList, entity, centerX, boxWidth, screenHeadPos.y + 6.0f);

    // Distance
    if (cfg.espDistance) {
        const float distance = playerManager.CalculateDistance(
            I::Read<Vector3>(entity + OFF_VecAbsOrigin), localPos);

        const ImVec2 textSize = ImGui::CalcTextSize("000m");
        const float textX = screenPos.x - (textSize.x / 2.0f);
        ESP::DrawDistance(drawList, textX, screenPos.y + 5.0f, distance);
    }
}

void RenderBars(ImDrawList* drawList, uint64_t entity, float centerX, float boxWidth, float startY) {
    if (!cfg.espHealth && !cfg.showShieldBar) return;

    const float barWidth = boxWidth * 0.75f; // Réduit pour plus de discrétion
    const float barHeight = cfg.reducedEffects ? 3.0f : 4.0f;
    const float startX = centerX - (barWidth / 2.0f);
    float currentY = startY;

    // Health bar
    if (cfg.espHealth) {
        const int health = I::Read<int>(entity + OFF_iHealth);
        const int maxHealth = I::Read<int>(entity + OFF_iMaxHealth);

        if (health > 0) {
            ESP::DrawHealthBar(drawList, startX, currentY, barWidth, barHeight,
                static_cast<float>(health), static_cast<float>(maxHealth), cfg.espAlpha);
            currentY += barHeight + 2.0f;
        }
    }

    // Shield bar
    if (cfg.showShieldBar) {
        const int shield = I::Read<int>(entity + OFF_shieldHealth);
        const int maxShield = I::Read<int>(entity + OFF_shieldHealthMax);

        if (shield > 0) {
            ESP::DrawShieldBar(drawList, startX, currentY, barWidth, barHeight,
                static_cast<float>(shield), static_cast<float>(maxShield), cfg.espAlpha);
        }
    }
}