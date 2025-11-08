// main.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

// Includes du projet
#include "globals.hpp"
#include "game/Config.hpp"
#include "game/Aimbot.hpp"
#include "overlay/HijackedOverlay.hpp"
#include "overlay/Menu.hpp"
#include "Entity.hpp"
#include "game/ESP.hpp"
#include "game/render.hpp"

using namespace std;

// Function to display system info
void DisplaySystemInfo() {
    cout << "========================================" << endl;
    cout << "  Apex Legends External - v3.0.0" << endl;
    cout << "       Hijacked Overlay Method" << endl;
    cout << "========================================" << endl;
    cout << endl;
    cout << "Features:" << endl;
    cout << "  [+] ESP (Multiple box types)" << endl;
    cout << "  [+] Skeleton Display" << endl;
    cout << "  [+] Snaplines (Configurable)" << endl;
    cout << "  [+] Memory Aimbot" << endl;
    cout << "  [+] Health/Shield Bars" << endl;
    cout << "  [+] Distance Display" << endl;
    cout << "  [+] Name Tags" << endl;
    cout << "========================================" << endl << endl;
}

// Function to validate module base
bool ValidateModuleBase(uint64_t base) {
    if (!base) return false;

    WORD dosSignature = read<WORD>(base);
    if (dosSignature != 0x5A4D) { // "MZ"
        return false;
    }

    DWORD peOffset = read<DWORD>(base + 0x3C);
    if (peOffset > 0x1000) { // PE offset shouldn't be this far
        return false;
    }

    DWORD peSignature = read<DWORD>(base + peOffset);
    return (peSignature == 0x00004550); // "PE\0\0"
}

