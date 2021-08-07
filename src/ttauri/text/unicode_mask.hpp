// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "grapheme.hpp"
#include <cstdint>
#include <cstddef>

namespace tt {

/** A mask of unicode code-points.
 *
 * This mask is used to determine which unicode code points are supported by
 * a font. We need a fine grained mask so that we can find fallback glyphs for
 * all unicode code points that are supported by the fonts.
 *
 * At startup all the fonts are parsed and the unicode_mask is assembled,
 * after this the font is unmapped from memory, but the unicode mask stays
 * behind. Therefor the unicode mask should not use a lot of memory and be
 * very fast to assemble.
 */
class unicode_mask {
public:
    constexpr unicode_mask() noexcept {}
    constexpr unicode_mask(unicode_mask const &) noexcept = default;
    constexpr unicode_mask(unicode_mask &&) noexcept = default;
    constexpr unicode_mask &operator=(unicode_mask const &) noexcept = default;
    constexpr unicode_mask &operator=(unicode_mask &&) noexcept = default;

    constexpr size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr bool contains(char32_t c) const noexcept
    {
        auto it = std::upper_bound(std::begin(_entries), std::end(_entries), c, [](char32_t const &c, entry_type const &item) {
            return c < item.begin();
        });

        if (it != std::begin(_entries)) {
            return (it - 1)->contains(c);
        } else {
            return false;
        }
    }

    [[nodiscard]] bool contains_NFC(grapheme g) const noexcept
    {
        for (ttlet c : g.NFC()) {
            if (not contains(c)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool contains_NFD(grapheme g) const noexcept
    {
        for (ttlet c : g.NFD()) {
            if (not contains(c)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool contains(grapheme g) const noexcept
    {
       return contains_NFC(g) or contains_NFD(g);
    }

    constexpr void add(char32_t first, char32_t last) noexcept
    {
        auto it = std::lower_bound(std::begin(_entries), std::end(_entries), first, [](ttlet &item, ttlet &c) {
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

    [[nodiscard]] friend constexpr unicode_mask operator|(unicode_mask const &lhs, unicode_mask const &rhs) noexcept
    {
        auto r = unicode_mask{};
        r._entries.reserve(lhs._entries.size() * 2 + rhs._entries.size() * 2);

        auto lhs_it = std::begin(lhs._entries);
        auto rhs_it = std::begin(rhs._entries);
        ttlet lhs_end = std::end(lhs._entries);
        ttlet rhs_end = std::end(rhs._entries);

        while (lhs_it != lhs_end or rhs_it != rhs_end) {
            // clang-format off
            auto &it = 
                (lhs_it == lhs_end) ? rhs_it :
                (rhs_it == rhs_end) ? lhs_it :
                (lhs_it->begin() < rhs_it->begin()) ? lhs_it : rhs_it;
            // clang-format on

            ttlet new_begin = r._entries.empty() ? it->begin() : std::max(r._entries.back().end(), it->begin());
            ttlet new_end = it->end();
            if (new_begin < new_end) {
                r._entries.emplace_back(new_begin, new_end);
                r._size += new_end - new_begin;
            }
            ++it;
        }

        //r.optimize();
        return r;
    }

    constexpr unicode_mask &operator|=(unicode_mask const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Optimize storage.
     */
    constexpr void optimize() noexcept
    {
        auto it = std::begin(_entries);
        auto next_it = it;
        while (next_it != std::end(_entries)) {
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

        if (it != std::end(_entries) and not it->empty()) {
            // The current entry was the last element that is still in use.
            ++it;
        }
        tt_axiom(it == std::end(_entries) or it->empty());

        _entries.erase(it, std::end(_entries));
        tt_axiom(holds_invariant());
    }

    void shrink_to_fit() noexcept
    {
        _entries.shrink_to_fit();
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        size_t total_size = 0;

        for (ttlet entry: _entries) {
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

private:
    class entry_type {
    public:
        static constexpr size_t size_bit = 11;
        static constexpr size_t size_mask = (1_uz << size_bit) - 1;

        constexpr entry_type() noexcept : _value(0) {}

        constexpr entry_type(char32_t first, char32_t last) noexcept :
            _value(static_cast<uint32_t>((first << size_bit) | (last - first)))
        {
            tt_axiom(last >= first);
            tt_axiom(last - first <= capacity());
        }

        constexpr entry_type(entry_type const &) noexcept = default;
        constexpr entry_type(entry_type &&) noexcept = default;
        constexpr entry_type &operator=(entry_type const &) noexcept = default;
        constexpr entry_type &operator=(entry_type &&) noexcept = default;

        [[nodiscard]] static constexpr size_t capacity() noexcept
        {
            return size_mask;
        }

        [[nodiscard]] constexpr size_t size() const noexcept
        {
            return static_cast<size_t>(_value & size_mask);
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return size() == 0;
        }

        [[nodiscard]] constexpr bool full() const noexcept
        {
            return size() == capacity();
        }

        [[nodiscard]] constexpr size_t room() const noexcept
        {
            return capacity() - size();
        }

        [[nodiscard]] constexpr char32_t begin() const noexcept
        {
            return static_cast<char32_t>(_value >> size_bit);
        }

        [[nodiscard]] constexpr char32_t end() const noexcept
        {
            return begin() + static_cast<char32_t>(size());
        }

        constexpr entry_type &add_back(size_t num_code_points) noexcept
        {
            return *this = entry_type{begin(), static_cast<char32_t>(end() + num_code_points)};
        }

        constexpr entry_type &remove_front(size_t num_code_points) noexcept
        {
            return *this = entry_type{static_cast<char32_t>(begin() + num_code_points), end()};
        }

        [[nodiscard]] constexpr bool contains(char32_t rhs) const noexcept
        {
            return (begin() <= rhs) and (rhs < end());
        }

    private:
        uint32_t _value;
    };

    using entries_type = std::vector<entry_type>;
    using iterator = typename entries_type::iterator;
    using const_iterator = typename entries_type::const_iterator;

    size_t _size = 0;
    entries_type _entries = {};
};

} // namespace tt
