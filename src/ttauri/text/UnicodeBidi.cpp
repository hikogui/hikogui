// Copyright 2020 Pokitec
// All rights reserved.

#include "UnicodeBidi.hpp"
#include "UnicodeData.hpp"
#include "../Application.hpp"

namespace tt {

struct BidiCharacter {
    /** First code-point of a grapheme.
     * Graphemes will not include automatic ligatures (such as 'fi').
     */
    char32_t codePoint;
    int8_t embeddingLevel;
    BidiClass bidiClass;

    /** The original bidi-class is used by L1.
     */
    BidiClass origBidiClass;

    BidiCharacter(char32_t codePoint) noexcept :
        codePoint(codePoint),
        embeddingLevel(0),
        bidiClass(application->unicodeData->getBidiClass(codePoint)),
        origBidiClass(bidiClass) {}
};

struct BidiParagraph {
    int8_t embeddingLevel;

    BidiParagraph(int8_t embeddingLevel) noexcept : embeddingLevel(embeddingLevel) {}
};

struct BidiIsolateSequence {
    std::vector<BidiCharacter>::iterator first;
    std::vector<BidiCharacter>::iterator last;

    bool startOfParagraph;
    bool endOfParagraph;
    BidiClass sos;
    BidiClass eos;
    int8_t embeddingLevel;

    BidiIsolateSequence(std::vector<BidiCharacter>::iterator first, int8_t embeddingLevel=-1) noexcept :
        first(first), last(first), startOfParagraph(embeddingLevel == -1), endOfParagraph(false),
        sos(BidiClass::Unknown), eos(BidiClass::Unknown), embeddingLevel(embeddingLevel) {}
};

struct BidiContext {
    std::vector<BidiCharacter> characters;
    std::vector<BidiParagraph> paragraphs;
    std::vector<BidiIsolateSequence> isolateSequences;
};

static void BidiP1_P3(BidiContext &context) noexcept
{
    auto parBidiClass = BidiClass::Unknown;
    auto isolateLevel = 0;
    for (auto &character: context.characters) {
        // Classify each grapheme based on the first code-point.
        character.bidiClass = application->unicodeData->getBidiClass(character.codePoint);

        // P2. Find first L, AR or R bidi-class, ignoring isolated sections.
        switch (character.bidiClass) {
        case BidiClass::L:
        case BidiClass::AL:
        case BidiClass::R:
            if (isolateLevel == 0 && parBidiClass != BidiClass::Unknown) {
                parBidiClass = character.bidiClass;
            }
            break;
        case BidiClass::LRI:
        case BidiClass::RLI:
        case BidiClass::FSI:
            ++isolateLevel;
            break;
        case BidiClass::PDI:
            --isolateLevel;
            break;
        case BidiClass::B:
            // P3. AL or R means par level = 1, others level = 0.
            context.paragraphs.emplace_back(
                static_cast<int8_t>(parBidiClass == BidiClass::AL || parBidiClass == BidiClass::R)
            );
            isolateLevel = 0;
            parBidiClass = BidiClass::Unknown;
            break;
        default: tt_no_default;
        }
    }

    // P1. Split text in paragraphs, the paragraph delimiter comes at the end of the previous paragraph.
    // We will not physically split the paragraphs, but make sure the text does end in a paragraph.
    // We can not ourselves add the paragraph separator, because we do not know the text-style of an empty text.
    tt_assume(nonstd::ssize(context.characters) > 0 && context.characters.back().bidiClass == BidiClass::B);
}


struct BidiStackElement {
    int8_t embeddingLevel;
    BidiClass overrideStatus;
    bool isolateStatus;

