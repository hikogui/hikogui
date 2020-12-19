// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "unicode_data.hpp"
#include "font_book.hpp"
#include "../placement.hpp"
#include "../endian.hpp"
#include "../algorithm.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include "../required.hpp"
#include "../check.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <mutex>

namespace tt {

constexpr char32_t ASCII_MAX = 0x7f;
constexpr char32_t UNICODE_MASK = 0x1f'ffff;
constexpr char32_t UNICODE_MAX = 0x10'ffff;
constexpr char32_t UNICODE_REPLACEMENT_CHAR = 0x00'fffd;
constexpr char32_t UNICODE_INVALID_CHAR = 0x00'ffff;
constexpr char32_t UNICODE_CR_CHAR = 0x00'000a;
constexpr char32_t UNICODE_LF_CHAR = 0x00'000d;
constexpr char32_t UNICODE_PARAGRAPH_SEPARATOR_CHAR = 0x00'2029;

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

    char32_t startCharacter() const noexcept
    {
        return data.value() >> 43;
    }

    char32_t composingCharacter() const noexcept
    {
        return (data.value() >> 22) & UNICODE_MASK;
    }

    char32_t composedCharacter() const noexcept
    {
        return data.value() & UNICODE_MASK;
    }

    uint64_t searchValue() const noexcept
    {
        return data.value() >> 22;
    }
};

struct UnicodeData_Description {
    little_uint64_buf_t data;

    char32_t codePoint() const noexcept
    {
        return data.value() >> 43;
    }

    uint8_t decompositionOrder() const noexcept
    {
        return (data.value() >> 26) & 0xff;
    }

    bool decompositionIsCanonical() const noexcept
    {
        return ((data.value() >> 34) & 1) > 0;
    }

    uint8_t decompositionLength() const noexcept
    {
        return (data.value() >> 21) & 0x1f;
    }

    size_t decompositionOffset() const noexcept
    {
        return (data.value() & UNICODE_MASK) * sizeof(uint64_t);
    }

    char32_t decompositionCodePoint() const noexcept
    {
        return static_cast<char32_t>(data.value() & UNICODE_MASK);
    }

    unicode_bidi_class bidi_class() const noexcept
    {
        switch (codePoint()) {
        case 0x00'202a: return unicode_bidi_class::LRE;
        case 0x00'202d: return unicode_bidi_class::LRO;
        case 0x00'202b: return unicode_bidi_class::RLE;
        case 0x00'202e: return unicode_bidi_class::RLO;
        case 0x00'202c: return unicode_bidi_class::PDF;
        case 0x00'2066: return unicode_bidi_class::LRI;
        case 0x00'2067: return unicode_bidi_class::RLI;
        case 0x00'2068: return unicode_bidi_class::FSI;
        case 0x00'2069: return unicode_bidi_class::PDI;
        default: return static_cast<unicode_bidi_class>((data.value() >> 39) & 0x0f);
        }
    }
};

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

unicode_data::unicode_data(std::span<std::byte const> bytes) : bytes(bytes), view()
{
    init();
}

unicode_data::unicode_data(std::unique_ptr<ResourceView> view) : bytes(), view(std::move(view))
{
    bytes = this->view->bytes();
    init();
}

unicode_data::unicode_data(URL const &url) : unicode_data(url.loadView()) {}

void unicode_data::init()
{
    ssize_t offset = 0;
    ttlet header = make_placement_ptr<UnicodeData_Header>(bytes, offset);
    tt_parse_check(header->magic.value() == fourcc("bucd"), "Binary unicode file must begin with magic 'bucd'");
    tt_parse_check(header->version.value() == 1, "Binary unicode file version must be 1");

    descriptions_offset = offset;
    descriptions_count = header->nrDescriptions.value();
    tt_parse_check(
        check_placement_array<UnicodeData_Description>(bytes, offset, descriptions_count),
        "Unicode description table is beyond buffer");
    offset += sizeof(UnicodeData_Description) * descriptions_count;

    compositions_offset = offset;
    compositions_count = header->nrCompositions.value();
    tt_parse_check(
        check_placement_array<UnicodeData_Composition>(bytes, offset, compositions_count),
        "Unicode composition table is beyond buffer");
}

