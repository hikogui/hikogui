// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_bidi.hpp"
#include "../stack.hpp"
#include "../recursive_iterator.hpp"
#include <algorithm>

namespace hi::inline v1::detail {

[[nodiscard]] static unicode_bidi_class unicode_bidi_P2(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context,
    bool rule_X5c) noexcept;

[[nodiscard]] static int8_t unicode_bidi_P3(unicode_bidi_class paragraph_bidi_class) noexcept;

struct unicode_bidi_stack_element {
    int8_t embedding_level;
    unicode_bidi_class override_status;
    bool isolate_status;

    unicode_bidi_stack_element(int8_t embedding_level, unicode_bidi_class override_status, bool isolate_status) noexcept :
        embedding_level(embedding_level), override_status(override_status), isolate_status(isolate_status)
    {
    }
};

class unicode_bidi_level_run {
public:
    using iterator = unicode_bidi_char_info_iterator;
    using const_iterator = unicode_bidi_char_info_const_iterator;
    using reference = std::iterator_traits<iterator>::reference;

    unicode_bidi_level_run(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}

    [[nodiscard]] iterator begin() const noexcept
    {
        return _begin;
    }

    [[nodiscard]] iterator end() const noexcept
    {
        return _end;
    }

    [[nodiscard]] int8_t embedding_level() const noexcept
    {
        hi_axiom(_begin != _end);
        return _begin->embedding_level;
    }

    [[nodiscard]] bool ends_with_isolate_initiator() const noexcept
    {
        using enum unicode_bidi_class;

        hi_axiom(_begin != _end);
        hilet& last_char = *(_end - 1);
        return last_char.direction == LRI || last_char.direction == RLI || last_char.direction == FSI;
    }

    [[nodiscard]] bool starts_with_PDI() const noexcept
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

    unicode_bidi_isolated_run_sequence(unicode_bidi_level_run const& rhs) noexcept :
        runs({rhs}), sos(unicode_bidi_class::unknown), eos(unicode_bidi_class::unknown)
    {
    }

    [[nodiscard]] auto begin() noexcept
    {
        return recursive_iterator_begin(runs);
    }

    [[nodiscard]] auto end() noexcept
    {
        return recursive_iterator_end(runs);
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return recursive_iterator_begin(runs);
    }

    [[nodiscard]] auto end() const noexcept
    {
        return recursive_iterator_end(runs);
    }

    [[nodiscard]] friend auto begin(unicode_bidi_isolated_run_sequence& rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] friend auto begin(unicode_bidi_isolated_run_sequence const& rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] friend auto end(unicode_bidi_isolated_run_sequence& rhs) noexcept
    {
        return rhs.end();
    }

    [[nodiscard]] friend auto end(unicode_bidi_isolated_run_sequence const& rhs) noexcept
    {
        return rhs.end();
    }

    void add_run(unicode_bidi_level_run const& run) noexcept
    {
        runs.push_back(run);
    }

    [[nodiscard]] int8_t embedding_level() const noexcept
    {
        hi_axiom(not runs.empty());
        return runs.front().embedding_level();
    }

    [[nodiscard]] unicode_bidi_class embedding_direction() const noexcept
    {
        return (embedding_level() % 2) == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
    }

    [[nodiscard]] bool ends_with_isolate_initiator() const noexcept
    {
        hi_axiom(not runs.empty());
        return runs.back().ends_with_isolate_initiator();
    }
};

struct unicode_bidi_bracket_pair {
    unicode_bidi_isolated_run_sequence::iterator open;
    unicode_bidi_isolated_run_sequence::iterator close;

    unicode_bidi_bracket_pair(
        unicode_bidi_isolated_run_sequence::iterator open,
        unicode_bidi_isolated_run_sequence::iterator close) :
        open(std::move(open)), close(std::move(close))
    {
    }

