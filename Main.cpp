#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <combaseapi.h>
#include <msctf.h>
#include <strsafe.h>

#include <exception>

#include "resource.h"

static CRITICAL_SECTION sMutex;
static const CLSID kClassId = { 0x1e065a14, 0x7f7f, 0x4163, { 0xa7, 0xab, 0xbd, 0x2b, 0xb7, 0xbb, 0x72, 0x1b } };
// {D9D2FAFE-0184-4304-AF6E-EB54AE63645B}
static const GUID kProfileId = { 0xd9d2fafe, 0x184, 0x4304, { 0xaf, 0x6e, 0xeb, 0x54, 0xae, 0x63, 0x64, 0x5b } };

static LONG sRefCount = -1;
class ClassFactory;
static ClassFactory *sClassFactoryObjects[1] = { nullptr };
static HINSTANCE sDllInstanceHandle;

static const WCHAR kRegInfoPrefixCLSID[] = L"CLSID\\";
#define CLSID_STRLEN (38)
static const WCHAR TEXTSERVICE_DESC[] = L"KSeshKeyboard";
static const WCHAR kRegInfoKeyInProSvr32[] = L"InProcServer32";
static const WCHAR kRegInfoKeyThreadModel[] = L"ThreadingModel";
#define TEXTSERVICE_LANGID MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
#define TEXTSERVICE_MODEL L"Apartment"
#define TEXTSERVICE_ICON_INDEX -IDIS_KSESHKEYBOARD

static const GUID kSupportCategories[] = {
  GUID_TFCAT_TIP_KEYBOARD,
  GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
  GUID_TFCAT_TIPCAP_UIELEMENTENABLED, 
  GUID_TFCAT_TIPCAP_SECUREMODE,
  GUID_TFCAT_TIPCAP_COMLESS,
  GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
  GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, 
  GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv);
STDAPI DllCanUnloadNow();
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

static void UnsafeFreeGlobalObjects();
static void UnsafeBuildGlobalObjects();
static LONG DllAddRef();
static LONG DllRelease();

#include "ClassFactory.hpp"
#include "Processor.hpp"

static LONG DllAddRef() {
  return InterlockedIncrement(&sRefCount);
}

static LONG DllRelease() {
  LONG after = InterlockedDecrement(&sRefCount);
  if (after < 0) {
    EnterCriticalSection(&sMutex);
    if (sClassFactoryObjects[0] != nullptr) {
      UnsafeFreeGlobalObjects();
    }
    LeaveCriticalSection(&sMutex);
  }
  return after;
}

static void UnsafeBuildGlobalObjects() {
  sClassFactoryObjects[0] = new (std::nothrow)ClassFactory(kClassId, Processor::CreateInstance);
}

static void UnsafeFreeGlobalObjects() {
  ClassFactory *factory = sClassFactoryObjects[0];
  if (factory != nullptr) {
    sClassFactoryObjects[0] = nullptr;
    delete factory;
  }
}

const BYTE GuidSymbols[] = {
  3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
};

static const WCHAR HexDigits[] = L"0123456789ABCDEF";

static BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR* pCLSIDString) {
  WCHAR* pTemp = pCLSIDString;
  const BYTE* pBytes = (const BYTE*)&refGUID;

  DWORD j = 0;
  pTemp[j++] = L'{';
  for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++) {
    if (GuidSymbols[i] == '-') {
      pTemp[j++] = L'-';
    } else {
      pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0xF0) >> 4];
      pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0x0F)];
    }
  }

  pTemp[j++] = L'}';
  pTemp[j] = L'\0';

  return TRUE;
}

