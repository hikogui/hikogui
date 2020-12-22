// Copyright 2020 Pokitec
// All rights reserved.

#include "unicode_bidi.hpp"
#include "../stack.hpp"
#include "../recursive_iterator.hpp"
#include <algorithm>

namespace tt::detail {

[[nodiscard]] static unicode_bidi_class
unicode_bidi_P2(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept;
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
        tt_axiom(_begin != _end);
        return _begin->embedding_level;
    }

    [[nodiscard]] bool ends_with_isolate_initiator() const noexcept
    {
        using enum unicode_bidi_class;

        tt_axiom(_begin != _end);
        ttlet &last_char = *(_end - 1);
        return last_char.direction == LRI || last_char.direction == RLI || last_char.direction == FSI;
    }

    [[nodiscard]] bool starts_with_PDI() const noexcept
    {
        tt_axiom(_begin != _end);
        return _begin->direction == unicode_bidi_class::PDI;
    }

private:
    iterator _begin;
    iterator _end;
};

struct unicode_bidi_isolated_run_sequence {
    using run_container_type = std::vector<unicode_bidi_level_run>;
    using iterator = recursive_iterator<run_container_type>;
    using const_iterator = recursive_iterator<run_container_type const>;

    run_container_type runs;
    unicode_bidi_class sos;
    unicode_bidi_class eos;

    unicode_bidi_isolated_run_sequence(unicode_bidi_level_run const &rhs) noexcept :
        runs({rhs}), sos(unicode_bidi_class::unknown), eos(unicode_bidi_class::unknown)
    {
    }

    auto begin() noexcept
    {
        return recursive_iterator_begin(runs);
    }

    auto end() noexcept
    {
        return recursive_iterator_end(runs);
    }

    auto begin() const noexcept
    {
        return recursive_iterator_begin(runs);
    }

    auto end() const noexcept
    {
        return recursive_iterator_end(runs);
    }

    void add_run(unicode_bidi_level_run const &run) noexcept
    {
        runs.push_back(run);
    }

    [[nodiscard]] int8_t embedding_level() const noexcept
    {
        tt_axiom(!runs.empty());
        return runs.front().embedding_level();
    }

    [[nodiscard]] unicode_bidi_class embedding_direction() const noexcept
    {
        return (embedding_level() % 2) == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
    }

    [[nodiscard]] bool ends_with_isolate_initiator() const noexcept
    {
        tt_axiom(!runs.empty());
        return runs.back().ends_with_isolate_initiator();
    }
};

struct unicode_bidi_bracket_pair {
    size_t first;
    size_t second;
    unicode_bidi_class preceding_strong;
    bool has_inside_strong;
    bool inside_strong_matches_embedding;

    unicode_bidi_class direction = unicode_bidi_class::unknown;

    unicode_bidi_bracket_pair(
        size_t first,
        size_t second,
        unicode_bidi_class preceding_strong,
        bool has_inside_strong,
        bool inside_strong_matches_embedding) :
        first(first),
        second(second),
        preceding_strong(preceding_strong),
        has_inside_strong(has_inside_strong),
        inside_strong_matches_embedding(inside_strong_matches_embedding)
    {
    }

