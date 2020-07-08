// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include "UnicodeBidi.hpp"
#include "../ResourceView.hpp"
#include "../math.hpp"
#include "../URL.hpp"
#include "../required.hpp"
#include <nonstd/span>

namespace tt {
struct UnicodeData_Description;

enum class GraphemeUnitType : uint8_t {
    Other = 0,
    CR = 1,
    LF = 2,
    Control = 3,
    Extend = 4,
    ZWJ = 5,
    Regional_Indicator = 6,
    Prepend = 7,
    SpacingMark = 8,
    L = 9,
    V = 10,
    T = 11,
    LV = 12,
    LVT = 13,
    Extended_Pictographic = 14
};



struct GraphemeBreakState {
    GraphemeUnitType previous = GraphemeUnitType::Other;
    int RICount = 0;
    bool firstCharacter = true;
    bool inExtendedPictographic = false;

    void reset() noexcept {
        previous = GraphemeUnitType::Other;
        RICount = 0;
        firstCharacter = true;
        inExtendedPictographic = false;
    }
};


/** Bidirectional class
 * Unicode Standard Annex #9: https://unicode.org/reports/tr9/
 */
enum class BidiClass : uint8_t {
    Unknown = 0,
    L = 1, ///< Left-to-Right
    R = 2, ///< Right-to-Left
    AL = 3, ///< Right-to-Left Arabic
    EN = 4, ///< European Number
    ES = 5, ///< European Number Separator
    ET = 6, ///< European Number Terminator
    AN = 7, ///< Arabic Number
    CS = 8, ///< Common Number Separator
    NSM = 9, ///< Nonspacing Mark
    BN = 10, ///< Boundary Neutral
    B = 11, ///< Paragraph Separator
    S = 12, ///< Segment Separator
    WS = 13, ///< Whitespace
    ON = 14, ///< Other Neutrals
    // Explicit values.
    LRE, ///< Left-to-Right Embedding
    LRO, ///< Left-to-Right Override
    RLE, ///< Right-to-Left Embedding
    RLO, ///< Right-to-left Override
    PDF, ///< Pop Directional Format
    LRI, ///< Left-to-Right Isolate
    RLI, ///< Right-to-Left Isolate
    FSI, ///< First Strong Isolate
    PDI ///< Pop Directional Isolate
};

/** General Character class.
*/
enum GeneralCharacterClass {
    Unknown,
    Digit,
    Letter,
    WhiteSpace,
    ParagraphSeparator
};

/** This function should be called before reclassification by the bidi-algorithm.
*/
[[nodiscard]] constexpr GeneralCharacterClass to_GeneralCharacterClass(BidiClass bidiClass) noexcept {
    switch (bidiClass) {
    case BidiClass::Unknown: return GeneralCharacterClass::Unknown;
    case BidiClass::L: return GeneralCharacterClass::Letter;
    case BidiClass::R: return GeneralCharacterClass::Letter;
    case BidiClass::AL: return GeneralCharacterClass::Letter;
    case BidiClass::EN: return GeneralCharacterClass::Digit;
    case BidiClass::ES: return GeneralCharacterClass::Unknown;
    case BidiClass::ET: return GeneralCharacterClass::Unknown;
    case BidiClass::AN: return GeneralCharacterClass::Digit;
    case BidiClass::CS: return GeneralCharacterClass::Unknown;
    case BidiClass::NSM: return GeneralCharacterClass::Unknown;
    case BidiClass::BN: return GeneralCharacterClass::Unknown;
    case BidiClass::B: return GeneralCharacterClass::ParagraphSeparator;
    case BidiClass::S: return GeneralCharacterClass::Unknown;
    case BidiClass::WS: return GeneralCharacterClass::WhiteSpace;
    case BidiClass::ON: return GeneralCharacterClass::Unknown;
    case BidiClass::LRE: return GeneralCharacterClass::Unknown;
    case BidiClass::LRO: return GeneralCharacterClass::Unknown;
    case BidiClass::RLE: return GeneralCharacterClass::Unknown;
    case BidiClass::RLO: return GeneralCharacterClass::Unknown;
    case BidiClass::PDF: return GeneralCharacterClass::Unknown;
    case BidiClass::LRI: return GeneralCharacterClass::Unknown;
    case BidiClass::RLI: return GeneralCharacterClass::Unknown;
    case BidiClass::FSI: return GeneralCharacterClass::Unknown;
    case BidiClass::PDI: return GeneralCharacterClass::Unknown;
    default: tt_no_default;
    }
}

/** Unicode Data used for characterizing unicode code-points.
 */
class UnicodeData {
private:
    nonstd::span<std::byte const> bytes;

