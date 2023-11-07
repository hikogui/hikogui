// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ucd_bidi_classes.hpp"
#include "ucd_bidi_paired_bracket_types.hpp"
#include "ucd_bidi_mirroring_glyphs.hpp"
#include "ucd_decompositions.hpp"
#include "ucd_general_categories.hpp"
#include "../utility/utility.hpp"
#include "../container/container.hpp"
#include "../algorithm/algorithm.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <exception>
#include <cstddef>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>

hi_export_module(hikogui.unicode.unicode_bidi);


hi_export namespace hi::inline v1 {

struct unicode_bidi_context {
    enum class mode_type : uint8_t { LTR, RTL, auto_LTR, auto_RTL };

    mode_type direction_mode = mode_type::auto_LTR;
    bool enable_mirrored_brackets = true;
    bool enable_line_separator = true;

    constexpr unicode_bidi_context() noexcept = default;
    constexpr unicode_bidi_context(unicode_bidi_context const&) noexcept = default;
    constexpr unicode_bidi_context(unicode_bidi_context&&) noexcept = default;
    constexpr unicode_bidi_context& operator=(unicode_bidi_context const&) noexcept = default;
    constexpr unicode_bidi_context& operator=(unicode_bidi_context&&) noexcept = default;

    constexpr unicode_bidi_context(unicode_bidi_class text_direction) noexcept
    {
        if (text_direction == unicode_bidi_class::L) {
            direction_mode = mode_type::auto_LTR;
        } else if (text_direction == unicode_bidi_class::R) {
            direction_mode = mode_type::auto_RTL;
        } else {
            hi_no_default();
        }
    }
};

namespace detail {

struct unicode_bidi_char_info {
    /** Index from the first character in the original list.
     */
    std::size_t index;

    /** The current code point.
     * The value may change during the execution of the bidi algorithm.
     */
    char32_t code_point;

    /** The embedding level.
     * The value may change during the execution of the bidi algorithm.
     */
    int8_t embedding_level;

    /** Current computed direction of the code-point.
     * The value may change during the execution of the bidi algorithm.
     */
    unicode_bidi_class direction;

    /** The original bidi class of the code-point.
     * The value will NOT change during the execution of the bidi algorithm.
     */
    unicode_bidi_class bidi_class;

    /** The type of bidi-paired-bracket.
     */
    unicode_bidi_paired_bracket_type bracket_type;

    [[nodiscard]] constexpr unicode_bidi_char_info(std::size_t index, char32_t code_point) noexcept
    {
        this->index = index;
        this->code_point = code_point;
        this->embedding_level = 0;
        this->direction = this->bidi_class = ucd_get_bidi_class(code_point);
        this->bracket_type = ucd_get_bidi_paired_bracket_type(code_point);
    }

    /** Constructor for testing to bypass normal initialization.
     * WARNING: DO NOT USE EXCEPT IN UNIT TESTS.
     */
    [[nodiscard]] constexpr unicode_bidi_char_info(std::size_t index, unicode_bidi_class bidi_class) noexcept :
        index(index),
        code_point(U'\ufffd'),
        direction(bidi_class),
        bidi_class(bidi_class),
        bracket_type(unicode_bidi_paired_bracket_type::n),
        embedding_level(0)
    {
    }
};

using unicode_bidi_char_info_vector = std::vector<unicode_bidi_char_info>;
using unicode_bidi_char_info_iterator = unicode_bidi_char_info_vector::iterator;
using unicode_bidi_char_info_const_iterator = unicode_bidi_char_info_vector::const_iterator;

struct unicode_bidi_paragraph {
    using characters_type = std::vector<unicode_bidi_char_info>;

    characters_type characters;

    template<typename... Args>
    constexpr void emplace_character(Args&&...args) noexcept
    {
        characters.emplace_back(std::forward<Args>(args)...);
    }
};

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_P2(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context,
    bool rule_X5c) noexcept;

[[nodiscard]] constexpr int8_t unicode_bidi_P3(unicode_bidi_class paragraph_bidi_class) noexcept;

struct unicode_bidi_stack_element {
    int8_t embedding_level;
    unicode_bidi_class override_status;
    bool isolate_status;
};

class unicode_bidi_level_run {
public:
    using iterator = unicode_bidi_char_info_iterator;
    using const_iterator = unicode_bidi_char_info_const_iterator;
    using reference = std::iterator_traits<iterator>::reference;

    constexpr unicode_bidi_level_run(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}

    [[nodiscard]] constexpr iterator begin() const noexcept
    {
        return _begin;
    }

    [[nodiscard]] constexpr iterator end() const noexcept
    {
        return _end;
    }

    [[nodiscard]] constexpr int8_t embedding_level() const noexcept
    {
        hi_axiom(_begin != _end);
        return _begin->embedding_level;
    }

    [[nodiscard]] constexpr bool ends_with_isolate_initiator() const noexcept
    {
        using enum unicode_bidi_class;

        hi_axiom(_begin != _end);
        hilet& last_char = *(_end - 1);
        return last_char.direction == LRI || last_char.direction == RLI || last_char.direction == FSI;
    }

    [[nodiscard]] constexpr bool starts_with_PDI() const noexcept
    {
        hi_axiom(_begin != _end);
        return _begin->direction == unicode_bidi_class::PDI;
    }

private:
    iterator _begin;
    iterator _end;
};

struct unicode_bidi_isolated_run_sequence {
    using run_container_type = std::vector<unicode_bidi_level_run>;
    using iterator = recursive_iterator<run_container_type::iterator>;
    using const_iterator = recursive_iterator<run_container_type::const_iterator>;

    run_container_type runs;
    unicode_bidi_class sos;
    unicode_bidi_class eos;

    constexpr unicode_bidi_isolated_run_sequence(unicode_bidi_level_run const& rhs) noexcept :
        runs({rhs}), sos(unicode_bidi_class::ON), eos(unicode_bidi_class::ON)
    {
    }

    [[nodiscard]] constexpr auto begin() noexcept
    {
        return recursive_iterator_begin(runs);
    }

    [[nodiscard]] constexpr auto end() noexcept
    {
        return recursive_iterator_end(runs);
    }

    [[nodiscard]] constexpr auto begin() const noexcept
    {
        return recursive_iterator_begin(runs);
    }

    [[nodiscard]] constexpr auto end() const noexcept
    {
        return recursive_iterator_end(runs);
    }

    [[nodiscard]] constexpr friend auto begin(unicode_bidi_isolated_run_sequence& rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] constexpr friend auto begin(unicode_bidi_isolated_run_sequence const& rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] constexpr friend auto end(unicode_bidi_isolated_run_sequence& rhs) noexcept
    {
        return rhs.end();
    }

    [[nodiscard]] constexpr friend auto end(unicode_bidi_isolated_run_sequence const& rhs) noexcept
    {
        return rhs.end();
    }

    constexpr void add_run(unicode_bidi_level_run const& run) noexcept
    {
        runs.push_back(run);
    }

    [[nodiscard]] constexpr int8_t embedding_level() const noexcept
    {
        hi_axiom(not runs.empty());
        return runs.front().embedding_level();
    }

    [[nodiscard]] constexpr unicode_bidi_class embedding_direction() const noexcept
    {
        return (embedding_level() % 2) == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
    }

    [[nodiscard]] constexpr bool ends_with_isolate_initiator() const noexcept
    {
        hi_axiom(not runs.empty());
        return runs.back().ends_with_isolate_initiator();
    }
};

struct unicode_bidi_bracket_pair {
    unicode_bidi_isolated_run_sequence::iterator open;
    unicode_bidi_isolated_run_sequence::iterator close;

    constexpr unicode_bidi_bracket_pair(
        unicode_bidi_isolated_run_sequence::iterator open,
        unicode_bidi_isolated_run_sequence::iterator close) :
        open(std::move(open)), close(std::move(close))
    {
    }

    [[nodiscard]] constexpr friend auto
    operator<=>(unicode_bidi_bracket_pair const& lhs, unicode_bidi_bracket_pair const& rhs) noexcept
    {
        return lhs.open <=> rhs.open;
    }
};

constexpr void unicode_bidi_X1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level,
    unicode_bidi_context const& context) noexcept
{
    using enum unicode_bidi_class;

    constexpr int8_t max_depth = 125;

    auto next_even = [](int8_t x) -> int8_t {
        return (x % 2 == 0) ? x + 2 : x + 1;
    };

    auto next_odd = [](int8_t x) -> int8_t {
        return (x % 2 == 1) ? x + 2 : x + 1;
    };

    long long overflow_isolate_count = 0;
    long long overflow_embedding_count = 0;
    long long valid_isolate_count = 0;

    // X1.
    auto stack = hi::stack<unicode_bidi_stack_element, max_depth + 2>{};
    stack.emplace_back(paragraph_embedding_level, ON, false);

    for (auto it = first; it != last; ++it) {
        hilet current_embedding_level = stack.back().embedding_level;
        hilet current_override_status = stack.back().override_status;
        hilet next_odd_embedding_level = next_odd(current_embedding_level);
        hilet next_even_embedding_level = next_even(current_embedding_level);

        auto RLI_implementation = [&] {
            it->embedding_level = current_embedding_level;
            if (current_override_status != ON) {
                it->direction = current_override_status;
            }

            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_odd_embedding_level, ON, true);
            } else {
                ++overflow_isolate_count;
            }
        };

        auto LRI_implementation = [&] {
            it->embedding_level = current_embedding_level;
            if (current_override_status != ON) {
                it->direction = current_override_status;
            }

            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_even_embedding_level, ON, true);
            } else {
                ++overflow_isolate_count;
            }
        };

        switch (it->direction) {
        case RLE: // X2. Explicit embeddings
            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_odd_embedding_level, ON, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case LRE: // X3. Explicit embeddings
            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_even_embedding_level, ON, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case RLO: // X4. Explicit overrides
            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_odd_embedding_level, R, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case LRO: // X5. Explicit overrides
            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_even_embedding_level, L, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case RLI: // X5a. Isolates
            RLI_implementation();
            break;

        case LRI: // X5b. Isolates
            LRI_implementation();
            break;

        case FSI:
            { // X5c. Isolates
                auto sub_context = context;
                sub_context.direction_mode = unicode_bidi_context::mode_type::auto_LTR;
                hilet sub_paragraph_bidi_class = unicode_bidi_P2(it + 1, last, sub_context, true);
                hilet sub_paragraph_embedding_level = unicode_bidi_P3(sub_paragraph_bidi_class);
                if (sub_paragraph_embedding_level == 0) {
                    LRI_implementation();
                } else {
                    RLI_implementation();
                }
            }
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

            it->embedding_level = stack.back().embedding_level;
            if (stack.back().override_status != ON) {
                it->direction = stack.back().override_status;
            }
            break;

        case PDF: // X7. Terminating Embeddings and Overrides
            if (overflow_isolate_count > 0) {
                // PDF is in scope of isolate, wait until the isolate is terminated.
                ;
            } else if (overflow_embedding_count > 0) {
                --overflow_embedding_count;
            } else if (stack.back().isolate_status == false && stack.size() >= 2) {
                stack.pop_back();
            } else {
                // PDF does not match embedding character.
            }
            break;

        case B: // X8. End of Paragraph
            it->embedding_level = paragraph_embedding_level;
            return;

        case BN: // X6. Ignore
            break;

        default: // X6
            it->embedding_level = current_embedding_level;
            if (current_override_status != ON) {
                it->direction = current_override_status;
            }
        }
    }
}

