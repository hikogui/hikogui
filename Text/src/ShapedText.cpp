// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Text/globals.hpp"

namespace TTauri::Text {

[[nodiscard]] static std::vector<AttributedGrapheme> makeAttributedGraphemeVector(gstring const &text, TextStyle const &style) noexcept
{
    std::vector<AttributedGrapheme> r;
    r.reserve(ssize(text));

    int index = 0;
    for (let &grapheme: text) {
        r.emplace_back(grapheme, style, index++);
    }

    if (ssize(text) == 0 || text.back() != '\n') {
        r.emplace_back(Grapheme{'\n'}, style, index++);
    }

    return r;
}

struct BidiParData {
    int8_t level;

    BidiParData(int8_t level) noexcept : level(level) {}
};

struct BidiIsolateSequence {
    std::vector<AttributedGrapheme>::iterator first;
    std::vector<AttributedGrapheme>::iterator last;

    bool startOfParagraph;
    bool endOfParagraph;
    BidirectionalClass sos;
    BidirectionalClass eos;
    int8_t embeddingLevel;

    BidiIsolateSequence(std::vector<AttributedGrapheme>::iterator first, int8_t embeddingLevel=-1) noexcept :
        first(first), last(first), startOfParagraph(embeddingLevel == -1), endOfParagraph(false),
        sos(BidirectionalClass::Unknown), eos(BidirectionalClass::Unknown), embeddingLevel(embeddingLevel) {}
};

struct BidiContext {
    std::vector<BidiParData> parData;
    std::vector<BidiIsolateSequence> isolateSequences;
};

static void BidiP1_P3(BidiContext &context, std::vector<AttributedGrapheme> &text) noexcept
{
    ssize_t logicalIndex = 0;
    auto parBidiClass = BidirectionalClass::Unknown;
    auto isolateLevel = 0;
    for (auto &ag: text) {
        // First remember the logical index before reordering the graphemes.
        ag.logicalIndex = logicalIndex++;

        // Classify each grapheme based on the first code-point.
        ag.bidiClass = Text_globals->unicode_data->getBidirectionalClass(ag.grapheme[0]);
        ag.charClass = to_GeneralCharacterClass(ag.bidiClass);

        // P2. Find first L, AR or R bidi-class, ignoring isolated sections.
        switch (ag.bidiClass) {
        case BidirectionalClass::L:
        case BidirectionalClass::AL:
        case BidirectionalClass::R:
            if (isolateLevel == 0 && parBidiClass != BidirectionalClass::Unknown) {
                parBidiClass = ag.bidiClass;
            }
            break;
        case BidirectionalClass::LRI:
        case BidirectionalClass::RLI:
        case BidirectionalClass::FSI:
            ++isolateLevel;
            break;
        case BidirectionalClass::PDI:
            --isolateLevel;
            break;
        case BidirectionalClass::B:
            // P3. AL or R means par level = 1, others level = 0.
            context.parData.emplace_back(
                static_cast<int>(parBidiClass == BidirectionalClass::AL || parBidiClass == BidirectionalClass::R)
            );
            isolateLevel = 0;
            parBidiClass = BidirectionalClass::Unknown;
            break;
        }
    }

    // P1. Split text in paragraphs, the paragraph delimiter comes at the end of the previous paragraph.
    // We will not physically split the paragraphs, but make sure the text does end in a paragraph.
    // We can not ourselves add the paragraph separator, because we do not know the text-style of an empty text.
    ttauri_assume(ssize(text) > 0 && text.back().bidiClass == BidirectionalClass::B);
}


struct BidiStackElement {
    int8_t embeddingLevel;
    BidirectionalClass overrideStatus;
    bool isolateStatus;
};

[[nodiscard]] static int8_t nextEven(int8_t x) noexcept {
    return (x % 2 == 0) ? x + 2 : x + 1;
}

[[nodiscard]] static int8_t nextOdd(int8_t x) noexcept {
    return (x % 2 == 1) ? x + 2 : x + 1;
}

static void BidiX1_X8(BidiContext &context, std::vector<AttributedGrapheme> &text) noexcept
{
    constexpr int8_t max_depth = 125;

    std::vector<BidiStackElement> stack;
    auto overflowIsolateCount = 0;
    auto overflowEmbeddingCount = 0;
    auto validIsolateCount = 0;

    auto i = text.begin();
    for (let &parData : context.parData) {
        // X1.
        stack.clear();
        stack.emplace_back(parData.level, BidirectionalClass::Unknown, false);
        overflowIsolateCount = 0;
        overflowEmbeddingCount = 0;
        validIsolateCount = 0;

        for (; i->bidiClass != BidirectionalClass::B; ++i) {
            ttauri_assume(i != text.cend());

            let currentEmbedingLevel = stack.back().embeddingLevel;
            let currentOverrideStatus = stack.back().overrideStatus;
            let nextOddEmbedingLevel = nextOdd(currentEmbedingLevel);
            let nextEvenEmbedingLevel = nextEven(currentEmbedingLevel);

            switch (i->bidiClass) {
            case BidirectionalClass::RLE: // X2. Explicit embeddings
                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextOddEmbedingLevel, BidirectionalClass::Unknown, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidirectionalClass::LRE: // X3. Explicit embeddings
                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextEvenEmbedingLevel, BidirectionalClass::Unknown, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidirectionalClass::RLO: // X4. Explicit overrides
                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextOddEmbedingLevel, BidirectionalClass::R, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidirectionalClass::LRO: // X5. Explicit overrides
                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    stack.emplace_back(nextEvenEmbedingLevel, BidirectionalClass::L, false);
                } else if (overflowIsolateCount == 0) {
                    ++overflowEmbeddingCount;
                }
                break;

            case BidirectionalClass::RLI: // X5a. Isolates
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidirectionalClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }

                if (nextOddEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    ++validIsolateCount;
                    stack.emplace_back(nextOddEmbedingLevel, BidirectionalClass::Unknown, true);
                } else  {
                    ++overflowIsolateCount;
                }
                break;

            case BidirectionalClass::LRI: // X5b. Isolates
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidirectionalClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }

                if (nextEvenEmbedingLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0) {
                    ++validIsolateCount;
                    stack.emplace_back(nextEvenEmbedingLevel, BidirectionalClass::Unknown, true);
                } else  {
                    ++overflowIsolateCount;
                }
                break;

            case BidirectionalClass::FSI:
                // X5c. Determine paragraph-level of text up to matching PDI.
                // if 0 goto BidirectionalClass::LRI, if 1 goto BidirectionalClass::RLI.
                not_implemented;
                break;

            case BidirectionalClass::PDI: // X6a. Terminating Isolates
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
                if (stack.back().overrideStatus != BidirectionalClass::Unknown) {
                    i->bidiClass = stack.back().overrideStatus;
                }

            case BidirectionalClass::PDF: // X7. Terminating Embeddings and Overrides
                if (overflowIsolateCount > 0) {
                    // PDF is in scope of isolate, wait until the isolate is terminated.
                    ;
                } else if (overflowEmbeddingCount > 0) {
                    --overflowEmbeddingCount;
                } else if (stack.back().isolateStatus == false && ssize(stack) >= 2) {
                    stack.pop_back();
                } else {
                    // PDF does not match embedding character.
                }

            case BidirectionalClass::B: // X8. End of Paragraph
                // This should never appear in this loop.
                no_default;

            case BidirectionalClass::BN: // X6. Ignore
                break;

            default: // X6
                i->embeddingLevel = currentEmbedingLevel;
                if (currentOverrideStatus != BidirectionalClass::Unknown) {
                    i->bidiClass = currentOverrideStatus;
                }
            }
        }

        ++i; // Skip over paragraph-separator.
    }
}