    BidiStackElement(int8_t embeddingLevel, BidiClass overrideStatus, bool isolateStatus) noexcept :
        embeddingLevel(embeddingLevel), overrideStatus(overrideStatus), isolateStatus(isolateStatus) {}
};

[[nodiscard]] static int8_t nextEven(int8_t x) noexcept {
    return (x % 2 == 0) ? x + 2 : x + 1;
}

[[nodiscard]] static int8_t nextOdd(int8_t x) noexcept {
    return (x % 2 == 1) ? x + 2 : x + 1;
}

static void BidiX1_X8(BidiContext &context) noexcept
{
    constexpr int8_t max_depth = 125;

    std::vector<BidiStackElement> stack;
    auto overflowIsolateCount = 0;
    auto overflowEmbeddingCount = 0;
    auto validIsolateCount = 0;

    auto i = context.characters.begin();
    for (ttlet &paragraph : context.paragraphs) {
        // X1.
        stack.clear();
        stack.emplace_back(paragraph.embeddingLevel, BidiClass::Unknown, false);
        overflowIsolateCount = 0;
        overflowEmbeddingCount = 0;
        validIsolateCount = 0;

        for (; i->bidiClass != BidiClass::B; ++i) {
            tt_assume(i != context.characters.cend());

            ttlet currentEmbedingLevel = stack.back().embeddingLevel;
            ttlet currentOverrideStatus = stack.back().overrideStatus;
            ttlet nextOddEmbedingLevel = nextOdd(currentEmbedingLevel);
            ttlet nextEvenEmbedingLevel = nextEven(currentEmbedingLevel);

            switch (i->bidiClass) {
            case BidiClass::RLE: // X2. Explicit embeddings
                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextOddEmbedingLevel, BidiClass::Unknown, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidiClass::LRE: // X3. Explicit embeddings
                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextEvenEmbedingLevel, BidiClass::Unknown, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidiClass::RLO: // X4. Explicit overrides
                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextOddEmbedingLevel, BidiClass::R, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidiClass::LRO: // X5. Explicit overrides
                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextEvenEmbedingLevel, BidiClass::L, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidiClass::RLI: // X5a. Isolates
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidiClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }

                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    ++validIsolateCount;
                    stack.emplace_back(nextOddEmbedingLevel, BidiClass::Unknown, true);
                } else  {
                    ++overflowIsolateCount;
                }
                break;

            case BidiClass::LRI: // X5b. Isolates
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidiClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }

                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    ++validIsolateCount;
                    stack.emplace_back(nextEvenEmbedingLevel, BidiClass::Unknown, true);
                } else  {
                    ++overflowIsolateCount;
                }
                break;

            case BidiClass::FSI:
                // X5c. Determine paragraph-level of text up to matching PDI.
                // if 0 goto BidiClass::LRI, if 1 goto BidiClass::RLI.
                tt_not_implemented;
                break;

            case BidiClass::PDI: // X6a. Terminating Isolates
                if (overflowIsolateCount > 0) {
                    --overflowIsolateCount;
                } else if (validIsolateCount == 0) {
                    // Mismatched PDI, do nothing.
                    ;
                } else {
                    overflowEmbeddingCount = 0;
                    while (stack.back().isolateStatus == false) {
                        stack.pop_back();
                    }
                    stack.pop_back();
                    --validIsolateCount;
                }

                i->embeddingLevel = stack.back().embeddingLevel;
                if (stack.back().overrideStatus != BidiClass::Unknown) {
                    i->bidiClass = stack.back().overrideStatus;
                }

            case BidiClass::PDF: // X7. Terminating Embeddings and Overrides
                if (overflowIsolateCount > 0) {
                    // PDF is in scope of isolate, wait until the isolate is terminated.
                    ;
                } else if (overflowEmbeddingCount > 0) {
                    --overflowEmbeddingCount;
                } else if (stack.back().isolateStatus == false && nonstd::ssize(stack) >= 2) {
                    stack.pop_back();
                } else {
                    // PDF does not match embedding character.
                }

            case BidiClass::B: // X8. End of Paragraph
                                        // This should never appear in this loop.
                tt_no_default;

            case BidiClass::BN: // X6. Ignore
                break;

            default: // X6
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidiClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }
            }
        }

        ++i; // Skip over paragraph-separator.
    }
}

static bool BidiX9_valid(BidiClass const &x) noexcept
{
    switch (x) {
    case BidiClass::RLE:
    case BidiClass::LRE:
    case BidiClass::RLO:
    case BidiClass::LRO:
    case BidiClass::PDF:
    case BidiClass::BN:
        return false;
    default:
        return true;
    }
}

static bool BidiX9_valid(BidiCharacter const &x) noexcept
{
    return BidiX9_valid(x.bidiClass);
}

