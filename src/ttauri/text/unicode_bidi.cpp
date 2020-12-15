// Copyright 2020 Pokitec
// All rights reserved.

#include "unicode_bidi.hpp"
#include "unicode_data.hpp"
#include "../application.hpp"

namespace tt {

struct BidiParagraph {
    int8_t embedding_level;

    BidiParagraph(int8_t embedding_level) noexcept : embedding_level(embedding_level) {}
};

struct BidiIsolateSequence {
    std::vector<BidiCharacter>::iterator first;
    std::vector<BidiCharacter>::iterator last;

    bool startOfParagraph;
    bool endOfParagraph;
    unicode_bidi_class sos;
    unicode_bidi_class eos;
    int8_t embedding_level;

    BidiIsolateSequence(std::vector<BidiCharacter>::iterator first, int8_t embedding_level=-1) noexcept :
        first(first), last(first), startOfParagraph(embedding_level == -1), endOfParagraph(false),
        sos(bidi_class::Unknown), eos(bidi_class::Unknown), embedding_level(embedding_level) {}
};

struct unicode_bidi_stack_element {
    int8_t embedding_level;
    unicode_bidi_class override_status;
    bool isolate_status;

    BidiStackElement(int8_t embedding_level, unicode_bidi_class override_status, bool isolate_status) noexcept :
        embedding_level(embedding_level), override_status(override_status), isolate_status(isolate_status) {}
};

[[nodiscard]] static int8_t next_even(int8_t x) noexcept {
    return (x % 2 == 0) ? x + 2 : x + 1;
}

[[nodiscard]] static int8_t next_odd(int8_t x) noexcept {
    return (x % 2 == 1) ? x + 2 : x + 1;
}

static void X1(unicode_bidi_paragraph &paragraph) noexcept
{
    constexpr int8_t max_depth = 125;

    long long overflow_isolate_count = 0;
    long long overflow_embedding_count = 0;
    long long valid_isolate_count = 0;

    // X1.
    auto stack = std::vector<unicode_bidi_stack_element>{
        {paragraph.embeding_class, unicode_bidi_class::Unknown, false}
    };

    for (auto &character: paragraph.characters) {
        ttlet current_embedding_level = stack.back().embedding_level;
        ttlet current_override_status = stack.back().override_status;
        ttlet next_odd_embedding_level = next_odd(current_embedding_level);
        ttlet next_even_embedding_level = next_even(current_embedding_level);

        switch (character.bidi_class) {
        using enum unicode_bidi_class;
        case RLE: // X2. Explicit embeddings
            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_odd_embedding_level, unicode_bidi_class::Unknown, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case LRE: // X3. Explicit embeddings
            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_even_embedding_level, unicode_bidi_class::Unknown, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case RLO: // X4. Explicit overrides
            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_odd_embedding_level, unicode_bidi_class::R, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case LRO: // X5. Explicit overrides
            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_even_embedding_level, unicode_bidi_class::L, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case RLI: // X5a. Isolates
            character.embedding_level = current_embedding_level;
            if (current_override_status != unicode_bidi_class::Unknown) {
                character.bidi_class = current_override_status;
            }

            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_odd_embedding_level, unicode_bidi_class::Unknown, true);
            } else  {
                ++overflow_isolate_count;
            }
            break;

        case LRI: // X5b. Isolates
            character.embedding_level = current_embedding_level;
            if (current_override_status != unicode_bidi_class::Unknown) {
                character.bidi_class = current_override_status;
            }

            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_even_embedding_level, unicode_bidi_class::Unknown, true);
            } else  {
                ++overflow_isolate_count;
            }
            break;

        case FSI:
            // X5c. Determine paragraph-level of text up to matching PDI.
            // if 0 goto unicode_bidi_class::LRI, if 1 goto unicode_bidi_class::RLI.
            break;

        case PDI: // X6a. Terminating Isolates
            if (overflow_isolate_count > 0) {
                --overflow_isolate_count;
            } else if (valid_isolate_count == 0) {
                // Mismatched PDI, do nothing.
                ;
            } else {
                overflow_embedding_count = 0;
                while (stack.back().isolate_status == false) {
                    stack.pop_back();
                }
                stack.pop_back();
                --valid_isolate_count;
            }

            character.embedding_level = stack.back().embedding_level;
            if (stack.back().override_status != unicode_bidi_class::Unknown) {
                character.bidi_class = stack.back().override_status;
            }
            break;

        case PDF: // X7. Terminating Embeddings and Overrides
            if (overflow_isolate_count > 0) {
                // PDF is in scope of isolate, wait until the isolate is terminated.
                ;
            } else if (overflow_embedding_count > 0) {
                --overflow_embedding_count;
            } else if (stack.back().isolate_status == false && std::ssize(stack) >= 2) {
                stack.pop_back();
            } else {
                // PDF does not match embedding character.
            }
            break;

        case B: // X8. End of Paragraph
            character.embedding_level = paragraph.embedding_level;
            break;

        case BN: // X6. Ignore
            break;

        default: // X6
            character.embedding_level = current_embedding_level;
            if (current_override_status != unicode_bidi_class::Unknown) {
                character.bidi_class = current_override_status;
            }
        }
    }
}