    [[nodiscard]] friend auto operator<=>(unicode_bidi_bracket_pair const& lhs, unicode_bidi_bracket_pair const& rhs) noexcept
    {
        return lhs.open <=> rhs.open;
    }
};

[[nodiscard]] static int8_t next_even(int8_t x) noexcept
{
    return (x % 2 == 0) ? x + 2 : x + 1;
}

[[nodiscard]] static int8_t next_odd(int8_t x) noexcept
{
    return (x % 2 == 1) ? x + 2 : x + 1;
}

static void unicode_bidi_X1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level,
    unicode_bidi_context const& context) noexcept
{
    using enum unicode_bidi_class;

    constexpr int8_t max_depth = 125;

    long long overflow_isolate_count = 0;
    long long overflow_embedding_count = 0;
    long long valid_isolate_count = 0;

    // X1.
    auto stack = hi::stack<unicode_bidi_stack_element, max_depth + 2>{{paragraph_embedding_level, unknown, false}};

    for (auto it = first; it != last; ++it) {
        hilet current_embedding_level = stack.back().embedding_level;
        hilet current_override_status = stack.back().override_status;
        hilet next_odd_embedding_level = next_odd(current_embedding_level);
        hilet next_even_embedding_level = next_even(current_embedding_level);

        switch (it->direction) {
        case RLE: // X2. Explicit embeddings
            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_odd_embedding_level, unknown, false);
            } else if (overflow_isolate_count == 0) {
                ++overflow_embedding_count;
            }
            break;

        case LRE: // X3. Explicit embeddings
            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                stack.emplace_back(next_even_embedding_level, unknown, false);
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
RLI:
            it->embedding_level = current_embedding_level;
            if (current_override_status != unknown) {
                it->direction = current_override_status;
            }

            if (next_odd_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_odd_embedding_level, unknown, true);
            } else {
                ++overflow_isolate_count;
            }
            break;

        case LRI: // X5b. Isolates
LRI:
            it->embedding_level = current_embedding_level;
            if (current_override_status != unknown) {
                it->direction = current_override_status;
            }

            if (next_even_embedding_level <= max_depth && overflow_isolate_count == 0 && overflow_embedding_count == 0) {
                ++valid_isolate_count;
                stack.emplace_back(next_even_embedding_level, unknown, true);
            } else {
                ++overflow_isolate_count;
            }
            break;

        case FSI: { // X5c. Isolates
            auto sub_context = context;
            sub_context.direction_mode = unicode_bidi_context::mode_type::auto_LTR;
            hilet sub_paragraph_bidi_class = unicode_bidi_P2(it + 1, last, sub_context, true);
            hilet sub_paragraph_embedding_level = unicode_bidi_P3(sub_paragraph_bidi_class);
            if (sub_paragraph_embedding_level == 0) {
                goto LRI;
            } else {
                goto RLI;
            }
        } break;

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
            if (stack.back().override_status != unknown) {
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
            if (current_override_status != unknown) {
                it->direction = current_override_status;
            }
        }
    }
}

[[nodiscard]] static unicode_bidi_char_info_iterator
unicode_bidi_X9(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    return std::remove_if(first, last, [](hilet& character) {
        using enum unicode_bidi_class;

        return character.direction == RLE || character.direction == LRE || character.direction == RLO ||
            character.direction == LRO || character.direction == PDF || character.direction == BN;
    });
}

static void unicode_bidi_W1(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto previous_bidi_class = sequence.sos;
    for (auto& char_info : sequence) {
        if (char_info.direction == NSM) {
            switch (previous_bidi_class) {
            case LRI:
            case RLI:
            case FSI:
            case PDI: char_info.direction = ON; break;
            default: char_info.direction = previous_bidi_class; break;
            }
        }

        previous_bidi_class = char_info.direction;
    }
}

static void unicode_bidi_W2(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto& char_info : sequence) {
        switch (char_info.direction) {
        case R:
        case L:
        case AL: last_strong_direction = char_info.direction; break;
        case EN:
            if (last_strong_direction == AL) {
                char_info.direction = AN;
            }
            break;
        default:;
        }
    }
}

