#pragma once

class ClassFactory : public IClassFactory {
public:
  ClassFactory(REFCLSID rclsid, HRESULT(*pfnCreateInstance)(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj))
    : fClassId(rclsid), fCreateInstance(pfnCreateInstance)
  {
  }

  // IUnknown::QueryInterface
  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown)) {
      *ppvObj = this;
      DllAddRef();
      return NOERROR;
    }
    *ppvObj = nullptr;

    return E_NOINTERFACE;
  }

  // IUnknown::AddRef
  STDMETHODIMP_(ULONG) AddRef() override {
    return DllAddRef() + 1;
  }

  // IUnknown::Release
  STDMETHODIMP_(ULONG) Release() override {
    return DllRelease() + 1;
  }

  // IClassFactory::CreateInstance
  STDMETHODIMP CreateInstance(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj) override {
    return fCreateInstance(pUnkOuter, riid, ppvObj);
  }

  // IClassFactory::LockServer
  STDMETHODIMP LockServer(BOOL lock) override {
    if (lock) {
      DllAddRef();
    } else {
      DllRelease();
    }
    return S_OK;
  }

public:
  REFCLSID fClassId;

private:
  HRESULT (*fCreateInstance)(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj);
};
