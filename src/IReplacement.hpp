#pragma once

enum class IReplacement : DWORD {
  SmallDotlessIAndCombiningRightHalfRingAbove = 0,
  SmallIAndCombiningRightHalfRingAbove,
  SmallIAndCombiningCyrillicPsiliPneumata,
  SmallIAndCombiningInvertedBreveBelow,
  SmallGlottalI,
  Unchanged,

  IReplacementMin = SmallDotlessIAndCombiningRightHalfRingAbove,
  IReplacementMax = Unchanged,
};

inline std::wstring GetIReplacement(IReplacement t) {
  switch (t) {
  case IReplacement::SmallDotlessIAndCombiningRightHalfRingAbove:
    return std::wstring(unicode::kLatinSmallLetterDotlessI) + std::wstring(unicode::kCombiningRightHalfRingAbove);
  case IReplacement::SmallIAndCombiningRightHalfRingAbove:
    return std::wstring(unicode::kLatinSmallLetterI) + std::wstring(unicode::kCombiningRightHalfRingAbove);
  case IReplacement::SmallIAndCombiningCyrillicPsiliPneumata:
    return std::wstring(unicode::kLatinSmallLetterI) + std::wstring(unicode::kCombiningCyrillicPsiliPneumata);
  case IReplacement::SmallIAndCombiningInvertedBreveBelow:
    return std::wstring(unicode::kLatinSmallLetterI) + std::wstring(unicode::kCombiningInvertedBreveBelow);
  case IReplacement::SmallGlottalI:
    return unicode::kLatinSmallLetterGlottalI;
  case IReplacement::Unchanged:
  default:
    break;
  }
  return L"i";
}

inline std::wstring GetIReplacementDescription(IReplacement t) {
  switch (t) {
  case IReplacement::SmallDotlessIAndCombiningRightHalfRingAbove:
    return L"ı + U+0357";
  case IReplacement::SmallIAndCombiningRightHalfRingAbove:
    return L"i + U+0357";
  case IReplacement::SmallIAndCombiningCyrillicPsiliPneumata:
    return L"i + U+0486";
  case IReplacement::SmallIAndCombiningInvertedBreveBelow:
    return L"i + U+032F";
  case IReplacement::SmallGlottalI:
    return L"U+A7BD";
  case IReplacement::Unchanged:
  default:
    break;
  }
  return L"U+0069 (unchanged)";
}
