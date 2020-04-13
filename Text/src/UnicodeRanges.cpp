// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Text/UnicodeRanges.hpp"

namespace TTauri::Text {

/** Table is based on: https://docs.microsoft.com/en-us/typography/opentype/spec/os2#ur
*/
constexpr std::array unicodeRangeToBitPosition = {
    std::pair<char32_t,int>{0x000000, 0}, // Basic Latin
    std::pair<char32_t,int>{0x000080, 1}, // Latin-1 Supplement
    std::pair<char32_t,int>{0x000100, 2}, // Latin Extended-A	-017F	
    std::pair<char32_t,int>{0x000180, 3}, // 	Latin Extended-B	-024F	
    std::pair<char32_t,int>{0x000250, 4}, // 	IPA Extensions	-02AF	
    std::pair<char32_t,int>{0x0002B0, 5}, // 	Spacing Modifier Letters	-02FF	
    std::pair<char32_t,int>{0x000300, 6}, // 	Combining Diacritical Marks	-036F	
    std::pair<char32_t,int>{0x000370, 7}, // 	Greek and Coptic	-03FF	
    std::pair<char32_t,int>{0x000400, 9}, // 	Cyrillic	-04FF	
    std::pair<char32_t,int>{0x000500, 9}, //  Cyrillic Supplement	-052F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x000530, 10}, // 	Armenian	-058F	
    std::pair<char32_t,int>{0x000590, 11}, // 	Hebrew	-05FF	
    std::pair<char32_t,int>{0x000600, 13}, // 	Arabic	-06FF	
    std::pair<char32_t,int>{0x000700, 71}, // 	Syriac	-074F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x000750, 13}, //  Arabic Supplement	-077F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x000780, 72}, // 	Thaana	-07BF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x0007C0, 14}, // 	NKo	-07FF	Added in OpenType 1.5 for OS/2 version 4. See below for other version differences.
    std::pair<char32_t,int>{0x000900, 15}, // 	Devanagari	-097F	
    std::pair<char32_t,int>{0x000980, 16}, // 	Bengali	-09FF	
    std::pair<char32_t,int>{0x000A00, 17}, // 	Gurmukhi	-0A7F	
    std::pair<char32_t,int>{0x000A80, 18}, // 	Gujarati	-0AFF	
    std::pair<char32_t,int>{0x000B00, 19}, // 	Oriya	-0B7F	
    std::pair<char32_t,int>{0x000B80, 20}, // 	Tamil	-0BFF	
    std::pair<char32_t,int>{0x000C00, 21}, // 	Telugu	-0C7F	
    std::pair<char32_t,int>{0x000C80, 22}, // 	Kannada	-0CFF	
    std::pair<char32_t,int>{0x000D00, 23}, // 	Malayalam	-0D7F	
    std::pair<char32_t,int>{0x000D80, 73}, // 	Sinhala	-0DFF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x000E00, 24}, // 	Thai	-0E7F	
    std::pair<char32_t,int>{0x000E80, 25}, // 	Lao	-0EFF	
    std::pair<char32_t,int>{0x000F00, 70}, // 	Tibetan	-0FFF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001000, 74}, // 	Myanmar	-109F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x0010A0, 26}, // 	Georgian	-10FF	
    std::pair<char32_t,int>{0x001100, 28}, // 	Hangul Jamo	-11FF	
    std::pair<char32_t,int>{0x001200, 75}, // 	Ethiopic	-137F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001380, 75}, // Ethiopic Supplement	-139F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0013A0, 76}, // 	Cherokee	-13FF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001400, 77}, // 	Unified Canadian Aboriginal Syllabics	-167F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001680, 78}, // 	Ogham	-169F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x0016A0, 79}, // 	Runic	-16FF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001700, 84}, // 	Tagalog	-171F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x001720, 84}, //  Hanunoo	-173F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x001740, 84}, //  Buhid	-175F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x001760, 84}, //  Tagbanwa	-177F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x001780, 80}, // 	Khmer	-17FF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001800, 81}, // 	Mongolian	-18AF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x001900, 93}, // 	Limbu	-194F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001950, 94}, // 	Tai Le	-197F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001980, 95}, // 	New Tai Lue	-19DF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0019E0, 80}, //  Khmer Symbols	-19FF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001A00, 96}, // 	Buginese	-1A1F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001B00, 27}, // 	Balinese	-1B7F	Added in OpenType 1.5 for OS/2 version 4. See below for other version differences.
    std::pair<char32_t,int>{0x001B80, 112}, // 	Sundanese	-1BBF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001C00, 113}, // 	Lepcha	-1C4F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001C50, 114}, // 	Ol Chiki	-1C7F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001D00, 4}, //  Phonetic Extensions	-1D7F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001D80, 4}, //  Phonetic Extensions Supplement	-1DBF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001DC0, 6}, // Combining Diacritical Marks Supplement	-1DFF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x001E00, 29}, // 	Latin Extended Additional	-1EFF	
    std::pair<char32_t,int>{0x001F00, 30}, // 	Greek Extended	-1FFF	
    std::pair<char32_t,int>{0x002000, 31}, // 	General Punctuation	-206F	
    std::pair<char32_t,int>{0x002070, 32}, // 	Superscripts And Subscripts	-209F	
    std::pair<char32_t,int>{0x0020A0, 33}, // 	Currency Symbols	-20CF	
    std::pair<char32_t,int>{0x0020D0, 34}, // 	Combining Diacritical Marks For Symbols	-20FF	
    std::pair<char32_t,int>{0x002100, 35}, // 	Letterlike Symbols	-214F	
    std::pair<char32_t,int>{0x002150, 36}, // 	Number Forms	-218F	
    std::pair<char32_t,int>{0x002190, 37}, // 	Arrows	-21FF	
    std::pair<char32_t,int>{0x002200, 38}, // 	Mathematical Operators	-22FF	
    std::pair<char32_t,int>{0x002300, 39}, // 	Miscellaneous Technical	-23FF	
    std::pair<char32_t,int>{0x002400, 40}, // 	Control Pictures	-243F	
    std::pair<char32_t,int>{0x002440, 41}, // 	Optical Character Recognition	-245F	
    std::pair<char32_t,int>{0x002460, 42}, // 	Enclosed Alphanumerics	-24FF	
    std::pair<char32_t,int>{0x002500, 43}, // 	Box Drawing	-257F	
    std::pair<char32_t,int>{0x002580, 44}, // 	Block Elements	-259F	
    std::pair<char32_t,int>{0x0025A0, 45}, // 	Geometric Shapes	-25FF	
    std::pair<char32_t,int>{0x002600, 46}, // 	Miscellaneous Symbols	-26FF	
    std::pair<char32_t,int>{0x002700, 47}, // 	Dingbats	-27BF	
    std::pair<char32_t,int>{0x0027C0, 38}, //  Miscellaneous Mathematical Symbols-A	-27EF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x0027F0, 37}, //  Supplemental Arrows-A	-27FF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x002800, 82}, // 	Braille Patterns	-28FF	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x002900, 37}, //  Supplemental Arrows-B	-297F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x002980, 38}, //  Miscellaneous Mathematical Symbols-B	-29FF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x002A00, 38}, //  Supplemental Mathematical Operators	-2AFF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x002B00, 37}, //  Miscellaneous Symbols and Arrows	-2BFF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002C00, 97}, // 	Glagolitic	-2C5F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002C60, 29}, //  Latin Extended-C	2C60-2C7F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002C80, 8}, // 	Coptic	-2CFF	Added in OpenType 1.5 for OS/2 version 4. See below for other version differences.
    std::pair<char32_t,int>{0x002D00, 26}, //  Georgian Supplement	2D00-2D2F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002D30, 98}, // 	Tifinagh	-2D7F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002D80, 75}, // Ethiopic Extended	-2DDF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002DE0, 9}, //  Cyrillic Extended-A	-2DFF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002E00, 31}, //  Supplemental Punctuation	-2E7F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x002E80, 59}, //  CJK Radicals Supplement	-2EFF	Added in OpenType 1.3 for OS/2 version 2.
    std::pair<char32_t,int>{0x002F00, 59}, //  Kangxi Radicals	-2FDF	Added in OpenType 1.3 for OS/2 version 2.
    std::pair<char32_t,int>{0x002FF0, 59}, //  Ideographic Description Characters	-2FFF	Added in OpenType 1.3 for OS/2 version 2.
    std::pair<char32_t,int>{0x003000, 48}, // 	CJK Symbols And Punctuation	-303F	
    std::pair<char32_t,int>{0x003040, 49}, // 	Hiragana	-309F	
    std::pair<char32_t,int>{0x0030A0, 50}, // 	Katakana	-30FF	
    std::pair<char32_t,int>{0x003100, 51}, // 	Bopomofo	-312F	
    std::pair<char32_t,int>{0x003130, 52}, // 	Hangul Compatibility Jamo	-318F	
    std::pair<char32_t,int>{0x003190, 59}, //  Kanbun	-319F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x0031A0, 51}, //  Bopomofo Extended	-31BF	Added in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x0031C0, 61}, // 	CJK Strokes	-31EF	Range added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0031F0, 50}, //  Katakana Phonetic Extensions	-31FF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x003200, 54}, // 	Enclosed CJK Letters And Months	-32FF	
    std::pair<char32_t,int>{0x003300, 55}, // 	CJK Compatibility	-33FF	
    std::pair<char32_t,int>{0x003400, 59}, //  CJK Unified Ideographs Extension A	-4DBF	Added in OpenType 1.3 for OS/2 version 2.
    std::pair<char32_t,int>{0x004DC0, 99}, // 	Yijing Hexagram Symbols	-4DFF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x004E00, 59}, // 	CJK Unified Ideographs	-9FFF	
    std::pair<char32_t,int>{0x00A000, 83}, // 	Yi Syllables	-A48F	First assigned in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x00A490, 83}, //  Yi Radicals	-A4CF	Added in OpenType 1.3, extending OS/2 version 2.
    std::pair<char32_t,int>{0x00A500, 12}, // 	Vai	-A63F	Added in OpenType 1.5 for OS/2 version 4. See below for other version differences.
    std::pair<char32_t,int>{0x00A640, 9}, //  Cyrillic Extended-B	-A69F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A700, 5}, // Modifier Tone Letters	-A71F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A720, 29}, //  Latin Extended-D	-A7FF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A800, 100}, // 	Syloti Nagri	-A82F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A840, 53}, // 	Phags-pa	-A87F	Added in OpenType 1.5 for OS/2 version 4. See below for other version differences.
    std::pair<char32_t,int>{0x00A880, 115}, // 	Saurashtra	-A8DF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A900, 116}, // 	Kayah Li	-A92F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00A930, 117}, // 	Rejang	-A95F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00AA00, 118}, // 	Cham	-AA5F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00AC00, 56}, // 	Hangul Syllables	-D7AF	
    std::pair<char32_t,int>{0x00E000, 60}, // 	Private Use Area (plane 0)	-F8FF	
    std::pair<char32_t,int>{0x00F900, 61}, //  CJK Compatibility Ideographs	-FAFF	
    std::pair<char32_t,int>{0x00FB00, 62}, // 	Alphabetic Presentation Forms	-FB4F	
    std::pair<char32_t,int>{0x00FB50, 63}, // 	Arabic Presentation Forms-A	-FDFF	
    std::pair<char32_t,int>{0x00FE00, 91}, // 	Variation Selectors	-FE0F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x00FE10, 65}, // 	Vertical Forms	-FE1F	Range added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x00FE20, 64}, // 	Combining Half Marks	-FE2F	
    std::pair<char32_t,int>{0x00FE30, 65}, //  CJK Compatibility Forms	-FE4F	
    std::pair<char32_t,int>{0x00FE50, 66}, // 	Small Form Variants	-FE6F	
    std::pair<char32_t,int>{0x00FE70, 67}, // 	Arabic Presentation Forms-B	-FEFF	
    std::pair<char32_t,int>{0x00FF00, 68}, // 	Halfwidth And Fullwidth Forms	-FFEF	
    std::pair<char32_t,int>{0x00FFF0, 69}, // 	Specials	-FFFF	
    std::pair<char32_t,int>{0x010000, 101}, // 	Linear B Syllabary	-1007F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010000, 57}, // 	Non-Plane 0	-10FFFF	Implies at least one character beyond the Basic Multilingual Plane. First assigned in OpenType 1.3 for OS/2 version 2.
    std::pair<char32_t,int>{0x010080, 101}, //  Linear B Ideograms	-100FF	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010100, 101}, //  Aegean Numbers	-1013F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010140, 102}, // 	Ancient Greek Numbers	-1018F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010190, 119}, // 	Ancient Symbols	-101CF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0101D0, 120}, // 	Phaistos Disc	-101FF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010280, 121}, //  Lycian	-1029F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0102A0, 121}, // 	Carian	-102DF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010300, 85}, // 	Old Italic	-1032F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x010330, 86}, // 	Gothic	-1034F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x010380, 103}, // 	Ugaritic	-1039F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x0103A0, 104}, // 	Old Persian	-103DF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010400, 87}, // 	Deseret	-1044F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x010450, 105}, // 	Shavian	-1047F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010480, 106}, // 	Osmanya	-104AF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010800, 107}, // 	Cypriot Syllabary	-1083F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010900, 58}, // 	Phoenician	-1091F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010920, 121}, //  Lydian	-1093F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x010A00, 108}, // 	Kharoshthi	-10A5F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x012000, 110}, // 	Cuneiform	-123FF	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x012400, 110}, //  Cuneiform Numbers and Punctuation	-1247F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x01D000, 88}, // 	Byzantine Musical Symbols	-1D0FF	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x01D100, 88}, //  Musical Symbols	-1D1FF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x01D200, 88}, //  Ancient Greek Musical Notation	-1D24F	Added in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x01D300, 109}, // 	Tai Xuan Jing Symbols	-1D35F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x01D360, 111}, // 	Counting Rod Numerals	-1D37F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x01D400, 89}, // 	Mathematical Alphanumeric Symbols	-1D7FF	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x01F000, 122}, //  Mahjong Tiles	-1F02F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x01F030, 122}, // 	Domino Tiles	-1F09F	First assigned in OpenType 1.5 for OS/2 version 4.
    std::pair<char32_t,int>{0x020000, 59}, //  CJK Unified Ideographs Extension B	-2A6DF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x02F800, 61}, //  CJK Compatibility Ideographs Supplement	-2FA1F	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x0E0000, 92}, // 	Tags	-E007F	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x0E0100, 91}, //  Variation Selectors Supplement	-E01EF	Added in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x0F0000, 90}, // 	Private Use (plane 15)	-FFFFD	First assigned in OpenType 1.4 for OS/2 version 3.
    std::pair<char32_t,int>{0x100000, 90}, //  Private Use (plane 16)	-10FFFD	Added in OpenType 1.4 for OS/2 version 3.
};

void UnicodeRanges::add(char32_t first, char32_t last) noexcept
{
    ttauri_assert(first < last);
    auto first_ = std::upper_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), first, [](let &value, let &element) { return value < element.first; });
    auto last_ = std::upper_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), last, [](let &value, let &element) { return value < element.first; });
    for (auto i = (first_ - 1); i != last_; ++i) {
        set_bit(i->second);
    }
}

void UnicodeRanges::add(char32_t c) noexcept
{
    auto i = std::upper_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), c, [](let &value, let &element) { return value < element.first; });
    return set_bit((i - 1)->second);
}

[[nodiscard]] bool UnicodeRanges::contains(char32_t c) const noexcept
{
    auto i = std::upper_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), c, [](let &value, let &element) { return value < element.first; });
    return get_bit((i - 1)->second);
}

}