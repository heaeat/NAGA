/*
 * Process Hacker -
 *   image verification
 *
 * Copyright (C) 2009-2013 wj32
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "verify.h"
#include "verifyp.h"

bool
PhVerifyFileEx(
	_In_ PPH_VERIFY_FILE_INFO Information,
	_Out_ VERIFY_RESULT *VerifyResult,
	_Out_opt_ PCERT_CONTEXT **Signatures,
	_Out_opt_ PULONG NumberOfSignatures
);

VOID
PhFreeVerifySignatures(
	_In_ PCERT_CONTEXT *Signatures,
	_In_ ULONG NumberOfSignatures
);

bool
PhGetSignerNameFromCertificate(
	_In_ PCERT_CONTEXT Certificate,
	_Out_ std::wstring& SignerName
);



static boost::mutex _init_lock;
static bool _initialized = false;
static GUID WinTrustActionGenericVerifyV2 = WINTRUST_ACTION_GENERIC_VERIFY_V2;
static GUID DriverActionVerify = DRIVER_ACTION_VERIFY;

static HANDLE _heap_handle = NULL;
static HMODULE wintrust = NULL;
static HMODULE crypt32 = NULL;
static _CryptCATAdminCalcHashFromFileHandle CryptCATAdminCalcHashFromFileHandle = nullptr;
static _CryptCATAdminCalcHashFromFileHandle2 CryptCATAdminCalcHashFromFileHandle2 = nullptr;	// after Win8
static _CryptCATAdminAcquireContext CryptCATAdminAcquireContext = nullptr;
static _CryptCATAdminAcquireContext2 CryptCATAdminAcquireContext2 = nullptr;					// after Win8
static _CryptCATAdminEnumCatalogFromHash CryptCATAdminEnumCatalogFromHash = nullptr;
static _CryptCATCatalogInfoFromContext CryptCATCatalogInfoFromContext = nullptr;
static _CryptCATAdminReleaseCatalogContext CryptCATAdminReleaseCatalogContext = nullptr;
static _CryptCATAdminReleaseContext CryptCATAdminReleaseContext = nullptr;
static _WTHelperProvDataFromStateData WTHelperProvDataFromStateData_I = nullptr;
static _WTHelperGetProvSignerFromChain WTHelperGetProvSignerFromChain_I = nullptr;
static _WinVerifyTrust WinVerifyTrust_I = nullptr;
static _CertNameToStr CertNameToStr_I = nullptr;
static _CertDuplicateCertificateContext CertDuplicateCertificateContext_I = nullptr;
static _CertFreeCertificateContext CertFreeCertificateContext_I = nullptr;



///
#define VF_READY(err_ret) \
do \
{ \
	boost::lock_guard<boost::mutex> lock(_init_lock); \
	if (true != _initialized) \
	{ \
		log_err "Not initialized." log_end; \
		return err_ret; \
	} \
} while(false)


__inline PVOID PhAllocate(SIZE_T Size)
{
	return HeapAlloc(_heap_handle, 0, Size);
}

__inline VOID PhFree(_Frees_ptr_opt_ PVOID Memory)
{ 
	HeapFree(_heap_handle, 0, Memory);
}













///
bool PhpVerifyInitialize()
{
	boost::lock_guard<boost::mutex> lock(_init_lock);
	
	//
	//	create heap for verifier
	// 
	_heap_handle = HeapCreate(0, 0, 0);
	if (NULL == _heap_handle)
	{
		log_err "HeapCreate() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	initialize wintrust
	// 
	wintrust = LoadLibraryW(L"wintrust.dll");
	crypt32 = LoadLibraryW(L"crypt32.dll");

	if (NULL == wintrust || NULL == crypt32)
	{
		log_err "Can not load wintrust|crypt32. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

    CryptCATAdminCalcHashFromFileHandle = (_CryptCATAdminCalcHashFromFileHandle)GetProcAddress(wintrust, "CryptCATAdminCalcHashFromFileHandle");
    CryptCATAdminCalcHashFromFileHandle2 = (_CryptCATAdminCalcHashFromFileHandle2)GetProcAddress(wintrust, "CryptCATAdminCalcHashFromFileHandle2");
    CryptCATAdminAcquireContext = (_CryptCATAdminAcquireContext)GetProcAddress(wintrust, "CryptCATAdminAcquireContext");
    CryptCATAdminAcquireContext2 = (_CryptCATAdminAcquireContext2)GetProcAddress(wintrust, "CryptCATAdminAcquireContext2");
    CryptCATAdminEnumCatalogFromHash = (_CryptCATAdminEnumCatalogFromHash)GetProcAddress(wintrust, "CryptCATAdminEnumCatalogFromHash");
    CryptCATCatalogInfoFromContext = (_CryptCATCatalogInfoFromContext)GetProcAddress(wintrust, "CryptCATCatalogInfoFromContext");
    CryptCATAdminReleaseCatalogContext = (_CryptCATAdminReleaseCatalogContext)GetProcAddress(wintrust, "CryptCATAdminReleaseCatalogContext");
    CryptCATAdminReleaseContext = (_CryptCATAdminReleaseContext)GetProcAddress(wintrust, "CryptCATAdminReleaseContext");
    WTHelperProvDataFromStateData_I = (_WTHelperProvDataFromStateData)GetProcAddress(wintrust, "WTHelperProvDataFromStateData");
    WTHelperGetProvSignerFromChain_I = (_WTHelperGetProvSignerFromChain)GetProcAddress(wintrust, "WTHelperGetProvSignerFromChain");
    WinVerifyTrust_I = (_WinVerifyTrust)GetProcAddress(wintrust, "WinVerifyTrust");
    CertNameToStr_I = (_CertNameToStr)GetProcAddress(crypt32, "CertNameToStrW");
    CertDuplicateCertificateContext_I = (_CertDuplicateCertificateContext)GetProcAddress(crypt32, "CertDuplicateCertificateContext");
    CertFreeCertificateContext_I = (_CertFreeCertificateContext)GetProcAddress(crypt32, "CertFreeCertificateContext");
	
	if (NULL == CryptCATAdminCalcHashFromFileHandle ||
		/* NULL == CryptCATAdminCalcHashFromFileHandle2 || */
		NULL == CryptCATAdminAcquireContext ||
		/* NULL == CryptCATAdminAcquireContext2 || */
		NULL == CryptCATAdminEnumCatalogFromHash ||
		NULL == CryptCATCatalogInfoFromContext ||
		NULL == CryptCATAdminReleaseCatalogContext ||
		NULL == CryptCATAdminReleaseContext ||
		NULL == WTHelperProvDataFromStateData_I ||
		NULL == WTHelperGetProvSignerFromChain_I ||
		NULL == WinVerifyTrust_I ||
		NULL == CertNameToStr_I ||
		NULL == CertDuplicateCertificateContext_I ||
		NULL == CertFreeCertificateContext_I)
	{
		FreeLibrary(wintrust);
		FreeLibrary(crypt32);
		return false;
	}

	_initialized = true;
	return true;
}


