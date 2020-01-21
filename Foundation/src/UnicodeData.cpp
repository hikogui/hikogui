
#include "TTauri/Foundation/UnicodeData.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include <algorithm>

namespace TTauri {

constexpr char32_t ASCII_MAX = 0x7f;
constexpr char32_t UNICODE_MASK = 0x1f'ffff;
constexpr char32_t UNICODE_MAX = 0x10'ffff;
constexpr char32_t UNICODE_REPLACEMENT_CHAR = 0x00'fffd;
constexpr char32_t UNICODE_INVALID_CHAR = 0x00'ffff;
constexpr char32_t UNICODE_CR_CHAR = 0x00'000a;
constexpr char32_t UNICODE_LF_CHAR = 0x00'000d;

constexpr char32_t HANGUL_SBASE = 0xac00;
constexpr char32_t HANGUL_LBASE = 0x1100;
constexpr char32_t HANGUL_VBASE = 0x1161;
constexpr char32_t HANGUL_TBASE = 0x11a7;
constexpr char32_t HANGUL_LCOUNT = 19;
constexpr char32_t HANGUL_VCOUNT = 21;
constexpr char32_t HANGUL_TCOUNT = 28;
constexpr char32_t HANGUL_NCOUNT = HANGUL_VCOUNT * HANGUL_TCOUNT;
constexpr char32_t HANGUL_SCOUNT = HANGUL_LCOUNT * HANGUL_NCOUNT;

static bool isHangulLPart(char32_t codePoint)
{
    return codePoint >= HANGUL_LBASE && codePoint < (HANGUL_LBASE + HANGUL_LCOUNT);
}

static bool isHangulVPart(char32_t codePoint)
{
    return codePoint >= HANGUL_VBASE && codePoint < (HANGUL_VBASE + HANGUL_VCOUNT);
}

static bool isHangulTPart(char32_t codePoint)
{
    return codePoint >= HANGUL_TBASE && codePoint < (HANGUL_TBASE + HANGUL_TCOUNT);
}

static bool isHangulSyllable(char32_t codePoint)
{
    return codePoint >= HANGUL_SBASE && codePoint < (HANGUL_SBASE + HANGUL_SCOUNT);
}

static bool isHangulLVPart(char32_t codePoint)
{
    return isHangulSyllable(codePoint) && ((codePoint - HANGUL_SBASE) % HANGUL_TCOUNT) == 0;
}

struct UnicodeData_Composition {
    little_uint64_buf_t data;

    force_inline char32_t startCharacter() const noexcept {
        return data.value() >> 43;
    }

    force_inline char32_t composingCharacter() const noexcept {
        return (data.value() >> 22) & UNICODE_MASK;
    }

    force_inline char32_t composedCharacter() const noexcept {
        return data.value() & UNICODE_MASK;
    }

    force_inline uint64_t searchValue() const noexcept {
        return data.value() >> 22;
    }
};

struct UnicodeData_Description {
    little_uint64_buf_t data;

    force_inline char32_t codePoint() const noexcept {
        return data.value() >> 43;
    }

    force_inline uint8_t decompositionOrder() const noexcept {
        return (data.value() >> 26) & 0xff;
    }

    force_inline bool decompositionIsCanonical() const noexcept {
        return ((data.value() >> 34) & 1) > 0;
    }

    force_inline GraphemeUnitType graphemeUnitType() const noexcept {
        return static_cast<GraphemeUnitType>((data.value() >> 35) & 0xf);
    }

    force_inline uint8_t decompositionLength() const noexcept {
        return (data.value() >> 21) & 0x1f;
    }

    force_inline size_t decompositionOffset() const noexcept {
        return (data.value() & UNICODE_MASK) * sizeof(uint64_t);
    }

    force_inline char32_t decompositionCodePoint() const noexcept {
        return static_cast<char32_t>(data.value() & UNICODE_MASK);
    }