static void X9(unicode_bidi_paragraph &paragraph) noexcept
{
    auto new_end = std::remove_if(std::begin(paragraph), std::end(paragraph), [](ttlet &character) {
        return
            character.bidi_class == unicode_bidi_class::RLE &&
            character.bidi_class == unicode_bidi_class::LRE &&
            character.bidi_class == unicode_bidi_class::RLO &&
            character.bidi_class == unicode_bidi_class::LRO &&
            character.bidi_class == unicode_bidi_class::PDF &&
            character.bidi_class == unicode_bidi_class::BN;
    });

    paragraph.erase(new_end, std::end(paragraph));
}

static void X10(unicode_bidi_paragraph &paragraph) noexcept
{
    // X10. Find all sequences matching the same embedded level
    {
        auto i = context.characters.begin();
        for (ttlet &paragraph : context.paragraphs) {
            context.isolateSequences.emplace_back(i);
            auto &firstSequenceOfParagraph = context.isolateSequences.back();

            for (; i->bidi_class != unicode_bidi_class::B; ++i) {
                if (BidiX9_valid(*i)) {
                    if (context.isolateSequences.back().embedding_level == -1) {
                        context.isolateSequences.back().embedding_level = i->embedding_level;

                    } else if (context.isolateSequences.back().embedding_level != i->embedding_level) {
                        context.isolateSequences.back().last = i;
                        context.isolateSequences.emplace_back(i, context.isolateSequences.back().embedding_level);
                    }
                }
            }
            // Complete the last isolate sequence of a paragraph and include the paragraph separator.
            context.isolateSequences.back().last = ++i;
            context.isolateSequences.back().endOfParagraph = true;

            // All sequences after the first already know which embedding level they belong to (the reason for the
            // creation of the sequence is a different embedding level). But if the first sequence is not set, then
            // use the paragraph's embedding level.
            if (firstSequenceOfParagraph.embedding_level == -1) {
                firstSequenceOfParagraph.embedding_level = paragraph.embedding_level;
            }
        }
    }

    // Also find the start-of-sequence and end-of-sequence direction.
    for (auto i = context.isolateSequences.begin(); i != context.isolateSequences.end(); ++i) {
        if (i->startOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->sos = (i->embedding_level % 2 == 1) ? unicode_bidi_class::R : unicode_bidi_class::L;
        } else {
            ttlet maxEmbeddingLevel = std::max(i->embedding_level, (i-1)->embedding_level);
            i->sos = (maxEmbeddingLevel % 2 == 1) ? unicode_bidi_class::R : unicode_bidi_class::L;
        }
        if (i->endOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->eos = (i->embedding_level % 2 == 1) ? unicode_bidi_class::R : unicode_bidi_class::L;
        } else {
            ttlet maxEmbeddingLevel = std::max(i->embedding_level, (i+1)->embedding_level);
            i->eos = (maxEmbeddingLevel % 2 == 1) ? unicode_bidi_class::R : unicode_bidi_class::L;
        }
    }
}

// W1. Nonspacing marks will take on the direction of previous character, or neutral after isolation chars. 
static void BidiW1(BidiIsolateSequence &sequence) noexcept
{
    auto prevbidi_class = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidi_class) {
        case unicode_bidi_class::RLE:
        case unicode_bidi_class::LRE:
        case unicode_bidi_class::RLO:
        case unicode_bidi_class::LRO:
        case unicode_bidi_class::PDF:
        case unicode_bidi_class::BN: // X9 Ignore
            break;
        case unicode_bidi_class::LRI:
        case unicode_bidi_class::RLI:
        case unicode_bidi_class::FSI:
        case unicode_bidi_class::PDI:
            prevbidi_class = unicode_bidi_class::ON;
            break;
        case unicode_bidi_class::NSM:
            i->bidi_class = prevbidi_class;
            break;
        default:
            prevbidi_class = i->bidi_class;
        }
    }
}

