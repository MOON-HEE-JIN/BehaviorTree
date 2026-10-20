
#include <stdio.h>


#include <atomic>

#include <iostream>
#include <conio.h>

#include "WinThread.h"

static HANDLE hWinThread;
static HANDLE hWinLogicThread;

int main()
{
	
	hWinThread = StartWinAPIThread();

	WaitForSingleObject(hWinThread, INFINITE);

	CloseHandle(hWinThread);
	return 0;
}
