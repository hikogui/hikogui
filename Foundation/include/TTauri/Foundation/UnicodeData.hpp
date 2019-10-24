// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gsl/gsl>

namespace TTauri {
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

class UnicodeData {
private:
    gsl::span<std::byte const> bytes;
    std::unique_ptr<ResourceView> view;

    size_t descriptions_offset;
    size_t descriptions_count;

    size_t compositions_offset;
    size_t compositions_count;
public:
    /*! Load a true type font.
    * The methods in this class will parse the true-type font at run time.
    * This also means that the bytes passed into this constructor will need to
    * remain available.
    */
    UnicodeData(gsl::span<std::byte const> bytes);
    UnicodeData(std::unique_ptr<ResourceView> view);
    UnicodeData() = delete;
    UnicodeData(UnicodeData const &other) = delete;
    UnicodeData &operator=(UnicodeData const &other) = delete;
    UnicodeData(UnicodeData &&other) = delete;
    UnicodeData &operator=(UnicodeData &&other) = delete;
    ~UnicodeData() = default;

    /*! Convert text to Unicode-NFD normal form.
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

    /*! Convert text to Unicode-NFC normal form.
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

    /*! Convert text to Unicode-NFKD normal form.
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     */
    std::u32string toNFKD(std::u32string_view text) const noexcept;

    /*! Convert text to Unicode-NFKC normal form.
     * Do not pass code-units above 0x1f'ffff nor the code-unit 0x00'ffff.
     * Code units between 0x11'0000 and 0x1f'ffff will pass through. 
     *
     * \param text to normalize, in-place.
     * \param composeCRLF Compose CR-LF combinations to LF.
     */
    std::u32string toNFKC(std::u32string_view text, bool composeCRLF=false) const noexcept;

    /*! Check if for a graphemeBreak before the character.
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

    /*! Get the bidirectional class for a code-point.
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


    /*! Reorder text after decomposition.
     * decompose() must be called before this function. The decompose() function
     * will add the decompositionOrder in bits 28:21 of each code-unit.
     */
    static void reorder(std::u32string &text) noexcept;

    /*! Clean the code-unit.
    * This function should be called after reorder() or after compose() to remove
    * temporary information from the code-units.
    */
    static void clean(std::u32string &text) noexcept;


    /*! Compose the characters in the text.
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

template<>
std::unique_ptr<UnicodeData> parseResource(URL const &location);

}