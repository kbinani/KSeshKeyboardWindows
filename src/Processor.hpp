#pragma once

class Processor : public ITfTextInputProcessorEx, public ITfKeyEventSink {
public:
  Processor() : fRefCount(1), fThreadManager(nullptr), fClientId(TF_CLIENTID_NULL), fLangBarItemButton(nullptr) {
    DllAddRef();
  }

  ~Processor() {
    DllRelease();
  }

  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    if (!ppvObj) {
      return E_INVALIDARG;
    }
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor)) {
      *ppvObj = dynamic_cast<ITfTextInputProcessor*>(this);
    } else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx)) {
      *ppvObj = dynamic_cast<ITfTextInputProcessorEx*>(this);
    } else if (IsEqualIID(riid, IID_ITfKeyEventSink)) {
      *ppvObj = dynamic_cast<ITfKeyEventSink*>(this);
    } else {
      *ppvObj = nullptr;
    }
    if (*ppvObj) {
      AddRef();
      return S_OK;
    } else {
      return E_NOINTERFACE;
    }
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return ++fRefCount;
  }

  STDMETHODIMP_(ULONG) Release() override {
    LONG after = --fRefCount;
    if (fRefCount == 0) {
      delete this;
    }
    return after;
  }

  static HRESULT CreateInstance(_In_ IUnknown* pUnkOuter, REFIID riid, _Outptr_ void** ppvObj) {
    if (!ppvObj) {
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
    defer{
      processor->Release();
    };
    return processor->QueryInterface(riid, ppvObj);
  }

  STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId) override {
    return ActivateEx(pThreadMgr, tfClientId, 0);
  }

  STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags) override {
    fThreadManager = pThreadMgr;
    fThreadManager->AddRef();
    fClientId = tfClientId;
    fActivateFlags = dwFlags;
    if (!InitKeyEventSink()) {
      Deactivate();
      return E_FAIL;
    }
    if (!InitLangBarItemButton()) {
      Deactivate();
      FileLogger::Println("fail");
      return E_FAIL;
    }
    return S_OK;
  }

  STDMETHODIMP Deactivate() override {
    DeinitKeyEventSink();
    DeinitLangBarItemButton();
    if (fThreadManager) {
      fThreadManager->Release();
      fThreadManager = nullptr;
    }
    fClientId = TF_CLIENTID_NULL;
    return S_OK;
  }

  STDMETHODIMP OnSetFocus(BOOL fForeground) override {
    return S_OK;
  }

  STDMETHODIMP OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) override {
    return S_OK;
  }

  STDMETHODIMP OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) override {
    if (!pIsEaten) {
      return E_INVALIDARG;
    }

    WCHAR ch = ConvertVKey((UINT)wParam);
    static std::map<WCHAR, std::wstring> const sMapping = {
      {L'D', L"ḏ"},
      {L'T', L"ṯ"},
      {L'A', L"ꜣ"},
      {L'H', L"ḥ"},
      {L'x', L"ḫ"},
      {L'X', L"ẖ"},
      {L'S', L"š"},
      {L'a', L"ꜥ"},
      {L'q', L"ḳ"},
      {L'i', L"ꞽ"},
    };
    auto found = sMapping.find(ch);
    if (found == sMapping.end()) {
      *pIsEaten = FALSE;
      return S_OK;
    } else {
      *pIsEaten = TRUE;

      EditSession* session = new (std::nothrow) EditSession(pContext, found->second);
      if (!session) {
        return S_FALSE;
      }
      defer{
        session->Release();
      };
      HRESULT hr = E_FAIL;
      hr = pContext->RequestEditSession(fClientId, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
      return hr;
    }
  }

  STDMETHODIMP OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) override {
    return S_OK;
  }

  STDMETHODIMP OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pIsEaten) override {
    return S_OK;
  }

  STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pIsEaten) override {
    return S_OK;
  }

private:
  bool InitKeyEventSink() {
    ITfKeystrokeMgr* manager = nullptr;
    if (FAILED(fThreadManager->QueryInterface(IID_ITfKeystrokeMgr, (void**)&manager))) {
      return FALSE;
    }
    defer{
      manager->Release();
    };

    return manager->AdviseKeyEventSink(fClientId, (ITfKeyEventSink*)this, TRUE) == S_OK;
  }

  void DeinitKeyEventSink() {
    ITfKeystrokeMgr* manager = nullptr;
    if (FAILED(fThreadManager->QueryInterface(IID_ITfKeystrokeMgr, (void**)&manager))) {
      return;
    }
    defer{
      manager->Release();
    };

    manager->UnadviseKeyEventSink(fClientId);
  }

  WCHAR ConvertVKey(UINT code) {
    UINT scanCode = 0;
    scanCode = MapVirtualKeyW(code, 0);

    BYTE abKbdState[256] = { '\0' };
    if (!GetKeyboardState(abKbdState)) {
      return 0;
    }

    WCHAR wch = '\0';
    if (ToUnicode(code, scanCode, abKbdState, &wch, 1, 0) == 1) {
      return wch;
    }

    return 0;
  }

  bool InitLangBarItemButton() {
    FileLogger::Println(__FUNCTION__);
    ITfLangBarItemMgr* manager = nullptr;
    HRESULT hr = fThreadManager->QueryInterface(IID_ITfLangBarItemMgr, (void**)&manager);
    if (FAILED(hr) || !manager) {
      FileLogger::Println("1");
      return false;
    }
    defer{
      manager->Release();
    };
    auto button = new (std::nothrow) LangBarItemButton();
    if (button == nullptr) {
      FileLogger::Println("2");
      return false;
    }
    fLangBarItemButton = button;
    manager->AddItem(button);
    if (FAILED(hr)) {
      FileLogger::Println("3");
    }
    return hr == S_OK;
  }

  void DeinitLangBarItemButton() {
    if (!fLangBarItemButton) {
      return;
    }
    ITfLangBarItemMgr* manager = nullptr;
    HRESULT hr = fThreadManager->QueryInterface(IID_ITfLangBarItemMgr, (void**)&manager);
    if (FAILED(hr) || !manager) {
      return;
    }
    defer{
      manager->Release();
    };
    manager->RemoveItem(fLangBarItemButton);
    fLangBarItemButton->Release();
    fLangBarItemButton = nullptr;
  }

private:
  LONG fRefCount;
  ITfThreadMgr* fThreadManager;
  TfClientId fClientId;
  DWORD fActivateFlags;
  LangBarItemButton* fLangBarItemButton;
};
