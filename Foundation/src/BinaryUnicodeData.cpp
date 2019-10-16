
#include "TTauri/Foundation/BinaryUnicodeData.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include <algorithm>

namespace TTauri {

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

struct BinaryUnicodeData_CanonicalComposition {
    little_uint32_buf_t startCharacter;
    little_uint32_buf_t composingCharacter;
    little_uint32_buf_t composedCharacter;

    force_inline uint64_t startPlusComposingCharacter() const noexcept {
        return
            static_cast<uint64_t>(startCharacter.value()) |
            static_cast<uint64_t>(composingCharacter.value()) << 32;
    }
};

struct DecompositionInfo {
    little_uint32_buf_t _offset;
    uint8_t _flagsAndLength;
    uint8_t _order;

    size_t offset() const noexcept {
        return _offset.value();
    }

    uint8_t order() const noexcept {
        return _order;
    }

    uint8_t length() const noexcept {
        return _flagsAndLength & 0x1f;
    }

    bool isCanonical() const noexcept {
        return length() > 0 && (_flagsAndLength & 0x20) > 0;
    }

    bool isCompatible() const noexcept {
        return length() > 0 && (_flagsAndLength & 0x20) == 0;
    }
};

struct BinaryUnicodeData_CodeUnit {
    little_uint32_buf_t codePoint;
    DecompositionInfo decompositionInfo;
    GraphemeUnitType graphemeUnitType;
    uint8_t reserved;
};

struct BinaryUnicodeData_Header {
    little_uint32_buf_t magic;
    little_uint32_buf_t version;
    little_uint32_buf_t nrCodeUnits;
    little_uint32_buf_t nrCanonicalCompositions;
    /* Canonical decomposition will include ligatures, so that the resulting
     * text will not include ligatures.
     *
     * After the header we will get the data for each of the code units.
     * These are ordered low to high to allow for binary search.
     *      BinaryUnicodeData_CodeUnit codeUnits[nrCodeUnits];
     *
     * The next section are the canonical composition rules, these
     * are ordered low to high, startCharacter first followed by the composingCharacter, to
     * allow binary search. The codeUnits can point into this table for canonical two-unit decomposition.
     *      BinaryUnicodeData_CanonicalComposition canonicalComposition[nrCanonicalCompositions];
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

    codeUnits_offset = offset;
    codeUnits_count = header->nrCodeUnits.value();
    parse_assert(check_placement_array<BinaryUnicodeData_CodeUnit>(bytes, offset, codeUnits_count));

    canonicalCompositions_offset = offset;
    canonicalCompositions_count = header->nrCanonicalCompositions.value();
    parse_assert(check_placement_array<BinaryUnicodeData_CanonicalComposition>(bytes, offset, canonicalCompositions_count));
}

BinaryUnicodeData_CodeUnit const *BinaryUnicodeData::getCodeUnitInfo(char32_t c) const noexcept
{
    let codeUnits = unsafe_make_placement_array<BinaryUnicodeData_CodeUnit>(bytes, rvalue_cast(codeUnits_offset), codeUnits_count);
    let i = std::lower_bound(codeUnits.begin(), codeUnits.end(), c, [](auto &element, auto value) {
        return element.codePoint.value() < value;
    });

    if (i == codeUnits.end() || i->codePoint.value() != c) {
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
        let info = getCodeUnitInfo(codePoint);
        if (info) {
            bool mustDecompose = canonical ?
                (info->decompositionInfo.isCanonical() || (decomposeLigatures && isCanonicalLigature(codePoint))):
                info->decompositionInfo.isCompatible();

            if (mustDecompose) {
                let offset = info->decompositionInfo.offset();
                let nrCodePoints = info->decompositionInfo.length();
                if (check_placement_array<little_uint32_buf_at>(bytes, offset, nrCodePoints)) {
                    let decomposition = unsafe_make_placement_array<little_uint32_buf_at>(bytes, rvalue_cast(offset), nrCodePoints);
                    for (let codePoint: decomposition) {
                        // Recusively decompose.
                        decompose(result, static_cast<char32_t>(codePoint.value()) | upperBits, canonical);
                    }

                } else {
                    // Error in the file-format, replace with REPLACEMENT_CHARACTER U+FFFD.
                    result += static_cast<char32_t>(0xfffd) | upperBits;
                }

            } else {
                let order = info->decompositionInfo.order();
                let newCodePoint = order == 0 ?
                    c :
                    (info->codePoint.value() | (static_cast<char32_t>(info->decompositionInfo.order()) << 24) | COMBINING_CHARACTER_MASK);

                result += newCodePoint;
            }
        } else {
            // Code Point not found in unicode data, replace with REPLACEMENT_CHARACTER U+FFFD.
            result += static_cast<char32_t>(0xfffd) | upperBits;
        }
    }
}

void BinaryUnicodeData::normalizeDecompositionOrder(std::u32string &result) const noexcept
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

char32_t BinaryUnicodeData::compose(char32_t startCharacter, char32_t composingCharacter) const noexcept
{
    uint64_t startPlusComposingCharacter =
        (static_cast<uint64_t>(startCharacter) & CODE_POINT_MASK) |
        ((static_cast<uint64_t>(composingCharacter) & CODE_POINT_MASK) << 32);

    let canonicalCompositions = unsafe_make_placement_array<BinaryUnicodeData_CanonicalComposition>(
        bytes, rvalue_cast(canonicalCompositions_offset), canonicalCompositions_count
    );

    let i = std::lower_bound(canonicalCompositions.begin(), canonicalCompositions.end(), startPlusComposingCharacter, [](auto &element, auto value) {
        return element.startPlusComposingCharacter() < value;
    });

    if (i != canonicalCompositions.end() && i->startPlusComposingCharacter() == startPlusComposingCharacter) {
        return i->composedCharacter.value();
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
        let info = getCodeUnitInfo(c);
        if (info) {
            return checkGraphemeBreak_unitType(info->graphemeUnitType, state);
        } else {
            state.reset();
            return true;
        }
    }
}

size_t BinaryUnicodeData::compose(std::u32string &text) const noexcept
{
    if (text.size() == 1) {
        return 0;
    }

    auto graphemeBreakState = GraphemeBreakState{};
    char32_t prevC = text[0];
    size_t j = 0;
    for (size_t i = 1; i < text.size(); i++) {
        let c = text[i];
        if (let newC = compose(prevC, c)) {
            // Recurse compose.
            prevC = newC;
        } else {
            if (checkGraphemeBreak(prevC, graphemeBreakState)) {
                prevC |= GRAPHEME_BREAK_MASK;
            }
            text[j++] = prevC;
            prevC = c;
        }
    }
    text[j++] = prevC;
    return j;
}

}