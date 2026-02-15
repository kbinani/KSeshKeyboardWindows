#pragma once

class SettingsDialog {
public:
  explicit SettingsDialog(Settings settings) : fOriginal(settings), fSettings(settings) {
  }

  Settings show() {
    if (DialogBoxParamW(sDllInstanceHandle, MAKEINTRESOURCEW(IDD_SETTINGS_DIALOG), nullptr, SettingsDialogProc, reinterpret_cast<LPARAM>(this)) == IDOK) {
      return fSettings;
    } else {
      return fOriginal;
    }
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

    CheckDlgButton(hwnd, 3001, fSettings.fReplaceSmallQ ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, 4001, fSettings.fReplaceCapitalY ? BST_CHECKED : BST_UNCHECKED);
    CheckRadioButton(
      hwnd,
      2000 + static_cast<DWORD>(IReplacement::IReplacementMin),
      2000 + static_cast<DWORD>(IReplacement::IReplacementMax),
      2000 + static_cast<DWORD>(fSettings.fIReplacement)
    );
  }

  static INT_PTR CALLBACK SettingsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
      auto dialog = reinterpret_cast<SettingsDialog*>(lParam);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dialog));
      dialog->update(hwnd);
      return TRUE;
    }
    case WM_COMMAND: {
      SettingsDialog* dialog = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      for (DWORD i = static_cast<DWORD>(IReplacement::IReplacementMin); i <= static_cast<DWORD>(IReplacement::IReplacementMax); i++) {
        if (IsDlgButtonChecked(hwnd, i + 2000) == BST_CHECKED) {
          dialog->fSettings.fIReplacement = static_cast<IReplacement>(i);
        }
      }
      dialog->fSettings.fReplaceSmallQ = IsDlgButtonChecked(hwnd, 3001) == BST_CHECKED;
      dialog->fSettings.fReplaceCapitalY = IsDlgButtonChecked(hwnd, 4001) == BST_CHECKED;

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
        dialog->fSettings = s;
        dialog->update(hwnd);
        break;
      }
      default:
        break;
      }
      std::wstring title = L"KSesh IME Settings";
      if (!dialog->fOriginal.equals(dialog->fSettings)) {
        title += L"*";
      }
      SetWindowTextW(hwnd, title.c_str());
      return TRUE;
    }
    }
    return FALSE;
  }

private:
  Settings fOriginal;
  Settings fSettings;
};
