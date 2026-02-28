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
#include "Version.h"

// {1E065A14-7F7F-4163-A7AB-BD2BB7BB721B}
CLSID constexpr kClassId = { 0x1e065a14, 0x7f7f, 0x4163, { 0xa7, 0xab, 0xbd, 0x2b, 0xb7, 0xbb, 0x72, 0x1b } };
// {D9D2FAFE-0184-4304-AF6E-EB54AE63645B}
GUID constexpr kProfileId = { 0xd9d2fafe, 0x184, 0x4304, { 0xaf, 0x6e, 0xeb, 0x54, 0xae, 0x63, 0x64, 0x5b } };

static CRITICAL_SECTION sMutex;
static LONG sRefCount = -1;
class ClassFactory;
static ClassFactory *sClassFactoryObjects = nullptr;
static HINSTANCE sDllInstanceHandle = nullptr;

WCHAR constexpr kRegInfoPrefixCLSID[] = L"CLSID\\";
int constexpr kGUIDStringLength = 38;
WCHAR constexpr kRegInfoKeyInProSvr32[] = L"InProcServer32";
WCHAR constexpr kRegInfoKeyThreadModel[] = L"ThreadingModel";

WCHAR constexpr kTextServiceDescription[] = L"Ancient Egyptian Transliteration";
LANGID constexpr kTextServiceLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
WCHAR constexpr kTextServiceModel[] = L"Apartment";

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

#include "Defer.hpp"
#include "FileLogger.hpp"
#include "GUID.hpp"
#include "ClassFactory.hpp"
#include "EditSession.hpp"
#include "RegKey.hpp"
#include "Unicode.hpp"
#include "IReplacement.hpp"
#include "Settings.hpp"
#include "SettingsDialog.hpp"
#include "LangBarItemButton.hpp"
#include "Processor.hpp"
#include "Install.hpp"

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
  *ppv = nullptr;
  if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown)) {
    if (sClassFactoryObjects && sClassFactoryObjects->HasClassId(rclsid)) {
      *ppv = dynamic_cast<IClassFactory*>(sClassFactoryObjects);
    }
  }
  if (*ppv) {
    DllAddRef();
    return NOERROR;
  }
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