static bool BidiX9_valid(BidirectionalClass const &x) noexcept
{
    switch (x) {
    case BidirectionalClass::RLE:
    case BidirectionalClass::LRE:
    case BidirectionalClass::RLO:
    case BidirectionalClass::LRO:
    case BidirectionalClass::PDF:
    case BidirectionalClass::BN:
        return false;
    default:
        return true;
    }
}

static bool BidiX9_valid(AttributedGrapheme const &x) noexcept
{
    return BidiX9_valid(x.bidiClass);
}

static void BidiX10(BidiContext &context, std::vector<AttributedGrapheme> &text) noexcept
{
    // X10. Find all sequences matching the same embedded level
    auto i = text.begin();
    for (let &parData : context.parData) {
        context.isolateSequences.emplace_back(i);
        auto &firstSequenceOfParagraph = context.isolateSequences.back();

        for (; i->bidiClass != BidirectionalClass::B; ++i) {
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
            firstSequenceOfParagraph.embeddingLevel = parData.level;
        }
    }

    // Also find the start-of-sequence and end-of-sequence direction.
    for (auto i = context.isolateSequences.begin(); i != context.isolateSequences.end(); ++i) {
        if (i->startOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->sos = (i->embeddingLevel % 2 == 1) ? BidirectionalClass::R : BidirectionalClass::L;
        } else {
            let maxEmbeddingLevel = std::max(i->embeddingLevel, (i-1)->embeddingLevel);
            i->sos = (maxEmbeddingLevel % 2 == 1) ? BidirectionalClass::R : BidirectionalClass::L;
        }
        if (i->endOfParagraph) {
            // level is higher or equal to paragraph-level.
            i->eos = (i->embeddingLevel % 2 == 1) ? BidirectionalClass::R : BidirectionalClass::L;
        } else {
            let maxEmbeddingLevel = std::max(i->embeddingLevel, (i+1)->embeddingLevel);
            i->eos = (maxEmbeddingLevel % 2 == 1) ? BidirectionalClass::R : BidirectionalClass::L;
        }
    }
}

