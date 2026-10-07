
#include <stdio.h>


#include <atomic>

#include <iostream>
#include <conio.h>

#include "WinThread.h"

static HANDLE hWinThread;
static HANDLE hWinLogicThread;

int main()
{
	//int tets = GetPrivateProfileInt(L"TT", L"SS", 0, L"C:\\Users\\USER\\source\\repos\\BhaviorTreeTest_Project\\BhaviorTreeTest_Project\\Test.ini");

	hWinThread = StartWinAPIThread();
	Sleep(1000);
	hWinLogicThread = StartLogicThread();

	WaitForSingleObject(hWinThread, INFINITE);
	WaitForSingleObject(hWinLogicThread, INFINITE);
	return 0;
}
