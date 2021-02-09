// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../tokenizer.hpp"
#include "../required.hpp"
#include "../URL.hpp"
#include "../datum.hpp"
#include <string>
#include <optional>
#include <string_view>

namespace tt {

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
[[nodiscard]] datum parseJSON(std::string_view text);

/** Parse a JSON string.
 * @param file URL pointing to the file to parse.
 * @return A datum representing the parsed object.
 */
[[nodiscard]] datum parseJSON(tt::URL const &file);

/** Dump an datum object into a JSON string.
 * @param root datum-object to serialize
 * @return The JSON serialized object as a string
 */
[[nodiscard]] std::string dumpJSON(datum const &root);

}
