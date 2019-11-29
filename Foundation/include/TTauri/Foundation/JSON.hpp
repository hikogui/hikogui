// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tokenizer.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/datum.hpp"
#include <string>
#include <optional>
#include <string_view>

namespace TTauri {

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
datum parseJSON(std::string_view text);

/** Parse a JSON string.
 * @param file URL pointing to the file to parse.
 * @return A datum representing the parsed object.
 */
datum parseJSON(TTauri::URL const &file);

/** Dump an datum object into a JSON string.
 * @param root datum-object to serialize
 * @return The JSON serialized object as a string
 */
std::string dumpJSON(datum const &root);

}
