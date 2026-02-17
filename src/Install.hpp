#pragma once

static BOOL RegisterServer() {
  DWORD copiedStringLen = 0;
  HKEY regKeyHandle = nullptr;
  HKEY regSubkeyHandle = nullptr;
  WCHAR achIMEKey[ARRAYSIZE(kRegInfoPrefixCLSID) + kGUIDStringLength] = { '\0' };
  WCHAR achFileName[MAX_PATH] = { '\0' };

  defer{
    if (regSubkeyHandle) {
      RegCloseKey(regSubkeyHandle);
    }
    if (regKeyHandle) {
      RegCloseKey(regKeyHandle);
    }
  };

  if (!CLSIDToString(kClassId, achIMEKey + ARRAYSIZE(kRegInfoPrefixCLSID) - 1)) {
    return FALSE;
  }

  memcpy(achIMEKey, kRegInfoPrefixCLSID, sizeof(kRegInfoPrefixCLSID) - sizeof(WCHAR));

  if (RegCreateKeyExW(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKeyHandle, &copiedStringLen) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegSetValueExW(regKeyHandle, NULL, 0, REG_SZ, (BYTE const*)kTextServiceDescription, (_countof(kTextServiceDescription)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegCreateKeyExW(regKeyHandle, kRegInfoKeyInProSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regSubkeyHandle, &copiedStringLen) != ERROR_SUCCESS) {
    return FALSE;
  }
  copiedStringLen = GetModuleFileNameW(sDllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
  copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
  if (RegSetValueExW(regSubkeyHandle, NULL, 0, REG_SZ, (BYTE const*)achFileName, (copiedStringLen) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegSetValueExW(regSubkeyHandle, kRegInfoKeyThreadModel, 0, REG_SZ, (BYTE const*)kTextServiceModel, (_countof(kTextServiceModel)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
    return FALSE;
  }

  return TRUE;
}

static LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey) {
  HKEY regKeyHandle = nullptr;
  LONG res = 0;
  FILETIME time;
  WCHAR stringBuffer[256] = { '\0' };
  DWORD size = ARRAYSIZE(stringBuffer);

  if (RegOpenKeyW(hParentKey, lpszKey, &regKeyHandle) != ERROR_SUCCESS) {
    return ERROR_SUCCESS;
  }

  res = ERROR_SUCCESS;
  while (RegEnumKeyExW(regKeyHandle, 0, stringBuffer, &size, NULL, NULL, NULL, &time) == ERROR_SUCCESS) {
    stringBuffer[ARRAYSIZE(stringBuffer) - 1] = '\0';
    res = RecurseDeleteKey(regKeyHandle, stringBuffer);
    if (res != ERROR_SUCCESS) {
      break;
    }
    size = ARRAYSIZE(stringBuffer);
  }
  RegCloseKey(regKeyHandle);

  return res == ERROR_SUCCESS ? RegDeleteKeyW(hParentKey, lpszKey) : res;
}

static void UnregisterServer() {
  WCHAR achIMEKey[ARRAYSIZE(kRegInfoPrefixCLSID) + kGUIDStringLength] = { '\0' };

  if (!CLSIDToString(kClassId, achIMEKey + ARRAYSIZE(kRegInfoPrefixCLSID) - 1)) {
    return;
  }

  memcpy(achIMEKey, kRegInfoPrefixCLSID, sizeof(kRegInfoPrefixCLSID) - sizeof(WCHAR));

  RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}

static BOOL RegisterCategories() {
  ITfCategoryMgr* manager = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return FALSE;
  }

  for (GUID guid : kSupportCategories) {
    hr = manager->RegisterCategory(kClassId, guid, kClassId);
  }

  manager->Release();

  return hr == S_OK;
}

static void UnregisterCategories() {
  ITfCategoryMgr* manager = nullptr;

  HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return;
  }

  for (GUID guid : kSupportCategories) {
    manager->UnregisterCategory(kClassId, guid, kClassId);
  }

  manager->Release();

  return;
}

static BOOL RegisterProfiles() {
  ITfInputProcessorProfileMgr* manager = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return FALSE;
  }
  defer{
    manager->Release();
  };

  WCHAR achIconFile[MAX_PATH] = { '\0' };
  DWORD cchA = 0;
  cchA = GetModuleFileNameW(sDllInstanceHandle, achIconFile, MAX_PATH);
  cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
  achIconFile[cchA] = '\0';

  size_t lenOfDesc = 0;
  if (StringCchLengthW(kTextServiceDescription, STRSAFE_MAX_CCH, &lenOfDesc) != S_OK) {
    return FALSE;
  }
  hr = manager->RegisterProfile(
    kClassId,
    kTextServiceLanguageId,
    kProfileId,
    kTextServiceDescription,
    static_cast<ULONG>(lenOfDesc),
    achIconFile,
    cchA,
    (UINT)(-IDIS_ICON_BRAND),
    NULL,
    0,
    TRUE,
    0);

  return hr == S_OK;
}

void UnregisterProfiles() {
  ITfInputProcessorProfileMgr* manager = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&manager);
  if (FAILED(hr) || !manager) {
    return;
  }
  defer{
    manager->Release();
  };

  manager->UnregisterProfile(kClassId, kTextServiceLanguageId, kProfileId, 0);
}
