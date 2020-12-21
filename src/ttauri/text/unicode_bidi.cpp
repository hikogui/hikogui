// Copyright 2020 Pokitec
// All rights reserved.

#include "unicode_bidi.hpp"
#include "../stack.hpp"
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

class unicode_bidi_level_run {
public:
    using iterator = unicode_bidi_char_info_iterator;

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
    std::vector<unicode_bidi_level_run> runs;
    unicode_bidi_class sos;
    unicode_bidi_class eos;

    unicode_bidi_isolated_run_sequence(unicode_bidi_level_run const &rhs) noexcept :
        runs({rhs}), sos(unicode_bidi_class::unknown), eos(unicode_bidi_class::unknown)
    {
    }

    auto begin() noexcept
    {
        return std::begin(runs);
    }

    auto end() noexcept
    {
        return std::end(runs);
    }

    auto begin() const noexcept
    {
        return std::begin(runs);
    }

    auto end() const noexcept
    {
        return std::end(runs);
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

static void unicode_bidi_W1(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto previous_bidi_class = sequence.sos;
    for (auto &run : sequence) {
        for (auto &char_info : run) {
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
}

static void unicode_bidi_W2(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto &run : sequence) {
        for (auto &char_info : run) {
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
}

static void unicode_bidi_W3(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto &run : sequence) {
        for (auto &char_info : run) {
            if (char_info.direction == AL) {
                char_info.direction = R;
            }
        }
    }
}

static void unicode_bidi_W4(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    unicode_bidi_char_info *back1 = nullptr;
    unicode_bidi_char_info *back2 = nullptr;
    for (auto &run : sequence) {
        for (auto &char_info : run) {
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
}

static void unicode_bidi_W5(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto ETs = std::vector<unicode_bidi_char_info *>{};
    auto previous_bidi_class = sequence.sos;

    for (auto &run : sequence) {
        for (auto &char_info : run) {
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
}

static void unicode_bidi_W6(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    for (auto &run : sequence) {
        for (auto &char_info : run) {
            if (char_info.direction == ET || char_info.direction == ES || char_info.direction == CS) {
                char_info.direction = ON;
            }
        }
    }
}

static void unicode_bidi_W7(unicode_bidi_isolated_run_sequence &sequence) noexcept
{
    using enum unicode_bidi_class;

    auto last_strong_direction = sequence.sos;
    for (auto &run : sequence) {
        for (auto &char_info : run) {
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
}

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
    for (ttlet &run : isolated_run_sequence) {
        for (ttlet &character : run) {
            if (character.description->bidi_bracket_type() == unicode_bidi_bracket_type::o) {
                if (stack.full()) {
                    goto finished;
                } else {
                    stack.emplace_back(character.description->bidi_mirrored_glyph(), index, last_strong);
                }

            } else if (character.description->bidi_bracket_type() == unicode_bidi_bracket_type::c) {
                for (auto it = stack.end() - 1; it >= stack.begin(); --it) {
                    if (it->mirrored_bracket == character.code_point) {
                        pairs.emplace_back(
                            it->index, index, it->last_strong, it->has_inside_strong, it->inside_strong_matches_embedding);
                        stack.pop_back(it);
                        break;
                    }
                }
            }

            if (character.description->bidi_class() == unicode_bidi_class::L ||
                character.description->bidi_class() == unicode_bidi_class::R) {
                last_strong = character.description->bidi_class();

                for (auto &item : stack) {
                    item.has_inside_strong = true;
                    item.inside_strong_matches_embedding |= character.description->bidi_class() == embedding_direction;
                }
            }

            ++index;
        }
    }

finished:
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
    for (auto &run : isolated_run_sequence) {
        for (auto &character : run) {
            if (character.description->bidi_bracket_type() == unicode_bidi_bracket_type::o) {
                last_bracket_changed_from_L_to_R = false;

                for (ttlet &pair : bracket_pairs) {
                    if (pair.first == index) {
                        if (pair.direction == unicode_bidi_class::L) {
                            character.direction = unicode_bidi_class::L;
                        } else if (pair.direction == unicode_bidi_class::R) {
                            if (character.direction == unicode_bidi_class::L) {
                                last_bracket_changed_from_L_to_R = true;
                            }
                            character.direction = unicode_bidi_class::R;
                        }
                        break;
                    }
                }

            } else if (character.description->bidi_bracket_type() == unicode_bidi_bracket_type::c) {
                last_bracket_changed_from_L_to_R = false;

                for (ttlet &pair : bracket_pairs) {
                    if (pair.second == index) {
                        if (pair.direction == unicode_bidi_class::L) {
                            character.direction = unicode_bidi_class::L;
                        } else if (pair.direction == unicode_bidi_class::R) {
                            if (character.direction == unicode_bidi_class::L) {
                                last_bracket_changed_from_L_to_R = true;
                            }
                            character.direction = unicode_bidi_class::R;
                        }
                        break;
                    }
                }

            } else if (character.description->bidi_class() == unicode_bidi_class::NSM) {
                if (last_bracket_changed_from_L_to_R) {
                    character.direction = unicode_bidi_class::R;
                }

                last_bracket_changed_from_L_to_R = false;

            } else {
                last_bracket_changed_from_L_to_R = false;
            }

            ++index;
        }
    }
}

static void unicode_bidi_N1(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    tt_not_implemented();

    ttlet embedding_direction = isolated_run_sequence.embedding_direction();

    auto prev_direction = isolated_run_sequence.sos;

    for (auto &run : isolated_run_sequence) {
        for (auto &character : run) {
        }
    }
}

static void unicode_bidi_N2(unicode_bidi_isolated_run_sequence &isolated_run_sequence)
{
    ttlet embedding_direction = isolated_run_sequence.embedding_direction();

    for (auto &run: isolated_run_sequence) {
        for (auto &character: run) {
            if (is_NI(character.direction)) {
                character.direction = embedding_direction;
            }
        }
    }
}

static void unicode_bidi_X10(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    int8_t paragraph_embedding_level) noexcept
{
    tt_axiom (first != last);

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
    }
}

//
// static void BidiN0(BidiIsolateSequence &sequence) noexcept
//{
//    tt_not_implemented();
//}
//
// static void BidiN1(BidiIsolateSequence &sequence) noexcept
//{
//    auto lastDirection = sequence.sos;
//    std::optional<decltype(sequence.first)> firstNI;
//
//    for (auto i = sequence.first; i != sequence.last; ++i) {
//        switch (i->bidi_class) {
//        case unicode_bidi_class::B:
//        case unicode_bidi_class::S:
//        case unicode_bidi_class::WS:
//        case unicode_bidi_class::ON:
//        case unicode_bidi_class::FSI:
//        case unicode_bidi_class::LRI:
//        case unicode_bidi_class::RLI:
//        case unicode_bidi_class::PDI:
//        case unicode_bidi_class::RLE:// X9 Ignore
//        case unicode_bidi_class::LRE:// X9 Ignore
//        case unicode_bidi_class::RLO:// X9 Ignore
//        case unicode_bidi_class::LRO:// X9 Ignore
//        case unicode_bidi_class::PDF:// X9 Ignore
//        case unicode_bidi_class::BN: // X9 Ignore
//            if (!firstNI) {
//                firstNI = i;
//            }
//            break;
//
//        case unicode_bidi_class::L:
//            if (firstNI) {
//                if (lastDirection == unicode_bidi_class::L) {
//                    for (auto j = *firstNI; j != i; ++j) {
//                        j->bidi_class = lastDirection;
//                    }
//                }
//                firstNI = {};
//            }
//            lastDirection = unicode_bidi_class::L;
//            break;
//
//        case unicode_bidi_class::R:
//        case unicode_bidi_class::AL:
//        case unicode_bidi_class::EN: // European Numbers are treated as R.
//        case unicode_bidi_class::AN: // Arabic Numbers are treated as R.
//            if (firstNI) {
//                if (lastDirection == unicode_bidi_class::L) {
//                    for (auto j = *firstNI; j != i; ++j) {
//                        j->bidi_class = lastDirection;
//                    }
//                }
//                firstNI = {};
//            }
//            lastDirection = unicode_bidi_class::R;
//            break;
//
//        default:
//            tt_no_default();
//        }
//    }
//
//    if (firstNI) {
//        if (lastDirection == sequence.eos) {
//            for (auto j = *firstNI; j != sequence.last; ++j) {
//                j->bidi_class = lastDirection;
//            }
//        }
//        firstNI = {};
//    }
//}
//
//// N2. remaining NI get embedding level direction.
// static void BidiN2(BidiIsolateSequence &sequence) noexcept
//{
//    for (auto i = sequence.first; i != sequence.last; ++i) {
//        switch (i->bidi_class) {
//        case unicode_bidi_class::B:
//        case unicode_bidi_class::S:
//        case unicode_bidi_class::WS:
//        case unicode_bidi_class::ON:
//        case unicode_bidi_class::FSI:
//        case unicode_bidi_class::LRI:
//        case unicode_bidi_class::RLI:
//        case unicode_bidi_class::PDI:
//            i->bidi_class = (sequence.embedding_level % 2 == 0) ? unicode_bidi_class::L : unicode_bidi_class::R;
//            break;
//        default:;
//        }
//    }
//}
//
// static void BidiN(BidiContext &context) noexcept
//{
//    for (auto &sequence: context.isolateSequences) {
//        BidiN0(sequence);
//        BidiN1(sequence);
//        BidiN2(sequence);
//    }
//}
//
// static void BidiI1_I2(BidiContext &context) noexcept
//{
//    for (auto &character: context.characters) {
//        if (character.embedding_level % 2 == 0) {
//            // I1.
//            switch (character.bidi_class) {
//            case unicode_bidi_class::R:
//                ++character.embedding_level;
//                break;
//            case unicode_bidi_class::AN:
//            case unicode_bidi_class::EN:
//                character.embedding_level += 2;
//                break;
//            default:;
//            }
//        } else {
//            // I2.
//            switch (character.bidi_class) {
//            case unicode_bidi_class::L:
//            case unicode_bidi_class::AN:
//            case unicode_bidi_class::EN:
//                ++character.embedding_level;
//                break;
//            default:;
//            }
//        }
//    }
//}
//
// static void BidiL1(BidiContext &context) noexcept
//{
//    auto i = context.characters.begin();
//    std::optional<decltype(i)> firstWS = {};
//
//    for (ttlet &paragraph : context.paragraphs) {
//        for (; i->original_bidi_class != unicode_bidi_class::B; ++i) {
//            switch (i->original_bidi_class) {
//            case unicode_bidi_class::S:
//                i->embedding_level = paragraph.embedding_level;
//                break;
//
//            case unicode_bidi_class::WS:
//            case unicode_bidi_class::FSI:
//            case unicode_bidi_class::LRI:
//            case unicode_bidi_class::RLI:
//            case unicode_bidi_class::PDI:
//            case unicode_bidi_class::RLE:// X9 Ignore
//            case unicode_bidi_class::LRE:// X9 Ignore
//            case unicode_bidi_class::RLO:// X9 Ignore
//            case unicode_bidi_class::LRO:// X9 Ignore
//            case unicode_bidi_class::PDF:// X9 Ignore
//            case unicode_bidi_class::BN: // X9 Ignore
//                if (i->codePoint == 0x2028) { // Line Separator.
//                    i->embedding_level = paragraph.embedding_level;
//                    if (firstWS) {
//                        for (auto j = *firstWS; j != i; ++j) {
//                            j->embedding_level = paragraph.embedding_level;
//                        }
//                        firstWS = {};
//                    }
//                } else {
//                    // Make sure X9. characters are categorized to their original class.
//                    i->bidi_class = i->original_bidi_class;
//                    if (!firstWS) {
//                        firstWS = i;
//                    }
//                }
//                break;
//
//            default:
//                firstWS = {};
//            }
//        }
//
//        // unicode_bidi_class::B.
//        if (firstWS) {
//            for (auto j = *firstWS; j != i; ++j) {
//                j->embedding_level = paragraph.embedding_level;
//            }
//            firstWS = {};
//        }
//        i->embedding_level = paragraph.embedding_level;
//        ++i;
//    }
//}

// static std::vector<std::pair<ssize_t,ssize_t>> BidiL2(BidiContext &context) noexcept
//{
//}

// static std::vector<std::pair<ssize_t,ssize_t>> BidiL(BidiContext &context) noexcept
//{
//    BidiL1(context);
//    return BidiL2(context);
//}

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

[[nodiscard]] static unicode_bidi_char_info_iterator
unicode_bidi_P1_paragraph(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept
{
    auto paragraph_bidi_class = unicode_bidi_P2(first, last);
    auto paragraph_embedding_level = unicode_bidi_P3(paragraph_bidi_class);

    unicode_bidi_X1(first, last, paragraph_embedding_level);
    last = unicode_bidi_X9(first, last);
    unicode_bidi_X10(first, last, paragraph_embedding_level);
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