[[nodiscard]] constexpr unicode_bidi_char_info_iterator
unicode_bidi_X9(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    return std::remove_if(first, last, [](hilet& character) {
        using enum unicode_bidi_class;

        return character.direction == RLE || character.direction == LRE || character.direction == RLO ||
            character.direction == LRO || character.direction == PDF || character.direction == BN;
    });
}

constexpr void unicode_bidi_W1(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto previous_bidi_class = sequence.sos;
    for (auto& char_info : sequence) {
        if (char_info.direction == NSM) {
            switch (previous_bidi_class) {
            case LRI:
            case RLI:
            case FSI:
            case PDI:
                char_info.direction = ON;
                break;
            default:
                char_info.direction = previous_bidi_class;
                break;
            }
        }

        previous_bidi_class = char_info.direction;
    }
}

constexpr void unicode_bidi_W2(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto& char_info : sequence) {
        switch (char_info.direction) {
        case R:
        case L:
        case AL:
            last_strong_direction = char_info.direction;
            break;
        case EN:
            if (last_strong_direction == AL) {
                char_info.direction = AN;
            }
            break;
        default:;
        }
    }
}

constexpr void unicode_bidi_W3(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto& char_info : sequence) {
        if (char_info.direction == AL) {
            char_info.direction = R;
        }
    }
}

