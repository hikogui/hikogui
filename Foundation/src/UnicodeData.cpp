
#include "TTauri/Foundation/UnicodeData.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include <algorithm>

namespace TTauri {

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

constexpr char32_t HANGUL_SBASE = 0xac00;
constexpr char32_t HANGUL_LBASE = 0x1100;
constexpr char32_t HANGUL_VBASE = 0x1161;
constexpr char32_t HANGUL_TBASE = 0x11a7;
constexpr size_t HANGUL_LCOUNT = 19;
constexpr size_t HANGUL_VCOUNT = 21;
constexpr size_t HANGUL_TCOUNT = 28;
constexpr size_t HANGUL_NCOUNT = HANGUL_VCOUNT * HANGUL_TCOUNT;
constexpr size_t HANGUL_SCOUNT = HANGUL_LCOUNT * HANGUL_NCOUNT;
constexpr char32_t HANGUL_SEND = HANGUL_SBASE + HANGUL_SCOUNT;
constexpr char32_t HANGUL_LEND = HANGUL_LBASE + HANGUL_LCOUNT;
constexpr char32_t HANGUL_VEND = HANGUL_VBASE + HANGUL_VCOUNT;
constexpr char32_t HANGUL_TEND = HANGUL_TBASE + HANGUL_TCOUNT;




struct UnicodeData_Composition {
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

struct UnicodeData_Description {
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

struct UnicodeData_GraphemeBreakState {
    GraphemeUnitType previous = GraphemeUnitType::Other;
    bool RI_odd = false;

