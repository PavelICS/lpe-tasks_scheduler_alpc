// CreateProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Windows.h"


int main()
{
	STARTUPINFO				startupInfo;
	PROCESS_INFORMATION		processInfo;

	RtlSecureZeroMemory(&startupInfo, sizeof(startupInfo));
	RtlSecureZeroMemory(&processInfo, sizeof(processInfo));
	startupInfo.cb = sizeof(startupInfo);
	GetStartupInfo(&startupInfo);

	if (CreateProcessW(L"calc.exe", NULL, NULL, NULL, FALSE, 0, NULL,
		NULL, &startupInfo, &processInfo))
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
}
