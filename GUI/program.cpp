#pragma warning(disable:4996)

#include "stdafx.h"
#include "program.h"
using namespace std;

std::list<pprogram> softwares;
std::list<pprogram> removers;


//	제거대상 목록과 설치된 프로그램을 비교하기 위한 함수
bool compare_lists(std::list<pprogram> *my_list) {

	get_installed_program();
	read_json();
	
	int result;

	for (auto remover : removers)
	{
		for (auto software : softwares) {
			result = wcscmp(software->id(), remover->id());
			pprogram temp;
			wstring guid;

			if (result == 0) {
				//
				//	yessign 예외처리
				//
				guid = remover->id();
				wstring yes_id = L"yessign7ActiveX";
				if (guid.find(yes_id) != wstring::npos) {
					std::wstringstream path_strm;
					path_strm << L"C:\\Windows\\SysWOW64\\yessign7Clear.exe";
					temp = new program(software->id(), software->name(), software->version(), software->version(), path_strm.str().c_str());
					my_list->push_back(temp);
					continue;
				}

				//
				//	사일런트 옵션이 존재할 경우 경로 뒤에 붙여줌
				//
				if (wcslen(remover->uninstaller()) > 0) {
					std::wstring path = L"";
					std::wstringstream path_strm;
					path_strm << software->uninstaller() << remover->uninstaller();
					temp = new program(software->id(), software->name(), software->version(), software->version(), path_strm.str().c_str());
				}
				else {
					temp = new program(software->id(), software->name(), software->version(), software->version(), software->uninstaller());
				}
				my_list->push_back(temp);
			}
		}
	}

	for (auto software : softwares) {
		delete software;
	}

	for (auto remover : removers) {
		delete remover;
	}

	softwares.clear();
	removers.clear();

	return true;
}

// 설치된 프로그램을 받아오기 위한 함수
bool get_installed_program(void)
{
	
	softwares.clear();
	get_installed_programs(softwares);

	log_warn "[ 설치된 파일 목록 ]" log_end;

	for (auto software : softwares)
	{
		log_info
			",%ws,%ws,%ws,%ws",
			software->id(),
			software->name(),
			software->version(),
			software->uninstaller()
			log_end;
	}
	getchar();	
	return true;
	
}

bool get_all_program(list<pprogram> *installed_list) {
	get_installed_programs(*installed_list);
	return true;
}

// json 파일을 읽어와 파싱하는 함수
void read_json(void) {
	
	log_warn "[ 제거할 파일 목록 ]" log_end;

	removers.clear();
	
	CkJsonObject json;

	
	// json 파일 읽어오기
	bool success = json.LoadFile("result.json");
	if (success != true) {
		cout << json.lastErrorText() << "\r\n";
		return;
	}

	// 제거대상 리스트 확인하기
	CkJsonArray *programs = json.ArrayOf("programs");
	if (programs == 0) {
		cout << "not found" << "\r\n";
		return;
	}
	
	// 제거대상 갯수
	int numPrograms = programs->get_Size();

	// 각각 읽어오기
	for (int i = 0; i < numPrograms; i++) {
		CkJsonObject *programObj = programs->ObjectAt(i);
		pprogram temp = new program(MbsToWcsEx(programObj->stringOf("guid")).c_str(),
									MbsToWcsEx(programObj->stringOf("name")).c_str(),
									L"",
									L"",
									MbsToWcsEx(programObj->stringOf("silent")).c_str());
		removers.push_back(temp);
		delete programObj;
	}
	delete programs;
	
	json.dispose();
	getchar();

}
