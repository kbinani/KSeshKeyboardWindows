#pragma once

class SettingsDialog {
public:
  explicit SettingsDialog(Settings settings) : fOriginal(settings), fSettings(settings), fHandle(nullptr) {
  }

  Settings show(HWND parent) {
    if (DialogBoxParamW(sDllInstanceHandle, MAKEINTRESOURCEW(IDD_SETTINGS_DIALOG), parent, SettingsDialogProc, reinterpret_cast<LPARAM>(this)) == IDOK) {
      return fSettings;
    } else {
      return fOriginal;
    }
  }

  void focus() {
    if (!fHandle) {
      return;
    }
    SetActiveWindow(fHandle);
  }

private:
  void update(HWND hwnd) {
    for (DWORD i = static_cast<DWORD>(IReplacement::IReplacementMin); i <= static_cast<DWORD>(IReplacement::IReplacementMax); i++) {
      auto ir = static_cast<IReplacement>(i);
      auto label = StringFromIReplacement(ir) + L": " + DescriptionFromIReplacement(ir);
      SetDlgItemTextW(hwnd, 2000 + i, label.c_str());
    }
    SetDlgItemTextW(hwnd, 3001, L"Replace q (small) with ḳ: U+1E33");
    SetDlgItemTextW(hwnd, 4001, L"Replace Y (capital) with ï: U+00EF");
    SetDlgItemTextW(hwnd, 5001, L"Replace A (capital) with capital Ꜣ");
    SetDlgItemTextW(hwnd, 6001, L"Replace a (small) with capital ꜥ");

    CheckRadioButton(
      hwnd,
      2000 + static_cast<DWORD>(IReplacement::IReplacementMin),
      2000 + static_cast<DWORD>(IReplacement::IReplacementMax),
      2000 + static_cast<DWORD>(fSettings.fIReplacement)
    );
    CheckDlgButton(hwnd, 3001, fSettings.fReplaceSmallQ ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, 4001, fSettings.fReplaceCapitalY ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, 5001, fSettings.fCapitalAleph ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, 6001, fSettings.fCapitalAin ? BST_CHECKED : BST_UNCHECKED);
  }

  void updateTitle(HWND hwnd) {
    std::wstring title = L"KSesh IME Settings";
    if (!fOriginal.equals(fSettings)) {
      title += L"*";
    }
    SetWindowTextW(hwnd, title.c_str());
  }

  void initDialog(HWND hwnd) {
    fHandle = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    update(hwnd);
    updateTitle(hwnd);
  }

  void handleCommand(HWND hwnd, WPARAM wParam) {
    for (DWORD i = static_cast<DWORD>(IReplacement::IReplacementMin); i <= static_cast<DWORD>(IReplacement::IReplacementMax); i++) {
      if (IsDlgButtonChecked(hwnd, i + 2000) == BST_CHECKED) {
        fSettings.fIReplacement = static_cast<IReplacement>(i);
      }
    }
    fSettings.fReplaceSmallQ = IsDlgButtonChecked(hwnd, 3001) == BST_CHECKED;
    fSettings.fReplaceCapitalY = IsDlgButtonChecked(hwnd, 4001) == BST_CHECKED;
    fSettings.fCapitalAleph = IsDlgButtonChecked(hwnd, 5001) == BST_CHECKED;
    fSettings.fCapitalAin = IsDlgButtonChecked(hwnd, 6001) == BST_CHECKED;

    switch (LOWORD(wParam)) {
    case IDOK: {
      EndDialog(hwnd, IDOK);
      break;
    }
    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;
    case IDRESET_TO_DEFAULTS: {
      Settings s;
      fSettings = s;
      update(hwnd);
      break;
    }
    default:
      break;
    }
    updateTitle(hwnd);
  }

  static INT_PTR CALLBACK SettingsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
      auto dialog = reinterpret_cast<SettingsDialog*>(lParam);
      dialog->initDialog(hwnd);
      return TRUE;
    }
    case WM_COMMAND: {
      auto dialog = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      dialog->handleCommand(hwnd, wParam);
      return TRUE;
    }
    default:
      break;
    }
    return FALSE;
  }

private:
  Settings fOriginal;
  Settings fSettings;
  HWND fHandle;
};