static void unicode_bidi_W3(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto& char_info : sequence) {
        if (char_info.direction == AL) {
            char_info.direction = R;
        }
    }
}

static void unicode_bidi_W4(unicode_bidi_isolated_run_sequence& sequence) noexcept
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

static void unicode_bidi_W5(unicode_bidi_isolated_run_sequence& sequence) noexcept
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

        default: starts_with_EN = false; ET_start = end(sequence);
        }
    }
}

static void unicode_bidi_W6(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto& char_info : sequence) {
        if (char_info.direction == ET || char_info.direction == ES || char_info.direction == CS) {
            char_info.direction = ON;
        }
    }
}

static void unicode_bidi_W7(unicode_bidi_isolated_run_sequence& sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto& char_info : sequence) {
        switch (char_info.direction) {
        case R:
        case L: last_strong_direction = char_info.direction; break;
        case EN:
            if (last_strong_direction == L) {
                char_info.direction = L;
            }
            break;
        default:;
        }
    }
}

static std::vector<unicode_bidi_bracket_pair> unicode_bidi_BD16(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    struct bracket_start {
        unicode_bidi_isolated_run_sequence::iterator it;
        char32_t mirrored_bracket;

        bracket_start(unicode_bidi_isolated_run_sequence::iterator it, char32_t mirrored_bracket) noexcept :
            it(std::move(it)), mirrored_bracket(mirrored_bracket)
        {
        }
    };

    using enum unicode_bidi_class;

    auto pairs = std::vector<unicode_bidi_bracket_pair>{};
    auto stack = hi::stack<bracket_start, 63>{};

    for (auto it = begin(isolated_run_sequence); it != end(isolated_run_sequence); ++it) {
        if (it->direction == ON) {
            switch (it->description->bidi_bracket_type()) {
            case unicode_bidi_bracket_type::o:
                if (stack.full()) {
                    goto stop_processing;

                } else {
                    // If there is a canonical equivalent of the opening bracket, find it's mirrored glyph
                    // to compare with the closing bracket.
                    auto mirrored_glyph = it->description->bidi_mirroring_glyph();
                    auto canonical_equivalent = it->description->canonical_equivalent();
                    if (canonical_equivalent != U'\uffff') {
                        hilet& canonical_equivalent_description = unicode_description::find(canonical_equivalent);
                        hi_axiom(canonical_equivalent_description.bidi_bracket_type() == unicode_bidi_bracket_type::o);
                        mirrored_glyph = canonical_equivalent_description.bidi_mirroring_glyph();
                    }

                    stack.emplace_back(it, mirrored_glyph);
                }
                break;

            case unicode_bidi_bracket_type::c:
                for (auto jt = stack.end() - 1; jt >= stack.begin(); --jt) {
                    if (jt->mirrored_bracket == it->code_point ||
                        jt->mirrored_bracket == it->description->canonical_equivalent()) {
                        pairs.emplace_back(jt->it, it);
                        stack.pop_back(jt);
                        break;
                    }
                }
                break;

            default:;
            }
        }
    }

stop_processing:
    std::sort(begin(pairs), end(pairs));
    return pairs;
}

[[nodiscard]] static unicode_bidi_class unicode_bidi_N0_strong(unicode_bidi_class direction)
{
    using enum unicode_bidi_class;

    switch (direction) {
    case L: return L;
    case R:
    case EN:
    case AN: return R;
    default: return ON;
    }
}

[[nodiscard]] static unicode_bidi_class unicode_bidi_N0_preceding_strong_type(
    unicode_bidi_isolated_run_sequence& isolated_run_sequence,
    unicode_bidi_isolated_run_sequence::iterator& open_bracket) noexcept
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

[[nodiscard]] static unicode_bidi_class
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

