#pragma once

class LangBarItemButton : public ITfLangBarItemButton, public ITfSource {
public:
  explicit LangBarItemButton(GUID id)
    : fRefCount(1)
    , fId(id)
    , fLangBarItemSink(nullptr)
    , fDescription(L"Settings") {
    DllAddRef();
  }

  ~LangBarItemButton() {
    DllRelease();
  }

  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    if (!ppvObj) {
      return E_INVALIDARG;
    }
    *ppvObj = nullptr;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfLangBarItem) || IsEqualIID(riid, IID_ITfLangBarItemButton)) {
      *ppvObj = dynamic_cast<ITfLangBarItemButton*>(this);
    } else if (IsEqualIID(riid, IID_ITfSource)) {
      *ppvObj = dynamic_cast<ITfSource*>(this);
    }
    if (*ppvObj == nullptr) {
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return ++fRefCount;
  }

  STDMETHODIMP_(ULONG) Release() override {
    LONG after = --fRefCount;
    if (after == 0) {
      delete this;
    }
    return after;
  }

  STDMETHODIMP GetInfo(_Out_ TF_LANGBARITEMINFO* pInfo) override {
    FileLogger::Println(__FUNCTION__);
    if (!pInfo) {
      return E_INVALIDARG;
    }
    pInfo->clsidService = kClassId;
    pInfo->guidItem = fId;
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    pInfo->ulSort = 0;
    StringCchCopyW(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), fDescription.c_str());
    return S_OK;
  }

  STDMETHODIMP GetStatus(_Out_ DWORD* pdwStatus) override {
    FileLogger::Println(__FUNCTION__);
    if (!pdwStatus) {
      return E_INVALIDARG;
    }
    *pdwStatus = 0;
    return S_OK;
  }

  STDMETHODIMP Show(BOOL fShow) override {
    FileLogger::Println(__FUNCTION__);
    return S_OK;
  }

  STDMETHODIMP GetTooltipString(_Out_ BSTR* pbstrToolTip) override {
    FileLogger::Println(__FUNCTION__);
    return S_OK;
  }

  STDMETHODIMP OnClick(TfLBIClick click, POINT pt, _In_ const RECT* prcArea) override {
    FileLogger::Println(__FUNCTION__);
    return S_OK;
  }

  STDMETHODIMP InitMenu(_In_ ITfMenu* pMenu) override {
    FileLogger::Println(__FUNCTION__);
    pMenu->AddMenuItem(1, 0, nullptr, nullptr, L"wa", (ULONG)wcslen(L"wa"), nullptr);
    return S_OK;
  }

  STDMETHODIMP OnMenuSelect(UINT wID) override {
    FileLogger::Println(__FUNCTION__);
    return S_OK;
  }

  STDMETHODIMP GetIcon(_Out_ HICON* phIcon) override {
    FileLogger::Println(__FUNCTION__);
    if (!phIcon) {
      return E_INVALIDARG;
    }
    *phIcon = (HICON)LoadImageW(
      sDllInstanceHandle,
      MAKEINTRESOURCEW(IDIS_KSESHKEYBOARD),
      IMAGE_ICON,
      16,
      16,
      LR_DEFAULTCOLOR
    );
    FileLogger::Println(__FUNCTION__ + std::string(*phIcon == nullptr ? ": fail" : ": ok"));
    return *phIcon == nullptr ? E_FAIL : S_OK;
  }

  STDMETHODIMP GetText(_Out_ BSTR* pbstrText) override {
    FileLogger::Println(__FUNCTION__);
    *pbstrText = SysAllocString(fDescription.c_str());

    return (*pbstrText == nullptr) ? E_OUTOFMEMORY : S_OK;
  }

  STDMETHODIMP AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown* punk, __RPC__out DWORD* pdwCookie) override {
    FileLogger::Println(__FUNCTION__);

    if (!IsEqualIID(IID_ITfLangBarItemSink, riid)) {
      return CONNECT_E_CANNOTCONNECT;
    }

    if (fLangBarItemSink != nullptr) {
      return CONNECT_E_ADVISELIMIT;
    }

    if (punk == nullptr) {
      return E_INVALIDARG;
    }
    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void**)&fLangBarItemSink) != S_OK) {
      fLangBarItemSink = nullptr;
      return E_NOINTERFACE;
    }

    *pdwCookie = fCookie;
    return S_OK;
  }

  STDMETHODIMP UnadviseSink(DWORD dwCookie) override {
    FileLogger::Println(__FUNCTION__);
    if (dwCookie != fCookie) {
      return CONNECT_E_NOCONNECTION;
    }

    if (fLangBarItemSink == nullptr) {
      return CONNECT_E_NOCONNECTION;
    }

    fLangBarItemSink->Release();
    fLangBarItemSink = nullptr;
    return S_OK;
  }

private:
  LONG fRefCount;
  GUID fId;
  DWORD const fCookie = 0;
  ITfLangBarItemSink* fLangBarItemSink;
  std::wstring const fDescription;
};
