// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "code_point_iterator.hpp"
#include "unicode_text_segmentation.hpp"
#include "grapheme.hpp"
#include "../required.hpp"
#include <type_traits>
#include <iterator_traits>

namespace tt {

template<typename Iterator>
class grapheme_iterator {
public:
    typename iterator = Iterator;

    typename difference_type = std::iterator_traits<iterator>::difference_type;
    typename value_type = char32_t;
    typename iterator_category = std::random_access_iterator_tag;

    [[nodiscard]] constexpr grapheme_iterator(code_point_iterator const &) = default;
    [[nodiscard]] constexpr grapheme_iterator(code_point_iterator &&) = default;
    [[nodiscard]] constexpr grapheme_iterator &operator=(code_point_iterator const &) = default;
    [[nodiscard]] constexpr grapheme_iterator &operator=(code_point_iterator &&) = default;
    ~grapheme_iterator() = default;

    [[nodiscard]] constexpr grapheme_iterator(iterator const &it) noexcept : it(itr), forward_break_state() {}

    [[nodiscard]] grapheme operator*() noexcept {
        ttlet last_it = std::next(*this).it;
        auto r = grapheme(it, last_it);
    }

    grapheme_iterator &operator++() noexcept
    {
        while (!breaks_grapheme(*(++it), forward_break_state)) {}
        return *this;
    }

    grapheme_iterator &operator++(int) noexcept
    {
        auto &tmp = *this;
        ++(*this);
        return *this;
    }

    [[nodiscard]] friend operator==(grapheme_iterator const &lhs, grapheme_iterator const &rhs) noexcept
    {
        return lhs.it == rhs.it;
    }

    [[nodiscard]] friend operator<=>(grapheme_iterator const &lhs, grapheme_iterator const &rhs) noexcept
    {
        return lhs.it <=> rhs.it;
    }

private:
    iterator it;
    grapheme_break_state forward_break_state;
};

} // namespace tt