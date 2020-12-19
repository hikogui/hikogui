// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include "unicode_bidi_class.hpp"
#include "../ResourceView.hpp"
#include "../math.hpp"
#include "../URL.hpp"
#include "../required.hpp"
#include <span>

namespace tt {
struct UnicodeData_Description;

/** Unicode Data used for characterizing unicode code-points.
 */
class unicode_data {
public:
    static inline std::unique_ptr<unicode_data> global;

    /** Load binary unicode data.
     * The bytes passed into this constructor will need to remain available.
     */
    unicode_data(std::span<std::byte const> bytes);

    /** Load binary unicode data from a resource.
     */
    unicode_data(std::unique_ptr<ResourceView> view);

    unicode_data(URL const &url);

    unicode_data() = delete;
    unicode_data(unicode_data const &other) = delete;
    unicode_data &operator=(unicode_data const &other) = delete;
    unicode_data(unicode_data &&other) = delete;
    unicode_data &operator=(unicode_data &&other) = delete;
    ~unicode_data() = default;

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
    std::u32string toNFD(std::u32string_view text, bool decomposeLigatures=false, bool decomposeLF=false) const noexcept;

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
    std::u32string toNFC(std::u32string_view text, bool decomposeLigatures=false, bool decomposeLF=false, bool composeCRLF=false) const noexcept;

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

private:
    std::span<std::byte const> bytes;

    /** A view to the binary unicode_data::global.
     */
    std::unique_ptr<ResourceView> view;

    size_t descriptions_offset;
    size_t descriptions_count;

    size_t compositions_offset;
    size_t compositions_count;

    void init();

    UnicodeData_Description const *getDescription(char32_t codePoint) const noexcept;
    uint8_t getDecompositionOrder(char32_t codePoint) const noexcept;

    char32_t compose(char32_t startCharacter, char32_t composingCharacter, bool composeCRLF) const noexcept;
    void decomposeCodePoint(
        std::u32string &result,
        char32_t codePoint,
        bool decomposeCompatible,
        bool decomposeLigatures,
        bool decomposeLF) const noexcept;
    std::u32string decompose(std::u32string_view text, bool decomposeCompatible, bool decomposeLigatures=false, bool decomposeLF=false) const noexcept;


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