    void reset() noexcept {
        previous = GraphemeUnitType::Other;
        RI_odd = false;
    }
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

UnicodeData_Description const *UnicodeData::getDescription(char32_t c) const noexcept
{
    let descriptions = unsafe_make_placement_array<UnicodeData_Description>(bytes, rvalue_cast(descriptions_offset), descriptions_count);
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

void UnicodeData::decomposeCodePoint(std::u32string &result, char32_t c, bool decomposeCompatible, bool decomposeLigatures) const noexcept
{
    let codePoint = c & CODE_POINT_MASK;
    let upperBits = c & ORDER_MASK;

    if (codePoint <= 127 || codePoint >= 0x110000) {
        // ASCII characters and code-points above unicode plane-16 are not decomposed.
        result += c;

    } else if (codePoint >= HANGUL_SBASE && codePoint <= HANGUL_SEND) {
        let SIndex = codePoint - HANGUL_SBASE;
        let LIndex = static_cast<char32_t>(SIndex / HANGUL_NCOUNT);
        let VIndex = static_cast<char32_t>((SIndex % HANGUL_NCOUNT) / HANGUL_TCOUNT);
        let TIndex = static_cast<char32_t>(SIndex % HANGUL_TCOUNT);
        result += (HANGUL_LBASE + LIndex) | upperBits;
        result += (HANGUL_VBASE + VIndex) | upperBits;
        if (TIndex > 0) {
            result += (HANGUL_TBASE + TIndex) | upperBits;
        }

    } else {
        let description = getDescription(codePoint);
        if (description) {
            let decompositionLength = description->decompositionLength();

            bool mustDecompose =
                (decompositionLength > 0) & (
                    decomposeCompatible |
                    description->decompositionIsCanonical() |
                    (decomposeLigatures & isCanonicalLigature(codePoint))
                 );

            if (mustDecompose) {
                if (decompositionLength == 1) {
                    let codePoint = description->decompositionCodePoint();
                    decomposeCodePoint(result, codePoint | upperBits, decomposeCompatible, decomposeLigatures);

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

                            decomposeCodePoint(result, codePoint1 | upperBits, decomposeCompatible, decomposeLigatures);
                            if (i + 1 < decompositionLength) {
                                decomposeCodePoint(result, codePoint2 | upperBits, decomposeCompatible, decomposeLigatures);
                            }
                            if (i + 2 < decompositionLength) {
                                decomposeCodePoint(result, codePoint3 | upperBits, decomposeCompatible, decomposeLigatures);
                            }
                        }

                    } else {
                        // Error in the file-format, replace with REPLACEMENT_CHARACTER U+FFFD.
                        result += static_cast<char32_t>(0xfffd) | upperBits;
                    }
                }

            } else {
                let order = description->decompositionOrder();
                let newCodePoint = order == 0 ?
                    c :
                    (description->codePoint() | (static_cast<char32_t>(order) << 24) | COMBINING_CHARACTER_MASK);

                result += newCodePoint;
            }

        } else {
            // Don't decompose code-points without a description in the UnicodeData.
            result += c;
        }
    }
}

void UnicodeData::normalizeDecompositionOrder(std::u32string &result) const noexcept
{
    for_each_cluster(result.begin(), result.end(),
        [](auto x) { return (x & COMBINING_CHARACTER_MASK) == 0; },
        [](auto s, auto e) { 
            std::stable_sort(s, e, [](auto a, auto b) {
                return (a & ORDER_MASK) < (b & ORDER_MASK);
            });
        }
    );

    for (auto &c: result) {
        if ((c & COMBINING_CHARACTER_MASK) > 0) {
            c &= CODE_POINT_MASK;
        }
    }
}

std::u32string UnicodeData::decompose(std::u32string_view text, bool decomposeCompatible, bool decomposeLigatures) const noexcept
{
    std::u32string result;
    result.reserve(text.size() * 3);

    for (let c: text) {
        decomposeCodePoint(result, c, decomposeCompatible, decomposeLigatures);
    }

    normalizeDecompositionOrder(result);
    return result;
}

std::u32string UnicodeData::canonicalDecompose(std::u32string_view text, bool decomposeLigatures) const noexcept
{
    return decompose(text, false, decomposeLigatures);
}

std::u32string UnicodeData::compatibleDecompose(std::u32string_view text) const noexcept
{
    return decompose(text, true);
}

/*! Check if there is a grapheme break between two units.
*/
static bool checkGraphemeBreak_unitType(GraphemeUnitType type, UnicodeData_GraphemeBreakState &state) noexcept
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

GraphemeUnitType UnicodeData::getGraphemeUnitType(char32_t c) const noexcept {
    if (c >= 0x110000) {
        return GraphemeUnitType::Other;

    } else if (c >= HANGUL_SBASE && c < HANGUL_SEND) {
        let SIndex = c - HANGUL_SBASE;
        return (SIndex % HANGUL_TCOUNT) == 0 ? GraphemeUnitType::LV : GraphemeUnitType::LVT;

    } else if (c >= HANGUL_LBASE && c < HANGUL_LEND) {
        return GraphemeUnitType::L;

    } else if (c >= HANGUL_VBASE && c < HANGUL_VEND) {
        return GraphemeUnitType::V;

    } else if (c >= HANGUL_TBASE && c < HANGUL_TEND) {
        return GraphemeUnitType::T;

    } else {
        let description = getDescription(c);
        if (description) {
            return description->graphemeUnitType();
        } else {
            return GraphemeUnitType::Other;
        }
    }
}

bool UnicodeData::checkGraphemeBreak(char32_t c, UnicodeData_GraphemeBreakState &state) const noexcept
{
    let codePoint = c & CODE_POINT_MASK;
    return checkGraphemeBreak_unitType(getGraphemeUnitType(codePoint), state);
}

char32_t UnicodeData::compose(char32_t startCharacter, char32_t composingCharacter) const noexcept
{
    uint64_t searchValue =
        ((static_cast<uint64_t>(startCharacter) & CODE_POINT_MASK) << 21) |
        (static_cast<uint64_t>(composingCharacter) & CODE_POINT_MASK);

    let compositions = unsafe_make_placement_array<UnicodeData_Composition>(
        bytes, rvalue_cast(compositions_offset), compositions_count
        );

    let i = std::lower_bound(compositions.begin(), compositions.end(), searchValue, [](auto &element, auto value) {
        return element.searchValue() < value;
        });

    if (i != compositions.end() && i->searchValue() == searchValue) {
        return i->composedCharacter();
    } else {
        return 0;
    }
}

void UnicodeData::compose(std::u32string &text) const noexcept
{
    if (text.size() <= 1) {
        return;
    }

    auto graphemeBreakState = UnicodeData_GraphemeBreakState{};
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
    text.resize(j);
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