// W1. Nonspacing marks will take on the direction of previous character, or neutral after isolation chars. 
static void BidiW1(BidiIsolateSequence &sequence) noexcept
{
    auto prevBidiClass = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidirectionalClass::RLE:
        case BidirectionalClass::LRE:
        case BidirectionalClass::RLO:
        case BidirectionalClass::LRO:
        case BidirectionalClass::PDF:
        case BidirectionalClass::BN: // X9 Ignore
            break;
        case BidirectionalClass::LRI:
        case BidirectionalClass::RLI:
        case BidirectionalClass::FSI:
        case BidirectionalClass::PDI:
            prevBidiClass = BidirectionalClass::ON;
            break;
        case BidirectionalClass::NSM:
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
        case BidirectionalClass::R:
        case BidirectionalClass::L:
        case BidirectionalClass::AL:
            lastStrongDirection = i->bidiClass;
            break;
        case BidirectionalClass::EN:
            if (lastStrongDirection == BidirectionalClass::AL) {
                i->bidiClass = BidirectionalClass::AN;
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
        if (i->bidiClass == BidirectionalClass::AL) {
            i->bidiClass = BidirectionalClass::R;
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
            (i-1)->bidiClass == BidirectionalClass::EN &&
            (i->bidiClass == BidirectionalClass::ES || i->bidiClass == BidirectionalClass::CS) &&
            (i+1)->bidiClass == BidirectionalClass::EN
        ) {
            i->bidiClass = BidirectionalClass::EN;

        } else if (
            (i-1)->bidiClass == BidirectionalClass::AN &&
            i->bidiClass == BidirectionalClass::CS &&
            (i+1)->bidiClass == BidirectionalClass::AN
        ) {
            i->bidiClass = BidirectionalClass::AN;
        }
    }
}

