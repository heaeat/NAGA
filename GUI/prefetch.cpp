#include "stdafx.h"
#include "prefetch.h"


void get_prefetch_info(void) {

	// 현재경로 받아오기
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\PECmd.exe -d \"\C:\\Windows\\Prefetch\"\ --csv \"\C:\\Temp\"\"";

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInfo;
	startupInfo.cb = sizeof(STARTUPINFO);
	//::CreateProcess(NULL, (LPWSTR)current_dir.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	::CreateProcess(NULL, (LPWSTR)current_dir.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

}