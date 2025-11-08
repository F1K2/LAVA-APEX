// globals.hpp
#pragma once
#include <Windows.h>
#include "Structs.hpp"

// Variables globales - déclarées ici, définies dans render.cpp
extern uint64_t GameBase;
extern HWND hwnd;
extern Vector2 ScreenSize;
extern bool isRunning;
extern uint64_t cr3;

// Function declarations
uint64_t GetEntityById(int entityId, uint64_t baseAddr);