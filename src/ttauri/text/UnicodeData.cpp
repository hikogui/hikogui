// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "ttauri/text/UnicodeData.hpp"
#include "ttauri/placement.hpp"
#include "ttauri/endian.hpp"
#include "ttauri/algorithm.hpp"
#include "ttauri/exceptions.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/required.hpp"
#include <algorithm>

namespace tt {

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

    tt_force_inline char32_t startCharacter() const noexcept {
        return data.value() >> 43;
    }

    tt_force_inline char32_t composingCharacter() const noexcept {
        return (data.value() >> 22) & UNICODE_MASK;
    }

    tt_force_inline char32_t composedCharacter() const noexcept {
        return data.value() & UNICODE_MASK;
    }

    tt_force_inline uint64_t searchValue() const noexcept {
        return data.value() >> 22;
    }
};

struct UnicodeData_Description {
    little_uint64_buf_t data;

    tt_force_inline char32_t codePoint() const noexcept {
        return data.value() >> 43;
    }

    tt_force_inline uint8_t decompositionOrder() const noexcept {
        return (data.value() >> 26) & 0xff;
    }

    tt_force_inline bool decompositionIsCanonical() const noexcept {
        return ((data.value() >> 34) & 1) > 0;
    }

    tt_force_inline GraphemeUnitType graphemeUnitType() const noexcept {
        return static_cast<GraphemeUnitType>((data.value() >> 35) & 0xf);
    }

    tt_force_inline uint8_t decompositionLength() const noexcept {
        return (data.value() >> 21) & 0x1f;
    }

    tt_force_inline size_t decompositionOffset() const noexcept {
        return (data.value() & UNICODE_MASK) * sizeof(uint64_t);
    }

    tt_force_inline char32_t decompositionCodePoint() const noexcept {
        return static_cast<char32_t>(data.value() & UNICODE_MASK);
    }