// W5. A sequence of European terminators become European-number when adjacent to them.
static void BidiW5(BidiIsolateSequence &sequence) noexcept
{
    bool foundEN = false;
    std::optional<decltype(sequence.first)> firstET = {};

    for (auto i = sequence.first; i != sequence.last; ++i) {
        if (i->bidiClass == BidirectionalClass::ET) {
            if (foundEN) {
                i->bidiClass = BidirectionalClass::EN;
            } else if (!firstET) {
                firstET = i;
            }
        } else if (i->bidiClass == BidirectionalClass::EN) {
            if (firstET) {
                for (auto j = *firstET; j != i; ++j) {
                    j->bidiClass = BidirectionalClass::EN;
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
            i->bidiClass == BidirectionalClass::ET ||
            i->bidiClass == BidirectionalClass::ES ||
            i->bidiClass == BidirectionalClass::CS
        ) {
            i->bidiClass = BidirectionalClass::ON;
        }
    }
}

// W7. If European number is preceded by a string L then the number is converted to L.
static void BidiW7(BidiIsolateSequence &sequence) noexcept
{
    auto lastStrongDirection = sequence.sos;

    for (auto i = sequence.first; i != sequence.last; ++i) {
        switch (i->bidiClass) {
        case BidirectionalClass::R:
        case BidirectionalClass::L:
            lastStrongDirection = i->bidiClass;
            break;
        case BidirectionalClass::EN:
            if (lastStrongDirection == BidirectionalClass::L) {
                i->bidiClass = BidirectionalClass::L;
            }
            break;
        default:;
        }
    }
}

static void BidiW(BidiContext &context, std::vector<AttributedGrapheme> &text) noexcept
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

static void BidiAlgorithm(std::vector<AttributedGrapheme> &text) noexcept
{
    auto context = BidiContext{};

    BidiP1_P3(context, text);
    BidiX1_X8(context, text);
    BidiX10(context, text);
    BidiW(context, text);
}

[[nodiscard]] static std::vector<AttributedGlyph> graphemes_to_glyphs(std::vector<AttributedGrapheme> const &text) noexcept
{
    // The end-of-paragraph (linefeed) must end text.
    ttauri_assume(ssize(text) >= 1 && text.back().grapheme == '\n');

    std::vector<AttributedGlyph> glyphs;
    glyphs.reserve(size(text));

    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    for (let &ag: text) {
        let font_id = font_book.find_font(ag.style.family_id, ag.style.variant);

        // The end-of-paragraph is represented by a space glyph, which is usefull for
        // producing a correct cursor at an empty line at the end of a paragraph.
        let g = (ag.grapheme == '\n') ? Grapheme{0} : ag.grapheme;
        glyphs.emplace_back(ag, font_book.find_glyph(font_id, g));
    }

    return glyphs;
}

static void morph_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{

}

static void load_metrics_for_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{
    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    auto font_id = FontID{};
    Font const *font = nullptr;
    auto next_i = glyphs.begin();
    for (auto i = next_i; i != glyphs.end(); i = next_i) {
        next_i = i + 1;

        // Get a pointer to the actual font, when the font_id of the glyph changes.
        let new_font_id = i->glyphs.font_id();
        if (font_id != new_font_id) {
            font_id = new_font_id;
            font = &(font_book.get_font(font_id));
        }
        ttauri_assume(font != nullptr);
        let next_is_same_font = (next_i != glyphs.end()) && (next_i->glyphs.font_id() == font_id);

        // Get the metrics of the main glyph.
        let first_glyph = i->glyphs.front();
        let next_glyph = next_is_same_font ? next_i->glyphs.front() : GlyphID{};

        if (!font->loadGlyphMetrics(first_glyph, i->metrics, next_glyph)) {
            LOG_ERROR("Could not load metrics for glyph {} in font {} - {}", static_cast<int>(i->glyphs[0]), font->description.family_name, font->description.sub_family_name);
            // failed to load metrics. Switch to glyph zero and load again.
            i->glyphs.clear();
            i->glyphs.set_font_id(font_id);
            i->glyphs += GlyphID{0};
            if (!font->loadGlyphMetrics(i->glyphs[0], i->metrics)) {
                // Using null-metrics when even the null-glyph can not be found.
                LOG_ERROR("Could not load metrics for null-glyph in font {} - {}", font->description.family_name, font->description.sub_family_name);
            }
        }

        // Scale the metrics according to font-size of this glyph.
        i->metrics *= i->style.size;

        // XXX merge the bounding box of combining glyphs in the metrics.
    }
}

[[nodiscard]] static std::vector<AttributedGlyphLine> make_lines(std::vector<AttributedGlyph> &glyphs, float maximum_width) noexcept
{
    std::vector<AttributedGlyphLine> lines;

    float width = 0.0f;
    auto start_of_line = glyphs.begin();
    auto end_of_word = glyphs.begin();
    float end_of_word_width = 0.0f;

    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        if (i->charClass == GeneralCharacterClass::WhiteSpace) {
            // When a line is created the whitespace should be appended to the
            // end-of-line so cursor position can still be calculated correctly.
            end_of_word = i + 1;
            // For the width we do not count the whitespace after the word.
            end_of_word_width = width;
        }

        if (i->charClass != GeneralCharacterClass::ParagraphSeparator) {
            // Do not include the width of the endOfParagraph marker.
            width += i->metrics.advance.x();
        }

        if ((width > maximum_width) && (start_of_line != end_of_word)) {
            // Line is to long, and exists of at least a full word.
            lines.emplace_back(start_of_line, end_of_word, end_of_word_width);

            // Skip back in the for-loop.
            i = end_of_word;
            start_of_line = end_of_word;
            end_of_word = end_of_word;
            end_of_word_width = 0.0f;
        }
    }

    if (start_of_line != glyphs.end()) {
        // Any whitespace at the end of the paragraph should be kept in the last line.
        lines.emplace_back(start_of_line, glyphs.end(), width);
    }

    return lines;
}

/** Calculate the size of the text.
 * @return The extent of the text and the base line position of the middle line.
 */
[[nodiscard]] static std::pair<vec,float> calculate_text_size(std::vector<AttributedGlyphLine> const &lines) noexcept
{
    auto size = vec{0.0f, 0.0f};

    if (ssize(lines) == 0) {
        return {size, 0.0f};
    }

    // Top of first line.
    size = vec{
        lines.front().width,
        lines.front().lineGap + lines.front().ascender
    };
    auto base_line = size.height();

    auto nr_lines = ssize(lines);
    auto half_nr_lines = nr_lines / 2;
    auto odd_nr_lines = nr_lines % 2 == 1;
    for (ssize_t i = 1; i != nr_lines; ++i) {
        size = vec{
            std::max(size.width(), lines[i].width),
            size.height() + lines[i-1].descender + std::max(lines[i-1].lineGap, lines[i].lineGap) + lines[i].ascender
        };

        if (i == half_nr_lines) {
            if (odd_nr_lines) {
                // Take the base line of the middle line.
                base_line = size.height();
            } else {
                // Take the base line of the line-gap between the two middle lines.
                base_line = size.height() - lines[i].ascender - std::max(lines[i-1].lineGap, lines[i].lineGap) * 0.5f;
            }
        }
    }

    // Bottom of last line.
    size.height(size.height() + lines.back().descender + lines.back().lineGap);

    return {size, size.height() - base_line};
}

static void position_glyphs(std::vector<AttributedGlyphLine> &lines, HorizontalAlignment alignment, vec extent) noexcept
{
    // Draw lines from the top-to-down.
    float y = extent.height();
    for (ssize_t i = 0; i != ssize(lines); ++i) {
        auto &line = lines[i];
        if (i == 0) {
            y -= line.lineGap + line.ascender;
        } else {
            let &prev_line = lines[i-1];
            y -= prev_line.descender + std::max(prev_line.lineGap, line.lineGap) + line.ascender;
        }

        float x = 0.0f;
        if (alignment == HorizontalAlignment::Left) {
            x = 0.0f;
        } else if (alignment == HorizontalAlignment::Right) {
            x = extent.width() - line.width;
        } else if (alignment == HorizontalAlignment::Center) {
            x = extent.width() * 0.5f - line.width * 0.5f;
        } else {
            no_default;
        }

        auto position = vec(x, y);
        for (auto &glyph: line) {
            glyph.position = position;
            position += glyph.metrics.advance;
        }
    }
}

/** Shape the text.
* The given text is in logical-order; the order in which humans write text.
* The resulting glyphs are in left-to-right display order.
*
* The following operations are executed on the text by the `shape_text()` function:
*  - Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
*  - Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
*  - Morph attributed-glyphs using the Font's morph algorithm.
*  - Calculate advance for each attributed-glyph using the Font's advance and kern algorithms.
*  - Add line-breaks to the text to fit within the maximum-width.
*  - Calculate actual size of the text
*  - Align the text within the given extent size.
*
* @param text The text to be shaped.
* @param alignment How the text should be horizontally-aligned inside the maximum_width.
* @param max_width Maximum width that the text should flow into.
* @return size of the resulting text, shaped text.
*/
[[nodiscard]] static std::pair<vec,std::vector<AttributedGlyphLine>> shape_text(
    std::vector<AttributedGrapheme> text,
    HorizontalAlignment alignment,
    float maximum_width=std::numeric_limits<float>::max()) noexcept
{
    std::vector<AttributedGlyph> attributed_glyphs;

    // Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
    bidi_algorithm(text);

    // Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
    auto glyphs = graphemes_to_glyphs(text);

    // Morph attributed-glyphs using the Font's morph algorithm.
    morph_glyphs(glyphs);

    // Load metric for each attributed-glyph using many of the Font's tables.
    load_metrics_for_glyphs(glyphs);

    // Split the text up in lines, based on line-feeds and line-wrapping.
    auto lines = make_lines(glyphs, maximum_width);

    // Calculate actual size of the box, no smaller than the minimum_size.
    let [extent, base] = calculate_text_size(lines);

    // Align the text within the actual box size.
    position_glyphs(lines, alignment, extent);

    return {extent, lines};
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, HorizontalAlignment alignment, float maximum_width) noexcept
{
    ttauri_assume((alignment == HorizontalAlignment::Left) || (maximum_width < std::numeric_limits<float>::max()));
    std::tie(this->extent, this->lines) = shape_text(text, alignment, maximum_width);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), alignment, maximum_width) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(to_gstring(text), style, alignment, maximum_width) {}


