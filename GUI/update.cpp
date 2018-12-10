#include "stdafx.h"
#include "update.h"

typedef list< pair<std::string, std::string> > list_t;

bool get_update_info(list<pblackp> *black_list) {

	list_t update_list;
	if (!runCompare()) {
		log_err "runCompare() err" log_end;
	}
	if (!parse_compare(&update_list)) {
		log_err "parse_compare() err" log_end;
	}
	for (auto up : update_list) {
		log_info "%s %s", up.first.c_str(), up.second.c_str() log_end;
	}
	if (!find_veraport(black_list, update_list)) {
		log_err "find_veraport() err" log_end;
	}
}

bool runCompare(void) {
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


bool parse_compare(list_t *update_list) {
	std::wstring current_dir = get_current_module_dirEx();
	std::stringstream strm;
	strm << WcsToMbsEx(current_dir.c_str()) << "\\compare.json";
	log_info "Open file. file=%s", strm.str().c_str() log_end;

	CkJsonObject json;
	// json 파일 읽어오기
	bool success = json.LoadFile(strm.str().c_str());
	if (success != true) {
		log_err "%s", json.lastErrorText() log_end;
		return false;
	}

	// 은행 리스트 확인하기
	CkJsonArray *banks = json.ArrayOf("bank");
	if (banks == 0) {
		log_err "not found" log_end;
		return false;
	}

	// 은행 갯수
	int numBanks = banks->get_Size();

	// 각각 읽어오기
	for (int i = 0; i < numBanks; i++) {
		CkJsonObject *bankObj = banks->ObjectAt(i);
		bankObj->put_Utf8(true);
		
		update_list->push_back(
		make_pair(bankObj->stringOf("name"), 
		bankObj->stringOf("site")));
		delete bankObj;
	}
	delete banks;
	json.dispose();
	getchar();
}


bool find_veraport(list<pblackp> *black_list, list_t update_list) {
	
	for (auto black : *black_list) {
		wstring name_str = black->name();
		wstring veraport_str =  L"Veraport";

		if (name_str.find(veraport_str) != wstring::npos) {
			if (!update_list.empty()) {		//	리스트가 비어있지 않으면
				stringstream bank_str;
				for (auto update : update_list) {
					bank_str << update.first << " ";
				}

				wstringstream w_bank_str;
				w_bank_str << MbsToWcsEx(bank_str.str().c_str());
				log_info "%ws", MbsToWcsEx(bank_str.str().c_str()).c_str() log_end;
				black->setBank(w_bank_str.str());
				black->setUpdate(L"Y");
				log_info "setUpdate" log_end;
				
				
			}
		
		}
	}
	
	return true;
}