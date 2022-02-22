// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "grapheme.hpp"
#include <cstdint>
#include <cstddef>

namespace tt::inline v1 {
namespace detail {

class unicode_mask_entry {
public:
    static constexpr std::size_t size_bit = 11;
    static constexpr std::size_t size_mask = (1_uz << size_bit) - 1;

    constexpr unicode_mask_entry() noexcept : _value(0) {}

    constexpr unicode_mask_entry(char32_t first, char32_t last) noexcept :
        _value(static_cast<uint32_t>((first << size_bit) | (last - first)))
    {
        tt_axiom(last >= first);
        tt_axiom(last - first <= capacity());
    }

    constexpr unicode_mask_entry(unicode_mask_entry const &) noexcept = default;
    constexpr unicode_mask_entry(unicode_mask_entry &&) noexcept = default;
    constexpr unicode_mask_entry &operator=(unicode_mask_entry const &) noexcept = default;
    constexpr unicode_mask_entry &operator=(unicode_mask_entry &&) noexcept = default;

    [[nodiscard]] static constexpr std::size_t capacity() noexcept
    {
        return size_mask;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return static_cast<std::size_t>(_value & size_mask);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] constexpr bool full() const noexcept
    {
        return size() == capacity();
    }

    [[nodiscard]] constexpr std::size_t room() const noexcept
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

    constexpr unicode_mask_entry &add_back(std::size_t num_code_points) noexcept
    {
        return *this = unicode_mask_entry{begin(), static_cast<char32_t>(end() + num_code_points)};
    }

    constexpr unicode_mask_entry &remove_front(std::size_t num_code_points) noexcept
    {
        return *this = unicode_mask_entry{static_cast<char32_t>(begin() + num_code_points), end()};
    }

    [[nodiscard]] constexpr bool contains(char32_t rhs) const noexcept
    {
        return (begin() <= rhs) and (rhs < end());
    }

private:
    uint32_t _value;
};

} // namespace detail

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

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return _size;
    }

    /** Check if the given code point is covered by this mask.
     */
    [[nodiscard]] bool contains(char32_t c) const noexcept;

    /** Check if the full grapheme normalized to NFC is covered by this mask.
     */
    [[nodiscard]] bool contains_composed(grapheme g) const noexcept;

    /** Check if the full grapheme normalized to NFD is covered by this mask.
     */
    [[nodiscard]] bool contains_decomposed(grapheme g) const noexcept;

    /** Check if the full grapheme is covered by this mask.
     */
    [[nodiscard]] bool contains(grapheme g) const noexcept;

    /** Check if all the code-points in other are covered by this mask.
     */
    [[nodiscard]] bool contains(unicode_mask const &other) const noexcept;

    /** Add a range of unicode code points to this mask.
     *
     * @param first The first code-point to add.
     * @param last One beyond the last code-point to add.
     */
    void add(char32_t first, char32_t last) noexcept;

    /** Combine two masks.
     */
    [[nodiscard]] friend unicode_mask operator|(unicode_mask const &lhs, unicode_mask const &rhs) noexcept
    {
        auto r = unicode_mask{};
        r._entries.reserve(lhs._entries.size() * 2 + rhs._entries.size() * 2);

        auto lhs_it = begin(lhs._entries);
        auto rhs_it = begin(rhs._entries);
        ttlet lhs_end = end(lhs._entries);
        ttlet rhs_end = end(rhs._entries);

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

        // r.optimize();
        return r;
    }

    /** Combine two masks.
     */
    unicode_mask &operator|=(unicode_mask const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Optimize storage.
     */
    void optimize() noexcept;

    void shrink_to_fit() noexcept;

    /** Check to see if the mask is still valid.
     */
    [[nodiscard]] bool holds_invariant() const noexcept;

private:
    using entry_type = detail::unicode_mask_entry;
    using entries_type = std::vector<entry_type>;
    using iterator = typename entries_type::iterator;
    using const_iterator = typename entries_type::const_iterator;

    std::size_t _size = 0;
    entries_type _entries = {};
};

} // namespace tt::inline v1
