// gmacLaunch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <ShlObj.h>
#include <memory.h>

// Zeus - codename for all GMS2-related stuff in YoYo.
#define ZEUS 0

#if !ZEUS
#define MAPPING_FNAME L"YYMappingFileTestYY"
#define GMS_PATH_APPEND L"\\GameMaker-Studio\\GMAssetCompiler.exe"
#else
#define MAPPING_FNAME L"YY2016ZeusTestYY"
#define GMS_PATH_APPEND L"\\TODO\\TODO"
#endif

std::wstring *GetArgvAsStr(int _argc, wchar_t *argv[])
{
	std::wstring *ret = new std::wstring();

	for (int i = 0; i < _argc; i++)
	{
		ret->append(argv[i]);
		if (i < _argc - 1) ret->push_back(L' ');
	}

	return ret;
}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	TCHAR szAppdata[1024]; // on recent Windows versions you can get rid of the MAX_PATH limitation so we allocate a little bit more.
	BOOL appData = SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szAppdata);
	std::wstring appDataString(szAppdata);
	if (!SUCCEEDED(appData))
	{
		std::wcout << L"[gmacLaunch-ERR]: Unable to get AppData path!" << std::endl;
		return GetLastError();
	}

	appDataString.append(GMS_PATH_APPEND);

	/*
		THE MAGIC:

			- YoYo's technique is to check for a "mapping file"
			basically it's a file, but instead of existing on your HDD, it exists somewhere in memory

			the filename differs from GM:S 1.4 and GMS 2.

			in latest GMS 2 revisions they finally got rid of this, instead, they pass the path to license.plist file, and check if it's valid.
			that means you just need to have a valid license, instead of making a mapping file and blahblahblah


		(the file MUST be created as READWRITE, otherwise, the magic won't work)
	*/

	HANDLE yyCreateFile = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_EXECUTE_READWRITE, 0, 0x1000, MAPPING_FNAME);
	if (yyCreateFile == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"[gmacLaunch-ERR]: Unable to create a mapping file!" << std::endl;
		return GetLastError();
	}
	
	// allocate structures needed for CreateProcess
	STARTUPINFO gmacSInfo;
	memset(&gmacSInfo, 0, sizeof(STARTUPINFO));
	PROCESS_INFORMATION gmacPInfo;
	memset(&gmacPInfo, 0, sizeof(PROCESS_INFORMATION));

	// transform argv into a single std::wstring
	std::wstring *gmacArgc = GetArgvAsStr(argc, argv);

	BOOL gmacOK = CreateProcessW(
		appDataString.c_str(),
		(LPWSTR)gmacArgc->c_str(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_UNICODE_ENVIRONMENT,
		nullptr,
		nullptr,
		&gmacSInfo,
		&gmacPInfo
	);

	// deallocate the argv string since we don't need it anymore.
	delete gmacArgc;

	HANDLE gmacProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, gmacPInfo.dwProcessId);
	if (gmacProc == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"[gmacLaunch-ERR]: UNABLE TO OPEN GMAC PROCESS" << std::endl;
		CloseHandle(yyCreateFile);
		return GetLastError();
	}

	// wait until gmac exits.
	WaitForSingleObject(gmacProc, INFINITE);

	// gmac exited, we don't need the mapping file anymore.
	CloseHandle(yyCreateFile);

	// since we're a direct wrapper, let's get GMAC's exit code and quit with it.
	DWORD gmacExitCode;
	GetExitCodeProcess(gmacProc, &gmacExitCode);

	CloseHandle(gmacProc);

	return gmacExitCode;
}