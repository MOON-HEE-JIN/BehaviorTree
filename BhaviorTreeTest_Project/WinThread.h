#pragma once
#include <Windows.h>
#include "CSimulatorManager.h"

HANDLE StartWinAPIThread();
HANDLE StartLogicThread();

unsigned int __stdcall WinAPIThread(void* arg);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

unsigned int __stdcall WinApiLogicThread(void* arg);

extern CSimulatorManager* g_SimulatorManager;