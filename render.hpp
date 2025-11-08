#pragma once
#include "../Structs.hpp"
#include "../OS-ImGui/imgui/imgui.h"
#include <string>

// Forward declarations
struct Config;
struct Matrix;

// Function declarations
void RenderESP();

void RenderPlayerESP(ImDrawList* drawList, uint64_t entity, const Vector3& screenPos,
    const Vector3& screenHeadPos, float boxWidth, float boxHeight,
    float centerX, const Vector3& localPos, const Matrix& viewMatrix);

void RenderBars(ImDrawList* drawList, uint64_t entity, float centerX, float boxWidth, float startY);

// Helper function to get bone positions
Vector3 GetBonePosition(uint64_t entity, int boneId);