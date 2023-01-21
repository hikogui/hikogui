// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../file/file_view.hpp"
#include "../tokenizer.hpp"
#include "../utility/module.hpp"
#include "../datum.hpp"
#include "../strings.hpp"
#include "../indent.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace hi::inline v1 {

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
[[nodiscard]] datum parse_JSON(std::string_view text);

[[nodiscard]] inline datum parse_JSON(std::string const& text)
{
    return parse_JSON(std::string_view{text});
}

[[nodiscard]] inline datum parse_JSON(char const *text)
{
    return parse_JSON(std::string_view{text});
}

/** Parse a JSON string.
 * @param path A path pointing to the file to parse.
 * @return A datum representing the parsed object.
 */
[[nodiscard]] inline datum parse_JSON(std::filesystem::path const& path)
{
    return parse_JSON(as_string_view(file_view(path)));
}

/** Dump an datum object into a JSON string.
 * @param root datum-object to serialize
 * @return The JSON serialized object as a string
 */
[[nodiscard]] std::string format_JSON(datum const& root);

} // namespace hi::inline v1
