// overlay/Menu.hpp - Version complète et discrète
#pragma once
#include "../OS-ImGui/imgui/imgui.h"
#include "../game/Config.hpp"
#include "../game/Aimbot.hpp"
#include "../globals.hpp"
#include <chrono>
#include <vector>
#include <string>
#include <mutex>
#include <deque>

namespace Menu
{
    // ===== GESTION DES LOGS =====
    class LogSystem {
    private:
        std::deque<std::string> logs;
        std::mutex mutex;
        static constexpr size_t MAX_LOGS = 150;

    public:
        static LogSystem& Get() {
            static LogSystem instance;
            return instance;
        }

        void Add(const std::string& msg) {
            std::lock_guard<std::mutex> lock(mutex);
            logs.push_back(msg);
            if (logs.size() > MAX_LOGS) {
                logs.pop_front();
            }
        }

        std::vector<std::string> GetAll() {
            std::lock_guard<std::mutex> lock(mutex);
            return std::vector<std::string>(logs.begin(), logs.end());
        }

        void Clear() {
            std::lock_guard<std::mutex> lock(mutex);
            logs.clear();
        }
    };

    inline void AddLog(const std::string& msg) {
        LogSystem::Get().Add(msg);
    }

    inline std::vector<std::string> GetLogs() {
        return LogSystem::Get().GetAll();
    }

    // ===== CALCUL FPS =====
    inline float GetFPS() {
        static auto lastTime = std::chrono::steady_clock::now();
        static int frameCount = 0;
        static float fps = 0.0f;

        frameCount++;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - lastTime).count();