    /** A view to the binary UnicodeData.
     */
    std::unique_ptr<ResourceView> view;

    size_t descriptions_offset;
    size_t descriptions_count;

    size_t compositions_offset;
    size_t compositions_count;
public:
    /** Load binary unicode data.
     * The bytes passed into this constructor will need to remain available.
     */
    UnicodeData(nonstd::span<std::byte const> bytes);

    /** Load binary unicode data from a resource.
     */
    UnicodeData(std::unique_ptr<ResourceView> view);

    UnicodeData() = delete;
    UnicodeData(UnicodeData const &other) = delete;
    UnicodeData &operator=(UnicodeData const &other) = delete;
    UnicodeData(UnicodeData &&other) = delete;
    UnicodeData &operator=(UnicodeData &&other) = delete;
    ~UnicodeData() = default;

    /** Convert text to Unicode-NFD normal form.
     * Certain ligatures, which are seen as separate graphemes by the user
     * may be decomposed when using the decomposeLigatures flag.
     *
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     * \param decomposeLigatures 'canonical'-ligatures are decomposed.
     */
    std::u32string toNFD(std::u32string_view text, bool decomposeLigatures=false) const noexcept;

    /** Convert text to Unicode-NFC normal form.
     * Certain ligatures, which are seen as separate graphemes by the user
     * may be decomposed when using the decomposeLigatures flag.
     *
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     * \param decomposeLigatures 'canonical'-ligatures are decomposed.
     * \param composeCRLF Compose CR-LF combinations to LF.
     */
    std::u32string toNFC(std::u32string_view text, bool decomposeLigatures=false, bool composeCRLF=false) const noexcept;

    /** Convert text to Unicode-NFKD normal form.
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     */
    std::u32string toNFKD(std::u32string_view text) const noexcept;

    /** Convert text to Unicode-NFKC normal form.
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     * \param composeCRLF Compose CR-LF combinations to LF.
     */
    std::u32string toNFKC(std::u32string_view text, bool composeCRLF=false) const noexcept;

    /** Check if for a graphemeBreak before the character.
     * Code-units must be tested in order, starting at the beginning of the text.
     *
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will be treated as GraphemeUnitType::Other. 
     *
     * \param codeUnit Current code-unit to test.
     * \param state Current state of the grapheme-break algorithm.
     * \return true when a grapheme break exists before the current code-unit.
     */
    bool checkGraphemeBreak(char32_t codeUnit, GraphemeBreakState &state) const noexcept;

    /** Get the bidirectional class for a code-point.
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will be treated as BidiClass::Unknown. 
     */
    BidiClass getBidiClass(char32_t codePoint) const noexcept;

private:
    void initialize();

    UnicodeData_Description const *getDescription(char32_t codePoint) const noexcept;
    GraphemeUnitType getGraphemeUnitType(char32_t codePoint) const noexcept;
    uint8_t getDecompositionOrder(char32_t codePoint) const noexcept;

    char32_t compose(char32_t startCharacter, char32_t composingCharacter, bool composeCRLF) const noexcept;
    void decomposeCodePoint(std::u32string &result, char32_t codePoint, bool decomposeCompatible, bool decomposeLigatures) const noexcept;
    std::u32string decompose(std::u32string_view text, bool decomposeCompatible, bool decomposeLigatures=false) const noexcept;


    /** Reorder text after decomposition.
     * decompose() must be called before this function. The decompose() function
     * will add the decompositionOrder in bits 28:21 of each code-unit.
     */
    static void reorder(std::u32string &text) noexcept;

    /** Clean the code-unit.
     * This function should be called after reorder() or after compose() to remove
     * temporary information from the code-units.
     */
    static void clean(std::u32string &text) noexcept;


    /** Compose the characters in the text.
     * Code-units outside of the unicode-planes will be passed through.
     *
     * Code-unit 0x00'ffff (not-a-character, invalid inside a unicode stream) is
     * used by the composition algorithm. Any 0x00'ffff in the text will be
     * removed by this algorithm.
     *
     * \param text to compose, in-place.
     * \param composeCRLF Compose CR-LF combinations to LF.
     */
    void compose(std::u32string &text, bool composeCRLF=false) const noexcept;
};

}

namespace tt {

template<>
std::unique_ptr<tt::UnicodeData> parseResource(URL const &location);

}