// W2. Convert European-numbers to Arabic numbers if it follows Arabic letters.
static void BidiW2(BidiIsolateSequence &sequence) noexcept
{
    auto lastStrongDirection = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidi_class) {
        case unicode_bidi_class::R:
        case unicode_bidi_class::L:
        case unicode_bidi_class::AL:
            lastStrongDirection = i->bidi_class;
            break;
        case unicode_bidi_class::EN:
            if (lastStrongDirection == unicode_bidi_class::AL) {
                i->bidi_class = unicode_bidi_class::AN;
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
        if (i->bidi_class == unicode_bidi_class::AL) {
            i->bidi_class = unicode_bidi_class::R;
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
            (i-1)->bidi_class == unicode_bidi_class::EN &&
            (i->bidi_class == unicode_bidi_class::ES || i->bidi_class == unicode_bidi_class::CS) &&
            (i+1)->bidi_class == unicode_bidi_class::EN
            ) {
            i->bidi_class = unicode_bidi_class::EN;

        } else if (
            (i-1)->bidi_class == unicode_bidi_class::AN &&
            i->bidi_class == unicode_bidi_class::CS &&
            (i+1)->bidi_class == unicode_bidi_class::AN
            ) {
            i->bidi_class = unicode_bidi_class::AN;
        }
    }
}