    tt_force_inline BidiClass bidiClass() const noexcept {
        switch (codePoint()) {
        case 0x00'202a: return BidiClass::LRE;
        case 0x00'202d: return BidiClass::LRO;
        case 0x00'202b: return BidiClass::RLE;
        case 0x00'202e: return BidiClass::RLO;
        case 0x00'202c: return BidiClass::PDF;
        case 0x00'2066: return BidiClass::LRI;
        case 0x00'2067: return BidiClass::RLI;
        case 0x00'2068: return BidiClass::FSI;
        case 0x00'2069: return BidiClass::PDI;
        default: return static_cast<BidiClass>((data.value() >> 39) & 0x0f);
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

UnicodeData::UnicodeData(nonstd::span<std::byte const> bytes) :
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
    ssize_t offset = 0;
    ttlet header = make_placement_ptr<UnicodeData_Header>(bytes, offset);
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
    ttlet descriptions = unsafe_make_placement_array<UnicodeData_Description>(bytes, rvalue_cast(descriptions_offset), descriptions_count);
    ttlet i = std::lower_bound(descriptions.begin(), descriptions.end(), codePoint, [](auto &element, auto value) {
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
        ttlet SIndex = codePoint - HANGUL_SBASE;
        return (SIndex % HANGUL_TCOUNT) == 0 ? GraphemeUnitType::LV : GraphemeUnitType::LVT;

    } else if (isHangulLPart(codePoint)) {
        return GraphemeUnitType::L;

    } else if (isHangulVPart(codePoint)) {
        return GraphemeUnitType::V;

    } else if (isHangulTPart(codePoint)) {
        return GraphemeUnitType::T;

    } else {
        ttlet description = getDescription(codePoint);
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
        ttlet description = getDescription(codePoint);
        if (description) {
            return description->decompositionOrder();
        } else {
            return 0;
        }
    }
}

BidiClass UnicodeData::getBidiClass(char32_t codePoint) const noexcept {
    if (codePoint <= ASCII_MAX && codePoint > UNICODE_MAX) {
        return BidiClass::Unknown;
    } else if (
        isHangulLPart(codePoint) || isHangulVPart(codePoint) || isHangulTPart(codePoint) ||
        isHangulSyllable(codePoint)
        ) {
        return BidiClass::L;
    } else {
        ttlet description = getDescription(codePoint);
        if (description) {
            return description->bidiClass();
        } else {
            return BidiClass::Unknown;
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
    ttlet &description = getDescription(codePoint);
    ttlet decompositionLength = description ? description->decompositionLength() : 0;
    ttlet mustDecompose =
        (decompositionLength > 0) && (
            decomposeCompatible ||
            description->decompositionIsCanonical() ||
            (decomposeLigatures && isCanonicalLigature(codePoint))
            );

    if (codePoint <= ASCII_MAX || codePoint > UNICODE_MAX) {
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
            decomposeCodePoint(result, description->decompositionCodePoint(), decomposeCompatible, decomposeLigatures);

        } else {
            ttlet offset = description->decompositionOffset();
            ttlet nrTriplets = (decompositionLength + 2) / 3;

            if (check_placement_array<little_uint64_buf_at>(bytes, offset, nrTriplets)) {
                ttlet decomposition = unsafe_make_placement_array<little_uint64_buf_at>(bytes, rvalue_cast(offset), nrTriplets);
                for (size_t tripletIndex = 0, i = 0; tripletIndex < nrTriplets; tripletIndex++, i+=3) {
                    ttlet triplet = decomposition[tripletIndex].value();
                    ttlet codePoint1 = static_cast<char32_t>(triplet >> 43);
                    ttlet codePoint2 = static_cast<char32_t>((triplet >> 22) & UNICODE_MASK);
                    ttlet codePoint3 = static_cast<char32_t>(triplet & UNICODE_MASK);

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

    for (ttlet codePoint: text) {
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
        ttlet LIndex = startCodePoint - HANGUL_LBASE;
        ttlet VIndex = composingCodePoint - HANGUL_VBASE;
        ttlet LVIndex = LIndex * HANGUL_NCOUNT + VIndex * HANGUL_TCOUNT;
        return HANGUL_SBASE + LVIndex;

    } else if (isHangulLVPart(startCodePoint) && isHangulTPart(composingCodePoint)) {
        ttlet TIndex = composingCodePoint - HANGUL_TBASE;
        return startCodePoint + TIndex;

    } else {
        ttlet compositions = unsafe_make_placement_array<UnicodeData_Composition>(
            bytes, rvalue_cast(compositions_offset), compositions_count
        );

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
void UnicodeData::compose(std::u32string &text, bool composeCRLF) const noexcept
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

                bool blockingPair =
                    prevDecompositionOrder != 0 &&
                    prevDecompositionOrder >= composingDecompositionOrder;

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
    ttlet lhs = state.previous;
    ttlet rhs = type;

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

    ttlet GB3 = (lhs == GraphemeUnitType::CR) && (rhs == GraphemeUnitType::LF);
    ttlet GB4 = (lhs == GraphemeUnitType::Control) || (lhs == GraphemeUnitType::CR) || (lhs == GraphemeUnitType::LF);
    ttlet GB5 = (rhs == GraphemeUnitType::Control) || (rhs == GraphemeUnitType::CR) || (rhs == GraphemeUnitType::LF);
    if (breakState == state_t::Unknown) {
        if (GB3) {
            breakState = state_t::DontBreak;
        } else if (GB4 || GB5) {
            breakState = state_t::Break;
        }
    }

    ttlet GB6 =
        (lhs == GraphemeUnitType::L) &&
        ((rhs == GraphemeUnitType::L) || (rhs == GraphemeUnitType::V) || (rhs == GraphemeUnitType::LV) | (rhs == GraphemeUnitType::LVT));
    ttlet GB7 =
        ((lhs == GraphemeUnitType::LV) || (lhs == GraphemeUnitType::V)) &&
        ((rhs == GraphemeUnitType::V) || (rhs == GraphemeUnitType::T));
    ttlet GB8 =
        ((lhs == GraphemeUnitType::LVT) || (lhs == GraphemeUnitType::T)) &&
        (rhs == GraphemeUnitType::T);
    if ((breakState == state_t::Unknown) && (GB6 || GB7 || GB8)) {
        breakState = state_t::DontBreak;
    }

    ttlet GB9 = ((rhs == GraphemeUnitType::Extend) || (rhs == GraphemeUnitType::ZWJ));
    ttlet GB9a = (rhs == GraphemeUnitType::SpacingMark);
    ttlet GB9b = (lhs == GraphemeUnitType::Prepend);
    if ((breakState == state_t::Unknown) & (GB9 || GB9a || GB9b)) {
        breakState = state_t::DontBreak;
    }

    ttlet GB11 = state.inExtendedPictographic && (lhs == GraphemeUnitType::ZWJ) && (rhs == GraphemeUnitType::Extended_Pictographic);
    if ((breakState == state_t::Unknown) && GB11) {
        breakState = state_t::DontBreak;
    }

    if (rhs == GraphemeUnitType::Extended_Pictographic) {
        state.inExtendedPictographic = true;
    } else if (!((rhs == GraphemeUnitType::Extend) || (rhs == GraphemeUnitType::ZWJ))) {
        state.inExtendedPictographic = false;
    }

    ttlet GB12_13 = (lhs == GraphemeUnitType::Regional_Indicator) && (rhs == GraphemeUnitType::Regional_Indicator) && ((state.RICount % 2) == 1);
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

}

namespace tt {

template<>
std::unique_ptr<tt::UnicodeData> parseResource(URL const &location)
{
    if (location.extension() == "bin") {
        auto view = location.loadView();

        try {
            auto unicodeData = std::make_unique<tt::UnicodeData>(std::move(view));
            return unicodeData;
        } catch (error &e) {
            e.set<url_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<url_tag>(location)
        );
    }
}

}
