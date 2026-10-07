
#pragma warning(disable : 4996)
#pragma comment (lib, "winmm.lib")
#include <process.h>
#include "WinThread.h"
#include "UnionDefine.h"


#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")

HBRUSH g_BRUSH_BG;

using namespace Gdiplus;

static HANDLE hLogic;

HANDLE StartWinAPIThread()
{
	return (HANDLE)_beginthreadex(NULL, 0, WinAPIThread, 0, 0, NULL);
}
HANDLE StartLogicThread()
{
	hLogic = (HANDLE)_beginthreadex(NULL, 0, WinApiLogicThread, 0, 0, NULL);
	return hLogic;
}


unsigned int __stdcall WinAPIThread(void* arg)
{

	LPCWSTR CLASS_NAME = L"윈도우 창";

	WNDCLASS wc = {};

	g_BRUSH_BG = CreateSolidBrush(RGB(37, 37, 38));
	//GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = g_BRUSH_BG;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"이게 윈도우여",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);

	if (hwnd == nullptr)
		return 0;

	ShowWindow(hwnd, SW_SHOW);

	timeBeginPeriod(1);

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	timeEndPeriod(1);
	GdiplusShutdown(gdiplusToken);
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
	{
		
	}
		break;
	case WM_CREATE:
	{
		g_SimulatorManager = new CSimulatorManager;
		SetTimer(hwnd, 1, 15, NULL);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
	{
		if (wParam == 1)
		{
			InvalidateRect(hwnd, 0, 0);
		}
		return 0;
	}
	case WM_SIZING:
	{
		InvalidateRect(hwnd, NULL, NULL);
		return 0;
	}
	case WM_PAINT:
	{
#pragma region BeginPaint
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		HDC hdcmem = CreateCompatibleDC(hdc);

		HBITMAP hbmem = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
		HBITMAP hbold = (HBITMAP)SelectObject(hdcmem, hbmem);

		FillRect(hdcmem, &ps.rcPaint, g_BRUSH_BG);
#pragma endregion BeginPaint
		//TODO
		g_SimulatorManager->Render(hdcmem);

#pragma region EndPaint
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top, hdcmem, 0, 0, SRCCOPY);

		SelectObject(hdcmem, hbold);
		DeleteObject(hbmem);
		DeleteDC(hdcmem);

		EndPaint(hwnd, &ps);
#pragma endregion EndPaint
		ValidateRect(hwnd, NULL);
		return 0;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

unsigned int __stdcall WinApiLogicThread(void* arg)
{
	timeBeginPeriod(1);
	int ret = 0;
	while (1)
	{
		ret = WaitForSingleObject(hLogic, 1);
		g_SimulatorManager->Update();
	}
	timeEndPeriod(1);
	return 0;
}
