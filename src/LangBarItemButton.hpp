#pragma once

class LangBarItemButton : public ITfLangBarItemButton, public ITfSource {
public:
  explicit LangBarItemButton(GUID id, std::function<void(Settings const&)> onChangeSettings)
    : fRefCount(1)
    , fId(id)
    , fLangBarItemSink(nullptr)
    , fMenuWindow(nullptr)
    , fOnChangeSettings(onChangeSettings) {
    DllAddRef();
    InitMenuWindow();
  }

  ~LangBarItemButton() {
    DllRelease();
    if (fMenuWindow) {
      DestroyWindow(fMenuWindow);
    }
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
    if (!pInfo) {
      return E_INVALIDARG;
    }
    pInfo->clsidService = kClassId;
    pInfo->guidItem = fId;
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    pInfo->ulSort = 0;
    StringCchCopyW(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), L"");
    return S_OK;
  }

  STDMETHODIMP GetStatus(_Out_ DWORD* pdwStatus) override {
    if (!pdwStatus) {
      return E_INVALIDARG;
    }
    *pdwStatus = 0;
    return S_OK;
  }

  STDMETHODIMP Show(BOOL fShow) override {
    return S_OK;
  }

  STDMETHODIMP GetTooltipString(_Out_ BSTR* pbstrToolTip) override {
    *pbstrToolTip = SysAllocString(L"KSesh IME v" KSESH_IME_VERSION);

    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
  }

  STDMETHODIMP OnClick(TfLBIClick click, POINT pt, _In_ const RECT* prcArea) override {
    switch (click) {
    case TF_LBI_CLK_RIGHT: {
      HMENU menu = CreatePopupMenu();
      if (!menu) {
        return E_FAIL;
      }
      UINT constexpr id = 1;
      AppendMenuW(menu, MF_STRING, id, L"Settings");
      UINT command = TrackPopupMenuEx(
        menu,
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON,
        pt.x,
        pt.y,
        GetFocus(),
        nullptr
      );
      DestroyMenu(menu);
      if (command == id) {
        if (fSettingsDialog) {
          fSettingsDialog->focus();
          return S_OK;
        }
        auto current = Settings::Load();
        fSettingsDialog = std::make_unique<SettingsDialog>(current);
        Settings after = fSettingsDialog->show(GetFocus());
        if (!after.equals(current)) {
          after.save();
          if (fOnChangeSettings) {
            fOnChangeSettings(after);
          }
        }
        fSettingsDialog = nullptr;
      }
      return S_OK;
    }
    case TF_LBI_CLK_LEFT:
      break;
    }
    return S_OK;
  }

  STDMETHODIMP InitMenu(_In_ ITfMenu* pMenu) override {
    return E_NOTIMPL;
  }

  STDMETHODIMP OnMenuSelect(UINT wID) override {
    return E_NOTIMPL;
  }

  STDMETHODIMP GetIcon(_Out_ HICON* phIcon) override {
    if (!phIcon) {
      return E_INVALIDARG;
    }
    bool dark = IsInDarkMode();
    WORD icon = dark ? IDIS_ICON_TRANSLITERATION_WHITE : IDIS_ICON_TRANSLITERATION_BLACK;
    *phIcon = (HICON)LoadImageW(
      sDllInstanceHandle,
      MAKEINTRESOURCEW(icon),
      IMAGE_ICON,
      24,
      24,
      LR_DEFAULTCOLOR
    );
    return *phIcon == nullptr ? E_FAIL : S_OK;
  }

  STDMETHODIMP GetText(_Out_ BSTR* pbstrText) override {
    *pbstrText = SysAllocString(L"Settings");

    return (*pbstrText == nullptr) ? E_OUTOFMEMORY : S_OK;
  }

  STDMETHODIMP AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown* punk, __RPC__out DWORD* pdwCookie) override {
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
  static bool IsInDarkMode() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(
      HKEY_CURRENT_USER,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
      L"AppsUseLightTheme",
      RRF_RT_REG_DWORD,
      nullptr,
      &value,
      &size
   );
    return value == 0;
  }

  void InitMenuWindow() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MenuWindowProc;
    wc.hInstance = sDllInstanceHandle;
    wc.lpszClassName = L"KSeshMenuWindow";
    RegisterClassExW(&wc);
    fMenuWindow = CreateWindowExW(
      0,
      L"KSeshMenuWindow",
      L"",
      WS_POPUP,
      0, 0, 0, 0,
      nullptr,
      nullptr,
      sDllInstanceHandle,
      nullptr
    );
    SetWindowLongPtrW(fMenuWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }

  static LRESULT CALLBACK MenuWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SETTINGCHANGE) {
      if (lParam && wcscmp((LPCWSTR)lParam, L"ImmersiveColorSet") == 0) {
        auto* button = reinterpret_cast<LangBarItemButton*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (button && button->fLangBarItemSink) {
          button->fLangBarItemSink->OnUpdate(TF_LBI_ICON);
        }
      }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
  }

private:
  LONG fRefCount;
  GUID fId;
  DWORD const fCookie = 0;
  ITfLangBarItemSink* fLangBarItemSink;
  HWND fMenuWindow;
  std::function<void(Settings const&)> fOnChangeSettings;
  std::unique_ptr<SettingsDialog> fSettingsDialog;
};
