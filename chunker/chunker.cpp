#include <Windows.h>
#include <stdio.h>

BOOL IsPathValidW(PWCHAR FilePath)
{
	HANDLE hFile = CreateFileW(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (hFile)
		CloseHandle(hFile);

	return TRUE;
}

SIZE_T StringLengthA(LPCSTR String)
{
	LPCSTR String2;

	for (String2 = String; *String2; ++String2);

	return (String2 - String);
}

BOOL CreateFraction(PBYTE DataBlock, DWORD dwWriteSize, PWCHAR OutputDirectory)
{
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	WCHAR OutputPath[MAX_PATH * sizeof(WCHAR)] = { 0 };
	DWORD dwOut = ERROR_SUCCESS;
	BOOL bFlag = FALSE;
	CHAR FileHeader[MAX_PATH] = { 0 };

	for (DWORD dwFractionCount = 0;;dwFractionCount++)
	{
		_snwprintf_s(OutputPath, MAX_PATH * sizeof(WCHAR), L"%ws\\Fraction%ld", OutputDirectory, dwFractionCount);
		if (IsPathValidW(OutputPath))
			continue;
		else {
			_snprintf_s(FileHeader, MAX_PATH, "<%ld>", dwFractionCount);

			if (strlen(FileHeader) < 32)
			{
				DWORD dwOffset = (DWORD)(32 - strlen(FileHeader));
				for (DWORD dwX = 0; dwX < dwOffset; dwX++) { strcat_s(FileHeader, " "); }
			}
			break;
		}
	}
	
	hHandle = CreateFileW(OutputPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	if (!WriteFile(hHandle, FileHeader, 32, &dwOut, NULL))
		goto EXIT_ROUTINE;

	dwOut = ERROR_SUCCESS;

	if (!WriteFile(hHandle, DataBlock, dwWriteSize, &dwOut, NULL))
		goto EXIT_ROUTINE;

	bFlag = TRUE;

EXIT_ROUTINE:
	if (hHandle)
		CloseHandle(hHandle);

	return bFlag;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	DWORD dwError = ERROR_SUCCESS;
	BOOL bFlag = FALSE;
	BOOL EndOfFile = FALSE;
	const int PARTS = 11;
	LARGE_INTEGER FileSize;

	INT Arguments;
	LPWSTR* szArgList = CommandLineToArgvW(GetCommandLineW(), &Arguments);

	hHandle = CreateFileW(szArgList[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD baseBlockSize = 0;
	DWORD remainder = 0;

	if (hHandle == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	if (!GetFileSizeEx(hHandle, &FileSize))
		goto EXIT_ROUTINE;

	baseBlockSize = (DWORD)(FileSize.QuadPart / PARTS);
	remainder = (DWORD)(FileSize.QuadPart % PARTS);

	for (int i = 0; i < PARTS; ++i)
	{
		DWORD thisBlockSize = baseBlockSize;
		if (i == PARTS - 1)
			thisBlockSize += remainder;

		BYTE* Buffer = new BYTE[thisBlockSize];
		DWORD dwRead = 0;
		if (!ReadFile(hHandle, Buffer, thisBlockSize, &dwRead, NULL) || dwRead != thisBlockSize)
		{
			delete[] Buffer;
			goto EXIT_ROUTINE;
		}

		if (!CreateFraction(Buffer, thisBlockSize, szArgList[2]))
		{
			delete[] Buffer;
			goto EXIT_ROUTINE;
		}

		delete[] Buffer;
	}

	bFlag = TRUE;

EXIT_ROUTINE:
	if (!bFlag)
		dwError = GetLastError();
	
	LocalFree(szArgList);

	if (hHandle)
		CloseHandle(hHandle);
	
	return dwError;
}
