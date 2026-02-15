#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <combaseapi.h>
#include <msctf.h>
#include <ctffunc.h>
#include <strsafe.h>
#include <olectl.h>

#include <exception>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include <format>
#include <chrono>

#include "resource.h"

static CRITICAL_SECTION sMutex;
// {1E065A14-7F7F-4163-A7AB-BD2BB7BB721B}
static CLSID const kClassId = { 0x1e065a14, 0x7f7f, 0x4163, { 0xa7, 0xab, 0xbd, 0x2b, 0xb7, 0xbb, 0x72, 0x1b } };
// {D9D2FAFE-0184-4304-AF6E-EB54AE63645B}
static GUID const kProfileId = { 0xd9d2fafe, 0x184, 0x4304, { 0xaf, 0x6e, 0xeb, 0x54, 0xae, 0x63, 0x64, 0x5b } };

static LONG sRefCount = -1;
class ClassFactory;
static ClassFactory *sClassFactoryObjects = nullptr;
static HINSTANCE sDllInstanceHandle;

static WCHAR const kRegInfoPrefixCLSID[] = L"CLSID\\";
#define CLSID_STRLEN (38)
static WCHAR const TEXTSERVICE_DESC[] = L"Ancient Egyptian Transliteration";
static WCHAR const kRegInfoKeyInProSvr32[] = L"InProcServer32";
static WCHAR const kRegInfoKeyThreadModel[] = L"ThreadingModel";
#define TEXTSERVICE_LANGID MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT)
#define TEXTSERVICE_MODEL L"Apartment"

static GUID const kSupportCategories[] = {
  GUID_TFCAT_TIP_KEYBOARD,
  GUID_TFCAT_TIPCAP_SECUREMODE,
  GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
  GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid,  void** ppv);
STDAPI DllCanUnloadNow();
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

static void UnsafeFreeGlobalObjects();
static void UnsafeBuildGlobalObjects();
static void DllAddRef();
static void DllRelease();
static std::string StringFromGUID(REFGUID guid);

#include "Defer.hpp"
#include "FileLogger.hpp"
#include "ClassFactory.hpp"
#include "EditSession.hpp"
#include "RegKey.hpp"
#include "Unicode.hpp"
#include "IReplacement.hpp"
#include "Settings.hpp"
#include "SettingsDialog.hpp"
#include "LangBarItemButton.hpp"
#include "Processor.hpp"

static void DllAddRef() {
  InterlockedIncrement(&sRefCount);
}

static void DllRelease() {
  if (InterlockedDecrement(&sRefCount) < 0) {
    EnterCriticalSection(&sMutex);
    if (sClassFactoryObjects != nullptr) {
      UnsafeFreeGlobalObjects();
    }
    LeaveCriticalSection(&sMutex);
  }
}

static void UnsafeBuildGlobalObjects() {
  sClassFactoryObjects = new (std::nothrow)ClassFactory(kClassId, Processor::CreateInstance);
}

static void UnsafeFreeGlobalObjects() {
  ClassFactory *factory = sClassFactoryObjects;
  if (factory != nullptr) {
    sClassFactoryObjects = nullptr;
    delete factory;
  }
}

static BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR* pCLSIDString) {
  WCHAR* pTemp = pCLSIDString;
  BYTE const* pBytes = (BYTE const*)&refGUID;

  BYTE const GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
  };
  WCHAR const HexDigits[] = L"0123456789ABCDEF";

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

static std::string StringFromGUID(REFGUID guid) {
  BYTE const* pBytes = (BYTE const*)&guid;

  BYTE const GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
  };
  char const HexDigits[] = "0123456789ABCDEF";
  std::string ret(38, '\0');
  DWORD j = 0;
  ret[j++] = '{';
  for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++) {
    if (GuidSymbols[i] == '-') {
      ret[j++] = L'-';
    } else {
      ret[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0xF0) >> 4];
      ret[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0x0F)];
    }
  }

  ret[j++] = L'}';
  return ret;
}

