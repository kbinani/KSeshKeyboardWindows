#pragma once

constexpr WCHAR kRegistrySettingReplaceQKey[] = L"ReplaceQ";
constexpr WCHAR kRegistrySettingReplaceYKey[] = L"ReplaceY";
constexpr WCHAR kRegistrySettingITypeKey[] = L"IType";

constexpr DWORD kRegistrySettingReplaceQDefault = 0;
constexpr DWORD kRegistrySettingReplaceYDefault = 1;
constexpr DWORD kRegistrySettingITypeDefault = 4;

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
    return s;
  }

  std::optional<std::wstring> map(WCHAR input) const {
    switch (input) {
    case L'D':
      return unicode::kLatinSmallLetterDWithLineBelow;
    case L'T':
      return unicode::kLatinCapitalLetterTWithLineBelow;
    case L'A':
      return unicode::kLatinSmallLetterEgyptologicalAlef;
    case L'H':
      return unicode::kLatinSmallLetterHWithDotBelow;
    case L'x':
      return unicode::kLatinSmallLetterHWithBreveBelow;
    case L'X':
      return unicode::kLatinSmallLetterHWithLineBelow;
    case L'S':
      return unicode::kLatinSmallLetterSWithCaron;
    case L'a':
      return unicode::kLatinSmallLetterEgyptologicalAin;
    case L'q':
      if (fReplaceSmallQ) {
        return unicode::kLatinSmallLetterKWithDotBelow;
      } else {
        return std::nullopt;
      }
    case L'i':
      return StringFromIReplacement(fIReplacement);
    case L'Y':
      if (fReplaceCapitalY) {
        return unicode::kLatinSmallLetterIWithDiaeresis;
      } else {
        return std::nullopt;
      }
    default:
      break;
    }
    return std::nullopt;
  }

  void save() {
    SaveRegistryDWORD(kRegistrySettingITypeKey, static_cast<DWORD>(fIReplacement));
    SaveRegistryDWORD(kRegistrySettingReplaceQKey, fReplaceSmallQ ? 1 : 0);
    SaveRegistryDWORD(kRegistrySettingReplaceYKey, fReplaceCapitalY ? 1 : 0);
  }

  bool equals(Settings const& other) const {
    return fIReplacement == other.fIReplacement && fReplaceSmallQ == other.fReplaceSmallQ && fReplaceCapitalY == other.fReplaceCapitalY;
  }

public:
  IReplacement fIReplacement = IReplacement::SmallGlottalI;
  bool fReplaceSmallQ = false;
  bool fReplaceCapitalY = true;
};
