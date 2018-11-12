#ifndef _PH_VERIFY_H
#define _PH_VERIFY_H

#include <wintrust.h>
#include <softpub.h>

typedef GUID *PGUID;
typedef LONG NTSTATUS;
#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)
#define PH_VERIFY_DEFAULT_SIZE_LIMIT (32 * 1024 * 1024)



typedef enum _VERIFY_RESULT
{
    VrUnknown = 0,
    VrNoSignature,
    VrTrusted,
    VrExpired,
    VrRevoked,
    VrDistrust,
    VrSecuritySettings,	// cryptographic operation failed due to a local security option setting.
    VrBadSignature
} VERIFY_RESULT, *PVERIFY_RESULT;

#define PH_VERIFY_PREVENT_NETWORK_ACCESS 0x1
#define PH_VERIFY_VIEW_PROPERTIES 0x2

typedef struct signature_info
{
public:
	signature_info(PCERT_CONTEXT *signatures,
				   ULONG number_of_signatures,
				   VERIFY_RESULT result)
		:
		_signatures(signatures),
		_number_of_signatures(number_of_signatures),
		_result(result)
	{}

	PCERT_CONTEXT *_signatures;
	ULONG _number_of_signatures;
	VERIFY_RESULT _result;
} *psignature_info;

typedef struct _PH_VERIFY_FILE_INFO
{
	HANDLE FileHandle;
    ULONG Flags; // PH_VERIFY_*

    ULONG FileSizeLimitForHash; // 0 for PH_VERIFY_DEFAULT_SIZE_LIMIT, -1 for unlimited
    ULONG NumberOfCatalogFileNames;
    PWSTR *CatalogFileNames;

	std::list<psignature_info> *secondary_signatures;

    HWND hWnd; // for PH_VERIFY_VIEW_PROPERTIES
} PH_VERIFY_FILE_INFO, *PPH_VERIFY_FILE_INFO;

bool PhpVerifyInitialize();
void PhpVerifyFinalize();


VERIFY_RESULT
PhVerifyFile(
	_In_ const wchar_t* file_name,
	_Out_ std::wstring* signer_name
	);

VERIFY_RESULT
PhVerifyFile(
	_In_ HANDLE file_handle,
	_Out_ std::wstring* signer_name
);

VERIFY_RESULT
PhVerifyFile(
	_In_ const wchar_t* file_name,
	_Out_ std::list<std::wstring> &signer_names
);

VERIFY_RESULT
PhVerifyFile(
	_In_ HANDLE file_handle,
	_Out_ std::list<std::wstring> &signer_names
);



#endif