    force_inline BidirectionalClass bidirectionalClass() const noexcept {
        switch (codePoint()) {
        case 0x00'202a: return BidirectionalClass::LRE;
        case 0x00'202d: return BidirectionalClass::LRO;
        case 0x00'202b: return BidirectionalClass::RLE;
        case 0x00'202e: return BidirectionalClass::RLO;
        case 0x00'202c: return BidirectionalClass::PDF;
        case 0x00'2066: return BidirectionalClass::LRI;
        case 0x00'2067: return BidirectionalClass::RLI;
        case 0x00'2068: return BidirectionalClass::FSI;
        case 0x00'2069: return BidirectionalClass::PDI;
        default: return static_cast<BidirectionalClass>((data.value() >> 39) & 0x0f);
        }
    }
};

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
    auto first_ = std::lower_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), first, [](let &a, let &b) { return a.first < b; });
    auto last_ = std::upper_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), last, [](let &a, let &b) { return a < b.first; });
    for (auto i = first_; i != last_; ++i) {
        set_bit(i->second);
    }
}

void UnicodeRanges::add(char32_t c) noexcept
{
    auto i = std::lower_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), c, [](let &a, let &b) { return a.first < b; });
    return set_bit(i->second);
}

[[nodiscard]] bool UnicodeRanges::contains(char32_t c) const noexcept
{
    auto i = std::lower_bound(unicodeRangeToBitPosition.cbegin(), unicodeRangeToBitPosition.cend(), c, [](let &a, let &b) { return a.first < b; });
    return get_bit(i->second);
}


struct UnicodeData_Header {
    little_uint32_buf_t magic;
    little_uint32_buf_t version;
    little_uint32_buf_t nrDescriptions;
    little_uint32_buf_t nrCompositions;
    /* Canonical decomposition will include ligatures, so that the resulting
     * text will not include ligatures.
     *
     * After the header we will get the data for each of the code units.
     * These are ordered low to high to allow for binary search.
     *      UnicodeData_Description codeUnits[nrDescriptions];
     *
     * The next section are the canonical composition rules, these
     * are ordered low to high, startCharacter first followed by the composingCharacter, to
     * allow binary search. The codeUnits can point into this table for canonical two-unit decomposition.
     *      UnicodeData_Composition canonicalComposition[nrCompositions];
     *
     * The last section are for rest of the decompositions, both canonical and compatible.
     *      little_uint32_buf_t restDecompositions;
     */
};

UnicodeData::UnicodeData(gsl::span<std::byte const> bytes) :
    bytes(bytes), view()
{
    initialize();
}

UnicodeData::UnicodeData(std::unique_ptr<ResourceView> view) :
    bytes(), view(std::move(view))
{
    bytes = this->view->bytes();
    initialize();
}

void UnicodeData::initialize()
{
    size_t offset = 0;
    let header = make_placement_ptr<UnicodeData_Header>(bytes, offset);
    parse_assert(header->magic.value() == fourcc("bucd"));
    parse_assert(header->version.value() == 1);

    descriptions_offset = offset;
    descriptions_count = header->nrDescriptions.value();
    parse_assert(check_placement_array<UnicodeData_Description>(bytes, offset, descriptions_count));
    offset += sizeof(UnicodeData_Description) * descriptions_count;

    compositions_offset = offset;
    compositions_count = header->nrCompositions.value();
    parse_assert(check_placement_array<UnicodeData_Composition>(bytes, offset, compositions_count));
}

UnicodeData_Description const *UnicodeData::getDescription(char32_t codePoint) const noexcept
{
    let descriptions = unsafe_make_placement_array<UnicodeData_Description>(bytes, rvalue_cast(descriptions_offset), descriptions_count);
    let i = std::lower_bound(descriptions.begin(), descriptions.end(), codePoint, [](auto &element, auto value) {
        return element.codePoint() < value;
    });

    if (i == descriptions.end() || i->codePoint() != codePoint) {
        return nullptr;
    } else {
        return &(*i);
    }
}

