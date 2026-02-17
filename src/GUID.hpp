#pragma once

static BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR* pCLSIDString) {
  WCHAR* pTemp = pCLSIDString;
  BYTE const* pBytes = (BYTE const*)&refGUID;

  BYTE const GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
  };
  WCHAR const HexDigits[] = L"0123456789ABCDEF";

  DWORD j = 0;
  pTemp[j++] = L'{';
  for (int i = 0; i < sizeof(GuidSymbols) && j < (kGUIDStringLength - 2); i++) {
    if (GuidSymbols[i] == '-') {
      pTemp[j++] = L'-';
    } else {
      pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0xF0) >> 4];
      pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0x0F)];
    }
  }

  pTemp[j++] = L'}';
  pTemp[j] = L'\0';

  return TRUE;
}

static std::string StringFromGUID(REFGUID guid) {
  BYTE const* pBytes = (BYTE const*)&guid;

  BYTE const GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
  };
  char const HexDigits[] = "0123456789ABCDEF";
  std::string ret(38, '\0');
  DWORD j = 0;
  ret[j++] = '{';
  for (int i = 0; i < sizeof(GuidSymbols) && j < (kGUIDStringLength - 2); i++) {
    if (GuidSymbols[i] == '-') {
      ret[j++] = L'-';
    } else {
      ret[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0xF0) >> 4];
      ret[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0x0F)];
    }
  }

  ret[j++] = L'}';
  return ret;
}
