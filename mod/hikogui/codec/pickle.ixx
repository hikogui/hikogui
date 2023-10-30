// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string>
#include <limits>

export module hikogui_codec_pickle;
import hikogui_codec_base_n;
import hikogui_codec_datum;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/** Encode and decode a type to and from a UTF-8 string.
 *
 * This codec is used to serialize data into strings, for example
 * in JSON configuration files.
 */
export template<typename T>
struct pickle {
    /** Encode the value of a type into a UTF-8 string.
     *
     * @param rhs The value to encode
     * @return The encoded value as a string.
     */
    [[nodiscard]] datum encode(T const& rhs) const noexcept
    {
        hi_static_not_implemented();
    }

    [[nodiscard]] datum encode(T const& rhs) const noexcept
        requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>)
    {
        auto *rhs_ = reinterpret_cast<std::byte const *>(&rhs);
        return datum{base64::encode({rhs_, sizeof(rhs_)})};
    }

    /** Decode a UTF-8 string into a value of a given type.
     *
     * @param rhs The string to decode
     * @return The decoded value.
     * @throws parse_error When the decoded value is incorrect.
     */
    [[nodiscard]] T decode(datum rhs) const
    {
        hi_static_not_implemented();
    }

    [[nodiscard]] T decode(datum rhs) const
        requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>)
    {
        if (auto *b = get_if<std::string>(rhs)) {
            auto tmp = base64::decode(*b);
            if (tmp.size() != sizeof(T)) {
                throw parse_error(
                    std::format("Length of base64 encoded object is {}, expected length {}", tmp.size(), sizeof(T)));
            }

            auto r = T{};
            std::memcpy(&r, tmp.data(), sizeof(T));
            return r;

        } else {
            throw parse_error(std::format("Expecting std::string to be encoded as a base64-string, got {}", rhs));
        }
    }
};

export template<numeric_integral T>
struct pickle<T> {
    [[nodiscard]] datum encode(T const& rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] T decode(datum rhs) const
    {
        if (auto *i = get_if<long long>(rhs)) {
            if (*i < std::numeric_limits<T>::lowest() or *i > std::numeric_limits<T>::max()) {
                throw parse_error(std::format("Encoded value is to out of range, got {}", rhs));
            }
            return static_cast<T>(*i);
        } else {
            throw parse_error(std::format("Expecting numeric integrals to be encoded as a long long, got {}", rhs));
        }
    }
};

export template<std::floating_point T>
struct pickle<T> {
    [[nodiscard]] datum encode(T const& rhs) const noexcept
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
            throw parse_error(std::format("Expecting floating point to be encoded as a double or long long, got {}", rhs));
        }
    }
};

export template<>
struct pickle<bool> {
    [[nodiscard]] datum encode(bool const& rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] bool decode(datum rhs) const
    {
        if (auto *b = get_if<bool>(rhs)) {
            return *b;

        } else {
            throw parse_error(std::format("Expecting bool to be encoded as a bool, got {}", rhs));
        }
    }
};

export template<>
struct pickle<std::string> {
    [[nodiscard]] datum encode(std::string const& rhs) const noexcept
    {
        return datum{rhs};
    }

    [[nodiscard]] std::string decode(datum rhs) const
    {
        if (auto *b = get_if<std::string>(rhs)) {
            return *b;

        } else {
            throw parse_error(std::format("Expecting std::string to be encoded as a string, got {}", rhs));
        }
    }
};

}} // namespace hi::v1