GraphemeUnitType UnicodeData::getGraphemeUnitType(char32_t codePoint) const noexcept {
    if (codePoint >= 0x110000) {
        return GraphemeUnitType::Other;

    } else if (isHangulSyllable(codePoint)) {
        let SIndex = codePoint - HANGUL_SBASE;
        return (SIndex % HANGUL_TCOUNT) == 0 ? GraphemeUnitType::LV : GraphemeUnitType::LVT;

    } else if (isHangulLPart(codePoint)) {
        return GraphemeUnitType::L;

    } else if (isHangulVPart(codePoint)) {
        return GraphemeUnitType::V;

    } else if (isHangulTPart(codePoint)) {
        return GraphemeUnitType::T;

    } else {
        let description = getDescription(codePoint);
        if (description) {
            return description->graphemeUnitType();
        } else {
            return GraphemeUnitType::Other;
        }
    }
}

uint8_t UnicodeData::getDecompositionOrder(char32_t codePoint) const noexcept {
    if (codePoint <= ASCII_MAX && codePoint > UNICODE_MAX) {
        return 0;
    } else if (
        isHangulLPart(codePoint) || isHangulVPart(codePoint) || isHangulTPart(codePoint) ||
        isHangulSyllable(codePoint)
    ) {
        return 0;
    } else {
        let description = getDescription(codePoint);
        if (description) {
            return description->decompositionOrder();
        } else {
            return 0;
        }
    }
}

BidirectionalClass UnicodeData::getBidirectionalClass(char32_t codePoint) const noexcept {
    if (codePoint <= ASCII_MAX && codePoint > UNICODE_MAX) {
        return BidirectionalClass::Unknown;
    } else if (
        isHangulLPart(codePoint) || isHangulVPart(codePoint) || isHangulTPart(codePoint) ||
        isHangulSyllable(codePoint)
        ) {
        return BidirectionalClass::L;
    } else {
        let description = getDescription(codePoint);
        if (description) {
            return description->bidirectionalClass();
        } else {
            return BidirectionalClass::Unknown;
        }
    }
}

/*! Detect canonical ligature.
 * A canonical ligatures will have the same meaning in the text
 * when it is in composed or decomposed form.
 */
static bool isCanonicalLigature(char32_t codePoint)
{
    switch (codePoint) {
    case 0xfb00: // ff
    case 0xfb01: // fi
    case 0xfb02: // fl
    case 0xfb03: // ffi
    case 0xfb04: // ffl
    case 0xfb05: // long st
    case 0xfb06: // st
    case 0xfb13: // men now
    case 0xfb14: // men ech
    case 0xfb15: // men ini
    case 0xfb16: // vew now
    case 0xfb17: // men xeh
        return true;
    default:
        return false;
    }
}

