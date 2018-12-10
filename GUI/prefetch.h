#pragma once
#pragma warning(disable:4996)

#include <iostream>
#include <string>
#include <Msi.h>
#include <stdio.h>

#include <map>
#include <CkJsonObject.h>
#include <CkJsonArray.h>


#include <fstream>
#include <sstream>
#include <istream>
#include <stdlib.h>
#include "list.h"
#include "phverify/verify.h"


using namespace std;

#pragma comment(lib, "msi.lib")
#pragma comment(lib,"ChilkatRelDll_x64.lib")

#define DAYCONTROL 60


/// prefetch 관련 함수
bool get_prefetch_info(list<punknownp> *unknown_list);
bool run_PECmd(void);
int delete_all_csv(LPCWSTR szDir, int recur);
wchar_t * find_timeline_file(wstring strm);
bool read_csv(wchar_t *filename, list<punknownp> *unknown_list);
bool check_recently_used(list<punknownp> *unknown_list);
FILETIME str_to_filetime(string &sTime);

/// volume과 관련된 함수
void get_volume_name();
void get_volume_serial();
void get_volume_path(__in PWCHAR VolumeName);
void parse_volume_serial(list<punknownp> *unknown_list);

///	인증서 관련 함수
bool check_certification(list<punknownp> *unknown_list);

/// 버젼 관련 함수
bool check_version(list<punknownp> *unknown_list);