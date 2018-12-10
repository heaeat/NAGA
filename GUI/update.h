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
#include "FileIoHelper.h"

using namespace std;
#pragma comment(lib, "msi.lib")
#pragma comment(lib,"ChilkatRelDll_x64.lib")

typedef list< pair<std::string, std::string> > list_t;

bool get_update_info(list<pblackp> *black_list);
bool runCompare(void);
bool parse_compare(list_t*update_list);
bool find_veraport(list<pblackp> *black_list, list_t update_list);
list_t get_update_list(void);