void UnicodeData::decomposeCodePoint(std::u32string &result, char32_t codePoint, bool decomposeCompatible, bool decomposeLigatures) const noexcept
{
    let &description = getDescription(codePoint);
    let decompositionLength = description ? description->decompositionLength() : 0;
    let mustDecompose =
        (decompositionLength > 0) && (
            decomposeCompatible ||
            description->decompositionIsCanonical() ||
            (decomposeLigatures && isCanonicalLigature(codePoint))
            );

    if (codePoint <= ASCII_MAX || codePoint > UNICODE_MAX) {
        // ASCII characters and code-points above unicode plane-16 are not decomposed.
        result += codePoint;

    } else if (isHangulSyllable(codePoint)) {
        let SIndex = codePoint - HANGUL_SBASE;
        let LIndex = static_cast<char32_t>(SIndex / HANGUL_NCOUNT);
        let VIndex = static_cast<char32_t>((SIndex % HANGUL_NCOUNT) / HANGUL_TCOUNT);
        let TIndex = static_cast<char32_t>(SIndex % HANGUL_TCOUNT);
        result += (HANGUL_LBASE + LIndex);
        result += (HANGUL_VBASE + VIndex);
        if (TIndex > 0) {
            result += (HANGUL_TBASE + TIndex);
        }

    } else if (mustDecompose) {
        if (decompositionLength == 1) {
            let codePoint = description->decompositionCodePoint();
            decomposeCodePoint(result, codePoint, decomposeCompatible, decomposeLigatures);

        } else {
            let offset = description->decompositionOffset();
            let nrTriplets = (decompositionLength + 2) / 3;

            if (check_placement_array<little_uint64_buf_at>(bytes, offset, nrTriplets)) {
                let decomposition = unsafe_make_placement_array<little_uint64_buf_at>(bytes, rvalue_cast(offset), nrTriplets);
                for (size_t tripletIndex = 0, i = 0; tripletIndex < nrTriplets; tripletIndex++, i+=3) {
                    let triplet = decomposition[tripletIndex].value();
                    let codePoint1 = static_cast<char32_t>(triplet >> 43);
                    let codePoint2 = static_cast<char32_t>((triplet >> 22) & UNICODE_MASK);
                    let codePoint3 = static_cast<char32_t>(triplet & UNICODE_MASK);

                    decomposeCodePoint(result, codePoint1, decomposeCompatible, decomposeLigatures);
                    if (i + 1 < decompositionLength) {
                        decomposeCodePoint(result, codePoint2, decomposeCompatible, decomposeLigatures);
                    }
                    if (i + 2 < decompositionLength) {
                        decomposeCodePoint(result, codePoint3, decomposeCompatible, decomposeLigatures);
                    }
                }

            } else {
                // Error in the file-format, replace with REPLACEMENT_CHARACTER U+FFFD.
                result += UNICODE_REPLACEMENT_CHAR;
            }
        }

    } else if (description) {
        // No decomposition available, or do not want to decompose.
        result += (codePoint | (description->decompositionOrder() << 21));
    } else {
        // No description available.
        result += codePoint;
    }
}


std::u32string UnicodeData::decompose(std::u32string_view text, bool decomposeCompatible, bool decomposeLigatures) const noexcept
{
    auto result = std::u32string{};
    result.reserve(text.size() * 3);

    for (let codePoint: text) {
        decomposeCodePoint(result, codePoint, decomposeCompatible, decomposeLigatures);
    }

    return result;
}

void UnicodeData::reorder(std::u32string &text) noexcept
{
    for_each_cluster(text.begin(), text.end(),
        [](auto x) { return (x >> 21) == 0; },
        [](auto s, auto e) {
            std::stable_sort(s, e, [](auto a, auto b) {
                return (a >> 21) < (b >> 21);
                });
        }
    );
}

void UnicodeData::clean(std::u32string &text) noexcept
{
    // clean up the text by removing the upper bits.
    for (auto &codePoint: text) {
        codePoint &= 0x1f'ffff;
    }
}