[[nodiscard]] ShapedText::const_iterator ShapedText::find(ssize_t index) const noexcept
{
    return std::find_if(cbegin(), cend(), [=](let &x) {
        return x.containsLogicalIndex(index);
    });
}

[[nodiscard]] rect ShapedText::rectangleOfGrapheme(ssize_t index) const noexcept
{
    let i = find(index);

    // The shaped text will always end with a paragraph separator '\n'.
    // Therefor even if the index points beyond the last character, it will still
    // be on the paragraph separator.
    ttauri_assume(i != cend());

    // We need the line to figure out the ascender/descender height of the line so that
    // the caret does not jump up and down as we walk the text.
    let line_i = i.parent();

    // This is a ligature.
    // The position is inside a ligature.
    // Place the cursor proportional inside the ligature, based on the font-metrics.
    let ligature_index = numeric_cast<int>(i->logicalIndex - index);
    let ligature_advance_left = i->metrics.advanceForGrapheme(ligature_index);
    let ligature_advance_right = i->metrics.advanceForGrapheme(ligature_index + 1);

    let ligature_position_left = i->position + ligature_advance_left;
    let ligature_position_right = i->position + ligature_advance_right;

    let p1 = ligature_position_left - vec(0.0, line_i->descender);
    let p2 = ligature_position_right + vec(0.0, line_i->ascender);
    return rect::p1p2(p1, p2);
}