static void BidiX10(BidiContext &context) noexcept
{
    // X10. Find all sequences matching the same embedded level
    {
        auto i = context.characters.begin();
        for (ttlet &paragraph : context.paragraphs) {
            context.isolateSequences.emplace_back(i);
            auto &firstSequenceOfParagraph = context.isolateSequences.back();

            for (; i->bidiClass != BidiClass::B; ++i) {
                if (BidiX9_valid(*i)) {
                    if (context.isolateSequences.back().embeddingLevel == -1) {
                        context.isolateSequences.back().embeddingLevel = i->embeddingLevel;

                    } else if (context.isolateSequences.back().embeddingLevel != i->embeddingLevel) {
                        context.isolateSequences.back().last = i;
                        context.isolateSequences.emplace_back(i, context.isolateSequences.back().embeddingLevel);
                    }
                }
            }
            // Complete the last isolate sequence of a paragraph and include the paragraph separator.
            context.isolateSequences.back().last = ++i;
            context.isolateSequences.back().endOfParagraph = true;

            // All sequences after the first already know which embedding level they belong to (the reason for the
            // creation of the sequence is a different embedding level). But if the first sequence is not set, then
            // use the paragraph's embedding level.
            if (firstSequenceOfParagraph.embeddingLevel == -1) {
                firstSequenceOfParagraph.embeddingLevel = paragraph.embeddingLevel;
            }
        }
    }

    // Also find the start-of-sequence and end-of-sequence direction.
    for (auto i = context.isolateSequences.begin(); i != context.isolateSequences.end(); ++i) {
        if (i->startOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->sos = (i->embeddingLevel % 2 == 1) ? BidiClass::R : BidiClass::L;
        } else {
            ttlet maxEmbeddingLevel = std::max(i->embeddingLevel, (i-1)->embeddingLevel);
            i->sos = (maxEmbeddingLevel % 2 == 1) ? BidiClass::R : BidiClass::L;
        }
        if (i->endOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->eos = (i->embeddingLevel % 2 == 1) ? BidiClass::R : BidiClass::L;
        } else {
            ttlet maxEmbeddingLevel = std::max(i->embeddingLevel, (i+1)->embeddingLevel);
            i->eos = (maxEmbeddingLevel % 2 == 1) ? BidiClass::R : BidiClass::L;
        }
    }
}

// W1. Nonspacing marks will take on the direction of previous character, or neutral after isolation chars. 
static void BidiW1(BidiIsolateSequence &sequence) noexcept
{
    auto prevBidiClass = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidiClass::RLE:
        case BidiClass::LRE:
        case BidiClass::RLO:
        case BidiClass::LRO:
        case BidiClass::PDF:
        case BidiClass::BN: // X9 Ignore
            break;
        case BidiClass::LRI:
        case BidiClass::RLI:
        case BidiClass::FSI:
        case BidiClass::PDI:
            prevBidiClass = BidiClass::ON;
            break;
        case BidiClass::NSM:
            i->bidiClass = prevBidiClass;
            break;
        default:
            prevBidiClass = i->bidiClass;
        }
    }
}

// W2. Convert European-numbers to Arabic numbers if it follows Arabic letters.
static void BidiW2(BidiIsolateSequence &sequence) noexcept
{
    auto lastStrongDirection = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidiClass::R:
        case BidiClass::L:
        case BidiClass::AL:
            lastStrongDirection = i->bidiClass;
            break;
        case BidiClass::EN:
            if (lastStrongDirection == BidiClass::AL) {
                i->bidiClass = BidiClass::AN;
            }
            break;
        default:;
        }
    }
}

// W3. Convert Arabic letters to right-to-left.
static void BidiW3(BidiIsolateSequence &sequence) noexcept
{
    for (auto i = sequence.first; i != sequence.last; ++i) {
        if (i->bidiClass == BidiClass::AL) {
            i->bidiClass = BidiClass::R;
        }
    }
}

