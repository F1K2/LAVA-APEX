#pragma once
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <Psapi.h>
#include <dwmapi.h>
#include <d3d9.h>
#include "OS-ImGui/imgui/imgui.h"
#include <chrono>
#include <thread>
#include "OS-ImGui/imgui/imgui_impl_win32.h"
#include "OS-ImGui/imgui/imgui_internal.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")

using namespace std;
namespace fs = std::filesystem;

int Width = GetSystemMetrics(SM_CXSCREEN);
int Height = GetSystemMetrics(SM_CYSCREEN);

ImFont* sidefont = nullptr;
IDirect3DTexture9* sillycar = nullptr;
IDirect3DTexture9* bg = nullptr;
IDirect3DTexture9* cleo = nullptr;
// silly car

struct CurrentProcess {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	int WindowLeft;
	int WindowRight;
	int WindowTop;
	int WindowBottom;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}Process;

struct OverlayWindow {
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}Overlay;

struct DirectX9Interface {
	IDirect3D9Ex* IDirect3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParameters = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}DirectX9;

#include "overlay/HijackedOverlay.hpp"
#include "game/render.hpp"