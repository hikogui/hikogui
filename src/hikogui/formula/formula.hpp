// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"
#include "formula_post_process_context.hpp"
#include "formula_parse_context.hpp"
#include <string>
#include <string_view>
#include <memory>

namespace hi::inline v1 {

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
std::unique_ptr<formula_node> parse_formula(formula_parse_context &context);

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
inline std::unique_ptr<formula_node> parse_formula(std::string_view::const_iterator first, std::string_view::const_iterator last)
{
    auto parse_context = formula_parse_context(first, last);
    auto e = parse_formula(parse_context);

    auto post_process_context = formula_post_process_context();
    e->post_process(post_process_context);
    return e;
}

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
inline std::unique_ptr<formula_node> parse_formula(std::string_view text)
{
    return parse_formula(text.cbegin(), text.cend());
}

/** Find the end of an formula.
 * This function will track nested brackets and strings, until the terminating_character is found.
 * @param first Iterator to the first character of the formula.
 * @param last Iterator to beyond the last character of the text.
 * @param terminating_string The string to find, which is not part of the formula.
 * @return Iterator to the terminating character if found, or last.
 */
std::string_view::const_iterator find_end_of_formula(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    std::string_view terminating_string);

} // namespace hi::inline v1