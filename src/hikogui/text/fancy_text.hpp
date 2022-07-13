

/** @file fancy_text.hpp
 *
 * Functions for decoding and encoding fancy text.
 *
 * Fancy text format:
 *  - Text strings
 * 
 */

#pragma once

#include "agstring.hpp"
#include <string>

namespace {

/** Decode a fancy string.
 *
 * @param str Input unicode-string.
 * @return The resulting attributed grapheme string.
 */
agstring decode_fancy_text(std::string_view str) noexcept;

std::string encode_fancy_text(agstring_view str) noexcept;


}