// W5. A sequence of European terminators become European-number when adjacent to them.
static void BidiW5(BidiIsolateSequence &sequence) noexcept
{
    bool foundEN = false;
    std::optional<decltype(sequence.first)> firstET = {};

    for (auto i = sequence.first; i != sequence.last; ++i) {
        if (i->bidi_class == unicode_bidi_class::ET) {
            if (foundEN) {
                i->bidi_class = unicode_bidi_class::EN;
            } else if (!firstET) {
                firstET = i;
            }
        } else if (i->bidi_class == unicode_bidi_class::EN) {
            if (firstET) {
                for (auto j = *firstET; j != i; ++j) {
                    j->bidi_class = unicode_bidi_class::EN;
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
            i->bidi_class == unicode_bidi_class::ET ||
            i->bidi_class == unicode_bidi_class::ES ||
            i->bidi_class == unicode_bidi_class::CS
            ) {
            i->bidi_class = unicode_bidi_class::ON;
        }
    }
}

// W7. If European number is preceded by a string L then the number is converted to L.
static void BidiW7(BidiIsolateSequence &sequence) noexcept
{
    auto lastStrongDirection = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidi_class) {
        case unicode_bidi_class::R:
        case unicode_bidi_class::L:
            lastStrongDirection = i->bidi_class;
            break;
        case unicode_bidi_class::EN:
            if (lastStrongDirection == unicode_bidi_class::L) {
                i->bidi_class = unicode_bidi_class::L;
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
    tt_not_implemented();
}

static void BidiN1(BidiIsolateSequence &sequence) noexcept
{
    auto lastDirection = sequence.sos;
    std::optional<decltype(sequence.first)> firstNI;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidi_class) {
        case unicode_bidi_class::B:
        case unicode_bidi_class::S:
        case unicode_bidi_class::WS:
        case unicode_bidi_class::ON:
        case unicode_bidi_class::FSI:
        case unicode_bidi_class::LRI:
        case unicode_bidi_class::RLI:
        case unicode_bidi_class::PDI:
        case unicode_bidi_class::RLE:// X9 Ignore
        case unicode_bidi_class::LRE:// X9 Ignore
        case unicode_bidi_class::RLO:// X9 Ignore
        case unicode_bidi_class::LRO:// X9 Ignore
        case unicode_bidi_class::PDF:// X9 Ignore
        case unicode_bidi_class::BN: // X9 Ignore
            if (!firstNI) {
                firstNI = i;
            }
            break;

        case unicode_bidi_class::L:
            if (firstNI) {
                if (lastDirection == unicode_bidi_class::L) {
                    for (auto j = *firstNI; j != i; ++j) {
                        j->bidi_class = lastDirection;
                    }
                }
                firstNI = {};
            }
            lastDirection = unicode_bidi_class::L;
            break;

        case unicode_bidi_class::R:
        case unicode_bidi_class::AL:
        case unicode_bidi_class::EN: // European Numbers are treated as R.
        case unicode_bidi_class::AN: // Arabic Numbers are treated as R.
            if (firstNI) {
                if (lastDirection == unicode_bidi_class::L) {
                    for (auto j = *firstNI; j != i; ++j) {
                        j->bidi_class = lastDirection;
                    }
                }
                firstNI = {};
            }
            lastDirection = unicode_bidi_class::R;
            break;

        default:
            tt_no_default();
        }
    }

    if (firstNI) {
        if (lastDirection == sequence.eos) {
            for (auto j = *firstNI; j != sequence.last; ++j) {
                j->bidi_class = lastDirection;
            }
        }
        firstNI = {};
    }
}

// N2. remaining NI get embedding level direction.
static void BidiN2(BidiIsolateSequence &sequence) noexcept
{
    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidi_class) {
        case unicode_bidi_class::B:
        case unicode_bidi_class::S:
        case unicode_bidi_class::WS:
        case unicode_bidi_class::ON:
        case unicode_bidi_class::FSI:
        case unicode_bidi_class::LRI:
        case unicode_bidi_class::RLI:
        case unicode_bidi_class::PDI:
            i->bidi_class = (sequence.embedding_level % 2 == 0) ? unicode_bidi_class::L : unicode_bidi_class::R;
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
        if (character.embedding_level % 2 == 0) {
            // I1.
            switch (character.bidi_class) {
            case unicode_bidi_class::R:
                ++character.embedding_level;
                break;
            case unicode_bidi_class::AN:
            case unicode_bidi_class::EN:
                character.embedding_level += 2;
                break;
            default:;
            }
        } else {
            // I2.
            switch (character.bidi_class) {
            case unicode_bidi_class::L:
            case unicode_bidi_class::AN:
            case unicode_bidi_class::EN:
                ++character.embedding_level;
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
        for (; i->original_bidi_class != unicode_bidi_class::B; ++i) {
            switch (i->original_bidi_class) {
            case unicode_bidi_class::S:
                i->embedding_level = paragraph.embedding_level;
                break;

            case unicode_bidi_class::WS:
            case unicode_bidi_class::FSI:
            case unicode_bidi_class::LRI:
            case unicode_bidi_class::RLI:
            case unicode_bidi_class::PDI:
            case unicode_bidi_class::RLE:// X9 Ignore
            case unicode_bidi_class::LRE:// X9 Ignore
            case unicode_bidi_class::RLO:// X9 Ignore
            case unicode_bidi_class::LRO:// X9 Ignore
            case unicode_bidi_class::PDF:// X9 Ignore
            case unicode_bidi_class::BN: // X9 Ignore
                if (i->codePoint == 0x2028) { // Line Separator.
                    i->embedding_level = paragraph.embedding_level;
                    if (firstWS) {
                        for (auto j = *firstWS; j != i; ++j) {
                            j->embedding_level = paragraph.embedding_level;
                        }
                        firstWS = {};
                    }
                } else {
                    // Make sure X9. characters are categorized to their original class.
                    i->bidi_class = i->original_bidi_class;
                    if (!firstWS) {
                        firstWS = i;
                    }
                }
                break;

            default:
                firstWS = {};
            }
        }

        // unicode_bidi_class::B.
        if (firstWS) {
            for (auto j = *firstWS; j != i; ++j) {
                j->embedding_level = paragraph.embedding_level;
            }
            firstWS = {};
        }
        i->embedding_level = paragraph.embedding_level;
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

static void unicode_bidi_P2(unicode_bidi_paragraph &paragraph)
{
    long long isolate_level = 0;

    for (ttlet &character: paragraph.characters) {
        switch (character.bidi_class) {
        using enum unicode_bidi_class;
        case L:
        case AL:
        case R:
            if (isolate_level == 0) {
                paragraph.bidi_class = character.bidi_class;
                return;
            }
            break;
        case LRI:
        case RLI:
        case FSI:
            ++isolate_level;
            break;
        case PDI:
            --isolate_level;
            break;
        default:;
        }
    }
}

static void unicode_bidi_P3(unicode_bidi_paragraph &paragraph) noexcept
{
    paragraph.embedding_level = static_cast<int8_t>(
        paragraph.bidi_class == unicode_bidi_class::AL ||
        paragraph.bidi_class == unicode_bidi_class::R
    );
}

void unicode_bidi_P1(unicode_bidi_paragraph &paragraph) noexcept
{
    unicode_bidi_P2(paragraph);
    unicode_bidi_P3(paragraph);
    unicode_bidi_X1(paragraph);
    unicode_bidi_X9(paragraph);
    unicode_bidi_X10(paragraph);
}

}
