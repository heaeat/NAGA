#include "stdafx.h"
#include "prefetch.h"




void get_prefetch_info() {
	map<string, string> csv_map;
	wstring result_path  = run_PECmd();
	wstring file_name = find_timeline_file(result_path);
	read_csv(const_cast<wchar_t *>(file_name.c_str()), &csv_map);
}

wstring run_PECmd(void) 
{
	// 현재경로 받아오기
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	std::wstring result_path = L"\"C:\\Temp\\result\"";
	strm << current_dir << L"\\PECmd.exe -d \"C:\\Windows\\Prefetch\" --csv " << result_path;

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
		return  NULL;
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hThread); 
	CloseHandle(processInfo.hProcess);
	return result_path;
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

wstring find_timeline_file(wstring path) {

	struct _wfinddata64i32_t file_search;
	long handle;

	wstringstream strm;
	strm << path << L"\\*Timeline.csv";
	handle = _tfindfirst(strm.str().c_str(), &file_search);
	if (handle == -1) return NULL;
	return file_search.name;
}


boolean read_csv(wchar_t *filename, map<string,string> *pdata)
{
	pair<map<string, string>::iterator, bool> pr;

	log_warn "[csv 파일 이름]" log_end;
	log_info "%ws\n", filename log_end;
	ifstream in_stream;
	string line;
	in_stream.open(filename);
	while (!in_stream.eof()) {
		getline(in_stream, line);
		if (line.length() <= 0 || line.find(",", 0) == string::npos) {
			continue;
		}
		char *token = strtok(const_cast<char *>(line.c_str()), ",");
		char value[30];
		strcpy(value, token);
		replace_string(line, token, "");
		pr = (*pdata).insert(pair<string, string>(line, value));
		if (true == pr.second) {
			log_info "%ws\n", line log_end;
		}
		else {
			cout << "Already exist ";
		}
	}
	in_stream.close();
	return true;
}

void replace_string(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace); pos += replace.length();
	}
}