[[nodiscard]] rect ShapedText::leftToRightCaret(ssize_t index, bool insertMode) const noexcept
{
    auto r = rectangleOfGrapheme(index);

    if (insertMode) {
        // Change width to a single pixel.
        r.width(1.0);
    }

    return r;
}

[[nodiscard]] std::vector<rect> ShapedText::selectionRectangles(ssize_t first, ssize_t last) const noexcept
{
    auto r = std::vector<rect>{};

    for (ssize_t i = first; i != last; ++i) {
        auto newRect = rectangleOfGrapheme(i);
        if (ssize(r) > 0 && overlaps(r.back(), newRect)) {
            r.back() |= newRect;
        } else {
            r.push_back(newRect);
        }
    }

    return r;
}


[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharAtCoordinate(vec coordinate) const noexcept
{
    for (let &line: lines) {
        auto i = line.find(coordinate);
        if (i == line.cend()) {
            continue;
        }

        if ((i + 1) == line.cend()) {
            // This character is the end of line, or end of paragraph.
            return i->logicalIndex;

        } else {
            let newLogicalIndex = i->relativeIndexAtCoordinate(coordinate);
            if (newLogicalIndex < 0) {
                return i->logicalIndex;
            } else if (newLogicalIndex >= i->graphemeCount) {
                // Closer to the next glyph.
                return (i+1)->logicalIndex;
            } else {
                return i->logicalIndex + newLogicalIndex;
            }
        }
    }
    return {};
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharOnTheLeft(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);
    if (i == cbegin()) {
        return {};
    } else if (logicalIndex != i->logicalIndex) {
        // Go left inside a ligature.
        return logicalIndex - 1;
    } else {
        --i;
        return i->logicalIndex + i->graphemeCount - 1;
    }
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharOnTheRight(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);
    if (i->isParagraphSeparator()) {
        return {};
    } else if (logicalIndex < (i->logicalIndex + i->graphemeCount)) {
        // Go right inside a ligature.
        return logicalIndex + 1;
    } else {
        ++i;
        return i->logicalIndex;
    }
}

/** Return the index at the left side of a word
*/
[[nodiscard]] std::pair<ssize_t,ssize_t> ShapedText::indicesOfWord(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);

    // If the position is the paragraph separator, adjust to one glyph to the left.
    if (i->isParagraphSeparator()) {
        if (i == cbegin()) {
            return {0, 0};
        } else {
            --i;
        }
    }

    if (i->isWhiteSpace()) {
        if (i == cbegin()) {
            // Whitespace at start of line is counted as a word.
            ;
        } else if (!(i-1)->isWhiteSpace()) {
            // The glyph on the left is not a white space, means we need to select the word on the left
            --i;
        } else {
            // Double white space select all the white spaces in a row.
            ;
        }
    }

    // Expand the word to left and right.
    auto [s, e] = bifind_cluster(cbegin(), cend(), i, [](let &x) {
        return x.isWord() ? 0 : x.isWhiteSpace() ? 1 : 2;
    });

    ttauri_assume(e != i);
    --e;
    return {s->logicalIndex, e->logicalIndex + e->graphemeCount};
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfWordOnTheLeft(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    let [s, e] = indicesOfWord(logicalIndex);
    
    // If the cursor was already on that edge, find the edges of the previous word.
    if (s == logicalIndex) {
        if (let tmp = indexOfCharOnTheLeft(s)) {
            let [s2, e2] = indicesOfWord(*tmp);
            return s2;
        }
    }
    return s;
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfWordOnTheRight(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    let [s, e] = indicesOfWord(logicalIndex);

    // If the cursor was already on that edge, find the edges of the next word.
    if (e == logicalIndex || find(e)->isWhiteSpace()) {
        if (let tmp = indexOfCharOnTheRight(e)) {
            let [s2, e2] = indicesOfWord(*tmp);
            return s2 == e ? e2 : s2;
        }       
    }
    return e;
}

[[nodiscard]] Path ShapedText::get_path() const noexcept
{
    Path r;

    if (ssize(*this) == 0) {
        return r;
    }

    for (let &attr_glyph: *this) {
        r += attr_glyph.get_path();
    }
    r.optimizeLayers();

    return r;
}


}