    [[nodiscard]] friend bool operator<(unicode_bidi_bracket_pair const &lhs, unicode_bidi_bracket_pair const &rhs) noexcept
    {
        return lhs.first < rhs.first;
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
    int8_t paragraph_embedding_level) noexcept
{
    using enum unicode_bidi_class;

    constexpr int8_t max_depth = 125;

    long long overflow_isolate_count = 0;
    long long overflow_embedding_count = 0;
    long long valid_isolate_count = 0;

    // X1.
    auto stack = tt::stack<unicode_bidi_stack_element, max_depth + 2>{{paragraph_embedding_level, unknown, false}};

    for (auto it = first; it != last; ++it) {
        ttlet current_embedding_level = stack.back().embedding_level;
        ttlet current_override_status = stack.back().override_status;
        ttlet next_odd_embedding_level = next_odd(current_embedding_level);
        ttlet next_even_embedding_level = next_even(current_embedding_level);

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
            ttlet sub_paragraph_bidi_class = unicode_bidi_P2(it + 1, last);
            ttlet sub_paragraph_embedding_level = unicode_bidi_P3(sub_paragraph_bidi_class);
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
            } else if (stack.back().isolate_status == false && std::ssize(stack) >= 2) {
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
    return std::remove_if(first, last, [](ttlet &character) {
        using enum unicode_bidi_class;

        return character.direction == RLE && character.direction == LRE && character.direction == RLO &&
            character.direction == LRO && character.direction == PDF && character.direction == BN;
    });
}

static void unicode_bidi_W1(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto previous_bidi_class = sequence.sos;
    for (auto &char_info : sequence) {
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

static void unicode_bidi_W2(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto &char_info : sequence) {
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

static void unicode_bidi_W3(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto &char_info : sequence) {
        if (char_info.direction == AL) {
            char_info.direction = R;
        }
    }
}

static void unicode_bidi_W4(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    unicode_bidi_char_info *back1 = nullptr;
    unicode_bidi_char_info *back2 = nullptr;
    for (auto &char_info : sequence) {
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

static void unicode_bidi_W5(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto ETs = std::vector<unicode_bidi_char_info *>{};
    auto previous_bidi_class = sequence.sos;

    for (auto &char_info : sequence) {
        if (char_info.direction == ET) {
            if (previous_bidi_class == EN) {
                char_info.direction = EN;
            } else {
                ETs.push_back(&char_info);
            }

        } else if (char_info.direction == EN) {
            std::for_each(std::begin(ETs), std::end(ETs), [](auto x) {
                x->direction = unicode_bidi_class::EN;
            });
            ETs.clear();

        } else {
            ETs.clear();
        }

        previous_bidi_class = char_info.direction;
    }
}

static void unicode_bidi_W6(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto &char_info : sequence) {
        if (char_info.direction == ET || char_info.direction == ES || char_info.direction == CS) {
            char_info.direction = ON;
        }
    }
}

static void unicode_bidi_W7(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto &char_info : sequence) {
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

static std::vector<unicode_bidi_bracket_pair> unicode_bidi_BD16(unicode_bidi_isolated_run_sequence const &isolated_run_sequence)
{
    struct bracket_start {
        char32_t mirrored_bracket;
        size_t index;
        unicode_bidi_class last_strong;

        bool has_inside_strong = false;
        bool inside_strong_matches_embedding = false;

        bracket_start(char32_t mirrored_bracket, size_t index, unicode_bidi_class last_strong) noexcept :
            mirrored_bracket(mirrored_bracket), index(index), last_strong(last_strong)
        {
        }
    };

    auto stack = tt::stack<bracket_start, 63>{};
    auto pairs = std::vector<unicode_bidi_bracket_pair>{};
    auto last_strong = isolated_run_sequence.sos;
    ttlet embedding_direction = isolated_run_sequence.embedding_direction();

    size_t index = 0;
    for (ttlet &char_info : isolated_run_sequence) {
        if (char_info.description->bidi_bracket_type() == unicode_bidi_bracket_type::o) {
            if (stack.full()) {
                break;
            } else {
                stack.emplace_back(char_info.description->bidi_mirrored_glyph(), index, last_strong);
            }

        } else if (char_info.description->bidi_bracket_type() == unicode_bidi_bracket_type::c) {
            for (auto it = stack.end() - 1; it >= stack.begin(); --it) {
                if (it->mirrored_bracket == char_info.code_point) {
                    pairs.emplace_back(
                        it->index, index, it->last_strong, it->has_inside_strong, it->inside_strong_matches_embedding);
                    stack.pop_back(it);
                    break;
                }
            }
        }

        if (char_info.description->bidi_class() == unicode_bidi_class::L ||
            char_info.description->bidi_class() == unicode_bidi_class::R) {
            last_strong = char_info.description->bidi_class();

            for (auto &item : stack) {
                item.has_inside_strong = true;
                item.inside_strong_matches_embedding |= char_info.description->bidi_class() == embedding_direction;
            }
        }

        ++index;
    }

    std::sort(std::begin(pairs), std::end(pairs));
    return pairs;
}

static void unicode_bidi_N0(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    auto bracket_pairs = unicode_bidi_BD16(isolated_run_sequence);
    ttlet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto &pair : bracket_pairs) {
        if (pair.inside_strong_matches_embedding) {
            // N0.b.
            pair.direction = embedding_direction;

        } else if (pair.has_inside_strong) {
            // N0.c.
            if (pair.preceding_strong != embedding_direction) {
                // B0.c.1.
                pair.direction = pair.preceding_strong;
            } else {
                // B0.c.2.
                pair.direction = embedding_direction;
            }
        } else {
            // B0.d.
            pair.direction = unicode_bidi_class::unknown;
        }
    }

    size_t index = 0;
    bool last_bracket_changed_from_L_to_R = false;
    for (auto &char_info : isolated_run_sequence) {
        if (char_info.description->bidi_bracket_type() == unicode_bidi_bracket_type::o) {
            last_bracket_changed_from_L_to_R = false;

            for (ttlet &pair : bracket_pairs) {
                if (pair.first == index) {
                    if (pair.direction == unicode_bidi_class::L) {
                        char_info.direction = unicode_bidi_class::L;
                    } else if (pair.direction == unicode_bidi_class::R) {
                        if (char_info.direction == unicode_bidi_class::L) {
                            last_bracket_changed_from_L_to_R = true;
                        }
                        char_info.direction = unicode_bidi_class::R;
                    }
                    break;
                }
            }

        } else if (char_info.description->bidi_bracket_type() == unicode_bidi_bracket_type::c) {
            last_bracket_changed_from_L_to_R = false;

            for (ttlet &pair : bracket_pairs) {
                if (pair.second == index) {
                    if (pair.direction == unicode_bidi_class::L) {
                        char_info.direction = unicode_bidi_class::L;
                    } else if (pair.direction == unicode_bidi_class::R) {
                        if (char_info.direction == unicode_bidi_class::L) {
                            last_bracket_changed_from_L_to_R = true;
                        }
                        char_info.direction = unicode_bidi_class::R;
                    }
                    break;
                }
            }

        } else if (char_info.description->bidi_class() == unicode_bidi_class::NSM) {
            if (last_bracket_changed_from_L_to_R) {
                char_info.direction = unicode_bidi_class::R;
            }

            last_bracket_changed_from_L_to_R = false;

        } else {
            last_bracket_changed_from_L_to_R = false;
        }

        ++index;
    }
}

static void unicode_bidi_N1(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    using enum unicode_bidi_class;

    auto prev_direction = isolated_run_sequence.sos;
    auto direction_before_NI = unicode_bidi_class::unknown;
    auto first_NI = std::end(isolated_run_sequence);

    for (auto it = std::begin(isolated_run_sequence); it != std::end(isolated_run_sequence); ++it) {
        auto &char_info = *it;
        if (first_NI != std::end(isolated_run_sequence)) {
            if (!is_NI(char_info.direction)) {
                auto next_it = it + 1;
                auto next_direction =
                    (next_it != std::end(isolated_run_sequence)) ? next_it->direction : isolated_run_sequence.eos;

                if (next_direction == EN || next_direction == AN) {
                    next_direction = R;
                }

                if ((next_direction == L || next_direction == R) && next_direction == direction_before_NI) {
                    std::for_each(first_NI, it, [next_direction](auto &item) {
                        item.direction = next_direction;
                    });
                }

                first_NI = std::end(isolated_run_sequence);
            }

        } else if (is_NI(char_info.direction)) {
            first_NI = it;
            direction_before_NI = (prev_direction == EN || prev_direction == AN) ? R : prev_direction;
        }

        prev_direction = char_info.direction;
    }
}

static void unicode_bidi_N2(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    ttlet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto &char_info : isolated_run_sequence) {
        if (is_NI(char_info.direction)) {
            char_info.direction = embedding_direction;
        }
    }
}

static void unicode_bidi_I1(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    using enum unicode_bidi_class;

    if ((isolated_run_sequence.embedding_level() % 2) == 0) {
        for (auto &char_info : isolated_run_sequence) {
            if (char_info.direction == R) {
                char_info.embedding_level += 1;
            } else if (char_info.direction == AN || char_info.direction == EN) {
                char_info.embedding_level += 2;
            }
        }
    }
}

static void unicode_bidi_I2(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    using enum unicode_bidi_class;

    if ((isolated_run_sequence.embedding_level() % 2) == 1) {
        for (auto &char_info : isolated_run_sequence) {
            if (char_info.direction == L || char_info.direction == AN || char_info.direction == EN) {
                char_info.embedding_level += 1;
            }
        }
    }
}

static void unicode_bidi_X10(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    tt_axiom(first != last);

    // Determine the runs of characters with equal embedding levels.
    std::vector<unicode_bidi_level_run> level_runs;

    auto embedding_level = first->embedding_level;
    auto run_start = first;
    for (auto it = first; it != last; ++it) {
        if (it->embedding_level != embedding_level) {
            level_runs.emplace_back(run_start, it);
            run_start = it;
        }
    }
    level_runs.emplace_back(run_start, last);
    std::reverse(std::begin(level_runs), std::end(level_runs));

    // Create a list of isolated run sequences, where level_runs with isolated initiators are
    // linked to level_runs with their matching PDI.
    std::vector<unicode_bidi_isolated_run_sequence> isolated_run_sequence_set;

    tt_axiom(!level_runs.empty());
    while (!level_runs.empty()) {
        isolated_run_sequence_set.emplace_back(level_runs.back());
        level_runs.pop_back();

        while (isolated_run_sequence_set.back().ends_with_isolate_initiator() && !level_runs.empty()) {
            auto isolation_level = 1;
            for (auto it = std::rbegin(level_runs); it != std::rend(level_runs); ++it) {
                if (it->starts_with_PDI() && --isolation_level == 0) {
                    isolated_run_sequence_set.back().add_run(*it);
                    level_runs.erase(std::next(it).base());
                    break;
                }
                if (it->ends_with_isolate_initiator()) {
                    ++isolation_level;
                }
            }
        }
    }

    // Compute the sos and eos of each isolation run sequence.
    tt_axiom(!isolated_run_sequence_set.empty());
    for (auto it = std::begin(isolated_run_sequence_set); it != std::end(isolated_run_sequence_set); ++it) {
        ttlet first_sequence = it == std::begin(isolated_run_sequence_set);
        ttlet last_sequence = it == std::end(isolated_run_sequence_set) - 1;

        ttlet start_embedding_level =
            std::max(it->embedding_level(), first_sequence ? paragraph_embedding_level : (it - 1)->embedding_level());
        ttlet end_embedding_level =
            std::max(it->embedding_level(), last_sequence ? paragraph_embedding_level : (it + 1)->embedding_level());

        it->sos = (start_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L;
        it->eos = (end_embedding_level % 2) == 1 ? unicode_bidi_class::R : unicode_bidi_class::L;
    }

    for (auto &isolated_run_sequence : isolated_run_sequence_set) {
        unicode_bidi_W1(isolated_run_sequence);
        unicode_bidi_W2(isolated_run_sequence);
        unicode_bidi_W3(isolated_run_sequence);
        unicode_bidi_W4(isolated_run_sequence);
        unicode_bidi_W5(isolated_run_sequence);
        unicode_bidi_W6(isolated_run_sequence);
        unicode_bidi_W7(isolated_run_sequence);
        unicode_bidi_N0(isolated_run_sequence);
        unicode_bidi_N1(isolated_run_sequence);
        unicode_bidi_N2(isolated_run_sequence);
        unicode_bidi_I1(isolated_run_sequence);
        unicode_bidi_I2(isolated_run_sequence);
    }
}

[[nodiscard]] static std::pair<int8_t, int8_t> unicode_bidi_L1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    using enum unicode_bidi_class;

    auto lowest_odd = std::numeric_limits<int8_t>::max();
    auto heighest = paragraph_embedding_level;
    auto preceding_is_segment = true;

    for (auto it = last - 1; it >= first; --it) {
        auto bidi_class = it->description->bidi_class();

        if (bidi_class == B || bidi_class == S) {
            it->embedding_level = paragraph_embedding_level;
            preceding_is_segment = true;

        } else if (preceding_is_segment && (bidi_class == WS || is_isolate_formatter(bidi_class))) {
            it->embedding_level = paragraph_embedding_level;
            preceding_is_segment = true;

        } else {
            heighest = std::max(heighest, it->embedding_level);
            if ((it->embedding_level % 2) == 1) {
                lowest_odd = std::min(lowest_odd, it->embedding_level);
            }

            preceding_is_segment = false;
        }
    }

    if ((paragraph_embedding_level % 2) == 1) {
        lowest_odd = std::min(lowest_odd, paragraph_embedding_level);
    }
    return {lowest_odd, heighest};
}

static void unicode_bidi_L2(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t lowest_odd, int8_t highest) noexcept
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

static void unicode_bidi_L3(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last) noexcept
{
}

static void unicode_bidi_L4(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    for (auto it = first; it != last; ++it) {
        if (it->direction == unicode_bidi_class::R && it->description->bidi_bracket_type() != unicode_bidi_bracket_type::n) {
            it->code_point = it->description->bidi_mirrored_glyph();
        }
    }
}


[[nodiscard]] static unicode_bidi_class
unicode_bidi_P2(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    using enum unicode_bidi_class;

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
            if (--isolate_level < 0) {
                // Stop at non-matching PDI, for handling rule X5c
                return unknown;
            }
            break;
        default:;
        }
    }
    return unknown;
}

[[nodiscard]] static int8_t unicode_bidi_P3(unicode_bidi_class paragraph_bidi_class) noexcept
{
    return static_cast<int8_t>(paragraph_bidi_class == unicode_bidi_class::AL || paragraph_bidi_class == unicode_bidi_class::R);
}

static void unicode_bidi_P1_line(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    ttlet [lowest_odd, highest] = unicode_bidi_L1(first, last, paragraph_embedding_level);
    unicode_bidi_L2(first, last, lowest_odd, highest);
    unicode_bidi_L3(first, last);
    unicode_bidi_L4(first, last);
}

[[nodiscard]] static unicode_bidi_char_info_iterator
unicode_bidi_P1_paragraph(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    auto paragraph_bidi_class = unicode_bidi_P2(first, last);
    auto paragraph_embedding_level = unicode_bidi_P3(paragraph_bidi_class);

    unicode_bidi_X1(first, last, paragraph_embedding_level);
    last = unicode_bidi_X9(first, last);
    unicode_bidi_X10(first, last, paragraph_embedding_level);

    // XXX Split paragraph into lines and run L1-L4.
    auto line_begin = first;
    for (auto it = first; it != last; ++it) {
        if (it->description->general_category() == unicode_general_category::Zl) {
            ttlet line_end = it + 1;
            unicode_bidi_P1_line(line_begin, line_end, paragraph_embedding_level);
            line_begin = line_end;
        }
    }

    return last;
}

[[nodiscard]] unicode_bidi_char_info_iterator
unicode_bidi_P1(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    auto it = first;
    auto paragraph_begin = it;
    while (it != last) {
        if (it->direction == unicode_bidi_class::B) {
            ttlet paragraph_end = it + 1;
            it = unicode_bidi_P1_paragraph(paragraph_begin, paragraph_end);

            // Move the removed items of the paragraph to the end of the text.
            std::rotate(it, paragraph_end, last);
            last -= std::distance(it, paragraph_end);

            paragraph_begin = it;

        } else {
            ++it;
        }
    }

    if (paragraph_begin != last) {
        last = unicode_bidi_P1_paragraph(paragraph_begin, last);
    }

    return last;
}

} // namespace tt::detail
