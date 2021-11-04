// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include "datum.hpp"
#include "concepts.hpp"
#include <string>
#include <limits>

namespace tt::inline v1 {

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
    [[nodiscard]] datum encode(T const &rhs) const noexcept;

    /** Decode a UTF-8 string into a value of a given type.
     *
     * @param rhs The string to decode
     * @return The decoded value.
     * @throws parse_error When the decoded value is incorrect.
     */
    [[nodiscard]] T decode(datum rhs) const;
};

template<numeric_integral T>
struct pickle<T> {
    [[nodiscard]] datum encode(T const &rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] T decode(datum rhs) const
    {
        if (auto *i = get_if<long long>(rhs)) {
            if (*i < std::numeric_limits<T>::lowest() or *i > std::numeric_limits<T>::max()) {
                throw parse_error("Encoded value is to out of range, got {}", rhs);
            }
            return static_cast<T>(*i);
        } else {
            throw parse_error("Expecting numeric integrals to be encoded as a long long, got {}", rhs);
        }
    }
};

template<std::floating_point T>
struct pickle<T> {
    [[nodiscard]] datum encode(T const &rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] T decode(datum rhs) const
    {
        if (auto *f = get_if<double>(rhs)) {
            return static_cast<T>(*f);

        } else if (auto *i = get_if<long long>(rhs)) {
            return static_cast<T>(*i);

        } else {
            throw parse_error("Expecting floating point to be encoded as a double or long long, got {}", rhs);
        }
    }
};

template<>
struct pickle<bool> {
    [[nodiscard]] datum encode(bool const &rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] bool decode(datum rhs) const
    {
        if (auto *b = get_if<bool>(rhs)) {
            return *b;

        } else {
            throw parse_error("Expecting bool to be encoded as a bool, got {}", rhs);
        }
    }
};

} // namespace tt
