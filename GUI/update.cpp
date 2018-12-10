#include "stdafx.h"
#include "update.h"

bool runCompare() {
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\compare.exe";

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInfo;
	startupInfo.cb = sizeof(STARTUPINFO);

	log_info "run %ws", strm.str().c_str() log_end;

	if (!CreateProcess(NULL,
		(LPWSTR)strm.str().c_str(),
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
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
	log_info "CreateProcess() success" log_end;
	return true;
}