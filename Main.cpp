#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <combaseapi.h>
#include <msctf.h>

#include <exception>

static CRITICAL_SECTION sMutex;
const CLSID kClassId = { 0x1e065a14, 0x7f7f, 0x4163, { 0xa7, 0xab, 0xbd, 0x2b, 0xb7, 0xbb, 0x72, 0x1b } };
static LONG sRefCount = -1;
class ClassFactory;
static ClassFactory *sClassFactoryObjects[1] = { nullptr };

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

BOOL APIENTRY DllMain(HMODULE module, DWORD reasonForCall, LPVOID reserved) {
  switch (reasonForCall) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
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
  return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() {
  return S_OK;
}

STDAPI DllRegisterServer() {
  return S_OK;
}

STDAPI DllUnregisterServer(){
  return S_OK;
}
