#pragma once

class Processor : public ITfTextInputProcessorEx,
  public ITfThreadMgrEventSink,
  public ITfTextEditSink,
  public ITfKeyEventSink
{
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
    } else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink)) {
      *ppvObj = (ITfThreadMgrEventSink *)this;
    } else if (IsEqualIID(riid, IID_ITfTextEditSink)) {
      *ppvObj = (ITfTextEditSink *)this;
    } else if (IsEqualIID(riid, IID_ITfKeyEventSink)) {
      *ppvObj = (ITfKeyEventSink*)this;
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

  // ITfThreadMgrEventSink::OnInitDocumentMgr
  STDMETHODIMP OnInitDocumentMgr(_In_ ITfDocumentMgr* pDocMgr) {
    return E_NOTIMPL;
  }

  // ITfThreadMgrEventSink::OnUninitDocumentMgr
  STDMETHODIMP OnUninitDocumentMgr(_In_ ITfDocumentMgr* pDocMgr) {
    return E_NOTIMPL;
  }

  // ITfThreadMgrEventSink::OnSetFocus
  STDMETHODIMP OnSetFocus(_In_ ITfDocumentMgr* pDocMgrFocus, _In_ ITfDocumentMgr* pDocMgrPrevFocus) {
    return S_OK;
  }

  // ITfThreadMgrEventSink::OnPushContext
  STDMETHODIMP OnPushContext(_In_ ITfContext* pContext) {
    return E_NOTIMPL;
  }

  // ITfThreadMgrEventSink::OnPopContext
  STDMETHODIMP OnPopContext(_In_ ITfContext* pContext) {
    return E_NOTIMPL;
  }

  // ITfKeyEventSink::OnSetFocus
  STDMETHODIMP OnSetFocus(BOOL fForeground) {
    return S_OK;
  }

  // ITfKeyEventSink::OnTestKeyDown
  STDMETHODIMP OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) {
    return S_OK;
  }

  // ITfKeyEventSink::OnKeyDown
  STDMETHODIMP OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) {
    return S_OK;
  }

  // ITfKeyEventSink::OnTestKeyUp
  STDMETHODIMP OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) {
    return S_OK;
  }

  // ITfKeyEventSink::OnKeyUp
  STDMETHODIMP OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) {
    return S_OK;
  }

  // ITfKeyEventSink::OnPreservedKey
  STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pIsEaten) {
    return S_OK;
  }

  // ITfTextEditSink::OnEndEdit
  STDMETHODIMP OnEndEdit(__RPC__in_opt ITfContext* pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) {
    return S_OK;
  }

private:
  LONG fRefCount;
  ITfThreadMgr* fThreadManager;
  TfClientId fClientId;
  DWORD fActivateFlags;
};
