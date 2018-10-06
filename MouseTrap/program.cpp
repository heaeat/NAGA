
#include "stdafx.h"
#include "log.h"

bool get_installed_program(void)
{

	//
	//	파일 로그를 초기화한다. 
	// 
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\MouseTrap.log";
	if (true != initialize_log(log_mask_all,
		log_level_debug,
		log_to_file | log_to_con | log_to_ods,
		strm.str().c_str()))
	{
		fwprintf(stderr, L"initialize_log() fail. give up! \n");
		return -1;
	}

	//
	//	로그의 출력 형식을 지정한다. 
	//
	set_log_format(true,false,false,false);


	HKEY hUninstKey = NULL;
	HKEY hAppKey = NULL;
	TCHAR sAppKeyName[1024];
	TCHAR sSubKey[1024];
	TCHAR sDisplayName[1024];
	const TCHAR *sRoot = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	long lResult = ERROR_SUCCESS;
	DWORD dwType = KEY_ALL_ACCESS;
	DWORD dwBufferSize = 0;

	// uninstall key 받아오기
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot, 0, KEY_READ, &hUninstKey) != ERROR_SUCCESS)
	{
		return false;
	}

	for (DWORD dwIndex = 0; lResult == ERROR_SUCCESS; dwIndex++)
	{
		//모든 서브키 받아오기
		dwBufferSize = sizeof(sAppKeyName);
		if ((lResult = RegEnumKeyEx(hUninstKey, dwIndex, sAppKeyName,
			&dwBufferSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			//서브키 열기
			wsprintf(sSubKey, L"%s\\%s", sRoot, sAppKeyName);
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS)
			{
				RegCloseKey(hAppKey);
				RegCloseKey(hUninstKey);
				return false;
			}

			//서브키로 이름 가져옴!
			dwBufferSize = sizeof(sDisplayName);
			if (RegQueryValueEx(hAppKey, L"DisplayName", NULL,
				&dwType, (unsigned char*)sDisplayName, &dwBufferSize) == ERROR_SUCCESS)
			{
				fwprintf(stderr, L"%s\n", sDisplayName);
//				log_info "%s",sDisplayName log_end;
			}
			else {
//				 에러!
			}

			RegCloseKey(hAppKey);
		}
	}

	RegCloseKey(hUninstKey);

	//
	//	로그 모듈을 종료한다. 
	//	
	finalize_log();


	return true;
}