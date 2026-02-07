#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
  return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() {
  return S_OK;
}

STDAPI DllRegisterServer() {
  return E_FAIL;
}

STDAPI DllUnregisterServer(){
  return S_OK;
}
