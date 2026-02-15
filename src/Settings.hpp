#pragma once

constexpr WCHAR kRegistrySettingReplaceQKey[] = L"ReplaceQ";
constexpr WCHAR kRegistrySettingReplaceYKey[] = L"ReplaceY";
constexpr WCHAR kRegistrySettingITypeKey[] = L"IType";
constexpr WCHAR kRegistrySettingCapitalAlephKey[] = L"CapitalAleph";
constexpr WCHAR kRegistrySettingCapitalAinKey[] = L"CapitalAin";

constexpr DWORD kRegistrySettingReplaceQDefault = 0;
constexpr DWORD kRegistrySettingReplaceYDefault = 1;
constexpr DWORD kRegistrySettingITypeDefault = 4;
constexpr DWORD kRegistrySettingCapitalAlephDefault = 0;
constexpr DWORD kRegistrySettingCapitalAinDefault = 0;

class Settings {
public:
  static Settings Load() {
    Settings s;
    s.fReplaceSmallQ = LoadRegistryDWORD(kRegistrySettingReplaceQKey, kRegistrySettingReplaceQDefault) != 0;
    s.fReplaceCapitalY = LoadRegistryDWORD(kRegistrySettingReplaceYKey, kRegistrySettingReplaceYDefault) != 0;
    switch (LoadRegistryDWORD(kRegistrySettingITypeKey, kRegistrySettingITypeDefault)) {
    case 0:
      s.fIReplacement = IReplacement::SmallDotlessIAndCombiningRightHalfRingAbove;
      break;
    case 1:
      s.fIReplacement = IReplacement::SmallIAndCombiningRightHalfRingAbove;
      break;
    case 2:
      s.fIReplacement = IReplacement::SmallIAndCombiningCyrillicPsiliPneumata;
      break;
    case 3:
      s.fIReplacement = IReplacement::SmallIAndCombiningInvertedBreveBelow;
      break;
    case 5:
      s.fIReplacement = IReplacement::Unchanged;
      break;
    case 4:
    default:
      s.fIReplacement = IReplacement::SmallGlottalI;
      break;
    }
    s.fCapitalAleph = LoadRegistryDWORD(kRegistrySettingCapitalAlephKey, kRegistrySettingCapitalAlephDefault);
    s.fCapitalAin = LoadRegistryDWORD(kRegistrySettingCapitalAinKey, kRegistrySettingCapitalAinDefault);
    return s;
  }

  std::wstring map(WCHAR input) const {
    switch (input) {
    case L'D':
      return unicode::kLatinSmallLetterDWithLineBelow;
    case L'T':
      return unicode::kLatinCapitalLetterTWithLineBelow;
    case L'A':
      if (fCapitalAleph) {
        return unicode::kLatinCapitalLetterEgyptologicalAlef;
      } else {
        return unicode::kLatinSmallLetterEgyptologicalAlef;
      }
    case L'H':
      return unicode::kLatinSmallLetterHWithDotBelow;
    case L'x':
      return unicode::kLatinSmallLetterHWithBreveBelow;
    case L'X':
      return unicode::kLatinSmallLetterHWithLineBelow;
    case L'S':
      return unicode::kLatinSmallLetterSWithCaron;
    case L'a':
      if (fCapitalAin) {
        return unicode::kLatinCapitalLetterEgyptologicalAin;
      } else {
        return unicode::kLatinSmallLetterEgyptologicalAin;
      }
    case L'q':
      if (fReplaceSmallQ) {
        return unicode::kLatinSmallLetterKWithDotBelow;
      } else {
        break;
      }
    case L'i':
      return StringFromIReplacement(fIReplacement);
    case L'Y':
      if (fReplaceCapitalY) {
        return unicode::kLatinSmallLetterIWithDiaeresis;
      } else {
        break;
      }
    default:
      break;
    }
    return std::wstring(1, input);
  }

  void save() {
    SaveRegistryDWORD(kRegistrySettingITypeKey, static_cast<DWORD>(fIReplacement));
    SaveRegistryDWORD(kRegistrySettingReplaceQKey, fReplaceSmallQ ? 1 : 0);
    SaveRegistryDWORD(kRegistrySettingReplaceYKey, fReplaceCapitalY ? 1 : 0);
    SaveRegistryDWORD(kRegistrySettingCapitalAlephKey, fCapitalAleph ? 1 : 0);
    SaveRegistryDWORD(kRegistrySettingCapitalAinKey, fCapitalAin ? 1 : 0);
  }

  bool equals(Settings const& other) const {
    return fIReplacement == other.fIReplacement
      && fReplaceSmallQ == other.fReplaceSmallQ
      && fReplaceCapitalY == other.fReplaceCapitalY
      && fCapitalAleph == other.fCapitalAleph
      && fCapitalAin == other.fCapitalAin;
  }

public:
  IReplacement fIReplacement = IReplacement::SmallGlottalI;
  bool fReplaceSmallQ = false;
  bool fReplaceCapitalY = true;
  bool fCapitalAleph = false;
  bool fCapitalAin = false;
};
