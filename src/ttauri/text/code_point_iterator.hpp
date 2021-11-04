// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../codec/UTF.hpp"
#include "../required.hpp"
#include <type_traits>
#include <iterator_traits>

namespace tt {
inline namespace v1 {

/** Iterate over code points (char32_t) through char8_t, char16_t or char32_t iterators.
 *
 * Named requirements: LegacyBirectionalIterator
 */
template<typename Iterator>
class code_point_iterator {
public:
    typename iterator = Iterator;
    typename it_value_type = std::iterator_traits<iterator>::value_type;
    typename it_base_type = std::remove_cv_t<it_value_type>;

    typename difference_type = std::iterator_traits<iterator>::difference_type;
    typename value_type = char32_t;
    typename iterator_category = std::random_access_iterator_tag;

    [[nodiscard]] constexpr code_point_iterator(code_point_iterator const &) = default;
    [[nodiscard]] constexpr code_point_iterator(code_point_iterator &&) = default;
    [[nodiscard]] constexpr code_point_iterator &operator=(code_point_iterator const &) = default;
    [[nodiscard]] constexpr code_point_iterator &operator=(code_point_iterator &&) = default;
    ~code_point_iterator() = default;

    [[nodiscard]] constexpr code_point_iterator(iterator const &it) noexcept : it(itr) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        if constexpr (std::is_same_v<it_base_type, char32_t>) {
            return *it;
        } else if constexpr (std::is_same_v<it_base_type, char16_t>) {
            return detail::utf16_to_utf32(it);
        } else if constexpr (std::is_same_v<it_base_type, char8_t>) {
            return detail::utf8_to_utf32(it);
        } else {
            tt_no_default();
        }
    }

    constexpr code_point_iterator &operator--()
    {
        if constexpr (std::is_same_v<it_base_type, char32_t>) {
            --it;

        } else if constexpr (std::is_same_v<it_base_type, char16_t>) {
            // Skip over all low surrogates
            while (*(--it) & 0xfc00 == 0xdc00) {}

        } else if constexpr (std::is_same_v<it_base_type, char8_t>) {
            // Skip over all code-units in the form 0b10------.
            while (*(--it) & 0xc0 == 0x80) {}

        } else {
            tt_no_default();
        }

        return *this;
    }

    constexpr code_point_iterator &operator+=(ssize_t n)
    {
        if constexpr (std::is_same_v<it_base_type, char32_t>) {
            ++it;

        } else if constexpr (std::is_same_v<it_base_type, char16_t>) {
            // Skip over all low surrogates
            while (*(++it) & 0xfc00 == 0xdc00) {}

        } else if constexpr (std::is_same_v<it_base_type, char8_t>) {
            // Skip over all code-units in the form 0b10------.
            while (*(++it) & 0xc0 == 0x80) {}

        } else {
            tt_no_default();
        }

        return *this;
    }

    constexpr code_point_iterator &operator++(int) noexcept
    {
        auto &tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr code_point_iterator &operator--(int) noexcept
    {
        auto &tmp = *this;
        --(*this);
        return tmp;
    }

    [[nodiscard]] constexpr friend bool
    operator==(code_point_iterator const &lhs, code_point_iterator const &rhs) noexcept = default;
    [[nodiscard]] constexpr friend auto
    operator<=>(code_point_iterator const &lhs, code_point_iterator const &rhs) noexcept = default;

private:
    iterator it;
};

template<typename Container>
[[nodiscard]] constexpr auto code_point_begin(Container const &container) noexcept
{
    return code_point_container(begin(container));
}

template<typename Container>
[[nodiscard]] constexpr auto code_point_end(Container const &container) noexcept
{
    return code_point_container(end(container));
}

}
} // namespace tt
