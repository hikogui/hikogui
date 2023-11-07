// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file lookahead_iterator.hpp Functions to create a lookahead_iterator from a forward iterator.
*/

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <iterator>
#include <bit>
#include <optional>
#include <string>
#include <stdexcept>
#include <array>

hi_export_module(hikogui.parser.lookahead_iterator);

hi_export namespace hi {
inline namespace v1 {

/** Lookahead iterator.
 *
 * This iterator adapter takes a forward input iterator and adapts it so that
 * you can look ahead beyond the current position of the iterator. This
 * is useful when writing a parser.
 */
template<size_t LookaheadCount, typename It, std::sentinel_for<It> ItEnd = std::default_sentinel_t>
class lookahead_iterator {
public:
    static_assert(std::has_single_bit(LookaheadCount), "LookaheadCount must be a power of two.");

    constexpr static size_t max_size = LookaheadCount;

    using value_type = std::iterator_traits<It>::value_type;
    using reference = value_type const &;
    using pointer = value_type const *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    class proxy {
    public:
        using value_type = value_type;
        using reference = reference;
        using pointer = pointer;

        constexpr proxy(proxy const &) noexcept = default;
        constexpr proxy(proxy &&) noexcept = default;
        constexpr proxy &operator=(proxy const &) noexcept = default;
        constexpr proxy &operator=(proxy &&) noexcept = default;
        constexpr proxy(value_type other) noexcept : _v(std::move(other)) {}

        constexpr reference operator*() const noexcept
        {
            return _v;
        }

        constexpr pointer operator->() const noexcept
        {
            return std::addressof(_v);
        }

    private:
        value_type _v;
    };

    constexpr lookahead_iterator() noexcept = default;
    constexpr lookahead_iterator(lookahead_iterator const &) noexcept = delete;
    constexpr lookahead_iterator(lookahead_iterator &&) noexcept = default;
    constexpr lookahead_iterator&operator=(lookahead_iterator const &) noexcept = delete;
    constexpr lookahead_iterator&operator=(lookahead_iterator &&) noexcept = default;

    constexpr explicit lookahead_iterator(It first, ItEnd last) noexcept : _it(first), _last(last), _size(0)
    {
        while (_size != max_size and _it != last) {
            add_one_to_lookahead();
        }
    }

    /** The number of entries can be looked ahead.
     *
     * @return Number of entries that can be looked ahead, including the current entry.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _size;
    }

    /** Check if the iterator is at end.
     *
     * @retval true Iterator at end.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
    {
        return empty();
    }

    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @return A reference to the item.
     */
    [[nodiscard]] constexpr reference operator[](size_t i) const noexcept
    {
        hi_axiom(i < _size);
        return _lookahead[(_tail + i) % max_size];
    }

    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @throws std::out_of_range when index beyond the lookahead buffer.
     * @return A reference to the item.
     */
    [[nodiscard]] constexpr reference at(size_t i) const
    {
        if (i < _size) {
            return (*this)[i];
        } else {
            throw std::out_of_range("lookahead_iterator::at()");
        }
    }

    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @retval std::nullopt When the index points beyond the lookahead buffer.
     * @return A reference to the item.
     */
    [[nodiscard]] constexpr std::optional<value_type> next(size_t i = 1) const noexcept
    {
        if (i < _size) {
            return (*this)[i];
        } else {
            return std::nullopt;
        }
    }

    /** Get a reference to the value at the iterator.
     */
    constexpr reference operator*() const noexcept
    {
        hi_axiom(_size != 0);
        return (*this)[0];
    }

    /** Get a pointer to the value at the iterator.
     */
    constexpr pointer operator->() const noexcept
    {
        hi_axiom(_size != 0);
        return std::addressof((*this)[0]);
    }

    /** Increment the iterator.
     */
    constexpr lookahead_iterator &operator++() noexcept
    {
        hi_axiom(_size != 0);
        --_size;
        ++_tail;
        add_one_to_lookahead();

        return *this;
    }

    constexpr lookahead_iterator &operator+=(size_t n) noexcept
    {
        for (auto i = 0_uz; i != n; ++i) {
            ++(*this);
        }
        return *this;
    }

    constexpr proxy operator++(int) noexcept
    {
        auto r = proxy{**this};
        ++(*this);
        return r;
    }

private:
    It _it;
    ItEnd _last;
    size_t _size = 0;
    size_t _tail = 0;

    std::array<value_type, max_size> _lookahead = {};

    constexpr void add_one_to_lookahead() noexcept
    {
        hi_axiom(_size < max_size);

        if (_it != _last) {
            _lookahead[(_tail + _size) % max_size] = *_it;
            ++_it;
            ++_size;
        }
    }
};

static_assert(std::movable<lookahead_iterator<2, std::string::iterator, std::string::iterator>>);
static_assert(std::is_same_v<std::iterator_traits<lookahead_iterator<2, std::string::iterator, std::string::iterator>>::value_type, char>);
static_assert(std::input_or_output_iterator<lookahead_iterator<2, std::string::iterator, std::string::iterator>>);
static_assert(std::weakly_incrementable<lookahead_iterator<2, std::string::iterator, std::string::iterator>>);

/** Create a lookahead_iterator from a forward iterator.
*
* @tparam LookaheadCount Number of items to lookahead, including the iterator itself. Must be a power-of-two.
* @param first A forward iterator.
* @param last A sentinel or end-iterator.
* @return A lookahead iterator.
*/
template<size_t LookaheadCount, typename It, std::sentinel_for<It> ItEnd = std::default_sentinel_t>
auto make_lookahead_iterator(It first, ItEnd last = std::default_sentinel) noexcept
{
    return lookahead_iterator<LookaheadCount, It, ItEnd>{first, last};
}

/** Create a lookahead_iterator from a forward iterator.
 *
 * @tparam LookaheadCount Number of items to lookahead, including the iterator itself. Must be a power-of-two.
 * @param first A forward iterator.
 * @param last A sentinel or end-iterator.
 * @return A lookahead iterator.
 */
template<size_t LookaheadCount, std::ranges::range Range>
auto make_lookahead_iterator(Range const &range) noexcept
{
    return make_lookahead_iterator<LookaheadCount>(std::ranges::begin(range), std::ranges::end(range));
}

}}

