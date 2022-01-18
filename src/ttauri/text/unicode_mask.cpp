// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_mask.hpp"

namespace tt::inline v1 {

[[nodiscard]] bool unicode_mask::contains(char32_t c) const noexcept
{
    auto it = std::upper_bound(begin(_entries), end(_entries), c, [](char32_t const &c, entry_type const &item) {
        return c < item.begin();
    });

    if (it != begin(_entries)) {
        return (it - 1)->contains(c);
    } else {
        return false;
    }
}

[[nodiscard]] bool unicode_mask::contains_composed(grapheme g) const noexcept
{
    for (ttlet c : g.composed()) {
        if (not contains(c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool unicode_mask::contains_decomposed(grapheme g) const noexcept
{
    for (ttlet c : g.decomposed()) {
        if (not contains(c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool unicode_mask::contains(grapheme g) const noexcept
{
    return contains_composed(g) or contains_decomposed(g);
}

[[nodiscard]] bool unicode_mask::contains(unicode_mask const &other) const noexcept
{
    auto this_it = begin(_entries);
    auto this_end = end(_entries);
    auto other_it = begin(other._entries);
    auto other_end = end(other._entries);

    if (other_it == other_end) {
        return true;
    }

    auto other_item = *other_it;
    while (true) {
        if (other_item.empty()) {
            // other_item was fully overlapped with this, advance to the next item from other.
            if (++other_it == other_end) {
                // Done.
                return true;
            }
            other_item = *other_it;

        } else if (this_it == this_end) {
            // other still contains data, but this has come to an end.
            return false;

        } else if (this_it->begin() > other_item.begin()) {
            // other_item still contains code-points while this_it is beyond it.
            return false;

        } else if (this_it->end() <= other_item.begin()) {
            // this_it was fully handled.
            ++this_it;

        } else {
            // this_it overlaps other_item.
            other_item.remove_front(std::min(this_it->end(), other_item.end()) - other_item.begin());
        }
    }
}

void unicode_mask::add(char32_t first, char32_t last) noexcept
{
    auto it = std::lower_bound(begin(_entries), end(_entries), first, [](ttlet &item, ttlet &c) {
        return item.begin() < c;
    });

    while (first != last) {
        if (it == _entries.end()) {
            // Append the items.
            ttlet last_to_insert = std::min({last, static_cast<char32_t>(first + entry_type::capacity())});

            it = _entries.emplace(it, first, last_to_insert) + 1;
            _size += last_to_insert - first;

            first = last_to_insert;

        } else if (first < it->begin()) {
            // Insert the left side before the current entry.
            ttlet last_to_insert = std::min({last, it->begin(), static_cast<char32_t>(first + entry_type::capacity())});

            it = _entries.emplace(it, first, last_to_insert) + 1;
            _size += last_to_insert - first;

            first = last_to_insert;

        } else if (first < it->end()) {
            // Ignore the left side that overlaps with the current entry.
            first = std::min(it->end(), last);
            ++it;

        } else {
            // We are behind the current entry, skip to the next entry.
            ++it;
        }
    }
}

void unicode_mask::optimize() noexcept
{
    auto it = begin(_entries);
    auto next_it = it;
    while (next_it != end(_entries)) {
        if (it == next_it) {
            ++next_it;

        } else if (it->full()) {
            // Can't optimize into a full entry, skip it.
            ++it;

        } else if (next_it->empty()) {
            // Next element was fully optimized, skip it.
            ++next_it;

        } else if (it->empty()) {
            // Current element was deleted copy next into it.
            *it = std::exchange(*next_it, {});

        } else if (it->end() == next_it->begin()) {
            // Current and next element are touching, optimize it.
            ttlet to_move = std::min(it->room(), next_it->size());
            it->add_back(to_move);
            next_it->remove_front(to_move);

        } else {
            // Current and next elements are not touching, advance only
            // the current iterator, so that the element may be moved in
            // the next iteration.
            ++it;
        }
    }

    if (it != end(_entries) and not it->empty()) {
        // The current entry was the last element that is still in use.
        ++it;
    }
    tt_axiom(it == end(_entries) or it->empty());

    _entries.erase(it, end(_entries));
    tt_axiom(holds_invariant());
}

void unicode_mask::shrink_to_fit() noexcept
{
    _entries.shrink_to_fit();
}

[[nodiscard]] bool unicode_mask::holds_invariant() const noexcept
{
    std::size_t total_size = 0;

    for (ttlet entry : _entries) {
        if (entry.empty()) {
            return false;
        }

        total_size += entry.size();
    }

    if (total_size != _size) {
        return false;
    }

    return true;
}

} // namespace tt::inline v1
