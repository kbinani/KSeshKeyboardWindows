#pragma once

class Processor : public ITfTextInputProcessorEx {
public:
  Processor() : fRefCount(1), fThreadManager(nullptr), fClientId(TF_CLIENTID_NULL) {
    DllAddRef();
  }

  ~Processor() {
    DllRelease();
  }

  // IUnknown::QueryInterface
  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    if (ppvObj == nullptr) {
      return E_INVALIDARG;
    }
    *ppvObj = nullptr;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor)) {
      *ppvObj = (ITfTextInputProcessor*)this;
    } else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx)) {
      *ppvObj = (ITfTextInputProcessorEx *)this;
    }
    if (*ppvObj) {
      AddRef();
      return S_OK;
    } else {
      return E_NOINTERFACE;
    }
  }

  // IUnknown::AddRef
  STDMETHODIMP_(ULONG) AddRef() override {
    return ++fRefCount;
  }

  // IUnknown::Release
  STDMETHODIMP_(ULONG) Release() override {
    LONG after = --fRefCount;
    if (fRefCount == 0) {
      delete this;
    }
    return after;
  }

  // IUnknown::CreateInstance
  static HRESULT CreateInstance(_In_ IUnknown* pUnkOuter, REFIID riid, _Outptr_ void** ppvObj) {
    if (ppvObj == nullptr) {
      return E_INVALIDARG;
    }
    *ppvObj = nullptr;
    if (pUnkOuter != nullptr) {
      return CLASS_E_NOAGGREGATION;
    }
    Processor* processor = new (std::nothrow) Processor();
    if (processor == nullptr) {
      return E_OUTOFMEMORY;
    }
    HRESULT result = processor->QueryInterface(riid, ppvObj);
    processor->Release();
    return result;
  }

  // ITfTextInputProcessor::Activate
  STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId) override {
    return ActivateEx(pThreadMgr, tfClientId, 0);
  }

  // ITfTextInputProcessorEx::ActivateEx
  STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags) override {
    fThreadManager = pThreadMgr;
    fThreadManager->AddRef();
    fClientId = tfClientId;
    fActivateFlags = dwFlags;
    return S_OK;
  }

  // ITfTextInputProcessorEx::Deactivate
  STDMETHODIMP Deactivate() override {
    if (fThreadManager != nullptr) {
      fThreadManager->Release();
    }
    fClientId = TF_CLIENTID_NULL;
    return S_OK;
  }

private:
  LONG fRefCount;
  ITfThreadMgr* fThreadManager;
  TfClientId fClientId;
  DWORD fActivateFlags;
};
