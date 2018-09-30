#include<stdio.h>
#include<windows.h>
#include<tchar.h>	//for TCHAR
#include<crtdbg.h> //for _ASSERTE
#include<malloc.h>
#include<winreg.h>
#include<stdlib.h>

/*
리턴 값의 의미가 모호하다. V
입력값 검증이 없다. V

변수의 사용
사용하는 시점에 선언하자. 
변수는 선언시 초기화하자. 

리소스 관리
할당한 메모리는 반드시 소멸하자. V
시스템 리소스 (레지스트리 키 핸들 등)는 반드시 반환해야 한다. V
유니코드 함수와 안시코드 함수, 변수를 구분하자.
char* 변수를 입력으로 받는 함수는 RegOpenKeyExA()
wchar* 변수를 입력으로 받는 함수는 RegOpenKeyExW()
귀찮으면 TCHAR* 타입의 변수를 사용하고, RegOpenKeyEx()
당연히 printf 도 _tprintf() 를 사용해야 합니다. V
char*, wchar* 를 정확히 인지하지 않은상태에서 혼용하면 안됨 
에러로그를 남길때는 최소한 어떤 이유로 에러가 났는지 추적에 필요한 단서를 남기자. 
리턴값 또는 GetLastError() 코드 정도는 기록하자. V
에러로그는 에러 추적을 위해 남기는 것이다. 
리턴값이 있는 외부 함수를 호출했으면 반드시 리턴 값을 확인하자. V

함수 설계 상의 문제 
value 값이 output 파라미터로 사용되고 있으나
value 값의 타입을 어디에서도 정확히 명시 하지 않고 있다(string 인지, dword 인지). 
전체적인 느낌으로는 string 타입이라고 가정하고 작성한 것 같네요.
아주 심각한 문제인데, 소멸된 메모리에 접근하는 문제가 있습니다. 
out-parameter 로 사용된 value 가 가리키는 포인터는 이미 소멸되었습니다.

*/


// ASTX : "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{19DD1D8D-927F-45DF-ADF4-75D38267848D}"

bool Reg_Read(_In_z_ const char* subkey, _Outptr_ TCHAR* value) { // SAL ?? 함수 인자의 용도에대한 주석

	_ASSERTE(subkey != nullptr);
	_ASSERTE(value == nullptr);
	if (subkey == nullptr || value != nullptr) {
		return false;
	}

	LONG ret;
	DWORD data_size = 1024;
	DWORD data_type;
	TCHAR* data_buffer = (TCHAR*)malloc(data_size);
	HKEY hKey;

	RtlZeroMemory(data_buffer, sizeof(data_buffer));

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		subkey, 0, KEY_READ, &hKey);

	if (ret != ERROR_SUCCESS) {
		_tprintf("RegOpenKey Failed! \n ");
		_tprintf("error code %d\n", GetLastError());
		return false;
	}

	_ASSERTE(hKey != nullptr);
	_ASSERTE(data_buffer != nullptr);

	RegQueryValueEx(hKey, "UninstallString", 0, &data_type, (LPBYTE)data_buffer, (DWORD*)&data_size);
	RegCloseKey(hKey);

	value = data_buffer;
	_tprintf("Value : %p ##  %s\n", value, value);
	data_buffer = NULL;
	return true;
}

int main() {
	TCHAR* value = nullptr;
	const char* subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{19DD1D8D-927F-45DF-ADF4-75D38267848D}";
	if (Reg_Read(subkey, value) == false) {
		_tprintf("Reg_Read error! : %d\n", GetLastError());
		return false;
	}
	//_ASSERTE(value != nullptr);
	_tprintf("Value : %p ##  %s\n", value, value); //여전히 value는 소멸된건가?  value = data_buffer; 하고 free 안했는디.
	free(value);
	getchar();

	return true;
}