// W4. Change European/Common separator to numbers when between numbers.
static void BidiW4(BidiIsolateSequence &sequence) noexcept
{
    if (std::distance(sequence.first, sequence.last) < 3) {
        return;
    }

    auto first = sequence.first + 1;
    auto last = sequence.last - 1;
    for (auto i = first; i != last; ++i) {
        if (
            (i-1)->bidiClass == BidiClass::EN &&
            (i->bidiClass == BidiClass::ES || i->bidiClass == BidiClass::CS) &&
            (i+1)->bidiClass == BidiClass::EN
            ) {
            i->bidiClass = BidiClass::EN;

        } else if (
            (i-1)->bidiClass == BidiClass::AN &&
            i->bidiClass == BidiClass::CS &&
            (i+1)->bidiClass == BidiClass::AN
            ) {
            i->bidiClass = BidiClass::AN;
        }
    }
}

// W5. A sequence of European terminators become European-number when adjacent to them.
static void BidiW5(BidiIsolateSequence &sequence) noexcept
{
    bool foundEN = false;
    std::optional<decltype(sequence.first)> firstET = {};

    for (auto i = sequence.first; i != sequence.last; ++i) {
        if (i->bidiClass == BidiClass::ET) {
            if (foundEN) {
                i->bidiClass = BidiClass::EN;
            } else if (!firstET) {
                firstET = i;
            }
        } else if (i->bidiClass == BidiClass::EN) {
            if (firstET) {
                for (auto j = *firstET; j != i; ++j) {
                    j->bidiClass = BidiClass::EN;
                }
                firstET = {};
            }
            foundEN = true;
        } else {
            foundEN = false;
            firstET = {};
        }
    }
}

// W6. Separators and Terminators are converted to ON.
static void BidiW6(BidiIsolateSequence &sequence) noexcept
{
    for (auto i = sequence.first; i != sequence.last; ++i) {
        if (
            i->bidiClass == BidiClass::ET ||
            i->bidiClass == BidiClass::ES ||
            i->bidiClass == BidiClass::CS
            ) {
            i->bidiClass = BidiClass::ON;
        }
    }
}

// W7. If European number is preceded by a string L then the number is converted to L.
static void BidiW7(BidiIsolateSequence &sequence) noexcept
{
    auto lastStrongDirection = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidiClass::R:
        case BidiClass::L:
            lastStrongDirection = i->bidiClass;
            break;
        case BidiClass::EN:
            if (lastStrongDirection == BidiClass::L) {
                i->bidiClass = BidiClass::L;
            }
            break;
        default:;
        }
    }
}

static void BidiW(BidiContext &context) noexcept
{
    for (auto &sequence: context.isolateSequences) {
        BidiW1(sequence);
        BidiW2(sequence);
        BidiW3(sequence);
        BidiW4(sequence);
        BidiW5(sequence);
        BidiW6(sequence);
        BidiW7(sequence);
    }
}

static void BidiN0(BidiIsolateSequence &sequence) noexcept
{
    tt_not_implemented;
}

static void BidiN1(BidiIsolateSequence &sequence) noexcept
{
    auto lastDirection = sequence.sos;
    std::optional<decltype(sequence.first)> firstNI;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidiClass::B:
        case BidiClass::S:
        case BidiClass::WS:
        case BidiClass::ON:
        case BidiClass::FSI:
        case BidiClass::LRI:
        case BidiClass::RLI:
        case BidiClass::PDI:
        case BidiClass::RLE:// X9 Ignore
        case BidiClass::LRE:// X9 Ignore
        case BidiClass::RLO:// X9 Ignore
        case BidiClass::LRO:// X9 Ignore
        case BidiClass::PDF:// X9 Ignore
        case BidiClass::BN: // X9 Ignore
            if (!firstNI) {
                firstNI = i;
            }
            break;

        case BidiClass::L:
            if (firstNI) {
                if (lastDirection == BidiClass::L) {
                    for (auto j = *firstNI; j != i; ++j) {
                        j->bidiClass = lastDirection;
                    }
                }
                firstNI = {};
            }
            lastDirection = BidiClass::L;
            break;

        case BidiClass::R:
        case BidiClass::AL:
        case BidiClass::EN: // European Numbers are treated as R.
        case BidiClass::AN: // Arabic Numbers are treated as R.
            if (firstNI) {
                if (lastDirection == BidiClass::L) {
                    for (auto j = *firstNI; j != i; ++j) {
                        j->bidiClass = lastDirection;
                    }
                }
                firstNI = {};
            }
            lastDirection = BidiClass::R;
            break;

        default:
            tt_no_default;
        }
    }

    if (firstNI) {
        if (lastDirection == sequence.eos) {
            for (auto j = *firstNI; j != sequence.last; ++j) {
                j->bidiClass = lastDirection;
            }
        }
        firstNI = {};
    }
}