char32_t UnicodeData::compose(char32_t startCodePoint, char32_t composingCodePoint, bool composeCRLF) const noexcept
{
    uint64_t searchValue =
        (static_cast<uint64_t>(startCodePoint) << 21) |
        static_cast<uint64_t>(composingCodePoint);

    if (composeCRLF && startCodePoint == UNICODE_CR_CHAR && composingCodePoint == UNICODE_LF_CHAR) {
        return UNICODE_LF_CHAR;

    } else if (isHangulLPart(startCodePoint) && isHangulVPart(composingCodePoint)) {
        let LIndex = startCodePoint - HANGUL_LBASE;
        let VIndex = composingCodePoint - HANGUL_VBASE;
        let LVIndex = LIndex * HANGUL_NCOUNT + VIndex * HANGUL_TCOUNT;
        return HANGUL_SBASE + LVIndex;

    } else if (isHangulLVPart(startCodePoint) && isHangulTPart(composingCodePoint)) {
        let TIndex = composingCodePoint - HANGUL_TBASE;
        return startCodePoint + TIndex;

    } else {
        let compositions = unsafe_make_placement_array<UnicodeData_Composition>(
            bytes, rvalue_cast(compositions_offset), compositions_count
            );

        let i = std::lower_bound(compositions.begin(), compositions.end(), searchValue, [](auto &element, auto value) {
            return element.searchValue() < value;
            });

        if (i != compositions.end() && i->searchValue() == searchValue) {
            return i->composedCharacter();
        } else {
            // No composition found, signal caller.
            return UNICODE_INVALID_CHAR;
        }
    }
}
void UnicodeData::compose(std::u32string &text, bool composeCRLF) const noexcept
{
    if (text.size() <= 1) {
        return;
    }

    size_t i = 0;
    size_t j = 0;
    while (i < text.size()) {
        let codeUnit = text[i++];
        let codePoint = codeUnit & 0x1f'ffff;
        let compositionOrder = codeUnit >> 21;
        let isStartCharacter = compositionOrder == 0;

        if (codePoint == UNICODE_INVALID_CHAR) {
            // code-unit was sniffed out by compositing, skip it.
        } else if (codePoint > UNICODE_MAX) {
            // Characters above plane-16 of unicode.
            text[j++] = codePoint;

        } else if (isStartCharacter) {
            // Try composing.
            auto startCodePoint = codePoint;
            char32_t prevDecompositionOrder = 0;
            for (size_t k = i; k < text.size(); k++) {
                let composingCodeUnit = text[k];
                let composingCodePoint = composingCodeUnit & 0x1f'ffff;
                let composingDecompositionOrder = composingCodeUnit >> 21;

                bool blockingPair =
                    prevDecompositionOrder != 0 &&
                    prevDecompositionOrder >= composingDecompositionOrder;

                bool composingIsStarter = composingDecompositionOrder == 0;

                let composedCodePoint = compose(startCodePoint, composingCodePoint, composeCRLF);
                if (composedCodePoint != UNICODE_INVALID_CHAR && !blockingPair) {
                    // Found a composition.
                    startCodePoint = composedCodePoint;
                    // The canonical combined DecompositionOrder is always zero.
                    prevDecompositionOrder = 0;
                    // Snuff out the code-unit.
                    text[k] = UNICODE_INVALID_CHAR; 

                } else if (composingIsStarter) {
                    // End after failing to compose with the next start-character.
                    break;

                } else {
                    // The start character is not composing with this composingC.
                    prevDecompositionOrder = composingDecompositionOrder;
                }
            }
            // Add the new combined character to the text.
            text[j++] = startCodePoint;

        } else {
            // Unable to compose this character.
            text[j++] = codePoint;
        }
    }

    text.resize(j);
}

std::u32string UnicodeData::toNFD(std::u32string_view text, bool decomposeLigatures) const noexcept
{
    auto result = decompose(text, false, decomposeLigatures);
    reorder(result);
    clean(result);
    return result;
}

std::u32string UnicodeData::toNFC(std::u32string_view text, bool decomposeLigatures, bool composeCRLF) const noexcept
{
    auto result = decompose(text, false, decomposeLigatures);
    reorder(result);
    compose(result, composeCRLF);
    clean(result);
    return result;
}

std::u32string UnicodeData::toNFKD(std::u32string_view text) const noexcept
{
    auto result = decompose(text, true);
    reorder(result);
    clean(result);
    return result;
}

std::u32string UnicodeData::toNFKC(std::u32string_view text, bool composeCRLF) const noexcept
{
    auto result = decompose(text, true);
    reorder(result);
    compose(result, composeCRLF);
    clean(result);
    return result;
}