constexpr void unicode_bidi_W4(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    unicode_bidi_char_info *back1 = nullptr;
    unicode_bidi_char_info *back2 = nullptr;
    for (auto& char_info : sequence) {
        if (char_info.direction == EN && back2 != nullptr && back2->direction == EN && back1 != nullptr &&
            (back1->direction == ES || back1->direction == CS)) {
            back1->direction = EN;
        }
        if (char_info.direction == AN && back2 != nullptr && back2->direction == AN && back1 != nullptr &&
            back1->direction == CS) {
            back1->direction = AN;
        }

        back2 = std::exchange(back1, &char_info);
    }
}

constexpr void unicode_bidi_W5(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto ET_start = end(sequence);
    auto starts_with_EN = false;

    for (auto it = begin(sequence); it != end(sequence); ++it) {
        auto& char_info = *it;

        switch (char_info.direction) {
        case ET:
            if (starts_with_EN) {
                char_info.direction = EN;
            } else if (ET_start == end(sequence)) {
                ET_start = it;
            }
            break;

        case EN:
            starts_with_EN = true;
            if (ET_start != end(sequence)) {
                for (auto jt = ET_start; jt != it; ++jt) {
                    jt->direction = EN;
                }
                ET_start = end(sequence);
            }
            break;

        default:
            starts_with_EN = false;
            ET_start = end(sequence);
        }
    }
}

