#include "stdafx.h"
#include "prefetch.h"
#include <io.h>
#include <time.h>



std::wstring result_path = L"\"C:\\Temp\\result\\";

bool get_prefetch_info() 
{
	map<string, string> csv_map;

	bool ret = run_PECmd();
	if (true != ret)
	{
		log_err "run_PECmd() failed." log_end;
		return false;
	}

	wchar_t *file_name = find_timeline_file(result_path);
	if (nullptr == file_name)
	{
		log_err "find_timeline_file() failed. result_path=%ws",
			result_path
			log_end;
		return false;
	}

	if (!read_csv(file_name, &csv_map))
	{
		log_err "read_csv() failed. file_name=%ws",
			file_name
			log_end;

		free(file_name); // <<!
		return false;
	}

	free(file_name); // <<!
	

	for (map<string, string>::iterator iter = csv_map.begin(); iter != csv_map.end(); iter++)
	{
		log_info "Key : %s  - Value : %s", iter->first.c_str(), iter->second.c_str() log_end;
	}

	string cur = get_current_time();
	log_info "current time : %s", cur.c_str() log_end;

	return true;
}

bool run_PECmd(void) 
{
	// 현재경로 받아오기
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\PECmd.exe -d \"C:\\Windows\\Prefetch\" --csv " << result_path;

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInfo;
	startupInfo.cb = sizeof(STARTUPINFO);


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

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hThread); 
	CloseHandle(processInfo.hProcess);
	
	return true;
}


int delete_all_csv(LPCWSTR szDir, int recur)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	int res = 1;

	TCHAR DelPath[MAX_PATH];
	TCHAR FullPath[MAX_PATH];
	TCHAR TempPath[MAX_PATH];

	lstrcpy(DelPath, szDir);
	lstrcpy(TempPath, szDir);
	if (lstrcmp(DelPath + lstrlen(DelPath) - 4, _T("\\*.*")) != 0) {
		lstrcat(DelPath, _T("\\*.*"));
	}

	hSrch = FindFirstFile(DelPath, &wfd);
	if (hSrch == INVALID_HANDLE_VALUE) {
		if (recur > 0) RemoveDirectory(TempPath);
		return -1;
	}

	while (res) {
		wsprintf(FullPath, _T("%s\\%s"), TempPath, wfd.cFileName);

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			SetFileAttributes(FullPath, FILE_ATTRIBUTE_NORMAL);
		}

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (lstrcmp(wfd.cFileName, _T("."))
				&& lstrcmp(wfd.cFileName, _T(".."))) {
				recur++;
				delete_all_csv(FullPath, recur);
				recur--;
			}
		}
		else {
			DeleteFile(FullPath);
		}

		res = FindNextFile(hSrch, &wfd);
	}

	FindClose(hSrch);

	if (recur > 0) 
		RemoveDirectory(TempPath);

	return 0;
}

wchar_t *find_timeline_file(wstring path) {

	struct _wfinddata_t file_search;
	long handle;

	wstringstream strm;
	strm << L"C:\\Temp\\result\\" << L"*Timeline.csv";
	handle = _tfindfirst(strm.str().c_str(), &file_search);

	if (handle == -1) {
		log_err "힝 읍써" log_end;
		return nullptr;
	}
	else {
		log_err "있써! %ws", file_search.name log_end;

		wchar_t *file_name = (wchar_t*)malloc(sizeof(wchar_t) * 260);
		wcscpy(file_name, file_search.name);
		return file_name;
	}
}

boolean read_csv(wchar_t *filename, map<string,string> *pdata)
{
	pair<map<string, string>::iterator, bool> pr;

	log_warn "[csv 파일 이름]" log_end;
	std::wstringstream strm;
	strm << L"C:\\Temp\\result\\" << filename;
	log_info "%ws", strm.str().c_str() log_end;

	ifstream in_stream;
	string line;
	in_stream.open(strm.str().c_str());

	if (!in_stream.good()) {
		log_err  "ifstream open err" log_end;
	}
	while (!in_stream.eof()) {
		getline(in_stream, line);
		if (line.length() <= 0 || line.find(",", 0) == string::npos) {
			continue;
		}

		char *token = strtok(const_cast<char *>(line.c_str()), ",");
		char value[30];
		strcpy(value, token);
		token = strtok(NULL, ",");
		pr = (*pdata).insert(pair<string, string>(string(token), string(value)));
		if (true == pr.second) {
			//log_warn "%s", line.c_str() log_end;
		}
		else {
			//log_err "Already exist" log_end;
		}
		
	}
	in_stream.close();
	return true;
}

string get_current_time() {
	//
	//	현재 시간을 time_t 타입으로 저장한다.
	//
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

boolean convert_date(map<string, string> csv_map, string cur) {
	for (map<string, string>::iterator iter = csv_map.begin(); iter != csv_map.end(); iter++)
	{
//		log_info "Key : %s  - Value : %s\n", iter->first.c_str(), iter->second.c_str() log_end;
		char *value = strtok(const_cast<char *>(iter->second.c_str()), " ");
		char date[30];
		strcpy(date, value);
		value = strtok(NULL, " ");
		char time[30];
		strcpy(time, value);
	}

	return true;
}