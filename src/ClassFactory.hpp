#pragma once

class ClassFactory : public IClassFactory {
public:
  ClassFactory(REFCLSID rclsid, HRESULT(*pfnCreateInstance)(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj))
    : fClassId(rclsid), fCreateInstance(pfnCreateInstance)
  {
  }

  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    if (!ppvObj) {
      return E_INVALIDARG;
    }
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown)) {
      *ppvObj = dynamic_cast<IClassFactory*>(this);
    } else {
      *ppvObj = nullptr;
    }
    if (*ppvObj) {
      DllAddRef();
      return NOERROR;
    } else {
      return E_NOINTERFACE;
    }
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    DllAddRef();
    return sRefCount + 1;
  }

  STDMETHODIMP_(ULONG) Release() override {
    DllRelease();
    return sRefCount + 1;
  }

  STDMETHODIMP CreateInstance(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj) override {
    return fCreateInstance(pUnkOuter, riid, ppvObj);
  }

  STDMETHODIMP LockServer(BOOL lock) override {
    if (lock) {
      DllAddRef();
    } else {
      DllRelease();
    }
    return S_OK;
  }

  bool HasClassId(REFCLSID other) const {
    return IsEqualGUID(other, fClassId);
  }

private:
  REFCLSID fClassId;
  HRESULT (*fCreateInstance)(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObj);
};
