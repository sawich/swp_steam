#define WIN32_LEAN_AND_MEAN
#define DLL_EXPORT __declspec(dllexport) 

#include <filesystem>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#ifdef _WIN32
#	define _X86_
#else
#	define _AMD64_
#endif
#include "../libs/detours/detours.h"

#define ATTACH(x) DetourAttach(&(PVOID&)Real_##x,Mine_##x)
#define DETACH(x) DetourAttach(&(PVOID&)Real_##x,Mine_##x)

extern "C" {
	HANDLE(WINAPI * Real_CreateFileW) (
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile
		) = CreateFileW;

	int(WINAPI * Real_MessageBoxA) (
		HWND hWnd,
		LPCSTR lpText,
		LPCSTR lpCaption,
		UINT uType) = MessageBoxA;
}


DLL_EXPORT HANDLE WINAPI Mine_CreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
) {
	DWORD pid = GetCurrentProcessId();
	char name[500];
	char fullPIDLogFilePath[1024];
	char logger[1024];
	size_t i;
	HANDLE hFile;
	char DataBuffer[500];
	DWORD dwBytesToWrite;
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;
	wchar_t wtext[1024];
	
	//sprintf_s(logger, "Entered hook => file name: %s, log file: %s\n", DataBuffer, fullPIDLogFilePath);
	//OutputDebugString(TEXT(logger));

	//converting full log file path to LPWSTR
	sprintf_s(fullPIDLogFilePath, "%s%d.txt", "Y:\\temp\\", pid);
	
	size_t converted{ 0 };
	mbstowcs_s(&converted, wtext, fullPIDLogFilePath, strlen(fullPIDLogFilePath) + 1);//Plus null
	LPWSTR ptr = wtext;
	hFile = Real_CreateFileW(ptr, FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		OutputDebugString(TEXT("Unable to open file\n"));

	wcstombs_s(&i, DataBuffer, (size_t)500, lpFileName, 500);
	sprintf_s(DataBuffer, "%s\r\n", DataBuffer);
	dwBytesToWrite = (DWORD)strlen(DataBuffer);
	//sprintf_s(logger, "length: %d\n", dwBytesToWrite);
	//OutputDebugString(logger);

	bErrorFlag = WriteFile(
		hFile,           // open file handle
		DataBuffer,      // start of data to write
		dwBytesToWrite,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);            // no overlapped structure

	if (FALSE == bErrorFlag)
		OutputDebugString(TEXT("write the requested file in our log\n"));
	else
		if (dwBytesWritten != dwBytesToWrite)
			OutputDebugString(TEXT("number of written bytes not equal to requested one\n"));
		else
			OutputDebugString(TEXT("documented succefully\n"));

	CloseHandle(hFile);

	//wcstombs_s(&i, name, (size_t)500, lpFileName, (size_t)500);
	//sprintf_s(fullPIDLogFilePath, "PID %d Tried to create a file named %s\n", pid, name);
	//OutputDebugString(TEXT(fullPIDLogFilePath));

	/*static UINT32 count = 0;

	auto path{ std::filesystem::path{ lpFileName } };
	if (!path.filename().compare("data12.v") && (++count) == 1) {
		MessageBoxW(NULL, path.filename().c_str(), L"file", MB_OK);

		return Real_CreateFileW(L"Y:\\temp\\data12.v", dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}*/

	return Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	_CRT_UNUSED(hDLL);
	_CRT_UNUSED(Reserved);

	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DetourRestoreAfterWith();		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		ATTACH(CreateFileW);

		DetourTransactionCommit();
		break;
	}
	case DLL_PROCESS_DETACH: {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DETACH(CreateFileW);

		DetourTransactionCommit();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}