#pragma once

class LangBarItemButton : public ITfLangBarItemButton, public ITfSource {
public:
  explicit LangBarItemButton(GUID id, std::function<void()> onChange)
    : fRefCount(1)
    , fId(id)
    , fLangBarItemSink(nullptr)
    , fMenuWindow(nullptr)
    , fOnChange(onChange) {
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
    FileLogger::Println(__FUNCTION__);
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
    switch (click) {
    case TF_LBI_CLK_RIGHT: {
      HMENU menu = CreatePopupMenu();
      if (!menu) {
        return E_FAIL;
      }
      AppendMenuW(menu, MF_STRING, 1, L"Settings");
      UINT command = TrackPopupMenuEx(
        menu,
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON,
        pt.x,
        pt.y,
        GetFocus(),
        nullptr
      );
      DestroyMenu(menu);
      if (command == 1) {
        DialogBoxParamW(sDllInstanceHandle, MAKEINTRESOURCEW(IDD_SETTINGS_DIALOG), nullptr, SettingsDialogProc, reinterpret_cast<LPARAM>(this));
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
    FileLogger::Println(__FUNCTION__);
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
    FileLogger::Println(__FUNCTION__);
    *pbstrText = SysAllocString(L"Settings");

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

  static INT_PTR CALLBACK SettingsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
      auto t = reinterpret_cast<LangBarItemButton*>(lParam);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(t));

      SetDlgItemTextW(hwnd, 2000, L"ı͗: ı + U+0357");
      SetDlgItemTextW(hwnd, 2001, L"i͗: i + U+0357");
      SetDlgItemTextW(hwnd, 2002, L"i҆: i + U+0486");
      SetDlgItemTextW(hwnd, 2003, L"i̯: i + U+032F");
      SetDlgItemTextW(hwnd, 2004, L"ꞽ: U+A7BD");
      SetDlgItemTextW(hwnd, 2005, L"i: U+0069 (unchanged)");
      SetDlgItemTextW(hwnd, 3001, L"Replace q (small) with ḳ: U+1E33");
      SetDlgItemTextW(hwnd, 4001, L"Replace Y (capital) with ï: U+00EF");

      DWORD replaceQ = LoadRegistryDWORD(kRegistrySettingReplaceQKey, kRegistrySettingReplaceQDefault);
      CheckDlgButton(hwnd, 3001, replaceQ == 0 ? BST_UNCHECKED : BST_CHECKED);
      DWORD replaceY = LoadRegistryDWORD(kRegistrySettingReplaceYKey, kRegistrySettingReplaceYDefault);
      CheckDlgButton(hwnd, 4001, replaceY == 0 ? BST_UNCHECKED : BST_CHECKED);
      DWORD iType = LoadRegistryDWORD(kRegistrySettingITypeKey, kRegistrySettingITypeDefault);
      CheckRadioButton(hwnd, 2000, 2005, 2000 + iType);
      return TRUE;
    }
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDOK: {
        LangBarItemButton *ptr =  reinterpret_cast<LangBarItemButton*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        for (int i = 2000; i <= 2005; i++) {
          if (IsDlgButtonChecked(hwnd, i) == BST_CHECKED) {
            SaveRegistryDWORD(kRegistrySettingITypeKey, i - 2000);
          }
        }
        auto replaceQ = IsDlgButtonChecked(hwnd, 3001) == BST_CHECKED;
        SaveRegistryDWORD(kRegistrySettingReplaceQKey, replaceQ ? 1 : 0);
        auto replaceY = IsDlgButtonChecked(hwnd, 4001) == BST_CHECKED;
        SaveRegistryDWORD(kRegistrySettingReplaceYKey, replaceY ? 1 : 0);
        if (ptr->fOnChange) {
          ptr->fOnChange();
        }
        EndDialog(hwnd, IDOK);
        break;
      }
      case IDCANCEL:
        EndDialog(hwnd, IDCANCEL);
        break;
      default:
        break;
      }
      return TRUE;
    }
    return FALSE;
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
  std::function<void()> fOnChange;
};