///
void PhpVerifyFinalize()
{
	boost::lock_guard<boost::mutex> lock(_init_lock);
	if (true != _initialized) return;

	//
	//	finalize wintrust
	// 
	FreeLibrary(wintrust);
	FreeLibrary(crypt32);

	CryptCATAdminCalcHashFromFileHandle = nullptr;
	CryptCATAdminCalcHashFromFileHandle2 = nullptr;
	CryptCATAdminAcquireContext = nullptr;
	CryptCATAdminAcquireContext2 = nullptr;
	CryptCATAdminEnumCatalogFromHash = nullptr;
	CryptCATCatalogInfoFromContext = nullptr;
	CryptCATAdminReleaseCatalogContext = nullptr;
	CryptCATAdminReleaseContext = nullptr;
	WTHelperProvDataFromStateData_I = nullptr;
	WTHelperGetProvSignerFromChain_I = nullptr;
	WinVerifyTrust_I = nullptr;
	CertNameToStr_I = nullptr;
	CertDuplicateCertificateContext_I = nullptr;
	CertFreeCertificateContext_I = nullptr;

	//
	//	destroy heap
	// 
	if (NULL != _heap_handle)
	{
		HeapDestroy(_heap_handle);
		_heap_handle = NULL;
	}

	_initialized = false;
}


///
VERIFY_RESULT 
PhpStatusToVerifyResult(
    _In_ LONG Status
    )
{
    switch (Status)
    {
    case 0:
        return VrTrusted;
    case TRUST_E_NOSIGNATURE:
        return VrNoSignature;
    case CERT_E_EXPIRED:
        return VrExpired;
    case CERT_E_REVOKED:
        return VrRevoked;
    case TRUST_E_EXPLICIT_DISTRUST:
        return VrDistrust;
    case CRYPT_E_SECURITY_SETTINGS:
        return VrSecuritySettings;
    case TRUST_E_BAD_DIGEST:
        return VrBadSignature;
    default:
        return VrSecuritySettings;
    }
}


