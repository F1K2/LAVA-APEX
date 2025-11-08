// game/Config.hpp
#pragma once
#include "../OS-ImGui/imgui/imgui.h"

struct Config {
    // Menu
    bool showMenu = false;

    // ESP Features
    bool enableESP = true;
    bool showBoxes = true;
    bool espName = true;
    bool espLines = false;
    bool headdot = true;
    bool espHealth = true;
    bool showShieldBar = true;
    bool espDistance = true;
    bool showSpectators = true;
    bool espTeamCheck = false;
    bool espVisibleOnly = false;
    bool snaplines = true;
    bool espHealthText = true;
    bool showSkeleton = false;

    // Box Types (0=Cornered, 1=Full, 2=Filled, 3=3D)
    int boxType = 0;

    // Snapline Position (0=Bottom, 1=Center, 2=Top)
    int snaplinePosition = 1;

    // Aimbot Features
    bool aimbotEnabled = true;
    bool aimbotVisibleOnly = false;
    bool aimbotTeamCheck = false;
    float aimbotFOV = 30.0f;
    float aimbotSmooth = 5.0f;
    int targetBone = 2; // 0=head, 1=neck, 2=chest
    float maxDistance = 200.0f;

    // Colors
    ImVec4 espColorVisible = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);
    ImVec4 espColorNotVisible = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    ImVec4 boxColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
    ImVec4 skeletonColor = ImVec4(1.0f, 1.0f, 1.0f, 0.9f);
    ImVec4 snaplineColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

    float boxThickness = 1.5f;
    float skeletonThickness = 1.5f;
    float snaplineThickness = 1.0f;

    // Distance Settings
    bool distanceLimit = true;
    float espMaxDistance = 300.0f;
    int maxPlayers = 60;

    // Stealth Settings
    float espAlpha = 0.7f; // Transparence globale
    bool reducedEffects = true; // Réduit les effets visuels
};

// Global config instance
inline Config cfg;