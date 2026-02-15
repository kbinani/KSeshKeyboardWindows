#pragma once

class Settings {
public:
  Settings() {
    fReplaceSmallQ = LoadRegistryDWORD(kRegistrySettingReplaceQKey, kRegistrySettingReplaceQDefault) != 0;
    fReplaceCapitalY = LoadRegistryDWORD(kRegistrySettingReplaceYKey, kRegistrySettingReplaceYDefault) != 0;
    switch (LoadRegistryDWORD(kRegistrySettingITypeKey, kRegistrySettingITypeDefault)) {
    case 0:
      fIReplacement = IReplacement::SmallDotlessIAndCombiningRightHalfRingAbove;
      break;
    case 1:
      fIReplacement = IReplacement::SmallIAndCombiningRightHalfRingAbove;
      break;
    case 2:
      fIReplacement = IReplacement::SmallIAndCombiningCyrillicPsiliPneumata;
      break;
    case 3:
      fIReplacement = IReplacement::SmallIAndCombiningInvertedBreveBelow;
      break;
    case 5:
      fIReplacement = IReplacement::SmallGlottalI;
      break;
    case 4:
    default:
      fIReplacement = IReplacement::Unchanged;
      break;
    }
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
      return GetIReplacement(fIReplacement);
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

public:
  IReplacement fIReplacement = IReplacement::SmallGlottalI;
  bool fReplaceSmallQ = false;
  bool fReplaceCapitalY = true;
};
