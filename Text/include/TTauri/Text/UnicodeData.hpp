// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/math.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gsl/gsl>

namespace TTauri::Text {
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

enum class BidirectionalClass : uint8_t {
    Unknown = 0,
    L = 1,
    R = 2,
    AL = 3,
    EN = 4,
    ES = 5,
    ET = 6,
    AN = 7,
    CS = 8,
    NSM = 9,
    BN = 10,
    B = 11,
    S = 12,
    WS = 13,
    ON = 14,
    // Explicit values.
    LRE,
    LRO,
    RLE,
    RLO,
    PDF,
    LRI,
    RLI,
    FSI,
    PDI
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

/** Unicode Ranges based on the OS/2 table in TrueType fonts.
 */
struct UnicodeRanges {
    uint32_t value[4];

    UnicodeRanges() noexcept {
        value[0] = 0;
        value[1] = 0;
        value[2] = 0;
        value[3] = 0;
    }

    UnicodeRanges(char32_t c) noexcept : UnicodeRanges() {
        add(c);
    }

    UnicodeRanges(Grapheme g) noexcept : UnicodeRanges() {
        for (ssize_t i = 0; i != ssize(g); ++i) {
            add(g[i]);
        }
    }

    operator bool () const noexcept {
        return (value[0] != 0) || (value[1] != 0) || (value[2] != 0) || (value[3] != 0);
    }

    /** Add code point to unicode-ranges.
    */
    void add(char32_t c) noexcept;

    /** Add code points to unicode-ranges.
     * @param first First code point.
     * @param last One beyond the last code point.
     */
    void add(char32_t first, char32_t last) noexcept;

    /** Check if the code point is present in the unicode-ranges.
     */
    [[nodiscard]] bool contains(char32_t c) const noexcept;

    [[nodiscard]] bool contains(Grapheme g) const noexcept {
        for (ssize_t i = 0; i != ssize(g); ++i) {
            if (!contains(g[i])) {
                return false;
            }
        }
        return true;
    }

    void set_bit(int i) noexcept {
        ttauri_assume(i > 0 && i < 128);
        value[i / 32] |= static_cast<uint32_t>(1) << (i % 32);
    }

    bool get_bit(int i) const noexcept {
        ttauri_assume(i > 0 && i < 128);
        return (value[i / 32] & static_cast<uint32_t>(1) << (i % 32)) != 0;
    }

    int popcount() const noexcept {
        int r = 0;
        for (int i = 0; i != 4; ++i) {
            r += ::TTauri::popcount(value[i]);
        }
        return r;
    }


    UnicodeRanges &operator|=(UnicodeRanges const &rhs) noexcept {
        for (int i = 0; i != 4; ++i) {
            value[i] |= rhs.value[i];
        }
        return *this;
    }

    [[nodiscard]] friend std::string to_string(UnicodeRanges const &rhs) noexcept {
        return fmt::format("{:08x}:{:08x}:{:08x}:{:08x}", rhs.value[3], rhs.value[2], rhs.value[1], rhs.value[0]);
    }

    /** The lhs has at least all bits on the rhs set.
     */
    [[nodiscard]] friend bool operator>=(UnicodeRanges const &lhs, UnicodeRanges const &rhs) noexcept {
        for (int i = 0; i < 4; i++) {
            if (!((lhs.value[i] & rhs.value[i]) == rhs.value[i])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] friend UnicodeRanges operator|(UnicodeRanges const &lhs, UnicodeRanges const &rhs) noexcept {
        auto r = lhs;
        r |= rhs;
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, UnicodeRanges const &rhs) {
        return lhs << to_string(rhs);
    }
};

/** Unicode Data used for characterizing unicode code-points.
 */
class UnicodeData {
private:
    gsl::span<std::byte const> bytes;

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
    UnicodeData(gsl::span<std::byte const> bytes);

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
     * Code units between 0x11'0000 and 0x1f'ffff will be treated as BidirectionalClass::Unknown. 
     */
    BidirectionalClass getBidirectionalClass(char32_t codePoint) const noexcept;

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

namespace TTauri {

template<>
std::unique_ptr<TTauri::Text::UnicodeData> parseResource(URL const &location);

}