int main()
{
    DisplaySystemInfo();

    // Step 1: Initialize driver
    cout << "[1/5] Initializing driver..." << endl;
    if (!mem::find_driver())
    {
        cerr << "[!] Failed to initialize driver!" << endl;
        cerr << "[!] Make sure:" << endl;
        cerr << "    - Driver is loaded" << endl;
        cerr << "    - Running as Administrator" << endl;
        cerr << "    - Device name: {d6579ab0-c95b-4463-9135-41gbcf16e4eg}" << endl;
        system("pause");
        return -1;
    }
    cout << "[+] Driver connected successfully!" << endl << endl;

    // Step 2: Wait for game process
    cout << "[2/5] Waiting for game process..." << endl;
    DWORD PID = 0;
    int attempts = 0;
    const int MAX_WAIT_ATTEMPTS = 60; // 2 minutes maximum

    while (!PID && attempts < MAX_WAIT_ATTEMPTS)
    {
        PID = mem::find_process(L"r5apex_dx12.exe");
        if (!PID)
        {
            attempts++;
            if (attempts % 5 == 0)
                cout << "[!] Still waiting for r5apex_dx12.exe... (" << attempts << "/" << MAX_WAIT_ATTEMPTS << ")" << endl;
            Sleep(2000);
        }
    }

    if (!PID) {
        cerr << "[!] Timeout waiting for game process!" << endl;
        system("pause");
        return -1;
    }

    system("cls");
    DisplaySystemInfo();
    cout << "[+] Game process found!" << endl;
    cout << "[+] PID: " << dec << PID << " (0x" << hex << uppercase << PID << ")" << endl;
    cout << "========================================" << endl << endl;

    // Step 3: Set target process and fetch CR3
    cout << "[3/5] Setting target process and fetching CR3..." << endl;
    mem::process_id = PID;

    cr3 = mem::fetch_cr3();
    if (!cr3)
    {
        cerr << "[!] Failed to fetch CR3!" << endl;
        cerr << "[!] This is required for physical memory access" << endl;
        system("pause");
        return -1;
    }
    cout << "[+] Process set successfully!" << endl;
    cout << "[+] CR3 (Directory Base): 0x" << hex << uppercase << cr3 << endl << endl;

    // Step 4: Get module base address
    cout << "[4/5] Getting module base address..." << endl;
    cout << "[*] Trying driver method (find_image)..." << endl;

    GameBase = mem::find_image();

    if (!GameBase)
    {
        cerr << "[!] Failed to get module base address!" << endl;
        cerr << "[!] Possible causes:" << endl;
        cerr << "    - Anti-cheat is blocking the driver" << endl;
        cerr << "    - Wrong security code" << endl;
        cerr << "    - Driver not properly initialized" << endl;
        cerr << "    - Game protection active" << endl;
        system("pause");
        return -1;
    }

    cout << "[+] Module Base: 0x" << hex << uppercase << GameBase << endl << endl;

    // Validate module base
    cout << "[*] Validating module base..." << endl;
    if (ValidateModuleBase(GameBase))
    {
        cout << "[+] Valid PE header detected!" << endl;
        cout << "[+] Module base validation passed!" << endl;
    }
    else
    {
        cerr << "[!] WARNING: Invalid module base!" << endl;
        cerr << "[!] The program may not work correctly!" << endl;
        cout << "[?] Continue anyway? (y/n): ";
        char choice;
        cin >> choice;
        if (choice != 'y' && choice != 'Y')
        {
            return -1;
        }
    }

    cout << endl << "========================================" << endl;
    cout << "  Memory access operational!" << endl;
    cout << "========================================" << endl << endl;

    // Step 5: Wait for game window
    cout << "[5/5] Waiting for game window..." << endl;
    attempts = 0;

    while (!hwnd && attempts < 30)
    {
        hwnd = FindWindowA("Respawn001", "Apex Legends");
        if (!hwnd)
        {
            attempts++;
            if (attempts % 5 == 0)
                cout << "[!] Still waiting for game window... (" << attempts << "/30)" << endl;
            Sleep(1000);
        }
    }

    if (!hwnd) {
        cerr << "[!] Timeout waiting for game window!" << endl;
        system("pause");
        return -1;
    }

    cout << "[+] Game window found!" << endl;
    cout << "[+] HWND: 0x" << hex << uppercase << hwnd << endl << endl;

    // Get screen size BEFORE initializing overlay
    RECT rect;
    if (GetWindowRect(hwnd, &rect))
    {
        ScreenSize.x = static_cast<float>(rect.right - rect.left);
        ScreenSize.y = static_cast<float>(rect.bottom - rect.top);
        cout << "[+] Screen size: " << dec << ScreenSize.x << "x" << ScreenSize.y << endl;
    }
    else
    {
        cerr << "[!] Failed to get window rect, using default 1920x1080" << endl;
        ScreenSize.x = 1920.0f;
        ScreenSize.y = 1080.0f;
    }

    // Initialize hijacked overlay
    cout << "[*] Initializing hijacked overlay..." << endl;
    if (!HijackedOverlay::Initialize("Apex Legends", "Respawn001"))
    {
        cerr << "[!] Failed to initialize overlay!" << endl;
        cerr << "[!] Possible causes:" << endl;
        cerr << "    - DirectX 11 not available" << endl;
        cerr << "    - Failed to create transparent window" << endl;
        cerr << "    - DWM composition disabled" << endl;
        system("pause");
        return -1;
    }

    cout << "[+] Overlay initialized successfully!" << endl << endl;

    cout << "========================================" << endl;
    cout << "         Overlay is now active!" << endl;
    cout << "========================================" << endl;
    cout << "[*] Controls:" << endl;
    cout << "    - INSERT: Toggle menu (interactive mode)" << endl;
    cout << "    - END: Exit program" << endl;
    cout << "    - RIGHT CLICK: Aimbot (when enabled)" << endl;
    cout << "========================================" << endl << endl;

    // Initialize configuration with stealth defaults
    isRunning = true;
    cfg.showMenu = false;     // Menu fermé au démarrage pour discrétion
    cfg.enableESP = true;     // ESP activé
    cfg.aimbotEnabled = false; // Aimbot désactivé par défaut

    // Add initial logs
    Menu::AddLog("[System] Overlay initialized successfully!");
    Menu::AddLog("[System] Version 3.0.0 - Full Feature Set");
    Menu::AddLog("[System] Press INSERT to toggle menu");
    Menu::AddLog("[Info] ESP active - Menu closed for stealth");
    Menu::AddLog("[Info] Right-click to use aimbot (when enabled)");

    // Main render loop
    cout << "[*] Starting render loop..." << endl;
    cout << "[*] System is now running in background mode..." << endl << endl;

    auto lastValidCheck = chrono::steady_clock::now();
    auto lastFPSUpdate = chrono::steady_clock::now();
    bool wasMenuOpen = cfg.showMenu;
    int frameCount = 0;
    float currentFPS = 0.0f;

    while (isRunning)
    {
        // Process Windows messages
        if (!HijackedOverlay::ProcessMessages())
        {
            cout << "[*] Received exit signal from window" << endl;
            Menu::AddLog("[System] Window closed, shutting down...");
            break;
        }

        // Check if target window is still valid (every 500ms)
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - lastValidCheck).count() > 500)
        {
            if (!HijackedOverlay::IsTargetWindowValid())
            {
                cout << "[!] Target window lost, exiting..." << endl;
                Menu::AddLog("[ERROR] Target window closed!");
                isRunning = false;
                break;
            }
            lastValidCheck = now;
        }

        // FPS counter
        frameCount++;
        auto fpsDuration = chrono::duration_cast<chrono::milliseconds>(now - lastFPSUpdate).count();
        if (fpsDuration >= 1000)
        {
            currentFPS = (frameCount * 1000.0f) / fpsDuration;
            frameCount = 0;
            lastFPSUpdate = now;
        }

        // Update overlay position to follow game window
        HijackedOverlay::UpdateOverlayPosition();

        // Begin ImGui frame
        HijackedOverlay::BeginFrame();

        // Log menu state changes
        if (cfg.showMenu != wasMenuOpen)
        {
            if (cfg.showMenu)
            {
                Menu::AddLog("[Menu] Opened - INTERACTIVE mode");
                cout << "[*] Menu opened - Interactive mode ON" << endl;
            }
            else
            {
                Menu::AddLog("[Menu] Closed - STEALTH mode");
                cout << "[*] Menu closed - Stealth mode ON" << endl;
            }
            wasMenuOpen = cfg.showMenu;
        }

        // === UPDATE AIMBOT (MUST BE CALLED BEFORE RENDERING) ===
        if (cfg.aimbotEnabled)
        {
            try
            {
                Aimbot::Update();
            }
            catch (const exception& e)
            {
                cerr << "[!] Exception in Aimbot: " << e.what() << endl;
                // Don't spam logs, just continue
            }
            catch (...)
            {
                // Silent fail for aimbot errors
            }
        }

        // === RENDER ESP ===
        if (cfg.enableESP)
        {
            try
            {
                RenderESP();
            }
            catch (const exception& e)
            {
                cerr << "[!] Exception in RenderESP: " << e.what() << endl;
                Menu::AddLog("[ERROR] ESP Exception: " + string(e.what()));
            }
            catch (...)
            {
                cerr << "[!] Unknown exception in RenderESP" << endl;
            }
        }

        // === DRAW FOV CIRCLE ===
        if (cfg.aimbotEnabled)
        {
            try
            {
                Aimbot::DrawFOVCircle();
            }
            catch (...)
            {
                // Silent fail
            }
        }

        // === RENDER MENU ===
        Menu::Render();

        // End ImGui frame and present
        HijackedOverlay::EndFrame();

        // Small sleep to reduce CPU usage (target ~144 FPS)
        this_thread::sleep_for(chrono::microseconds(6900));
    }

    cout << endl << "[*] Cleaning up resources..." << endl;
    Menu::AddLog("[System] Shutting down...");

    // Cleanup
    HijackedOverlay::Cleanup();

    cout << "[+] Cleanup complete!" << endl;
    cout << "[+] Statistics:" << endl;
    cout << "    - Final FPS: " << fixed << setprecision(1) << currentFPS << endl;
    cout << "    - Features used: ESP";
    if (cfg.aimbotEnabled) cout << ", Aimbot";
    if (cfg.showSkeleton) cout << ", Skeleton";
    if (cfg.snaplines) cout << ", Snaplines";
    cout << endl;
    cout << "[+] Thank you for using the software!" << endl;
    cout << "[+] Goodbye!" << endl;

    // Keep console open for 2 seconds to see final messages
    this_thread::sleep_for(chrono::seconds(2));

    return 0;
}