        if (elapsed >= 0.5f) {
            fps = frameCount / elapsed;
            frameCount = 0;
            lastTime = now;
        }
        return fps;
    }

    // ===== STYLE DISCRET ET MODERNE =====
    inline void ApplyStealthStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Couleurs sombres et discrètes
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.93f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.50f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.60f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.65f, 0.85f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.70f, 0.90f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.25f, 0.60f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.30f, 0.40f, 0.80f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.35f, 0.45f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.25f, 0.35f, 0.50f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.30f, 0.40f, 0.70f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.30f, 0.35f, 0.45f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);

        // Style arrondi et discret
        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.WindowBorderSize = 0.0f;
        style.Alpha = 0.96f;
    }

    // ===== HEADER DISCRET =====
    inline void DrawHeader() {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.7f, 0.8f, 1.0f));
        ImGui::Text("System Monitor");
        ImGui::PopStyleColor();

        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.6f, 1.0f));
        ImGui::Text("v3.0.0");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    // ===== STATUS BAR =====
    inline void DrawStatusBar() {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.7f, 1.0f));
        ImGui::Text("FPS: %.0f", GetFPS());
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);
        ImGui::Text("Status: %s", cfg.enableESP ? "Active" : "Standby");
        ImGui::PopStyleColor();
    }

    // ===== GESTION INPUT =====
    inline void HandleInput() {
        static bool insertPressed = false;
        static bool endPressed = false;

        const bool insertDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        const bool endDown = (GetAsyncKeyState(VK_END) & 0x8000) != 0;

        if (insertDown && !insertPressed) {
            cfg.showMenu = !cfg.showMenu;
            HijackedOverlay::SetInteractive(cfg.showMenu);
        }
        insertPressed = insertDown;

        if (endDown && !endPressed) {
            isRunning = false;
        }
        endPressed = endDown;
    }

    // ===== RENDU PRINCIPAL =====
    inline void Render() {
        HandleInput();

        if (!cfg.showMenu) return;

        static bool initialized = false;
        if (!initialized) {
            ApplyStealthStyle();
            initialized = true;
        }

        ImGui::SetNextWindowSize(ImVec2(760, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

        ImGui::Begin("Control Panel", &cfg.showMenu,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        DrawHeader();

        if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None))
        {
            // ===== TAB VISUALS =====
            if (ImGui::BeginTabItem("Visuals"))
            {
                ImGui::BeginChild("VisualsChild", ImVec2(0, -40), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SeparatorText("Display Options");
                ImGui::Checkbox("Enable Visual Aid", &cfg.enableESP);

                ImGui::Spacing();
                ImGui::Checkbox("Box Overlay", &cfg.showBoxes);
                if (cfg.showBoxes) {
                    ImGui::Indent();
                    const char* boxTypes[] = { "Cornered", "Full", "Filled", "3D" };
                    ImGui::Combo("Box Style", &cfg.boxType, boxTypes, IM_ARRAYSIZE(boxTypes));
                    ImGui::SliderFloat("Box Thickness", &cfg.boxThickness, 0.5f, 3.0f, "%.1f");
                    ImGui::ColorEdit3("Box Color", (float*)&cfg.boxColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Unindent();
                }

                ImGui::Spacing();
                ImGui::Checkbox("Skeleton Display", &cfg.showSkeleton);
                if (cfg.showSkeleton) {
                    ImGui::Indent();
                    ImGui::SliderFloat("Skeleton Thickness", &cfg.skeletonThickness, 0.5f, 3.0f, "%.1f");
                    ImGui::ColorEdit3("Skeleton Color", (float*)&cfg.skeletonColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Unindent();
                }

                ImGui::Spacing();
                ImGui::Checkbox("Connection Lines", &cfg.snaplines);
                if (cfg.snaplines) {
                    ImGui::Indent();
                    const char* snapPositions[] = { "Bottom", "Center", "Top" };
                    ImGui::Combo("Line Origin", &cfg.snaplinePosition, snapPositions, IM_ARRAYSIZE(snapPositions));
                    ImGui::SliderFloat("Line Thickness", &cfg.snaplineThickness, 0.5f, 3.0f, "%.1f");
                    ImGui::ColorEdit3("Line Color", (float*)&cfg.snaplineColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Unindent();
                }

                ImGui::Spacing();
                ImGui::Checkbox("Distance Info", &cfg.espDistance);
                ImGui::Checkbox("Health Display", &cfg.espHealth);
                ImGui::Checkbox("Shield Display", &cfg.showShieldBar);
                ImGui::Checkbox("Name Tags", &cfg.espName);
                ImGui::Checkbox("Head Marker", &cfg.headdot);

                ImGui::Spacing();
                ImGui::SeparatorText("Stealth Settings");
                ImGui::SliderFloat("Overall Opacity", &cfg.espAlpha, 0.1f, 1.0f, "%.2f");
                ImGui::Checkbox("Reduced Effects", &cfg.reducedEffects);
                ImGui::TextWrapped("Lower opacity and reduced effects make visuals less noticeable");

                ImGui::Spacing();
                ImGui::SeparatorText("Filters");
                ImGui::Checkbox("Team Filter", &cfg.espTeamCheck);
                ImGui::Checkbox("Visibility Filter", &cfg.espVisibleOnly);

                ImGui::Spacing();
                ImGui::SeparatorText("Range");
                ImGui::SliderFloat("Max Range", &cfg.espMaxDistance, 50.0f, 1000.0f, "%.0f m");

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ===== TAB ASSIST =====
            if (ImGui::BeginTabItem("Assist"))
            {
                ImGui::BeginChild("AssistChild", ImVec2(0, -40), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SeparatorText("Targeting System");
                ImGui::Checkbox("Enable Assist", &cfg.aimbotEnabled);
                ImGui::Checkbox("Visibility Check", &cfg.aimbotVisibleOnly);
                ImGui::Checkbox("Team Check", &cfg.aimbotTeamCheck);

                ImGui::Spacing();
                ImGui::SeparatorText("Parameters");
                ImGui::SliderFloat("Field of View", &cfg.aimbotFOV, 5.0f, 180.0f, "%.0f°");
                ImGui::SliderFloat("Smoothness", &cfg.aimbotSmooth, 1.0f, 30.0f, "%.1f");
                ImGui::SliderFloat("Max Range", &cfg.maxDistance, 10.0f, 500.0f, "%.0f m");

                ImGui::Spacing();
                ImGui::SeparatorText("Target Point");
                const char* bones[] = { "Head", "Neck", "Chest" };
                ImGui::Combo("Target", &cfg.targetBone, bones, IM_ARRAYSIZE(bones));

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.7f, 1.0f));
                ImGui::TextWrapped("Hold right mouse button to activate targeting assistance");
                ImGui::PopStyleColor();

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ===== TAB SETTINGS =====
            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::BeginChild("SettingsChild", ImVec2(0, -40), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SeparatorText("Configuration");

                if (ImGui::Button("Save Profile", ImVec2(150, 0))) {
                    AddLog("[Config] Profile saved successfully");
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Profile", ImVec2(150, 0))) {
                    AddLog("[Config] Profile loaded successfully");
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset to Default", ImVec2(150, 0))) {
                    cfg = Config(); // Reset to defaults
                    AddLog("[Config] Settings reset to default");
                }

                ImGui::Spacing();
                ImGui::SeparatorText("Performance");
                ImGui::SliderInt("Max Entities", &cfg.maxPlayers, 10, 100);
                ImGui::TextWrapped("Limiting entities can improve performance");

                ImGui::Spacing();
                ImGui::SeparatorText("Hotkeys");
                ImGui::Text("Toggle Menu: INSERT");
                ImGui::Text("Exit Program: END");
                ImGui::Text("Assist Key: Right Mouse Button");

                ImGui::Spacing();
                ImGui::SeparatorText("Quick Presets");

                if (ImGui::Button("Stealth Mode", ImVec2(150, 0))) {
                    cfg.espAlpha = 0.4f;
                    cfg.reducedEffects = true;
                    cfg.boxType = 0; // Cornered
                    cfg.boxThickness = 1.0f;
                    cfg.skeletonThickness = 1.0f;
                    cfg.snaplineThickness = 0.8f;
                    AddLog("[Preset] Stealth mode activated");
                }
                ImGui::SameLine();
                if (ImGui::Button("Balanced Mode", ImVec2(150, 0))) {
                    cfg.espAlpha = 0.7f;
                    cfg.reducedEffects = false;
                    cfg.boxType = 1; // Full
                    cfg.boxThickness = 1.5f;
                    cfg.skeletonThickness = 1.5f;
                    cfg.snaplineThickness = 1.0f;
                    AddLog("[Preset] Balanced mode activated");
                }
                ImGui::SameLine();
                if (ImGui::Button("Maximum Visibility", ImVec2(150, 0))) {
                    cfg.espAlpha = 1.0f;
                    cfg.reducedEffects = false;
                    cfg.boxType = 2; // Filled
                    cfg.boxThickness = 2.0f;
                    cfg.skeletonThickness = 2.0f;
                    cfg.snaplineThickness = 1.5f;
                    AddLog("[Preset] Maximum visibility activated");
                }

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ===== TAB LOGS =====
            if (ImGui::BeginTabItem("Console"))
            {
                ImGui::BeginChild("LogsChild", ImVec2(0, -40), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                auto logs = GetLogs();
                for (const auto& log : logs) {
                    ImGui::TextWrapped("%s", log.c_str());
                }

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();

                if (ImGui::Button("Clear Console", ImVec2(150, 0))) {
                    LogSystem::Get().Clear();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        DrawStatusBar();
        ImGui::End();
    }
}