// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../strings.hpp"
#include "../check.hpp"
#include <tuple>
#include <string_view>

namespace tt {

inline std::tuple<bool, size_t, bool, size_t> parse_spread_sheet_address(std::string_view address)
{
    bool relative_column_nr = true;
    size_t column_nr = 0;
    bool relative_row_nr = true;
    size_t row_nr = 0;

    if (address.starts_with("$")) {
        relative_column_nr = false;
        address.remove_prefix(1);
    }

    while (!address.empty() && is_alpha(address.front())) {
        column_nr *= 26;
        column_nr += address.front() - (is_upper(address.front()) ? 'A' : 'a') + 1;
        address.remove_prefix(1);
    }

    if (address.starts_with("$")) {
        relative_row_nr = false;
        address.remove_prefix(1);
    }

    while (!address.empty() && is_digit(address.front())) {
        row_nr *= 10;
        row_nr += address.front() - '0';
        address.remove_prefix(1);
    }

    tt_parse_check(address.empty(), "Extra characters in spread sheet address {}", address);

    return {relative_column_nr, column_nr - 1, relative_row_nr, row_nr - 1};
}

inline std::pair<size_t, size_t> parse_absolute_spread_sheet_address(std::string_view address)
{
    ttlet[relative_column_nr, column_nr, relative_row_nr, row_nr] = parse_spread_sheet_address(address);
    return {column_nr, row_nr};
}

} // namespace tt