///
BOOLEAN PhpGetSignaturesFromStateData(
    _In_ HANDLE StateData,
    _Out_ PCERT_CONTEXT **Signatures,
    _Out_ PULONG NumberOfSignatures
    )
{
    PCRYPT_PROVIDER_DATA provData;
    PCRYPT_PROVIDER_SGNR sgnr;
    PCERT_CONTEXT *signatures;
    ULONG i;
    ULONG numberOfSignatures;
    ULONG index;

    provData = WTHelperProvDataFromStateData_I(StateData);

    if (!provData)
    {
        *Signatures = NULL;
        *NumberOfSignatures = 0;
        return FALSE;
    }

    i = 0;
    numberOfSignatures = 0;
    for(;;)
    {
		sgnr = WTHelperGetProvSignerFromChain_I(provData, i, FALSE, 0);
		if (nullptr == sgnr) break;

        if (sgnr->csCertChain != 0)
            numberOfSignatures++;

        i++;
    }

    if (numberOfSignatures != 0)
    {
        signatures = (PCERT_CONTEXT*)PhAllocate(numberOfSignatures * sizeof(PCERT_CONTEXT));
        i = 0;
        index = 0;

        for (;;)
        {
			sgnr = WTHelperGetProvSignerFromChain_I(provData, i, FALSE, 0);
			if (nullptr == sgnr) break;

            if (sgnr->csCertChain != 0)
                signatures[index++] = (PCERT_CONTEXT)CertDuplicateCertificateContext_I(sgnr->pasCertChain[0].pCert);

            i++;
        }
    }
    else
    {
        signatures = NULL;
    }

    *Signatures = signatures;
    *NumberOfSignatures = numberOfSignatures;

    return TRUE;
}

///
VOID PhpViewSignerInfo(
    _In_ PPH_VERIFY_FILE_INFO Information,
    _In_ HANDLE StateData
    )
{
	static _CryptUIDlgViewSignerInfo cryptUIDlgViewSignerInfo = NULL;
	if (NULL == cryptUIDlgViewSignerInfo)
	{
		HMODULE cryptui = LoadLibrary(L"cryptui.dll");
		cryptUIDlgViewSignerInfo = (_CryptUIDlgViewSignerInfo)GetProcAddress(cryptui, 
																			 "CryptUIDlgViewSignerInfoW");		
	}
	
    if (cryptUIDlgViewSignerInfo)
    {
        CRYPTUI_VIEWSIGNERINFO_STRUCT viewSignerInfo = { sizeof(CRYPTUI_VIEWSIGNERINFO_STRUCT) };
        PCRYPT_PROVIDER_DATA provData;
        PCRYPT_PROVIDER_SGNR sgnr;

		provData = WTHelperProvDataFromStateData_I(StateData);
        if (!provData)
            return;

		sgnr = WTHelperGetProvSignerFromChain_I(provData, 0, FALSE, 0);
        if (!sgnr)
            return;

        viewSignerInfo.hwndParent = Information->hWnd;
        viewSignerInfo.pSignerInfo = sgnr->psSigner;
        viewSignerInfo.hMsg = provData->hMsg;
        viewSignerInfo.pszOID = szOID_PKIX_KP_CODE_SIGNING;
        cryptUIDlgViewSignerInfo(&viewSignerInfo);
    }
}

///
///	@bref	서명확인을 한다.
///	@note	서명여부만 확인하고 싶을때가 있다.(즉, 2차 서명정보가 필요없을때가 있다.)
///			때문에 불필요한 2차 서명정보를 구하는것을 방지하기 위해 secondary_signatures를 받을때만 
///			2차 서명정보를 구하도록 한다.
///	@param	secondary_signatures	secondary signatures를 얻고 싶으면 
///									std::list<psignature_info> 타입으로 넘겨주면 얻을 수 있다.
///