static BOOL RegisterServer() {
  DWORD copiedStringLen = 0;
  HKEY regKeyHandle = nullptr;
  HKEY regSubkeyHandle = nullptr;
  WCHAR achIMEKey[ARRAYSIZE(kRegInfoPrefixCLSID) + CLSID_STRLEN] = { '\0' };
  WCHAR achFileName[MAX_PATH] = { '\0' };

  defer{
    if (regSubkeyHandle) {
      RegCloseKey(regSubkeyHandle);
    }
    if (regKeyHandle) {
      RegCloseKey(regKeyHandle);
    }
  };

  if (!CLSIDToString(kClassId, achIMEKey + ARRAYSIZE(kRegInfoPrefixCLSID) - 1)) {
    return FALSE;
  }

  memcpy(achIMEKey, kRegInfoPrefixCLSID, sizeof(kRegInfoPrefixCLSID) - sizeof(WCHAR));

  if (RegCreateKeyExW(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKeyHandle, &copiedStringLen) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegSetValueExW(regKeyHandle, NULL, 0, REG_SZ, (BYTE const*)TEXTSERVICE_DESC, (_countof(TEXTSERVICE_DESC)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegCreateKeyExW(regKeyHandle, kRegInfoKeyInProSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regSubkeyHandle, &copiedStringLen) != ERROR_SUCCESS) {
    return FALSE;
  }
  copiedStringLen = GetModuleFileNameW(sDllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
  copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
  if (RegSetValueExW(regSubkeyHandle, NULL, 0, REG_SZ, (BYTE const*)achFileName, (copiedStringLen) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegSetValueExW(regSubkeyHandle, kRegInfoKeyThreadModel, 0, REG_SZ, (BYTE const*)TEXTSERVICE_MODEL, (_countof(TEXTSERVICE_MODEL)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }

  return TRUE;
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
  ITfCategoryMgr* manager = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return FALSE;
  }

  for (GUID guid : kSupportCategories) {
    hr = manager->RegisterCategory(kClassId, guid, kClassId);
  }

  manager->Release();

  return hr == S_OK;
}

static void UnregisterCategories() {
  ITfCategoryMgr* manager = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return;
  }

  for (GUID guid : kSupportCategories) {
    manager->UnregisterCategory(kClassId, guid, kClassId);
  }

  manager->Release();

  return;
}

static BOOL RegisterProfiles() {
  ITfInputProcessorProfileMgr* manager = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return FALSE;
  }
  defer{
    manager->Release();
  };

  WCHAR achIconFile[MAX_PATH] = { '\0' };
  DWORD cchA = 0;
  cchA = GetModuleFileNameW(sDllInstanceHandle, achIconFile, MAX_PATH);
  cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
  achIconFile[cchA] = '\0';

  size_t lenOfDesc = 0;
  if (StringCchLengthW(TEXTSERVICE_DESC, STRSAFE_MAX_CCH, &lenOfDesc) != S_OK) {
    return FALSE;
  }
  hr = manager->RegisterProfile(
    kClassId,
    TEXTSERVICE_LANGID,
    kProfileId,
    TEXTSERVICE_DESC,
    static_cast<ULONG>(lenOfDesc),
    achIconFile,
    cchA,
    (UINT)(-IDIS_ICON_BRAND),
    NULL,
    0,
    TRUE,
    0);

  return hr == S_OK;
}

void UnregisterProfiles() {
  ITfInputProcessorProfileMgr* manager = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return;
  }
  defer{
    manager->Release();
  };

  manager->UnregisterProfile(kClassId, TEXTSERVICE_LANGID, kProfileId, 0);
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

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv) {
  if (sClassFactoryObjects == nullptr) {
    EnterCriticalSection(&sMutex);
    if (sClassFactoryObjects == nullptr) {
      UnsafeBuildGlobalObjects();
    }
    LeaveCriticalSection(&sMutex);
  }
  if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown)) {
    if (nullptr != sClassFactoryObjects && sClassFactoryObjects->HasClassId(rclsid)) {
      *ppv = (void*)sClassFactoryObjects;
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
