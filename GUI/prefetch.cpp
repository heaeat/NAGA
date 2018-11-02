#include "stdafx.h"
#include "prefetch.h"


bool 
get_prefetch_info(
	void
	) 
{
	// 현재경로 받아오기
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\PECmd.exe -d \"C:\\Windows\\Prefetch\" --csv \"C:\\Temp\"";

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInfo;
	startupInfo.cb = sizeof(STARTUPINFO);

	if (!CreateProcess(NULL,
					  (LPWSTR)strm.str().c_str(),
					   NULL,
					   NULL,
					   FALSE,
					   0,
					   NULL,
					   NULL,
					   &startupInfo,
					   &processInfo))
	{
		log_err "CreateProcess() failed. cmd=%ws, gle=0x%08x",
			strm.str().c_str(),
			GetLastError()
			log_end;
		return false;
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hThread); 
	CloseHandle(processInfo.hProcess);
	return true;
}



