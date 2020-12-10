// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include "../tokenizer.hpp"
#include <vector>
#include <string_view>

namespace tt {

struct formula_parse_context {
    using const_iterator = typename std::vector<token_t>::const_iterator;

    std::string_view::const_iterator first;
    std::string_view::const_iterator last;

    std::vector<token_t> tokens;
    const_iterator token_it;

    formula_parse_context(std::string_view::const_iterator first, std::string_view::const_iterator last) :
        first(first), last(last), tokens(parseTokens(first, last)), token_it(tokens.begin()) {}

    [[nodiscard]] token_t const& operator*() const noexcept {
        return *token_it;
    }

    [[nodiscard]] token_t const *operator->() const noexcept {
        return &(*token_it);
    }

    formula_parse_context& operator++() noexcept {
        tt_axiom(token_it != tokens.end());
        tt_axiom(*token_it != tokenizer_name_t::End);
        ++token_it;
        return *this;
    }

    formula_parse_context operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
};

}