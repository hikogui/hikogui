
#include "TTauri/Foundation/BinaryUnicodeData.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include <algorithm>

namespace TTauri {

constexpr uint64_t CRLF_SEARCH_VALUE = (0x00'000a << 21) | 0x00'000d;

constexpr uint32_t CODE_UNIT_CANONICAL_DECOMPOSE = 1;
constexpr uint32_t CODE_UNIT_COMPATIBLE_DECOMPOSE = 2;

enum class GraphemeUnitType : uint8_t {
    Other,
    CR,
    LF,
    Control,
    Extend,
    ZWJ,
    Regional_Indicator,
    Prepend,
    SpacingMark,
    L,
    V,
    T,
    LV,
    LVT,
    Extended_Pictographic
};

/* char32_t bits.
 *  - bits 20:0 Code point
 *        0x00'0000 - 0x10'ffff Unicode code point
 *        0x11'0000 - 0x1f'ffff Application code point
 *  - bit  21   Combining character
 *  -     '0' bits 31:24 opaque used by application.
 *  -     '1' bits 31:24 Canonical Order
 *  - bits 22   Grapheme start.
 *  - bits 23   Reserved.
 */
constexpr char32_t CODE_POINT_MASK = 0x001f'ffff;
constexpr char32_t COMBINING_CHARACTER_MASK = 0x0020'0000;
constexpr char32_t GRAPHEME_BREAK_MASK = 0x0040'0000;
constexpr char32_t RESERVED_MASK = 0x0080'0000;
constexpr char32_t ORDER_MASK = 0xff00'0000;

struct BinaryUnicodeData_Composition {
    little_uint64_buf_t data;

    force_inline char32_t startCharacter() const noexcept {
        return data.value() >> 43;
    }

    force_inline char32_t composingCharacter() const noexcept {
        return (data.value() >> 22) & CODE_POINT_MASK;
    }

    force_inline char32_t composedCharacter() const noexcept {
        return data.value() & CODE_POINT_MASK;
    }

    force_inline uint64_t searchValue() const noexcept {
        return data.value() >> 22;
    }
};

struct BinaryUnicodeData_Description {
    little_uint32_buf_t data1;
    little_uint32_buf_t data2;

    force_inline char32_t codePoint() const noexcept {
        return data1.value() >> 11;
    }

    force_inline uint8_t decompositionOrder() const noexcept {
        return (data1.value() >> 3) & 0xff;
    }

    force_inline bool decompositionIsCanonical() const noexcept {
        return (data1.value() & 1) > 0;
    }

    force_inline GraphemeUnitType graphemeUnitType() const noexcept {
        return static_cast<GraphemeUnitType>(data2.value() >> 28);
    }

    force_inline uint8_t decompositionLength() const noexcept {
        return (data2.value() >> 21) & 0x1f;
    }

    force_inline size_t decompositionOffset() const noexcept {
        return (data2.value() & CODE_POINT_MASK) * sizeof(uint64_t);
    }

    force_inline char32_t decompositionCodePoint() const noexcept {
        return static_cast<char32_t>(data2.value() & CODE_POINT_MASK);
    }

    force_inline uint32_t searchValue() const noexcept {
        return data1.value() >> 11;
    }
};

struct BinaryUnicodeData_Header {
    little_uint32_buf_t magic;
    little_uint32_buf_t version;
    little_uint32_buf_t nrDescriptions;
    little_uint32_buf_t nrCompositions;
    /* Canonical decomposition will include ligatures, so that the resulting
     * text will not include ligatures.
     *
     * After the header we will get the data for each of the code units.
     * These are ordered low to high to allow for binary search.
     *      BinaryUnicodeData_Description codeUnits[nrDescriptions];
     *
     * The next section are the canonical composition rules, these
     * are ordered low to high, startCharacter first followed by the composingCharacter, to
     * allow binary search. The codeUnits can point into this table for canonical two-unit decomposition.
     *      BinaryUnicodeData_Composition canonicalComposition[nrCompositions];
     *
     * The last section are for rest of the decompositions, both canonical and compatible.
     *      little_uint32_buf_t restDecompositions;
     */
};

BinaryUnicodeData::BinaryUnicodeData(gsl::span<std::byte const> bytes) :
    bytes(bytes), view()
{
    initialize();
}

BinaryUnicodeData::BinaryUnicodeData(std::unique_ptr<ResourceView> view) :
    bytes(view->bytes()), view(std::move(view))
{
    initialize();
}

void BinaryUnicodeData::initialize()
{
    size_t offset = 0;
    let header = make_placement_ptr<BinaryUnicodeData_Header>(bytes, offset);
    parse_assert(header->magic.value() == fourcc("bucd"));
    parse_assert(header->version.value() == 1);

    descriptions_offset = offset;
    descriptions_count = header->nrDescriptions.value();
    parse_assert(check_placement_array<BinaryUnicodeData_Description>(bytes, offset, descriptions_count));
    offset += sizeof(BinaryUnicodeData_Description) * descriptions_count;

    compositions_offset = offset;
    compositions_count = header->nrCompositions.value();
    parse_assert(check_placement_array<BinaryUnicodeData_Composition>(bytes, offset, compositions_count));
}

BinaryUnicodeData_Description const *BinaryUnicodeData::getDescription(char32_t c) const noexcept
{
    let descriptions = unsafe_make_placement_array<BinaryUnicodeData_Description>(bytes, rvalue_cast(descriptions_offset), descriptions_count);
    let i = std::lower_bound(descriptions.begin(), descriptions.end(), static_cast<uint32_t>(c), [](auto &element, auto value) {
        return element.searchValue() < value;
    });

    if (i == descriptions.end() || i->codePoint() != c) {
        return nullptr;
    } else {
        return &(*i);
    }
}

/*! Detect canonical ligature.
 * A canonical ligatures will have the same meaning in the text
 * when it is in composed or decomposed form.
 */
static bool isCanonicalLigature(char32_t c)
{
    switch (c) {
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

void BinaryUnicodeData::decompose(std::u32string &result, char32_t c, bool canonical, bool decomposeLigatures) const noexcept
{
    let codePoint = c & CODE_POINT_MASK;
    let upperBits = c & ORDER_MASK;

    if (codePoint >= 0x110000) {
        // Characters above unicode plane-16 are not decomposed.
        result += codePoint;

    } else {
        // The upper 11 bits are left intact in the decomposed characters.
        let description = getDescription(codePoint);
        if (description) {
            let decompositionLength = description->decompositionLength();

            bool mustDecompose =
                (decompositionLength > 0) & (
                    (canonical == description->decompositionIsCanonical()) |
                    (decomposeLigatures & isCanonicalLigature(codePoint))
                 );

            if (mustDecompose) {
                if (decompositionLength == 1) {
                    let codePoint = description->decompositionCodePoint();
                    decompose(result, codePoint | upperBits, canonical);

                } else {
                    let offset = description->decompositionOffset();
                    let nrTriplets = (decompositionLength + 2) / 3;

                    if (check_placement_array<little_uint64_buf_at>(bytes, offset, nrTriplets)) {
                        let decomposition = unsafe_make_placement_array<little_uint64_buf_at>(bytes, rvalue_cast(offset), nrTriplets);
                        for (size_t tripletIndex = 0, i = 0; tripletIndex < nrTriplets; tripletIndex++, i+=3) {
                            let triplet = decomposition[tripletIndex].value();
                            let codePoint1 = static_cast<char32_t>(triplet >> 43);
                            let codePoint2 = static_cast<char32_t>((triplet >> 22) & CODE_POINT_MASK);
                            let codePoint3 = static_cast<char32_t>(triplet & CODE_POINT_MASK);

                            decompose(result, codePoint1 | upperBits, canonical);
                            if (i + 1 < decompositionLength) {
                                decompose(result, codePoint2 | upperBits, canonical);
                            }
                            if (i + 2 < decompositionLength) {
                                decompose(result, codePoint3 | upperBits, canonical);
                            }
                        }

                    } else {
                        // Error in the file-format, replace with REPLACEMENT_CHARACTER U+FFFD.
                        result += static_cast<char32_t>(0xfffd) | upperBits;
                    }
                }

            } else {
                // No decomposition available, or do not want to decompose.
                result += c;
            }
        } else {
            // Code Point not found in unicode data, replace with REPLACEMENT_CHARACTER U+FFFD.
            result += static_cast<char32_t>(0xfffd) | upperBits;
        }
    }
}

void BinaryUnicodeData::normalizeDecompositionOrder(std::u32string &text) const noexcept
{
    for_each_cluster(result.begin(), result.end(),
        [](auto x) { return (x & COMBINING_CHARACTER_MASK) != 0; },
        [](auto s, auto e) { 
            std::stable_sort(s, e, [](auto a, auto b) {
                return (a & ORDER_MASK) < (b & ORDER_MASK);
            });
        }
    );
}

std::u32string BinaryUnicodeData::decompose(std::u32string_view text, bool canonical, bool decomposeLigatures) const noexcept
{
    std::u32string result;
    result.reserve(text.size() * 3);

    for (let c: text) {
        decompose(result, c, canonical, decomposeLigatures);
    }

    normalizeDecompositionOrder(result);
    return result;
}

std::u32string BinaryUnicodeData::canonicalDecompose(std::u32string_view text, bool decomposeLigatures) const noexcept
{
    return decompose(text, true, decomposeLigatures);
}

std::u32string BinaryUnicodeData::compatibleDecompose(std::u32string_view text) const noexcept
{
    return decompose(text, false);
}

char32_t BinaryUnicodeData::compose(char32_t startCharacter, char32_t composingCharacter, bool composeCRLF) const noexcept
{
    uint64_t searchValue =
        ((static_cast<uint64_t>(startCharacter) & CODE_POINT_MASK) << 21) |
        (static_cast<uint64_t>(composingCharacter) & CODE_POINT_MASK);

    char32_t upperBits = startCharacter & ORDER_MASK;

    if (composeCRLF & (searchValue == CRLF_SEARCH_VALUE)) {
        return 0x00'000d | upperBits;
    }

    let compositions = unsafe_make_placement_array<BinaryUnicodeData_Composition>(
        bytes, rvalue_cast(compositions_offset), compositions_count
    );

    let i = std::lower_bound(compositions.begin(), compositions.end(), searchValue, [](auto &element, auto value) {
        return element.searchValue() < value;
    });

    if (i != compositions.end() && i->searchValue() == searchValue) {
        return i->composedCharacter() | upperBits;
    } else {
        return 0;
    }
}

struct GraphemeBreakState {
    GraphemeUnitType previous = GraphemeUnitType::Other;
    bool RI_odd = false;

    void reset() noexcept {
        previous = GraphemeUnitType::Other;
        RI_odd = false;
    }
};

/*! Check if there is a grapheme break between two units.
*/
static bool checkGraphemeBreak_unitType(GraphemeUnitType type, GraphemeBreakState &state) noexcept
{
    if (type == GraphemeUnitType::Regional_Indicator) {
        state.RI_odd = !state.RI_odd;
    } else {
        state.RI_odd = false;
    }

    let lhs = state.previous;
    let rhs = type;

    bool const GB3 = (lhs == GraphemeUnitType::CR) & (rhs == GraphemeUnitType::LF);
    bool const GB6 =
        (lhs == GraphemeUnitType::L) &
        ((rhs == GraphemeUnitType::L) | (rhs == GraphemeUnitType::V) | (rhs == GraphemeUnitType::LV) | (rhs == GraphemeUnitType::LVT));
    bool const GB7 =
        ((lhs == GraphemeUnitType::LV) | (lhs == GraphemeUnitType::V)) &
        ((rhs == GraphemeUnitType::V) | (rhs == GraphemeUnitType::T));
    bool const GB8 =
        ((lhs == GraphemeUnitType::LVT) | (lhs == GraphemeUnitType::T)) &
        (rhs == GraphemeUnitType::T);
    bool const GB9 = ((rhs == GraphemeUnitType::Extend) | (rhs == GraphemeUnitType::ZWJ));
    bool const GB9a = (rhs == GraphemeUnitType::SpacingMark);
    bool const GB9b = (lhs == GraphemeUnitType::Prepend);
    //let GB11 =
    bool const GB12 = (lhs == GraphemeUnitType::Regional_Indicator) && (rhs == GraphemeUnitType::Regional_Indicator) & state.RI_odd;
    bool const dontBreak = (GB3 | GB6 | GB7 | GB8 | GB9 | GB9a | GB9b | GB12);

    state.previous = type;
    return !dontBreak;
}

bool BinaryUnicodeData::checkGraphemeBreak(char32_t c, GraphemeBreakState &state) const noexcept
{
    let codePoint = c & CODE_POINT_MASK;
    if (codePoint >= 0x110000) {
        state.reset();
        return true;

    } else {
        let description = getDescription(c);
        if (description) {
            return checkGraphemeBreak_unitType(description->graphemeUnitType(), state);
        } else {
            state.reset();
            return true;
        }
    }
}

size_t BinaryUnicodeData::compose(std::u32string &text, bool composeCRLF, bool breakGraphemes) const noexcept
{
    if (text.size() == 1) {
        return 0;
    }

    auto graphemeBreakState = GraphemeBreakState{};
    char32_t prevC = text[0];
    size_t j = 0;
    for (size_t i = 1; i < text.size(); i++) {
        let c = text[i];
        if (let newC = compose(prevC, c, composeCRLF)) {
            // Recurse compose.
            prevC = newC;
        } else {
            if (breakGraphemes && checkGraphemeBreak(prevC, graphemeBreakState)) {
                prevC |= GRAPHEME_BREAK_MASK;
            }
            text[j++] = prevC;
            prevC = c;
        }
    }
    text[j++] = prevC;
    return j;
}

std::u32string BinaryUnicodeData::toNFD(std::u32string_view text, bool decomposeLigatures) const noexcept
{
    return decompose(text, false, decomposeLigatures);
}

std::u32string BinaryUnicodeData::toNFC(std::u32string_view text, bool decomposeLigatures, bool composeCRLF, bool breakGraphemes) const noexcept
{
    auto decomposedText = decompose(text, false, decomposeLigatures);
    compose(decompsedText, composeCRLF, breakGraphemes);
    return decomposedText;
}

std::u32string BinaryUnicodeData::toNFKD(std::u32string_view text) const noexcept
{
    return decompose(text, true);
}

std::u32string BinaryUnicodeData::toNFKC(std::u32string_view text, bool composeCRLF, bool breakGraphemes) const noexcept
{
    auto decomposedText = decompose(text, true);
    compose(decompsedText, composeCRLF, breakGraphemes);
    return decomposedText;
}

}