VERIFY_RESULT 
PhpVerifyFile(
    _In_ PPH_VERIFY_FILE_INFO Information,
    _In_ ULONG UnionChoice,
    _In_ PVOID UnionData,
    _In_ PGUID ActionId,
    _In_opt_ PVOID PolicyCallbackData,
    _Out_ PCERT_CONTEXT **Signatures,
	_Out_ PULONG NumberOfSignatures
    )
{
    WINTRUST_DATA trustData = { 0 };

    trustData.cbStruct = sizeof(WINTRUST_DATA);
    trustData.pPolicyCallbackData = PolicyCallbackData;
    trustData.dwUIChoice = WTD_UI_NONE;
    trustData.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
    trustData.dwUnionChoice = UnionChoice;
    trustData.dwStateAction = WTD_STATEACTION_VERIFY;
    trustData.dwProvFlags = WTD_SAFER_FLAG;
		
    trustData.pFile = (WINTRUST_FILE_INFO*)UnionData;

    if (UnionChoice == WTD_CHOICE_CATALOG)
        trustData.pCatalog = (WINTRUST_CATALOG_INFO*)UnionData;

    if (Information->Flags & PH_VERIFY_PREVENT_NETWORK_ACCESS)
    {
        trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
        trustData.dwProvFlags |= WTD_CACHE_ONLY_URL_RETRIEVAL;
    }

	WINTRUST_SIGNATURE_SETTINGS wss = {};
	if (nullptr != Information->secondary_signatures)
	{
		//
		//	First verify the primary signature (index 0) to determine how many secondary signatures
		//	are present. 
		//	We use WSS_VERIFY_SPECIFIC and dwIndex to do this, also setting
		//	WSS_GET_SECONDARY_SIG_COUNT to have the number of secondary signatures returned.
		//
		wss.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
		wss.dwFlags = WSS_GET_SECONDARY_SIG_COUNT | WSS_VERIFY_SPECIFIC;
		wss.dwIndex = 0;
		trustData.pSignatureSettings = &wss;
	}
	
    LONG status = WinVerifyTrust_I(NULL, ActionId, &trustData);
    PhpGetSignaturesFromStateData(trustData.hWVTStateData, 
								  Signatures, 
								  NumberOfSignatures);
    if (status == 0 && (Information->Flags & PH_VERIFY_VIEW_PROPERTIES))
        PhpViewSignerInfo(Information, trustData.hWVTStateData);

	if (nullptr != Information->secondary_signatures)
	{		
		// Now attempt to verify all secondary signatures that were found
		PCERT_CONTEXT *s_signatures;
		ULONG s_number_of_signatures;

		for (DWORD x = 1; x <= trustData.pSignatureSettings->cSecondarySigs; x++)
		{
			// Close the state data.
			trustData.dwStateAction = WTD_STATEACTION_CLOSE;
			WinVerifyTrust_I(NULL, ActionId, &trustData);
			trustData.hWVTStateData = NULL;

			// Caller must reset dwStateAction as it may have been changed during the last call
			trustData.dwStateAction = WTD_STATEACTION_VERIFY;
			trustData.pSignatureSettings->dwIndex = x;
			status = WinVerifyTrust_I(NULL, ActionId, &trustData);

			if (TRUE != PhpGetSignaturesFromStateData(trustData.hWVTStateData,
														&s_signatures,
														&s_number_of_signatures))
			{
				log_err "PhpGetSignaturesFromStateData(idx=%u) failed. SecondarySign count=%u",
					x,
					trustData.pSignatureSettings->cSecondarySigs
					log_end;
				break;
			}
			if (status == 0 && (Information->Flags & PH_VERIFY_VIEW_PROPERTIES))
				PhpViewSignerInfo(Information, trustData.hWVTStateData);

			psignature_info si = new signature_info(s_signatures,
													s_number_of_signatures,
													PhpStatusToVerifyResult(status));
			if (nullptr == si)
			{
				log_err "Can't allocate signature_info." log_end;
				break;
			}
			Information->secondary_signatures->push_back(si);
		}		
	}

    // Close the state data.
    trustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust_I(NULL, ActionId, &trustData);

    return PhpStatusToVerifyResult(status);
}