constexpr void unicode_bidi_W6(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto& char_info : sequence) {
        if (char_info.direction == ET || char_info.direction == ES || char_info.direction == CS) {
            char_info.direction = ON;
        }
    }
}

constexpr void unicode_bidi_W7(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto& char_info : sequence) {
        switch (char_info.direction) {
        case R:
        case L:
            last_strong_direction = char_info.direction;
            break;
        case EN:
            if (last_strong_direction == L) {
                char_info.direction = L;
            }
            break;
        default:;
        }
    }
}

constexpr std::vector<unicode_bidi_bracket_pair> unicode_bidi_BD16(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    struct bracket_start {
        unicode_bidi_isolated_run_sequence::iterator it;
        char32_t mirrored_bracket;
    };

    using enum unicode_bidi_class;

    auto pairs = std::vector<unicode_bidi_bracket_pair>{};
    auto stack = hi::stack<bracket_start, 63>{};

    for (auto it = begin(isolated_run_sequence); it != end(isolated_run_sequence); ++it) {
        if (it->direction == ON) {
            switch (it->bracket_type) {
            case unicode_bidi_paired_bracket_type::o:
                if (stack.full()) {
                    // Stop processing
                    std::sort(pairs.begin(), pairs.end());
                    return pairs;

                } else {
                    // If there is a canonical equivalent of the opening bracket, find it's mirrored glyph
                    // to compare with the closing bracket.
                    auto mirrored_glyph = ucd_get_bidi_mirroring_glyph(it->code_point);
                    if (hilet canonical_equivalent = ucd_get_decomposition(it->code_point).canonical_equivalent()) {
                        hi_axiom(ucd_get_bidi_paired_bracket_type(*canonical_equivalent) == unicode_bidi_paired_bracket_type::o);

                        mirrored_glyph = ucd_get_bidi_mirroring_glyph(*canonical_equivalent);
                    }

                    stack.emplace_back(it, mirrored_glyph);
                }
                break;

            case unicode_bidi_paired_bracket_type::c:
                {
                    hilet canonical_equivalent = ucd_get_decomposition(it->code_point).canonical_equivalent();
                    auto jt = stack.end();
                    while (jt != stack.begin()) {
                        --jt;

                        if (jt->mirrored_bracket == it->code_point or
                            (canonical_equivalent and jt->mirrored_bracket == *canonical_equivalent)) {
                            pairs.emplace_back(jt->it, it);
                            stack.pop_back(jt);
                            break;
                        }
                    }
                }
                break;

            default:;
            }
        }
    }

    std::sort(pairs.begin(), pairs.end());
    return pairs;
}

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_N0_strong(unicode_bidi_class direction)
{
    using enum unicode_bidi_class;

    switch (direction) {
    case L:
        return L;
    case R:
    case EN:
    case AN:
        return R;
    default:
        return ON;
    }
}

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_N0_preceding_strong_type(
    unicode_bidi_isolated_run_sequence& isolated_run_sequence,
    unicode_bidi_isolated_run_sequence::iterator const& open_bracket) noexcept
{
    using enum unicode_bidi_class;

    auto it = open_bracket;
    while (it != begin(isolated_run_sequence)) {
        --it;

        if (hilet direction = unicode_bidi_N0_strong(it->direction); direction != ON) {
            return direction;
        }
    }

    return isolated_run_sequence.sos;
}

[[nodiscard]] constexpr unicode_bidi_class
unicode_bidi_N0_enclosed_strong_type(unicode_bidi_bracket_pair const& pair, unicode_bidi_class embedding_direction) noexcept
{
    using enum unicode_bidi_class;

    auto opposite_direction = ON;
    for (auto it = pair.open + 1; it != pair.close; ++it) {
        hilet direction = unicode_bidi_N0_strong(it->direction);
        if (direction == ON) {
            continue;
        }
        if (direction == embedding_direction) {
            return direction;
        }
        opposite_direction = direction;
    }

    return opposite_direction;
}

