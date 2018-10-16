#pragma once
#pragma warning(disable:4996)

#include "log.h"

#include <errno.h>

#include <iostream>
#include <string>
#include <Msi.h>
#include <stdio.h>

#include <map>

#include <CkJsonObject.h>
#include <CkJsonArray.h>

#pragma comment(lib, "msi.lib")
#pragma comment(lib,"ChilkatRelDll_x64.lib")

using namespace std;


bool get_installed_program(void);
const wchar_t *convert_char(const char *c);
void read_json(void);
list<pprogram> compare_lists();