///
BOOLEAN PhpCalculateFileHash(
    _In_ HANDLE FileHandle,
    _In_ PWSTR HashAlgorithm,
    _Out_ PUCHAR *FileHash,
    _Out_ PULONG FileHashLength,
    _Out_ HANDLE *CatAdminHandle
    )
{
    HANDLE catAdminHandle;
    PUCHAR fileHash;
    ULONG fileHashLength;

    if (CryptCATAdminAcquireContext2)
    {
        if (!CryptCATAdminAcquireContext2(&catAdminHandle, &DriverActionVerify, HashAlgorithm, NULL, 0))
            return FALSE;
    }
    else
    {
        if (!CryptCATAdminAcquireContext(&catAdminHandle, &DriverActionVerify, 0))
            return FALSE;
    }

    fileHashLength = 32;
    fileHash = (PUCHAR)PhAllocate(fileHashLength);

    if (CryptCATAdminCalcHashFromFileHandle2)
    {
        if (!CryptCATAdminCalcHashFromFileHandle2(catAdminHandle, FileHandle, &fileHashLength, fileHash, 0))
        {
            PhFree(fileHash);
            fileHash = (PUCHAR)PhAllocate(fileHashLength);

            if (!CryptCATAdminCalcHashFromFileHandle2(catAdminHandle, FileHandle, &fileHashLength, fileHash, 0))
            {
                CryptCATAdminReleaseContext(catAdminHandle, 0);
                PhFree(fileHash);
                return FALSE;
            }
        }
    }
    else
    {
        if (!CryptCATAdminCalcHashFromFileHandle(FileHandle, &fileHashLength, fileHash, 0))
        {
            PhFree(fileHash);
            fileHash = (PUCHAR)PhAllocate(fileHashLength);

            if (!CryptCATAdminCalcHashFromFileHandle(FileHandle, &fileHashLength, fileHash, 0))
            {
                CryptCATAdminReleaseContext(catAdminHandle, 0);
                PhFree(fileHash);
                return FALSE;
            }
        }
    }

    *FileHash = fileHash;
    *FileHashLength = fileHashLength;
    *CatAdminHandle = catAdminHandle;

    return TRUE;
}

///
VERIFY_RESULT PhpVerifyFileFromCatalog(
    _In_ PPH_VERIFY_FILE_INFO Information,
    _In_opt_ PWSTR HashAlgorithm,
    _Out_ PCERT_CONTEXT **Signatures,
    _Out_ PULONG NumberOfSignatures
    )
{
    VERIFY_RESULT verifyResult = VrNoSignature;
    PCERT_CONTEXT *signatures;
    ULONG numberOfSignatures;
    WINTRUST_CATALOG_INFO catalogInfo = { 0 };
    LARGE_INTEGER fileSize;
    ULONG fileSizeLimit;
    PUCHAR fileHash;
    ULONG fileHashLength;    
    HANDLE catAdminHandle;
    HANDLE catInfoHandle;
    ULONG i;

    *Signatures = NULL;
    *NumberOfSignatures = 0;

	if (!get_file_size(Information->FileHandle, (int64_t)fileSize.QuadPart))
	{
		log_err "get_file_size() failed. " log_end;
		return VrUnknown;
	}

    signatures = NULL;
    numberOfSignatures = 0;

    if (Information->FileSizeLimitForHash != -1)
    {
        fileSizeLimit = PH_VERIFY_DEFAULT_SIZE_LIMIT;

        if (Information->FileSizeLimitForHash != 0)
            fileSizeLimit = Information->FileSizeLimitForHash;

        if (fileSize.QuadPart > fileSizeLimit)
            return VrNoSignature;
    }


	

    if (TRUE == PhpCalculateFileHash(Information->FileHandle,
									 HashAlgorithm,
									 &fileHash,
									 &fileHashLength,
									 &catAdminHandle))
    {
        std::wstring fileHashTag;
		bin_to_hexw_fast(fileHashLength, fileHash, true, fileHashTag);

        // Search the system catalogs.
        catInfoHandle = CryptCATAdminEnumCatalogFromHash(catAdminHandle,
														 fileHash,
														 fileHashLength,
														 0,
														 NULL);
        if (catInfoHandle)
        {
            CATALOG_INFO ci = { 0 };
            DRIVER_VER_INFO verInfo = { 0 };

            if (CryptCATCatalogInfoFromContext(catInfoHandle, &ci, 0))
            {
				//
                // Disable OS version checking by passing in a 
				// DRIVER_VER_INFO structure.
				//
                verInfo.cbStruct = sizeof(DRIVER_VER_INFO);

                catalogInfo.cbStruct = sizeof(catalogInfo);
                catalogInfo.pcwszCatalogFilePath = ci.wszCatalogFile;
				catalogInfo.hMemberFile = Information->FileHandle;
				catalogInfo.pcwszMemberTag = fileHashTag.c_str();
                catalogInfo.pbCalculatedFileHash = fileHash;
                catalogInfo.cbCalculatedFileHash = fileHashLength;
				catalogInfo.hCatAdmin = catAdminHandle;
                verifyResult = PhpVerifyFile(Information, 
											 WTD_CHOICE_CATALOG, 
											 &catalogInfo, 
											 &DriverActionVerify, 
											 &verInfo, 
											 &signatures, 
											 &numberOfSignatures);

                if (verInfo.pcSignerCertContext)
                    CertFreeCertificateContext_I(verInfo.pcSignerCertContext);
            }

            CryptCATAdminReleaseCatalogContext(catAdminHandle, catInfoHandle, 0);
        }
        else
        {
            // Search any user-supplied catalogs.

            for (i = 0; i < Information->NumberOfCatalogFileNames; i++)
            {
                PhFreeVerifySignatures(signatures, numberOfSignatures);

                catalogInfo.cbStruct = sizeof(catalogInfo);
                catalogInfo.pcwszCatalogFilePath = Information->CatalogFileNames[i];
				catalogInfo.hMemberFile = Information->FileHandle;
                catalogInfo.pcwszMemberTag = fileHashTag.c_str();
                catalogInfo.pbCalculatedFileHash = fileHash;
                catalogInfo.cbCalculatedFileHash = fileHashLength;
				catalogInfo.hCatAdmin = catAdminHandle;
                verifyResult = PhpVerifyFile(Information, 
											 WTD_CHOICE_CATALOG, 
											 &catalogInfo, 
											 &WinTrustActionGenericVerifyV2, 
											 NULL, 
											 &signatures, 
											 &numberOfSignatures);

                if (verifyResult == VrTrusted)
                    break;
            }
        }

        PhFree(fileHash);
        CryptCATAdminReleaseContext(catAdminHandle, 0);
    }

    *Signatures = signatures;
    *NumberOfSignatures = numberOfSignatures;

    return verifyResult;
}

