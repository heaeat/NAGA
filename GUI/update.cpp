#include "stdafx.h"
#include "update.h"

bool get_update_info(list<pblackp> *black_list) {

	list<std::string> update_list;
	if (!runCompare()) {
		log_err "runCompare() err" log_end;
	}
	if (!parse_compare(&update_list)) {
		log_err "parse_compare() err" log_end;
	}

	if (!parse_line(&update_list)) {
		log_err "parse_line() err" log_end;
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


bool parse_compare(list<std::string> *update_list) {
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\comapre.json";

	PFILE_CTX file_context = nullptr;
	if (true != OpenFileContext(strm.str().c_str(), true, file_context))
	{
		log_err "Can not open file. file=%ws", strm.str().c_str() log_end;
		return false;
	}
	log_info "Open file. file=%ws", strm.str().c_str() log_end;
	SmrtFileCtx context_guard(file_context);

	//
	// read line (0x0d0a)
	//
	char* buf = file_context->FileView;
	uint32_t prev = 0;
	uint32_t curr = 0;

	while (curr < file_context->FileSize)
	{
		if (buf[curr] == 0x0D && buf[curr + 1] == 0x0A)
		{
			uint32_t length = (curr - prev) * sizeof(char);
			char_ptr utf8_line((char*)malloc(length + sizeof(char)), [](char* p) {if (nullptr != p) free(p); });
			if (nullptr == utf8_line.get())
			{
				log_err "not enough memory. give up." log_end;
				return false;
			}
			memcpy(utf8_line.get(), &buf[prev], length);
			utf8_line.get()[length] = 0x00;

			//
			// update position pointers
			//
			curr += 2;
			prev = curr;

			//
			// UTF-8 string --> Wide Char string (windows default)
			//
			std::wstring wcs_string = Utf8MbsToWcsEx(utf8_line.get());
			std::string utf8_string = utf8_line.get();

			//
			// ToDo. 
			// line 문자열 파싱해서 필요한 작업하기 
			//
			std::string left_string = "[";
			std::string right_string = "]";

			if ((utf8_string.find(left_string) != std::string::npos) || (utf8_string.find(right_string) != std::string::npos)) {
				continue;
			}
			update_list->push_back(utf8_string);
		}
		else
		{
			++curr;
		}
	};
}

bool parse_line(list<std::string> *update_list) {
	list<std::string>::iterator iter;
	string quote_string = "\"";
	for (iter = update_list->begin(); iter != update_list->end(); iter++)
	{

		size_t location = (*iter).find(quote_string);
		if (location != string::npos) {
			log_info "%d", location log_end;
		}
		size_t s_location = (*iter).find(quote_string, location + 1);
		if (s_location != string::npos) {
			log_info "%d", s_location log_end;
		}
		string pure_name = (*iter).substr(location + 1, s_location - 2);
		(*iter) = pure_name;
	}
}

bool find_veraport(list<pblackp> *black_list, list<std::string> update_list) {
	for (auto black : *black_list) {
		stringstream name_str;
		name_str << WcsToMbsUTF8Ex(black->name());

		stringstream veraport_str;
		veraport_str << "Veraport";
		if (name_str.str().find(veraport_str.str()) != string::npos) {
			if (!update_list.empty()) {		//	리스트가 비어있지 않으면
				stringstream bank_str;
				for (auto update : update_list) {
					bank_str << update << " ";
				}
				wstringstream w_bank_str;
				w_bank_str << Utf8MbsToWcs(bank_str.str().c_str());
				black->setBank(w_bank_str.str().c_str());
				black->setUpdate(L"Y");
			}
			
		}
	}
}