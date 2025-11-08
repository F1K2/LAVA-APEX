#pragma once
#include "../OS-ImGui/imgui/imgui.h"
#include <algorithm>
#include <cmath>

namespace ESP {
    // Box Types
    inline void DrawCorneredBox(int x, int y, int w, int h, const ImColor& color, float thickness)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        const float thirdW = w / 3.0f;
        const float thirdH = h / 3.0f;

        // Top corners
        drawList->AddLine(ImVec2(x, y), ImVec2(x, y + thirdH), color, thickness);
        drawList->AddLine(ImVec2(x, y), ImVec2(x + thirdW, y), color, thickness);
        drawList->AddLine(ImVec2(x + w - thirdW, y), ImVec2(x + w, y), color, thickness);
        drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + thirdH), color, thickness);

        // Bottom corners
        drawList->AddLine(ImVec2(x, y + h - thirdH), ImVec2(x, y + h), color, thickness);
        drawList->AddLine(ImVec2(x, y + h), ImVec2(x + thirdW, y + h), color, thickness);
        drawList->AddLine(ImVec2(x + w - thirdW, y + h), ImVec2(x + w, y + h), color, thickness);
        drawList->AddLine(ImVec2(x + w, y + h - thirdH), ImVec2(x + w, y + h), color, thickness);
    }

    inline void DrawFullBox(int x, int y, int w, int h, const ImColor& color, float thickness)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
    }

    inline void DrawFilledBox(int x, int y, int w, int h, const ImColor& color, float thickness)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        ImColor fillColor = color;
        fillColor.Value.w *= 0.2f; // Transparence pour le remplissage

        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), fillColor, 0.0f);
        drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
    }

    inline void Draw3DBox(int x, int y, int w, int h, const ImColor& color, float thickness)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        const float offset = w * 0.1f;

        // Front rectangle
        drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);

        // Back rectangle (offset)
        drawList->AddRect(ImVec2(x - offset, y - offset),
            ImVec2(x + w - offset, y + h - offset), color, 0.0f, 0, thickness * 0.7f);

        // Connecting lines
        drawList->AddLine(ImVec2(x, y), ImVec2(x - offset, y - offset), color, thickness * 0.7f);
        drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w - offset, y - offset), color, thickness * 0.7f);
        drawList->AddLine(ImVec2(x, y + h), ImVec2(x - offset, y + h - offset), color, thickness * 0.7f);
        drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x + w - offset, y + h - offset), color, thickness * 0.7f);
    }

    // Skeleton Drawing
    inline void DrawBone(ImDrawList* drawList, const ImVec2& from, const ImVec2& to,
        const ImColor& color, float thickness)
    {
        if (from.x > 0 && from.y > 0 && to.x > 0 && to.y > 0) {
            drawList->AddLine(from, to, color, thickness);
        }
    }

    inline void DrawSkeleton(ImDrawList* drawList, const ImVec2 bones[], int boneCount,
        const ImColor& color, float thickness)
    {
        if (!drawList || boneCount < 8) return;

        // Spine: Head -> Neck -> Chest -> Pelvis
        DrawBone(drawList, bones[0], bones[1], color, thickness);  // Head to Neck
        DrawBone(drawList, bones[1], bones[2], color, thickness);  // Neck to Chest
        DrawBone(drawList, bones[2], bones[3], color, thickness);  // Chest to Pelvis

        // Left Arm: Chest -> L.Shoulder -> L.Elbow -> L.Hand
        DrawBone(drawList, bones[2], bones[4], color, thickness);
        DrawBone(drawList, bones[4], bones[5], color, thickness);
        DrawBone(drawList, bones[5], bones[6], color, thickness);

        // Right Arm: Chest -> R.Shoulder -> R.Elbow -> R.Hand
        DrawBone(drawList, bones[2], bones[7], color, thickness);
        DrawBone(drawList, bones[7], bones[8], color, thickness);
        DrawBone(drawList, bones[8], bones[9], color, thickness);

        if (boneCount >= 14) {
            // Left Leg: Pelvis -> L.Thigh -> L.Knee -> L.Foot
            DrawBone(drawList, bones[3], bones[10], color, thickness);
            DrawBone(drawList, bones[10], bones[11], color, thickness);
            DrawBone(drawList, bones[11], bones[12], color, thickness);

            // Right Leg: Pelvis -> R.Thigh -> R.Knee -> R.Foot
            DrawBone(drawList, bones[3], bones[13], color, thickness);
            DrawBone(drawList, bones[13], bones[14], color, thickness);
            DrawBone(drawList, bones[14], bones[15], color, thickness);
        }
    }

    // Snaplines
    inline void DrawSnapline(float fromX, float fromY, float toX, float toY,
        const ImColor& color, float thickness)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        drawList->AddLine(ImVec2(fromX, fromY), ImVec2(toX, toY), color, thickness);
    }

    // Other ESP elements
    inline void DrawFilledCircle(float x, float y, const ImColor& color, float radius)
    {
        ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, color);
    }

    inline ImU32 CalculateHealthColor(float fillRatio)
    {
        if (fillRatio < 0.3f) return IM_COL32(255, 50, 50, 255);
        if (fillRatio < 0.6f) return IM_COL32(255, 200, 0, 255);
        return IM_COL32(0, 255, 0, 255);
    }

    inline void DrawHealthBar(ImDrawList* drawList, float x, float y, float width, float height,
        float value, float maxValue, float alpha = 1.0f, ImU32 bgColor = IM_COL32(30, 30, 30, 200))
    {
        const float fillRatio = std::clamp(value / maxValue, 0.0f, 1.0f);

        // Apply alpha to background
        ImColor bg = bgColor;
        bg.Value.w *= alpha;

        // Background
        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), bg, 2.0f);

        // Fill
        if (fillRatio > 0.0f)
        {
            ImU32 fillColor = CalculateHealthColor(fillRatio);
            ImColor fill = fillColor;
            fill.Value.w *= alpha;
            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width * fillRatio, y + height), fill, 2.0f);
        }

        // Border
        ImColor border = IM_COL32(0, 0, 0, 200);
        border.Value.w *= alpha;
        drawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), border, 2.0f, 0, 1.5f);
    }

    inline void DrawShieldBar(ImDrawList* drawList, float x, float y, float width, float height,
        float value, float maxValue, float alpha = 1.0f, ImU32 bgColor = IM_COL32(20, 20, 40, 200))
    {
        const float fillRatio = std::clamp(value / maxValue, 0.0f, 1.0f);

        // Apply alpha
        ImColor bg = bgColor;
        bg.Value.w *= alpha;

        // Background
        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), bg, 2.0f);

        // Fill
        if (fillRatio > 0.0f)
        {
            ImColor fill = IM_COL32(100, 200, 255, 255);
            fill.Value.w *= alpha;
            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width * fillRatio, y + height),
                fill, 2.0f);
        }

        // Border
        ImColor border = IM_COL32(0, 0, 0, 200);
        border.Value.w *= alpha;
        drawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), border, 2.0f, 0, 1.5f);
    }

    inline void DrawTextWithOutline(ImDrawList* drawList, float x, float y, const ImColor& color, const char* text)
    {
        if (!drawList || !text) return;

        const ImU32 outlineColor = IM_COL32(0, 0, 0, 200);
        const ImVec2 textPos = ImVec2(x, y);

        // Outline (reduced for stealth)
        drawList->AddText(ImVec2(textPos.x - 1, textPos.y), outlineColor, text);
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y), outlineColor, text);
        drawList->AddText(ImVec2(textPos.x, textPos.y - 1), outlineColor, text);
        drawList->AddText(ImVec2(textPos.x, textPos.y + 1), outlineColor, text);

        // Main text
        drawList->AddText(textPos, color, text);
    }

    inline void DrawDistance(ImDrawList* drawList, float x, float y, float distance)
    {
        if (!drawList) return;

        char buffer[16];
        const int length = snprintf(buffer, sizeof(buffer), "%.0fm", distance);
        if (length > 0)
        {
            DrawTextWithOutline(drawList, x, y, IM_COL32(255, 255, 255, 200), buffer);
        }
    }

    inline void DrawPlayerName(ImDrawList* drawList, float x, float y, const char* name, float alpha = 1.0f)
    {
        ImColor color = IM_COL32(255, 255, 255, 255);
        color.Value.w *= alpha;
        DrawTextWithOutline(drawList, x, y, color, name);
    }
}