///
bool 
PhVerifyFileEx(
    _In_ PPH_VERIFY_FILE_INFO Information,
    _Out_ VERIFY_RESULT *VerifyResult,
    _Out_opt_ PCERT_CONTEXT **Signatures,
	_Out_opt_ PULONG NumberOfSignatures
    )
{
	VF_READY(false);

    VERIFY_RESULT verifyResult;
    PCERT_CONTEXT *signatures;
    ULONG numberOfSignatures;
    WINTRUST_FILE_INFO fileInfo = { 0 };

    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.hFile = Information->FileHandle;

    verifyResult = PhpVerifyFile(Information, 
								 WTD_CHOICE_FILE, 
								 &fileInfo, 
								 &WinTrustActionGenericVerifyV2, 
								 NULL, 
								 &signatures, 
								 &numberOfSignatures);
    if (verifyResult == VrNoSignature)
    {
        if (CryptCATAdminAcquireContext2 && CryptCATAdminCalcHashFromFileHandle2)
        {
            PhFreeVerifySignatures(signatures, numberOfSignatures);
            verifyResult = PhpVerifyFileFromCatalog(Information, 
													BCRYPT_SHA256_ALGORITHM, 
													&signatures, 
													&numberOfSignatures);
        }

        if (verifyResult != VrTrusted)
        {
            PhFreeVerifySignatures(signatures, numberOfSignatures);
            verifyResult = PhpVerifyFileFromCatalog(Information, 
													NULL, 
													&signatures, 
													&numberOfSignatures);
        }
    }

    *VerifyResult = verifyResult;

    if (Signatures)
        *Signatures = signatures;
    else
        PhFreeVerifySignatures(signatures, numberOfSignatures);

    if (NumberOfSignatures)
        *NumberOfSignatures = numberOfSignatures;

	return true;
}

///
VOID 
PhFreeVerifySignatures(
    _In_ PCERT_CONTEXT *Signatures,
    _In_ ULONG NumberOfSignatures
    )
{
    ULONG i;

    if (Signatures)
    {
        for (i = 0; i < NumberOfSignatures; i++)
            CertFreeCertificateContext_I(Signatures[i]);

        PhFree(Signatures);
    }
}

///
wchar_t* 
PhpGetCertNameString(
    _In_ PCERT_NAME_BLOB Blob
    )
{
    // 
    // CertNameToStr doesn't give us the correct buffer size 
	// unless we don't provide a buffer at all.
	// bufferSize includes null terminating character.
	//
	ULONG bufferSize = CertNameToStr_I(X509_ASN_ENCODING,
									   Blob,
									   CERT_X500_NAME_STR,
									   NULL,
									   0);
	_ASSERTE(0 != bufferSize);
	if (0 == bufferSize)
	{
		return NULL;
	}

	wchar_t* NameString = (wchar_t*)malloc(sizeof(wchar_t) * bufferSize);
	if (NULL == NameString) return NULL;
	
    bufferSize = CertNameToStr_I(X509_ASN_ENCODING, 
								 Blob,
								 CERT_X500_NAME_STR,
								 NameString,
								 bufferSize);
	NameString[bufferSize - 1] = 0x00;
	return NameString;
}

