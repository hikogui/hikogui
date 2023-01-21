// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../tokenizer.hpp"
#include <vector>
#include <string_view>

namespace hi::inline v1 {

struct formula_parse_context {
    using const_iterator = typename std::vector<token_t>::const_iterator;

    std::vector<token_t> tokens;
    const_iterator token_it;

    formula_parse_context(std::string_view::const_iterator first, std::string_view::const_iterator last) :
        tokens(parseTokens(first, last)), token_it(tokens.begin())
    {
    }

    [[nodiscard]] token_t const &operator*() const noexcept
    {
        return *token_it;
    }

    [[nodiscard]] token_t const *operator->() const noexcept
    {
        return &(*token_it);
    }

    formula_parse_context &operator++() noexcept
    {
        hi_assert(token_it != tokens.end());
        hi_assert(*token_it != tokenizer_name_t::End);
        ++token_it;
        return *this;
    }

    formula_parse_context operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
};

} // namespace hi::inline v1