constexpr void unicode_bidi_N0(unicode_bidi_isolated_run_sequence& isolated_run_sequence, unicode_bidi_context const& context)
{
    using enum unicode_bidi_class;

    if (not context.enable_mirrored_brackets) {
        return;
    }

    auto bracket_pairs = unicode_bidi_BD16(isolated_run_sequence);
    hilet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto& pair : bracket_pairs) {
        auto pair_direction = unicode_bidi_N0_enclosed_strong_type(pair, embedding_direction);

        if (pair_direction == ON) {
            continue;
        }

        if (pair_direction != embedding_direction) {
            pair_direction = unicode_bidi_N0_preceding_strong_type(isolated_run_sequence, pair.open);

            if (pair_direction == embedding_direction || pair_direction == ON) {
                pair_direction = embedding_direction;
            }
        }

        pair.open->direction = pair_direction;
        pair.close->direction = pair_direction;

        for (auto it = pair.open + 1; it != pair.close; ++it) {
            if (it->bidi_class != NSM) {
                break;
            }
            it->direction = pair_direction;
        }

        for (auto it = pair.close + 1; it != end(isolated_run_sequence); ++it) {
            if (it->bidi_class != NSM) {
                break;
            }
            it->direction = pair_direction;
        }
    }
}

constexpr void unicode_bidi_N1(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    using enum unicode_bidi_class;

    auto direction_before_NI = isolated_run_sequence.sos;
    auto first_NI = end(isolated_run_sequence);

    for (auto it = begin(isolated_run_sequence); it != end(isolated_run_sequence); ++it) {
        hilet& char_info = *it;
        if (first_NI != end(isolated_run_sequence)) {
            if (!is_NI(char_info.direction)) {
                hilet direction_after_NI = (it->direction == EN || it->direction == AN) ? R : it->direction;

                if ((direction_before_NI == L || direction_before_NI == R) && direction_before_NI == direction_after_NI) {
                    std::for_each(first_NI, it, [direction_before_NI](auto& item) {
                        item.direction = direction_before_NI;
                    });
                }

                first_NI = end(isolated_run_sequence);
                direction_before_NI = direction_after_NI;
            }

        } else if (is_NI(char_info.direction)) {
            first_NI = it;
        } else {
            direction_before_NI = (it->direction == EN || it->direction == AN) ? R : it->direction;
        }
    }

    if (first_NI != end(isolated_run_sequence) && direction_before_NI == isolated_run_sequence.eos) {
        std::for_each(first_NI, end(isolated_run_sequence), [direction_before_NI](auto& item) {
            item.direction = direction_before_NI;
        });
    }
}

constexpr void unicode_bidi_N2(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    hilet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto& char_info : isolated_run_sequence) {
        if (is_NI(char_info.direction)) {
            char_info.direction = embedding_direction;
        }
    }
}

constexpr void unicode_bidi_I1_I2(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    using enum unicode_bidi_class;

    for (auto& char_info : isolated_run_sequence) {
        if ((char_info.embedding_level % 2) == 0) {
            // I1
            if (char_info.direction == R) {
                char_info.embedding_level += 1;
            } else if (char_info.direction == AN || char_info.direction == EN) {
                char_info.embedding_level += 2;
            }
        } else {
            // I2
            if (char_info.direction == L || char_info.direction == AN || char_info.direction == EN) {
                char_info.embedding_level += 1;
            }
        }
    }
}

constexpr std::vector<unicode_bidi_level_run>
unicode_bidi_BD7(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    std::vector<unicode_bidi_level_run> level_runs;

    auto embedding_level = int8_t{0};
    auto run_start = first;
    for (auto it = first; it != last; ++it) {
        if (it == first) {
            embedding_level = it->embedding_level;

        } else if (it->embedding_level != embedding_level) {
            embedding_level = it->embedding_level;

            level_runs.emplace_back(run_start, it);
            run_start = it;
        }
    }
    if (run_start != last) {
        level_runs.emplace_back(run_start, last);
    }

    return level_runs;
}

[[nodiscard]] constexpr std::vector<unicode_bidi_isolated_run_sequence>
unicode_bidi_BD13(std::vector<unicode_bidi_level_run> level_runs) noexcept
{
    std::vector<unicode_bidi_isolated_run_sequence> r;

    std::reverse(begin(level_runs), end(level_runs));
    while (!level_runs.empty()) {
        auto isolated_run_sequence = unicode_bidi_isolated_run_sequence(level_runs.back());
        level_runs.pop_back();

        while (isolated_run_sequence.ends_with_isolate_initiator() && !level_runs.empty()) {
            // Search for matching PDI in the run_levels. This should have the same embedding level.
            auto isolation_level = 1;
            for (auto it = std::rbegin(level_runs); it != std::rend(level_runs); ++it) {
                if (it->starts_with_PDI() && --isolation_level == 0) {
                    hi_axiom(it->embedding_level() == isolated_run_sequence.embedding_level());
                    isolated_run_sequence.add_run(*it);
                    level_runs.erase(std::next(it).base());
                    break;
                }
                if (it->ends_with_isolate_initiator()) {
                    ++isolation_level;
                }
            }

            if (isolation_level != 0) {
                // No PDI that matches the isolate initiator of this isolated run sequence.
                break;
            }
        }

        r.push_back(std::move(isolated_run_sequence));
    }

    return r;
}