///	@brief	X 500 signatrue 에서 특정 섹션 문자열을 리턴한다.
/*
	OID.1.3.6.1.4.1.311.60.2.1.3=KR, 
	OID.1.3.6.1.4.1.311.60.2.1.2=Gyeonggi-do, 
	OID.1.3.6.1.4.1.311.60.2.1.1=Seongnam-si, 
	OID.2.5.4.15=Private Organization, 
	SERIALNUMBER=517-86-00555, 
	C=KR, 
	S=Gyeonggi-do,
	L=Seongnam-si, 
	O="Somma, Inc.", 
	CN="Somma, Inc."						<< "xxxx" 형태
 **
	C=US, 
	S=Washington, 
	L=Redmond, 
	O=Microsoft Corporation, 
	CN=Microsoft Windows Publisher			<< xxxx 형태
  **
	OID.1.3.6.1.4.1.311.60.2.1.3=SK, 
	OID.2.5.4.15=Private Organization, 
	SERIALNUMBER=31 333 532, 
	C=SK, 
	S=Slovakia, 
	L=Bratislava, 
	O="ESET, spol. s r.o.", 
	OU=Digital ID Class 3 - Microsoft Software Validation v2, 
	CN="ESET, spol. s r.o."

	
*/
bool
PhpGetX500Value(
    _In_ const wchar_t* String,
    _In_ const wchar_t* KeyName, 
	_Out_ std::wstring& Value
    )
{	
	std::wstringstream ptrn;
	
	//	
	//	KeyName 
	//
	ptrn << KeyName << L"=";	
	std::wstring token = extract_first_tokenExW(String, 
												ptrn.str().c_str(), 
												false);
	if (0 == token.compare(_null_stringw)) return false;
		
	if (token[0] != L'"')
	{
		//
		//	xxx 형태이므로 바로 다음 `,` 까지 잘라서 리턴한다. 
		//
		token = extract_first_tokenExW(token.c_str(), L",", true);
		if (0 == token.compare(_null_stringw)) return false;
	}
	else
	{
		//
		//	token 은 `"xxxx",` 또는 `"xxxx"`
		//
		//	첫번째 " 를 제거
		token = token.substr(1, token.size());	

		//
		//	두번째 " 이전까지 복사
		//
		token = extract_first_tokenExW(token.c_str(), L"\"", true);
		if (0 == token.compare(_null_stringw)) return false;
	}	
	
	Value = trimw(token);
	return true;;
}

///
bool
PhGetSignerNameFromCertificate(
	_In_ PCERT_CONTEXT Certificate,
	_Out_ std::wstring& SignerName
)
{
	PCERT_INFO certInfo;

	//
	// Cert context -> Cert info
	//
	certInfo = Certificate->pCertInfo;
	if (!certInfo)
	{
		log_info "No cert info." log_end;
		return false;
	}

	//
	// Cert info subject -> Subject X.500 string
	//
	wchar_ptr name(
		PhpGetCertNameString(&certInfo->Subject),
		[](wchar_t* p) {
			if (nullptr != p) { free(p); }
		});
	if (nullptr == name.get())
	{
		log_err "PhpGetCertNameString() failed." log_end;
		return false;
	}
	    
	//
	// Subject X.500 string -> CN or OU value
	//
    std::wstring cn_or_ou;
	if (true != PhpGetX500Value(name.get(), L"CN", cn_or_ou))
	{
		if (true != PhpGetX500Value(name.get(), L"OU", cn_or_ou))
		{
			log_err "No `CN` or `OU` found." log_end;
			return false;
		}
	}
	
	SignerName = cn_or_ou;
	return true;
}

/**
 * Verifies a file's digital signature.
 *
 * \param FileName A file name.
 * \param SignerName A variable which receives a pointer to a string containing the signer name. You
 * must free the string using PhDereferenceObject() when you no longer need it. Note that the signer
 * name may be NULL if it is not valid.
 *
 * \return A VERIFY_RESULT value.
 */
VERIFY_RESULT 
PhVerifyFile(
	_In_ const wchar_t* file_name,
	_Out_ std::wstring* signer_name
)
{
	VF_READY(VrUnknown);

	_ASSERTE(NULL != file_name);
	if (NULL == file_name) return VrUnknown;

	if (!is_file_existsW(file_name))
	{
		log_err "No file. file=%ws",
			file_name
			log_end;
		return VrUnknown;
	}

	handle_ptr file_handle(CreateFileW(file_name,
									   FILE_GENERIC_READ,
									   FILE_SHARE_READ | FILE_SHARE_DELETE,
									   NULL,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL),
						   [](HANDLE h)
							{
								if (INVALID_HANDLE_VALUE != h)
								{
									CloseHandle(h);
								}
							});
	if (INVALID_HANDLE_VALUE == file_handle.get())
	{
		log_err "CreateFileW() failed. file=%ws, gle=%u",
			file_name,
			GetLastError()
			log_end;
		return VrUnknown;
	}

	return PhVerifyFile(file_handle.get(), signer_name);
}