static BOOL RegisterServer() {
  DWORD copiedStringLen = 0;
  HKEY regKeyHandle = nullptr;
  HKEY regSubkeyHandle = nullptr;
  BOOL ret = FALSE;
  WCHAR achIMEKey[ARRAYSIZE(kRegInfoPrefixCLSID) + CLSID_STRLEN] = { '\0' };
  WCHAR achFileName[MAX_PATH] = { '\0' };

  if (!CLSIDToString(kClassId, achIMEKey + ARRAYSIZE(kRegInfoPrefixCLSID) - 1)) {
    return FALSE;
  }

  memcpy(achIMEKey, kRegInfoPrefixCLSID, sizeof(kRegInfoPrefixCLSID) - sizeof(WCHAR));

  if (RegCreateKeyExW(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKeyHandle, &copiedStringLen) == ERROR_SUCCESS) {
    if (RegSetValueExW(regKeyHandle, NULL, 0, REG_SZ, (const BYTE*)TEXTSERVICE_DESC, (_countof(TEXTSERVICE_DESC)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
      goto Exit;
    }

    if (RegCreateKeyExW(regKeyHandle, kRegInfoKeyInProSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regSubkeyHandle, &copiedStringLen) == ERROR_SUCCESS) {
      copiedStringLen = GetModuleFileNameW(sDllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
      copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
      if (RegSetValueExW(regSubkeyHandle, NULL, 0, REG_SZ, (const BYTE*)achFileName, (copiedStringLen) * sizeof(WCHAR)) != ERROR_SUCCESS) {
        goto Exit;
      }
      if (RegSetValueExW(regSubkeyHandle, kRegInfoKeyThreadModel, 0, REG_SZ, (const BYTE*)TEXTSERVICE_MODEL, (_countof(TEXTSERVICE_MODEL)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
        goto Exit;
      }

      ret = TRUE;
    }
  }

Exit:
  if (regSubkeyHandle) {
    RegCloseKey(regSubkeyHandle);
    regSubkeyHandle = nullptr;
  }
  if (regKeyHandle) {
    RegCloseKey(regKeyHandle);
    regKeyHandle = nullptr;
  }

  return ret;
}

static LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey) {
  HKEY regKeyHandle = nullptr;
  LONG res = 0;
  FILETIME time;
  WCHAR stringBuffer[256] = { '\0' };
  DWORD size = ARRAYSIZE(stringBuffer);

  if (RegOpenKeyW(hParentKey, lpszKey, &regKeyHandle) != ERROR_SUCCESS) {
    return ERROR_SUCCESS;
  }

  res = ERROR_SUCCESS;
  while (RegEnumKeyExW(regKeyHandle, 0, stringBuffer, &size, NULL, NULL, NULL, &time) == ERROR_SUCCESS) {
    stringBuffer[ARRAYSIZE(stringBuffer) - 1] = '\0';
    res = RecurseDeleteKey(regKeyHandle, stringBuffer);
    if (res != ERROR_SUCCESS) {
      break;
    }
    size = ARRAYSIZE(stringBuffer);
  }
  RegCloseKey(regKeyHandle);

  return res == ERROR_SUCCESS ? RegDeleteKeyW(hParentKey, lpszKey) : res;
}

static void UnregisterServer() {
  WCHAR achIMEKey[ARRAYSIZE(kRegInfoPrefixCLSID) + CLSID_STRLEN] = { '\0' };

  if (!CLSIDToString(kClassId, achIMEKey + ARRAYSIZE(kRegInfoPrefixCLSID) - 1)) {
    return;
  }

  memcpy(achIMEKey, kRegInfoPrefixCLSID, sizeof(kRegInfoPrefixCLSID) - sizeof(WCHAR));

  RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}

static BOOL RegisterCategories() {
  ITfCategoryMgr* pCategoryMgr = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
  if (FAILED(hr) || !pCategoryMgr) {
    return FALSE;
  }

  for (GUID guid : kSupportCategories) {
    hr = pCategoryMgr->RegisterCategory(kClassId, guid, kClassId);
  }

  pCategoryMgr->Release();

  return (hr == S_OK);
}

static void UnregisterCategories() {
  ITfCategoryMgr* pCategoryMgr = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
  if (FAILED(hr) || !pCategoryMgr) {
    return;
  }

  for (GUID guid : kSupportCategories) {
    pCategoryMgr->UnregisterCategory(kClassId, guid, kClassId);
  }

  pCategoryMgr->Release();

  return;
}

static BOOL RegisterProfiles() {
  ITfInputProcessorProfileMgr* pITfInputProcessorProfileMgr = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
  if (FAILED(hr) || !pITfInputProcessorProfileMgr) {
    return FALSE;
  }

  WCHAR achIconFile[MAX_PATH] = { '\0' };
  DWORD cchA = 0;
  cchA = GetModuleFileNameW(sDllInstanceHandle, achIconFile, MAX_PATH);
  cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
  achIconFile[cchA] = '\0';

  size_t lenOfDesc = 0;
  hr = StringCchLengthW(TEXTSERVICE_DESC, STRSAFE_MAX_CCH, &lenOfDesc);
  if (hr != S_OK) {
    goto Exit;
  }
  hr = pITfInputProcessorProfileMgr->RegisterProfile(
    kClassId,
    TEXTSERVICE_LANGID,
    kProfileId,
    TEXTSERVICE_DESC,
    static_cast<ULONG>(lenOfDesc),
    achIconFile,
    cchA,
    (UINT)TEXTSERVICE_ICON_INDEX,
    NULL,
    0,
    TRUE,
    0);

  if (FAILED(hr)) {
    goto Exit;
  }

Exit:
  if (pITfInputProcessorProfileMgr) {
    pITfInputProcessorProfileMgr->Release();
  }

  return (hr == S_OK);
}

void UnregisterProfiles(){
  HRESULT hr = S_OK;

  ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;
  hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,    IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
  if (FAILED(hr) || !pITfInputProcessorProfileMgr) {
    goto Exit;
  }

  hr = pITfInputProcessorProfileMgr->UnregisterProfile(kClassId, TEXTSERVICE_LANGID, kProfileId, 0);
  if (FAILED(hr))  {
    goto Exit;
  }

Exit:
  if (pITfInputProcessorProfileMgr)  {
    pITfInputProcessorProfileMgr->Release();
  }

  return;
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved) {
  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
    sDllInstanceHandle = hInstance;
    if (!InitializeCriticalSectionAndSpinCount(&sMutex, 0)) {
      return FALSE;
    }
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    DeleteCriticalSection(&sMutex);
    break;
  }
  return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
  if (sClassFactoryObjects[0] == nullptr) {
    EnterCriticalSection(&sMutex);
    if (sClassFactoryObjects[0] == nullptr) {
      UnsafeBuildGlobalObjects();
    }
    LeaveCriticalSection(&sMutex);
  }
  if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown)) {
    if (nullptr != sClassFactoryObjects[0] && IsEqualGUID(rclsid, sClassFactoryObjects[0]->fClassId)) {
      *ppv = (void*)sClassFactoryObjects[0];
      DllAddRef();
      return NOERROR;
    }
  }

  *ppv = nullptr;
  return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() {
  if (sRefCount >= 0) {
    return S_FALSE;
  } else {
    return S_OK;
  }
}

STDAPI DllRegisterServer() {
  if (!RegisterServer() || !RegisterProfiles() || !RegisterCategories()) {
    DllUnregisterServer();
    return E_FAIL;
  }
  return S_OK;
}

STDAPI DllUnregisterServer() {
  UnregisterProfiles();
  UnregisterCategories();
  UnregisterServer();
  return S_OK;
}
