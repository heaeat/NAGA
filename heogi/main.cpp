#include<stdio.h>
#include<windows.h>
#include<malloc.h>
#include<winreg.h>
#include<stdlib.h>


// ASTX : "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{19DD1D8D-927F-45DF-ADF4-75D38267848D}"

int Reg_Read(const char* subkey, TCHAR* value) {
	LONG ret;
	DWORD data_size = 1024;
	DWORD data_type;
	TCHAR* data_buffer = (TCHAR*)malloc(data_size);
	HKEY hKey;

	RtlZeroMemory(data_buffer, sizeof(data_buffer));

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		subkey,
		0, KEY_ALL_ACCESS, &hKey);
	
	if (ret != ERROR_SUCCESS) {
		printf("RegOpenKey Failed! \n ");
		return 0;
	}
	
	//memset(data_buffer, 0, sizeof(data_buffer));
	//data_size = sizeof(data_buffer);
	RegQueryValueEx(hKey, "UninstallString", 0, &data_type, (LPBYTE)data_buffer, (DWORD*)&data_size);
	RegCloseKey(hKey);
	value = data_buffer;
	printf("Value : %s\n", value);
	free(data_buffer);
	data_buffer = NULL;
	return 1;
}

int main() {
	
	TCHAR* value=nullptr;
	const char* subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{19DD1D8D-927F-45DF-ADF4-75D38267848D}";
	Reg_Read(subkey, value);
	printf("Value : %s\n", value);
	getchar();
	
}