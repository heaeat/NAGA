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


using namespace std;

#pragma comment(lib, "msi.lib")
#pragma comment(lib,"ChilkatRelDll_x64.lib")

void get_prefetch_info();
wstring run_PECmd(void);
int delete_all_csv(LPCWSTR szDir, int recur);
wstring find_timeline_file(wstring strm);
boolean read_csv(wchar_t *filename, map<string, string> *pdata);
void replace_string(std::string& subject, const std::string& search, const std::string& replace);