UnicodeData_Description const *unicode_data::getDescription(char32_t codePoint) const noexcept
{
    ttlet descriptions =
        unsafe_make_placement_array<UnicodeData_Description>(bytes, copy(descriptions_offset), descriptions_count);
    ttlet i = std::lower_bound(descriptions.begin(), descriptions.end(), codePoint, [](auto &element, auto value) {
        return element.codePoint() < value;
    });

    if (i == descriptions.end() || i->codePoint() != codePoint) {
        return nullptr;
    } else {
        return &(*i);
    }
}

uint8_t unicode_data::getDecompositionOrder(char32_t codePoint) const noexcept
{
    if (codePoint <= ASCII_MAX && codePoint > UNICODE_MAX) {
        return 0;
    } else if (isHangulLPart(codePoint) || isHangulVPart(codePoint) || isHangulTPart(codePoint) || isHangulSyllable(codePoint)) {
        return 0;
    } else {
        ttlet description = getDescription(codePoint);
        if (description) {
            return description->decompositionOrder();
        } else {
            return 0;
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
    default: return false;
    }
}

void unicode_data::decomposeCodePoint(
    std::u32string &result,
    char32_t codePoint,
    bool decomposeCompatible,
    bool decomposeLigatures,
    bool decomposeLF) const noexcept
{
    ttlet &description = getDescription(codePoint);
    ttlet decompositionLength = description ? description->decompositionLength() : 0;
    ttlet mustDecompose = (decompositionLength > 0) &&
        (decomposeCompatible || description->decompositionIsCanonical() ||
         (decomposeLigatures && isCanonicalLigature(codePoint)));

    if (decomposeLF && codePoint == UNICODE_LF_CHAR) {
        result += UNICODE_PARAGRAPH_SEPARATOR_CHAR;

    } else if (codePoint <= ASCII_MAX || codePoint > UNICODE_MAX) {
        // ASCII characters and code-points above unicode plane-16 are not decomposed.
        result += codePoint;

    } else if (isHangulSyllable(codePoint)) {
        ttlet SIndex = codePoint - HANGUL_SBASE;
        ttlet LIndex = static_cast<char32_t>(SIndex / HANGUL_NCOUNT);
        ttlet VIndex = static_cast<char32_t>((SIndex % HANGUL_NCOUNT) / HANGUL_TCOUNT);
        ttlet TIndex = static_cast<char32_t>(SIndex % HANGUL_TCOUNT);
        result += (HANGUL_LBASE + LIndex);
        result += (HANGUL_VBASE + VIndex);
        if (TIndex > 0) {
            result += (HANGUL_TBASE + TIndex);
        }

    } else if (mustDecompose) {
        if (decompositionLength == 1) {
            decomposeCodePoint(
                result, description->decompositionCodePoint(), decomposeCompatible, decomposeLigatures, decomposeLF);

        } else {
            ttlet offset = description->decompositionOffset();
            ttlet nrTriplets = (decompositionLength + 2) / 3;

            if (check_placement_array<little_uint64_buf_at>(bytes, offset, nrTriplets)) {
                ttlet decomposition = unsafe_make_placement_array<little_uint64_buf_at>(bytes, copy(offset), nrTriplets);
                for (size_t tripletIndex = 0, i = 0; tripletIndex < nrTriplets; tripletIndex++, i += 3) {
                    ttlet triplet = decomposition[tripletIndex].value();
                    ttlet codePoint1 = static_cast<char32_t>(triplet >> 43);
                    ttlet codePoint2 = static_cast<char32_t>((triplet >> 22) & UNICODE_MASK);
                    ttlet codePoint3 = static_cast<char32_t>(triplet & UNICODE_MASK);

                    decomposeCodePoint(result, codePoint1, decomposeCompatible, decomposeLigatures, decomposeLF);
                    if (i + 1 < decompositionLength) {
                        decomposeCodePoint(result, codePoint2, decomposeCompatible, decomposeLigatures, decomposeLF);
                    }
                    if (i + 2 < decompositionLength) {
                        decomposeCodePoint(result, codePoint3, decomposeCompatible, decomposeLigatures, decomposeLF);
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

std::u32string unicode_data::decompose(std::u32string_view text, bool decomposeCompatible, bool decomposeLigatures, bool decomposeLF) const noexcept
{
    auto result = std::u32string{};
    result.reserve(text.size() * 3);

    for (ttlet codePoint : text) {
        decomposeCodePoint(result, codePoint, decomposeCompatible, decomposeLigatures, decomposeLF);
    }

    return result;
}

void unicode_data::reorder(std::u32string &text) noexcept
{
    for_each_cluster(
        text.begin(),
        text.end(),
        [](auto x) {
            return (x >> 21) == 0;
        },
        [](auto s, auto e) {
            std::stable_sort(s, e, [](auto a, auto b) {
                return (a >> 21) < (b >> 21);
            });
        });
}

void unicode_data::clean(std::u32string &text) noexcept
{
    // clean up the text by removing the upper bits.
    for (auto &codePoint : text) {
        codePoint &= 0x1f'ffff;
    }
}

char32_t unicode_data::compose(char32_t startCodePoint, char32_t composingCodePoint, bool composeCRLF) const noexcept
{
    uint64_t searchValue = (static_cast<uint64_t>(startCodePoint) << 21) | static_cast<uint64_t>(composingCodePoint);

    if (composeCRLF && startCodePoint == UNICODE_CR_CHAR && composingCodePoint == UNICODE_LF_CHAR) {
        return UNICODE_LF_CHAR;

    } else if (composeCRLF && startCodePoint == UNICODE_PARAGRAPH_SEPARATOR_CHAR && composingCodePoint == UNICODE_LF_CHAR) {
        return UNICODE_LF_CHAR;

    } else if (isHangulLPart(startCodePoint) && isHangulVPart(composingCodePoint)) {
        ttlet LIndex = startCodePoint - HANGUL_LBASE;
        ttlet VIndex = composingCodePoint - HANGUL_VBASE;
        ttlet LVIndex = LIndex * HANGUL_NCOUNT + VIndex * HANGUL_TCOUNT;
        return HANGUL_SBASE + LVIndex;

    } else if (isHangulLVPart(startCodePoint) && isHangulTPart(composingCodePoint)) {
        ttlet TIndex = composingCodePoint - HANGUL_TBASE;
        return startCodePoint + TIndex;

    } else {
        ttlet compositions =
            unsafe_make_placement_array<UnicodeData_Composition>(bytes, copy(compositions_offset), compositions_count);

        ttlet i = std::lower_bound(compositions.begin(), compositions.end(), searchValue, [](auto &element, auto value) {
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
void unicode_data::compose(std::u32string &text, bool composeCRLF) const noexcept
{
    if (text.size() <= 1) {
        return;
    }

    size_t i = 0;
    size_t j = 0;
    while (i < text.size()) {
        ttlet codeUnit = text[i++];
        ttlet codePoint = codeUnit & 0x1f'ffff;
        ttlet compositionOrder = codeUnit >> 21;
        ttlet isStartCharacter = compositionOrder == 0;

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
                ttlet composingCodeUnit = text[k];
                ttlet composingCodePoint = composingCodeUnit & 0x1f'ffff;
                ttlet composingDecompositionOrder = composingCodeUnit >> 21;

                bool blockingPair = prevDecompositionOrder != 0 && prevDecompositionOrder >= composingDecompositionOrder;

                bool composingIsStarter = composingDecompositionOrder == 0;

                ttlet composedCodePoint = compose(startCodePoint, composingCodePoint, composeCRLF);
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

std::u32string unicode_data::toNFD(std::u32string_view text, bool decomposeLigatures, bool decomposeLF) const noexcept
{
    auto result = decompose(text, false, decomposeLigatures, decomposeLF);
    reorder(result);
    clean(result);
    return result;
}

std::u32string unicode_data::toNFC(std::u32string_view text, bool decomposeLigatures, bool decomposeLF, bool composeCRLF) const noexcept
{
    auto result = decompose(text, false, decomposeLigatures, decomposeLF);
    reorder(result);
    compose(result, composeCRLF);
    clean(result);
    return result;
}

std::u32string unicode_data::toNFKD(std::u32string_view text) const noexcept
{
    auto result = decompose(text, true);
    reorder(result);
    clean(result);
    return result;
}

std::u32string unicode_data::toNFKC(std::u32string_view text, bool composeCRLF) const noexcept
{
    auto result = decompose(text, true);
    reorder(result);
    compose(result, composeCRLF);
    clean(result);
    return result;
}

} // namespace tt
