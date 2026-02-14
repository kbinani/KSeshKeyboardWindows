#pragma once

constexpr WCHAR kRegistryPath[] = L"Software\\KSeshIME\\Settings";

constexpr WCHAR kRegistrySettingReplaceQKey[] = L"ReplaceQ";
constexpr WCHAR kRegistrySettingReplaceYKey[] = L"ReplaceY";
constexpr WCHAR kRegistrySettingITypeKey[] = L"IType";

constexpr DWORD kRegistrySettingReplaceQDefault = 0;
constexpr DWORD kRegistrySettingReplaceYDefault = 1;
constexpr DWORD kRegistrySettingITypeDefault = 4;

static bool SaveRegistryDWORD(LPCWSTR valueName, DWORD value) {
  HKEY hKey;
  LONG result = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    kRegistryPath,
    0,
    nullptr,
    REG_OPTION_NON_VOLATILE,
    KEY_WRITE,
    nullptr,
    &hKey,
    nullptr
  );
  if (result != ERROR_SUCCESS) {
    return false;
  }
  defer{
    RegCloseKey(hKey);
  };
  return RegSetValueExW(hKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD)) == ERROR_SUCCESS;
}

static DWORD LoadRegistryDWORD(LPCWSTR valueName, DWORD defaultValue) {
  HKEY hKey;
  LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_READ, &hKey);
  if (result != ERROR_SUCCESS) {
    return defaultValue;
  }
  defer{
    RegCloseKey(hKey);
  };

  DWORD value = defaultValue;
  DWORD dataSize = sizeof(DWORD);
  if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, (BYTE*)&value, &dataSize) == ERROR_SUCCESS) {
    return value;
  } else {
    return defaultValue;
  }
}