VERIFY_RESULT
PhVerifyFile(
	_In_ HANDLE file_handle,
	_Out_ std::wstring* signer_name
	)
{
    PH_VERIFY_FILE_INFO info = { 0 };
    VERIFY_RESULT verifyResult;
    PCERT_CONTEXT *signatures;
    ULONG numberOfSignatures;

    info.FileHandle = file_handle;
	info.Flags = 0;

	if (PhVerifyFileEx(&info,
					   &verifyResult,
					   &signatures,
					   &numberOfSignatures))
    {
		if (numberOfSignatures != 0 && nullptr != signer_name)
		{
			if (!PhGetSignerNameFromCertificate(signatures[0], *signer_name))
			{
				log_err "PhGetSignerNameFromCertificate() failed." log_end;
				//
				// No signer name but verified.
				//
			}
		}

        PhFreeVerifySignatures(signatures, numberOfSignatures);
        return verifyResult;
    }
    else
    {
        return VrNoSignature;
    }
}

VERIFY_RESULT
PhVerifyFile(
	_In_ const wchar_t* file_name,
	_Out_ std::list<std::wstring> &signer_names
)
{
	VF_READY(VrUnknown);

	_ASSERTE(NULL != file_name);
	if (NULL == file_name) return VrUnknown;

	if (!is_file_existsW(file_name))
	{
		log_err "No file. file=%ws",
			file_name
			log_end;
		return VrUnknown;
	}

	handle_ptr file_handle(CreateFileW(file_name,
									   FILE_GENERIC_READ,
									   FILE_SHARE_READ | FILE_SHARE_DELETE,
									   NULL,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL),
						   [](HANDLE h)
	{
		if (INVALID_HANDLE_VALUE != h)
		{
			CloseHandle(h);
		}
	});
	if (INVALID_HANDLE_VALUE == file_handle.get())
	{
		log_err "CreateFileW() failed. file=%ws, gle=%u",
			file_name,
			GetLastError()
			log_end;
		return VrUnknown;
	}
	return PhVerifyFile(file_handle.get(), signer_names);
}

VERIFY_RESULT
PhVerifyFile(
	_In_ HANDLE file_handle,
	_Out_ std::list<std::wstring> &signer_names
	)
{
	PH_VERIFY_FILE_INFO info = { 0 };
	info.FileHandle = file_handle;
	info.Flags = 0;
	info.secondary_signatures = new std::list<psignature_info>;
	if (nullptr == info.secondary_signatures)
	{
		log_err "Can't allocate std::list<psignature_info>." 
			log_end;
		return VrUnknown;
	}

	VERIFY_RESULT verifyResult;
	PCERT_CONTEXT *signatures;
	ULONG numberOfSignatures;
	if (true != PhVerifyFileEx(&info,
							   &verifyResult,
							   &signatures,
							   &numberOfSignatures))
	{
		log_err "PhVerifyFileEx() failed." log_end;

		delete info.secondary_signatures;
		info.secondary_signatures = nullptr;
		return VrUnknown;
	}

	std::wstring signer_name;
	if (numberOfSignatures != 0)
	{
		if (!PhGetSignerNameFromCertificate(signatures[0], signer_name))
		{
			log_err "PhGetSignerNameFromCertificate() failed." 
				log_end;
			//
			// No signer name but verified.
			//
		}
		else
		{
			signer_names.push_back(signer_name.c_str());
		}
	}
	PhFreeVerifySignatures(signatures, numberOfSignatures);


	for (auto ss : *info.secondary_signatures)
	{
		if (ss->_number_of_signatures != 0)
		{
			signer_name.clear();
			if (!PhGetSignerNameFromCertificate(ss->_signatures[0], signer_name))
			{
				log_err "PhGetSignerNameFromCertificate() failed." log_end;
				//
				// No signer name but verified.
				//
			}
			else
			{
				signer_names.push_back(signer_name.c_str());
			}
		}
		PhFreeVerifySignatures(ss->_signatures, ss->_number_of_signatures);
		delete ss;
	}
	info.secondary_signatures->clear();
	delete info.secondary_signatures;
	info.secondary_signatures = nullptr;

	return verifyResult;
}