[[nodiscard]] constexpr std::pair<unicode_bidi_class, unicode_bidi_class> unicode_bidi_X10_sos_eos(
    unicode_bidi_isolated_run_sequence& isolated_run_sequence,
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    if (begin(isolated_run_sequence) != end(isolated_run_sequence)) {
        // The calculations on the iterator for last_char_it is required because
        // calling child() on an end iterator is undefined behavior.
        hilet first_char_it = begin(isolated_run_sequence).child();
        hilet last_char_it = (end(isolated_run_sequence) - 1).child() + 1;

        hilet has_char_before = first_char_it != first;
        hilet has_char_after = last_char_it != last;

        hilet start_embedding_level = std::max(
            isolated_run_sequence.embedding_level(),
            has_char_before ? (first_char_it - 1)->embedding_level : paragraph_embedding_level);
        hilet end_embedding_level = std::max(
            isolated_run_sequence.embedding_level(),
            has_char_after && !isolated_run_sequence.ends_with_isolate_initiator() ? last_char_it->embedding_level :
                                                                                     paragraph_embedding_level);

        return {
            (start_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L,
            (end_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L};
    } else {
        return {
            (paragraph_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L,
            (paragraph_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L};
    }
}

constexpr void unicode_bidi_X10(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level,
    unicode_bidi_context const& context) noexcept
{
    auto isolated_run_sequence_set = unicode_bidi_BD13(unicode_bidi_BD7(first, last));

    // All sos and eos calculations must be done before W*, N*, I* parts are executed,
    // since those will change the embedding levels of the characters outside of the
    // current isolated_run_sequence that the unicode_bidi_X10_sos_eos() depends on.
    for (auto& isolated_run_sequence : isolated_run_sequence_set) {
        std::tie(isolated_run_sequence.sos, isolated_run_sequence.eos) =
            unicode_bidi_X10_sos_eos(isolated_run_sequence, first, last, paragraph_embedding_level);
    }

    for (auto& isolated_run_sequence : isolated_run_sequence_set) {
        unicode_bidi_W1(isolated_run_sequence);
        unicode_bidi_W2(isolated_run_sequence);
        unicode_bidi_W3(isolated_run_sequence);
        unicode_bidi_W4(isolated_run_sequence);
        unicode_bidi_W5(isolated_run_sequence);
        unicode_bidi_W6(isolated_run_sequence);
        unicode_bidi_W7(isolated_run_sequence);
        unicode_bidi_N0(isolated_run_sequence, context);
        unicode_bidi_N1(isolated_run_sequence);
        unicode_bidi_N2(isolated_run_sequence);
        unicode_bidi_I1_I2(isolated_run_sequence);
    }
}

[[nodiscard]] constexpr std::pair<int8_t, int8_t> unicode_bidi_L1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    using enum unicode_bidi_class;

    auto lowest_odd = std::numeric_limits<int8_t>::max();
    auto highest = paragraph_embedding_level;
    auto preceding_is_segment = true;

    auto it = last;
    while (it != first) {
        --it;

        auto bidi_class = it->bidi_class;

        if (bidi_class == B || bidi_class == S) {
            it->embedding_level = paragraph_embedding_level;
            preceding_is_segment = true;

        } else if (preceding_is_segment && (bidi_class == WS || is_isolate_formatter(bidi_class))) {
            it->embedding_level = paragraph_embedding_level;
            preceding_is_segment = true;

        } else {
            highest = std::max(highest, it->embedding_level);
            if ((it->embedding_level % 2) == 1) {
                lowest_odd = std::min(lowest_odd, it->embedding_level);
            }

            preceding_is_segment = false;
        }
    }

    if ((paragraph_embedding_level % 2) == 1) {
        lowest_odd = std::min(lowest_odd, paragraph_embedding_level);
    }

    if (lowest_odd > highest) {
        // If there where no odd levels below the highest level
        if (highest % 2 == 1) {
            // We need to reverse at least once if the highest was odd.
            lowest_odd = highest;
        } else {
            // We need to reverse at least twice if the highest was even.
            // This may yield a negative lowest_odd.
            lowest_odd = highest - 1;
        }
    }

    return {lowest_odd, highest};
}

constexpr void unicode_bidi_L2(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t lowest_odd,
    int8_t highest) noexcept
{
    for (int8_t level = highest; level >= lowest_odd; --level) {
        auto sequence_start = last;
        for (auto it = first; it != last; ++it) {
            if (sequence_start == last) {
                if (it->embedding_level >= level) {
                    sequence_start = it;
                }
            } else if (it->embedding_level < level) {
                std::reverse(sequence_start, it);
                sequence_start = last;
            }
        }
        if (sequence_start != last) {
            std::reverse(sequence_start, last);
        }
    }
}

constexpr void unicode_bidi_L3(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept {}

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_P2_default(unicode_bidi_context const& context) noexcept
{
    if (context.direction_mode == unicode_bidi_context::mode_type::auto_LTR) {
        return unicode_bidi_class::L;
    } else if (context.direction_mode == unicode_bidi_context::mode_type::auto_RTL) {
        return unicode_bidi_class::R;
    } else {
        hi_no_default();
    }
}

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_P2(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context,
    bool rule_X5c) noexcept
{
    using enum unicode_bidi_class;

    if (context.direction_mode == unicode_bidi_context::mode_type::LTR) {
        return unicode_bidi_class::L;
    } else if (context.direction_mode == unicode_bidi_context::mode_type::RTL) {
        return unicode_bidi_class::R;
    }

    long long isolate_level = 0;
    for (auto it = first; it != last; ++it) {
        switch (it->direction) {
        case L:
        case AL:
        case R:
            if (isolate_level == 0) {
                return it->direction;
            }
            break;
        case LRI:
        case RLI:
        case FSI:
            ++isolate_level;
            break;
        case PDI:
            if (isolate_level > 0) {
                --isolate_level;
            } else if (rule_X5c) {
                // End at the matching PDI, when recursing for rule X5c.
                return unicode_bidi_P2_default(context);
            }
            break;
        default:;
        }
    }
    return unicode_bidi_P2_default(context);
}

[[nodiscard]] constexpr int8_t unicode_bidi_P3(unicode_bidi_class paragraph_bidi_class) noexcept
{
    return wide_cast<int8_t>(paragraph_bidi_class == unicode_bidi_class::AL or paragraph_bidi_class == unicode_bidi_class::R);
}

constexpr void unicode_bidi_P1_line(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level,
    unicode_bidi_context const& context) noexcept
{
    hilet[lowest_odd, highest] = unicode_bidi_L1(first, last, paragraph_embedding_level);
    unicode_bidi_L2(first, last, lowest_odd, highest);
    unicode_bidi_L3(first, last);
    // L4 is delayed after the original array has been shuffled.
}

[[nodiscard]] constexpr std::pair<int8_t, unicode_bidi_class> unicode_bidi_P2_P3(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context) noexcept
{
    hilet default_paragraph_direction = unicode_bidi_P2(first, last, context, false);
    hilet paragraph_embedding_level = unicode_bidi_P3(default_paragraph_direction);
    hilet paragraph_direction = paragraph_embedding_level % 2 == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
    return {paragraph_embedding_level, paragraph_direction};
}

[[nodiscard]] constexpr std::pair<unicode_bidi_char_info_iterator, unicode_bidi_class> unicode_bidi_P1_paragraph(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context) noexcept
{
    hilet[paragraph_embedding_level, paragraph_direction] = unicode_bidi_P2_P3(first, last, context);

    unicode_bidi_X1(first, last, paragraph_embedding_level, context);
    last = unicode_bidi_X9(first, last);
    unicode_bidi_X10(first, last, paragraph_embedding_level, context);

    auto line_begin = first;
    for (auto it = first; it != last; ++it) {
        hilet general_category = ucd_get_general_category(it->code_point);
        if (context.enable_line_separator and general_category == unicode_general_category::Zl) {
            hilet line_end = it + 1;
            unicode_bidi_P1_line(line_begin, line_end, paragraph_embedding_level, context);
            line_begin = line_end;
        }
    }

    if (line_begin != last) {
        unicode_bidi_P1_line(line_begin, last, paragraph_embedding_level, context);
    }

    return {last, paragraph_direction};
}

[[nodiscard]] constexpr std::pair<unicode_bidi_char_info_iterator, std::vector<unicode_bidi_class>> unicode_bidi_P1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context) noexcept
{
    auto it = first;
    auto paragraph_begin = it;
    auto paragraph_directions = std::vector<unicode_bidi_class>{};
    while (it != last) {
        if (it->direction == unicode_bidi_class::B) {
            hilet paragraph_end = it + 1;
            hilet[new_paragraph_end, paragraph_bidi_class] = unicode_bidi_P1_paragraph(paragraph_begin, paragraph_end, context);
            paragraph_directions.push_back(paragraph_bidi_class);

            // Move the removed items of the paragraph to the end of the text.
            std::rotate(new_paragraph_end, paragraph_end, last);
            last -= std::distance(new_paragraph_end, paragraph_end);

            paragraph_begin = it = new_paragraph_end;
        } else {
            ++it;
        }
    }

    if (paragraph_begin != last) {
        hilet[new_paragraph_end, paragraph_bidi_class] = unicode_bidi_P1_paragraph(paragraph_begin, last, context);
        paragraph_directions.push_back(paragraph_bidi_class);
        last = new_paragraph_end;
    }

    return {last, std::move(paragraph_directions)};
}

template<typename OutputIt, typename SetCodePoint, typename SetTextDirection>
constexpr void unicode_bidi_L4(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    OutputIt output_it,
    SetCodePoint set_code_point,
    SetTextDirection set_text_direction) noexcept
{
    for (auto it = first; it != last; ++it, ++output_it) {
        hilet text_direction = it->embedding_level % 2 == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
        set_text_direction(*output_it, text_direction);
        if (it->direction == unicode_bidi_class::R and it->bracket_type != unicode_bidi_paired_bracket_type::n) {
            set_code_point(*output_it, ucd_get_bidi_mirroring_glyph(it->code_point));
        }
    }
}

} // namespace detail

/** Reorder a given range of characters based on the unicode_bidi algorithm.
 * This algorithm will:
 *  - Reorder the list of items
 *  - Change code points to a mirrored version.
 *  - Remove code points which controls the bidirectional algorithm
 *
 * It is likely that an application has the characters grouped as graphemes
 * and is accompanied with the original index and possible other information.
 * The `get_char` function returns the first code-point of a grapheme. The
 * `set_char` function is used when the code-point needs to be replaced with
 * a mirrored version.
 *
 * The bidirectional algorithm will work correctly with either a list of code points
 * or a list of first-code-point-of-graphemes.
 *
 * @param first The first iterator
 * @param last The last iterator
 * @param get_code_point A function to get the character of an item.
 * @param set_code_point A function to set the character in an item.
 * @param set_text_direction A function to set the text direction in an item.
 * @param context The context/configuration to use for the bidi-algorithm.
 * @return Iterator pointing one beyond the last element, the writing direction for each paragraph.
 */
template<typename It, typename GetCodePoint, typename SetCodePoint, typename SetTextDirection>
constexpr std::pair<It, std::vector<unicode_bidi_class>> unicode_bidi(
    It first,
    It last,
    GetCodePoint get_code_point,
    SetCodePoint set_code_point,
    SetTextDirection set_text_direction,
    unicode_bidi_context const& context = {})
{
    auto proxy = detail::unicode_bidi_char_info_vector{};
    proxy.reserve(std::distance(first, last));

    std::size_t index = 0;
    for (auto it = first; it != last; ++it) {
        proxy.emplace_back(index++, get_code_point(*it));
    }

    auto [proxy_last, paragraph_directions] = detail::unicode_bidi_P1(begin(proxy), end(proxy), context);
    last = shuffle_by_index(first, last, begin(proxy), proxy_last, [](hilet& item) {
        return item.index;
    });

    detail::unicode_bidi_L4(
        begin(proxy),
        proxy_last,
        first,
        std::forward<SetCodePoint>(set_code_point),
        std::forward<SetTextDirection>(set_text_direction));
    return {last, std::move(paragraph_directions)};
}

/** Get the unicode bidi direction for the first paragraph and context.
 *
 * @param first The first iterator
 * @param last The last iterator
 * @param get_code_point A function to get the code-point of an item.
 * @param context The context/configuration to use for the bidi-algorithm.
 * @return Iterator pointing one beyond the last element, the writing direction for each paragraph.
 */
template<typename It, typename GetCodePoint>
[[nodiscard]] constexpr unicode_bidi_class
unicode_bidi_direction(It first, It last, GetCodePoint get_code_point, unicode_bidi_context const& context = {})
{
    auto proxy = detail::unicode_bidi_char_info_vector{};
    proxy.reserve(std::distance(first, last));

    std::size_t index = 0;
    for (auto it = first; it != last; ++it) {
        proxy.emplace_back(index++, get_code_point(*it));
        if (proxy.back().direction == unicode_bidi_class::B) {
            // Break early when end-of-paragraph symbol is found.
            break;
        }
    }

    return detail::unicode_bidi_P2_P3(begin(proxy), end(proxy), context).second;
}

/** Removes control characters which will not survive the bidi-algorithm.
 *
 * All RLE, LRE, RLO, LRO, PDF, and BN characters are removed.
 *
 * @post Control characters between the first and last iterators are moved to the end.
 * @param first The first character.
 * @param last One beyond the last character.
 * @param code_point_func A function returning the code-point of the character.
 * @return The iterator one beyond the last character that is valid.
 */
template<typename It, typename EndIt, typename CodePointFunc>
constexpr It unicode_bidi_control_filter(It first, EndIt last, CodePointFunc const& code_point_func)
{
    return std::remove_if(first, last, [&](hilet& item) {
        hilet code_point = code_point_func(item);
        hilet bidi_class = ucd_get_bidi_class(code_point);
        return is_control(bidi_class);
    });
}

} // namespace hi::inline v1