static bool checkGraphemeBreak_unitType(GraphemeUnitType type, GraphemeBreakState &state) noexcept
{
    let lhs = state.previous;
    let rhs = type;

    enum class state_t {
        Unknown,
        Break,
        DontBreak,
    };

    state_t breakState = state_t::Unknown;

    bool  GB1 = state.firstCharacter;
    if ((breakState == state_t::Unknown) & GB1) {
        breakState = state_t::Break;
    }

    state.firstCharacter = false;

    let GB3 = (lhs == GraphemeUnitType::CR) && (rhs == GraphemeUnitType::LF);
    let GB4 = (lhs == GraphemeUnitType::Control) || (lhs == GraphemeUnitType::CR) || (lhs == GraphemeUnitType::LF);
    let GB5 = (rhs == GraphemeUnitType::Control) || (rhs == GraphemeUnitType::CR) || (rhs == GraphemeUnitType::LF);
    if (breakState == state_t::Unknown) {
        if (GB3) {
            breakState = state_t::DontBreak;
        } else if (GB4 || GB5) {
            breakState = state_t::Break;
        }
    }

    let GB6 =
        (lhs == GraphemeUnitType::L) &&
        ((rhs == GraphemeUnitType::L) || (rhs == GraphemeUnitType::V) || (rhs == GraphemeUnitType::LV) | (rhs == GraphemeUnitType::LVT));
    let GB7 =
        ((lhs == GraphemeUnitType::LV) || (lhs == GraphemeUnitType::V)) &&
        ((rhs == GraphemeUnitType::V) || (rhs == GraphemeUnitType::T));
    let GB8 =
        ((lhs == GraphemeUnitType::LVT) || (lhs == GraphemeUnitType::T)) &&
        (rhs == GraphemeUnitType::T);
    if ((breakState == state_t::Unknown) && (GB6 || GB7 || GB8)) {
        breakState = state_t::DontBreak;
    }

    let GB9 = ((rhs == GraphemeUnitType::Extend) || (rhs == GraphemeUnitType::ZWJ));
    let GB9a = (rhs == GraphemeUnitType::SpacingMark);
    let GB9b = (lhs == GraphemeUnitType::Prepend);
    if ((breakState == state_t::Unknown) & (GB9 || GB9a || GB9b)) {
        breakState = state_t::DontBreak;
    }

    let GB11 = state.inExtendedPictographic && (lhs == GraphemeUnitType::ZWJ) && (rhs == GraphemeUnitType::Extended_Pictographic);
    if ((breakState == state_t::Unknown) && GB11) {
        breakState = state_t::DontBreak;
    }

    if (rhs == GraphemeUnitType::Extended_Pictographic) {
        state.inExtendedPictographic = true;
    } else if (!((rhs == GraphemeUnitType::Extend) || (rhs == GraphemeUnitType::ZWJ))) {
        state.inExtendedPictographic = false;
    }

    let GB12_13 = (lhs == GraphemeUnitType::Regional_Indicator) && (rhs == GraphemeUnitType::Regional_Indicator) && ((state.RICount % 2) == 1);
    if ((breakState == state_t::Unknown) && (GB12_13)) {
        breakState = state_t::DontBreak;
    }

    if (rhs == GraphemeUnitType::Regional_Indicator) {
        state.RICount++;
    } else {
        state.RICount = 0;
    }

    // GB999
    if (breakState == state_t::Unknown) {
        breakState = state_t::Break;
    }

    state.previous = type;
    return breakState == state_t::Break;
}


bool UnicodeData::checkGraphemeBreak(char32_t codePoint, GraphemeBreakState &state) const noexcept
{
    return checkGraphemeBreak_unitType(getGraphemeUnitType(codePoint), state);
}

template<>
std::unique_ptr<UnicodeData> parseResource(URL const &location)
{
    if (location.extension() == "bin") {
        auto view = ResourceView::loadView(location);

        try {
            auto unicodeData = std::make_unique<UnicodeData>(std::move(view));
            return unicodeData;
        } catch (error &e) {
            e.set<"url"_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<"url"_tag>(location)
        );
    }
}
}