// N2. remaining NI get embedding level direction.
static void BidiN2(BidiIsolateSequence &sequence) noexcept
{
    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidiClass::B:
        case BidiClass::S:
        case BidiClass::WS:
        case BidiClass::ON:
        case BidiClass::FSI:
        case BidiClass::LRI:
        case BidiClass::RLI:
        case BidiClass::PDI:
            i->bidiClass = (sequence.embeddingLevel % 2 == 0) ? BidiClass::L : BidiClass::R;
            break;
        default:;
        }
    }
}

static void BidiN(BidiContext &context) noexcept
{
    for (auto &sequence: context.isolateSequences) {
        BidiN0(sequence);
        BidiN1(sequence);
        BidiN2(sequence);
    }
}

static void BidiI1_I2(BidiContext &context) noexcept
{
    for (auto &character: context.characters) {
        if (character.embeddingLevel % 2 == 0) {
            // I1.
            switch (character.bidiClass) {
            case BidiClass::R:
                ++character.embeddingLevel;
                break;
            case BidiClass::AN:
            case BidiClass::EN:
                character.embeddingLevel += 2;
                break;
            default:;
            }
        } else {
            // I2.
            switch (character.bidiClass) {
            case BidiClass::L:
            case BidiClass::AN:
            case BidiClass::EN:
                ++character.embeddingLevel;
                break;
            default:;
            }
        }
    }
}

static void BidiL1(BidiContext &context) noexcept
{
    auto i = context.characters.begin();
    std::optional<decltype(i)> firstWS = {};

    for (ttlet &paragraph : context.paragraphs) {
        for (; i->origBidiClass != BidiClass::B; ++i) {
            switch (i->origBidiClass) {
            case BidiClass::S:
                i->embeddingLevel = paragraph.embeddingLevel;
                break;

            case BidiClass::WS:
            case BidiClass::FSI:
            case BidiClass::LRI:
            case BidiClass::RLI:
            case BidiClass::PDI:
            case BidiClass::RLE:// X9 Ignore
            case BidiClass::LRE:// X9 Ignore
            case BidiClass::RLO:// X9 Ignore
            case BidiClass::LRO:// X9 Ignore
            case BidiClass::PDF:// X9 Ignore
            case BidiClass::BN: // X9 Ignore
                if (i->codePoint == 0x2028) { // Line Separator.
                    i->embeddingLevel = paragraph.embeddingLevel;
                    if (firstWS) {
                        for (auto j = *firstWS; j != i; ++j) {
                            j->embeddingLevel = paragraph.embeddingLevel;
                        }
                        firstWS = {};
                    }
                } else {
                    // Make sure X9. characters are categorized to their original class.
                    i->bidiClass = i->origBidiClass;
                    if (!firstWS) {
                        firstWS = i;
                    }
                }
                break;

            default:
                firstWS = {};
            }
        }

        // BidiClass::B.
        if (firstWS) {
            for (auto j = *firstWS; j != i; ++j) {
                j->embeddingLevel = paragraph.embeddingLevel;
            }
            firstWS = {};
        }
        i->embeddingLevel = paragraph.embeddingLevel;
        ++i;
    }
}

//static std::vector<std::pair<ssize_t,ssize_t>> BidiL2(BidiContext &context) noexcept
//{
//}

//static std::vector<std::pair<ssize_t,ssize_t>> BidiL(BidiContext &context) noexcept
//{
//    BidiL1(context);
//    return BidiL2(context);
//}

//static std::vector<std::pair<ssize_t,ssize_t>> BidiAlgorithm(BidiContext &context) noexcept
//{
//    BidiP1_P3(context);
//    BidiX1_X8(context);
//    BidiX10(context);
//    BidiW(context);
//    BidiN(context);
//    BidiI1_I2(context);
//    return BidiL(context);
//}

}