static void unicode_bidi_N0(unicode_bidi_isolated_run_sequence& isolated_run_sequence, unicode_bidi_context const& context)
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
            if (it->description->bidi_class() != NSM) {
                break;
            }
            it->direction = pair_direction;
        }

        for (auto it = pair.close + 1; it != end(isolated_run_sequence); ++it) {
            if (it->description->bidi_class() != NSM) {
                break;
            }
            it->direction = pair_direction;
        }
    }
}

static void unicode_bidi_N1(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
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

static void unicode_bidi_N2(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
{
    hilet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto& char_info : isolated_run_sequence) {
        if (is_NI(char_info.direction)) {
            char_info.direction = embedding_direction;
        }
    }
}

static void unicode_bidi_I1_I2(unicode_bidi_isolated_run_sequence& isolated_run_sequence)
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

static std::vector<unicode_bidi_level_run>
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

[[nodiscard]] static std::vector<unicode_bidi_isolated_run_sequence>
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

[[nodiscard]] static std::pair<unicode_bidi_class, unicode_bidi_class> unicode_bidi_X10_sos_eos(
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

static void unicode_bidi_X10(
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

[[nodiscard]] static std::pair<int8_t, int8_t> unicode_bidi_L1(
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

static void unicode_bidi_L2(
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

static void unicode_bidi_L3(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept {}

[[nodiscard]] static unicode_bidi_class unicode_bidi_P2_default(unicode_bidi_context const& context) noexcept
{
    if (context.direction_mode == unicode_bidi_context::mode_type::auto_LTR) {
        return unicode_bidi_class::L;
    } else if (context.direction_mode == unicode_bidi_context::mode_type::auto_RTL) {
        return unicode_bidi_class::R;
    } else {
        hi_no_default();
    }
}

[[nodiscard]] static unicode_bidi_class unicode_bidi_P2(
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
        case FSI: ++isolate_level; break;
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

[[nodiscard]] static int8_t unicode_bidi_P3(unicode_bidi_class paragraph_bidi_class) noexcept
{
    return static_cast<int8_t>(paragraph_bidi_class == unicode_bidi_class::AL || paragraph_bidi_class == unicode_bidi_class::R);
}

static void unicode_bidi_P1_line(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level,
    unicode_bidi_context const& context) noexcept
{
    hilet[lowest_odd, highest] = unicode_bidi_L1(first, last, paragraph_embedding_level);
    unicode_bidi_L2(first, last, lowest_odd, highest);
    unicode_bidi_L3(first, last);
    // L4 is delayed after the original array has been shuffled.

    if (context.move_lf_and_ps_to_end_of_line) {
        hilet it = std::find_if(first, last, [](hilet& item) {
            return item.description->general_category() == unicode_general_category::Zl or
                item.description->general_category() == unicode_general_category::Zp;
        });
        if (it != last) {
            it->direction = unicode_bidi_class::L;
            std::rotate(it, it + 1, last);
        }
    }
}

[[nodiscard]] static std::pair<unicode_bidi_char_info_iterator, unicode_bidi_class> unicode_bidi_P1_paragraph(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const& context) noexcept
{
    hilet default_paragraph_direction = unicode_bidi_P2(first, last, context, false);
    hilet paragraph_embedding_level = unicode_bidi_P3(default_paragraph_direction);
    hilet paragraph_direction = paragraph_embedding_level % 2 == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;

    unicode_bidi_X1(first, last, paragraph_embedding_level, context);
    last = unicode_bidi_X9(first, last);
    unicode_bidi_X10(first, last, paragraph_embedding_level, context);

    auto line_begin = first;
    for (auto it = first; it != last; ++it) {
        if (context.enable_line_separator and it->description->general_category() == unicode_general_category::Zl) {
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

[[nodiscard]] std::pair<unicode_bidi_char_info_iterator, std::vector<unicode_bidi_class>> unicode_bidi_P1(
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

} // namespace hi::inline v1::detail
