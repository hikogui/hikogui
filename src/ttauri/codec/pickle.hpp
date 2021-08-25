// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include <string>

namespace tt {

/** Encode and decode a type to and from a UTF-8 string.
 *
 * This codec is used to serialize data into strings, for example
 * in JSON configuration files.
 */
template<typename T>
struct pickle {
    /** Encode the value of a type into a UTF-8 string.
     *
     * @param rhs The value to encode
     * @return The encoded value as a string.
     */
    [[nodiscard]] std::string encode(T const &rhs) const noexcept;

    /** Decode a UTF-8 string into a value of a given type.
     *
     * @param rhs The string to decode
     * @return The decoded value.
     * @throws parse_error When the decoded value is incorrect.
     */
    [[nodiscard]] T decode(std::string_view rhs) const;
};

} // namespace tt
