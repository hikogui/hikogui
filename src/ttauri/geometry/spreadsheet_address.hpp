// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file
 * The functions in this file are for handling spreadsheet addresses.
 * 
 * Spreadsheet addresses are of the form:
 * ```
 * address_range := address ':' address;
 * address := '$'? column '$'? row;
 * column := [A-Z]+;
 * row := [0-9]+;
 * ```
 * 
 * Columns start at 'A' for the left most column.
 * After 'Z' follows 'AA' then 'AB'.
 * 
 * Rows start at '1' for the top most row.
 * 
 * A column or row that is prefixed with '$' is absolute, instead of relative.
 */

#pragma once

#include "../strings.hpp"
#include "../check.hpp"
#include <tuple>
#include <string_view>

namespace tt::inline v1 {


inline std::tuple<bool, size_t, bool, size_t> _parse_spreadsheet_address(std::string_view &address)
{
    bool column_nr_is_relative = true;
    size_t column_nr = 0;
    bool row_nr_is_relative = true;
    size_t row_nr = 0;

    if (address.starts_with("$")) {
        column_nr_is_relative = false;
        address.remove_prefix(1);
    }

    while (!address.empty() && is_alpha(address.front())) {
        column_nr *= 26;
        column_nr += address.front() - (is_upper(address.front()) ? 'A' : 'a') + 1;
        address.remove_prefix(1);
    }

    if (address.starts_with("$")) {
        row_nr_is_relative = false;
        address.remove_prefix(1);
    }

    while (!address.empty() && is_digit(address.front())) {
        row_nr *= 10;
        row_nr += address.front() - '0';
        address.remove_prefix(1);
    }

    return {column_nr_is_relative, column_nr - 1, row_nr_is_relative, row_nr - 1};
}

/** Parse a spreadsheet address.
 * 
 * @param address The address to parse.
 * @param start_column_nr A relative column in the address is added to the start-column.
 * @param start_row_nr A relative row in the address is added to the start-row.
 * @return The zero-based column and row index.
 */
inline std::pair<size_t, size_t> parse_spreadsheet_address(std::string_view address, size_t start_column_nr = 0, size_t start_row_nr = 0)
{
    auto [column_nr_is_relative, column_nr, row_nr_is_relative, row_nr] = _parse_spreadsheet_address(address);
    tt_parse_check(address.empty(), "Extra characters in spread sheet address {}", address);

    if (column_nr_is_relative) {
        column_nr += start_column_nr;
    }
    if (row_nr_is_relative) {
        row_nr += start_row_nr;
    }
    return {column_nr, row_nr};
}

inline std::tuple<size_t, size_t, size_t, size_t>
parse_spreadsheet_range(std::string_view address, size_t start_column_nr = 0, size_t start_row_nr = 0)
{
    auto [column_nr_is_relative1, column_nr1, row_nr_is_relative1, row_nr1] = _parse_spreadsheet_address(address);
    if (column_nr_is_relative1) {
        column_nr1 += start_column_nr;
    }
    if (row_nr_is_relative1) {
        row_nr1 += start_row_nr;
    }

    if (address.starts_with(":")) {
        address.remove_prefix(1);
        auto [column_nr_is_relative2, column_nr2, row_nr_is_relative2, row_nr2] = _parse_spreadsheet_address(address);
        tt_parse_check(address.empty(), "Extra characters in spread sheet address {}", address);

        if (column_nr_is_relative2) {
            column_nr2 += start_column_nr;
        }
        if (row_nr_is_relative2) {
            row_nr2 += start_row_nr;
        }

        tt_parse_check(column_nr1 <= column_nr2, "Column range must be in ascending direction");
        tt_parse_check(row_nr1 <= row_nr2, "Column range must be in ascending direction");
        return {column_nr1, row_nr1, column_nr2 + 1, row_nr2 + 1};

    } else {
        tt_parse_check(address.empty(), "Extra characters in spread sheet address {}", address);
        return {column_nr1, row_nr1, column_nr1 + 1, row_nr1 + 1};
    }
}

} // namespace tt
