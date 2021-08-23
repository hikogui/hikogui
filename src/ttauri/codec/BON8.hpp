// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../byte_string.hpp"
#include "../required.hpp"
#include "../datum.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include <cstddef>
#include <string>

namespace tt {
namespace detail {
constexpr auto BON8_code_float_min_one = uint8_t{0xba};
constexpr auto BON8_code_float_zero = uint8_t{0xbb};
constexpr auto BON8_code_float_one = uint8_t{0xbc};
constexpr auto BON8_code_array_empty = uint8_t{0xbd};
constexpr auto BON8_code_object_empty = uint8_t{0xbe};
constexpr auto BON8_code_null = uint8_t{0xbf};
constexpr auto BON8_code_bool_false = uint8_t{0xc0};
constexpr auto BON8_code_bool_true = uint8_t{0xc1};
constexpr auto BON8_code_int32 = uint8_t{0xf8};
constexpr auto BON8_code_int64 = uint8_t{0xf9};
constexpr auto BON8_code_binary32 = uint8_t{0xfa};
constexpr auto BON8_code_binary64 = uint8_t{0xfb};
constexpr auto BON8_code_array = uint8_t{0xfc};
constexpr auto BON8_code_object = uint8_t{0xfd};
constexpr auto BON8_code_eoc = uint8_t{0xfe};
constexpr auto BON8_code_eot = uint8_t{0xff};

/** Decode BON8 message from buffer.
 * @param ptr [in,out] Pointer to start of byte-buffer. After the call
 *            ptr will point one beyond the message.
 * @param last Pointer one beyond the end of the message.
 * @return The decoded message.
 */
[[nodiscard]] datum decode_BON8(cbyteptr &ptr, cbyteptr last);

[[nodiscard]] bstring encode_BON8(datum const &value);

/** BON8 encoder.
 */
class BON8_encoder {
    bool open_string;
    bstring output;

public:
    BON8_encoder() noexcept : open_string(false), output() {}

    /** Return a byte_string of the encoded object.
     */
    bstring const &get() const noexcept
    {
        return output;
    }

    /** And a signed integer.
     * @param value A signed integer.
     */
    void add(signed long long value) noexcept
    {
        open_string = false;

        if (value < std::numeric_limits<int32_t>::min()) {
            output += static_cast<std::byte>(BON8_code_int64);
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(value >> (56 - i * 8));
            }

        } else if (value < -33554432) {
            output += static_cast<std::byte>(BON8_code_int32);
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(value >> (24 - i * 8));
            }

        } else if (value < -262144) {
            value = -value - 1;
            output += static_cast<std::byte>(0xf0 + (value >> 22 & 0x07));
            output += static_cast<std::byte>(0xc0 + (value >> 16 & 0x3f));
            output += static_cast<std::byte>(value >> 8);
            output += static_cast<std::byte>(value);

        } else if (value < -1920) {
            value = -value - 1;
            output += static_cast<std::byte>(0xe0 + (value >> 14 & 0x0f));
            output += static_cast<std::byte>(0xc0 + (value >> 8 & 0x3f));
            output += static_cast<std::byte>(value);

        } else if (value < -10) {
            value = -value - 1;
            output += static_cast<std::byte>(0xc2 + (value >> 6 & 0x1f));
            output += static_cast<std::byte>(0xc0 + (value & 0x3f));

        } else if (value < 0) {
            value = -value - 1;
            output += static_cast<std::byte>(0xb0 + value);

        } else if (value <= 47) {
            output += static_cast<std::byte>(0x80 + value);

        } else if (value <= 3839) {
            output += static_cast<std::byte>(0xc2 + (value >> 7 & 0x1f));
            output += static_cast<std::byte>(value & 0x7f);

        } else if (value <= 524287) {
            output += static_cast<std::byte>(0xe0 + (value >> 15 & 0x0f));
            output += static_cast<std::byte>(value >> 8 & 0x7f);
            output += static_cast<std::byte>(value);

        } else if (value <= 67108863) {
            output += static_cast<std::byte>(0xf0 + (value >> 23 & 0x17));
            output += static_cast<std::byte>(value >> 16 & 0x7f);
            output += static_cast<std::byte>(value >> 8);
            output += static_cast<std::byte>(value);

        } else if (value <= std::numeric_limits<int32_t>::max()) {
            output += static_cast<std::byte>(BON8_code_int32);
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(value >> (24 - i * 8));
            }

        } else {
            output += static_cast<std::byte>(BON8_code_int64);
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(value >> (56 - i * 8));
            }
        }
    }

    /** And a unsigned integer.
     * @param value A unsigned integer.
     */
    void add(unsigned long long value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a signed integer.
     * @param value A signed integer.
     */
    void add(signed long value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a unsigned integer.
     * @param value A unsigned integer.
     */
    void add(unsigned long value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a signed integer.
     * @param value A signed integer.
     */
    void add(signed int value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a unsigned integer.
     * @param value A unsigned integer.
     */
    void add(unsigned int value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a signed integer.
     * @param value A signed integer.
     */
    void add(signed short value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a unsigned integer.
     * @param value A unsigned integer.
     */
    void add(unsigned short value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a signed integer.
     * @param value A signed integer.
     */
    void add(signed char value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** And a unsigned integer.
     * @param value A unsigned integer.
     */
    void add(unsigned char value) noexcept
    {
        return add(narrow_cast<signed long long>(value));
    }

    /** Add a floating point number.
     * @param value A floating point number.
     */
    void add(double value) noexcept
    {
        open_string = false;

        ttlet f32 = static_cast<float>(value);
        ttlet f32_64 = static_cast<double>(f32);

        if (value == -1.0) {
            output += static_cast<std::byte>(BON8_code_float_min_one);

        } else if (value == 0.0 || value == -0.0) {
            output += static_cast<std::byte>(BON8_code_float_zero);

        } else if (value == 1.0) {
            output += static_cast<std::byte>(BON8_code_float_one);

        } else if (f32_64 == value) {
            uint32_t u32;
            std::memcpy(&u32, &f32, sizeof(u32));

            output += static_cast<std::byte>(BON8_code_binary32);
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(u32 >> (24 - i * 8));
            }

        } else {
            uint64_t u64;
            std::memcpy(&u64, &value, sizeof(u64));

            output += static_cast<std::byte>(BON8_code_binary64);
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(u64 >> (56 - i * 8));
            }
        }
    }

    /** Add a floating point number.
     * @param value A floating point number.
     */
    void add(float value) noexcept
    {
        return add(narrow_cast<double>(value));
    }

    /** Add a boolean.
     * @param value A boolean value.
     */
    void add(bool value) noexcept
    {
        open_string = false;
        output += static_cast<std::byte>(value ? BON8_code_bool_true : BON8_code_bool_false);
    }

    /** Add a null.
     * @param value A null pointer.
     */
    void add(nullptr_t value) noexcept
    {
        open_string = false;
        output += static_cast<std::byte>(BON8_code_null);
    }

    /** Add a UTF-8 string.
     * It is important that the UTF-8 string is valid.
     *
     * @param value A UTF-8 string.
     */
    void add(std::string_view value) noexcept
    {
        if (open_string) {
            output += static_cast<std::byte>(BON8_code_eot);
        }

        if (std::ssize(value) == 0) {
            output += static_cast<std::byte>(BON8_code_eot);
            open_string = false;

        } else {
            int multi_byte = 0;

            for (ttlet _c : value) {
                ttlet c = static_cast<uint8_t>(_c);

                if constexpr (build_type::current == build_type::debug) {
                    if (multi_byte == 0) {
                        if (c >= 0xc2 && c <= 0xdf) {
                            multi_byte = 1;
                        } else if (c >= 0xe0 && c <= 0xef) {
                            multi_byte = 2;
                        } else if (c >= 0xf0 && c <= 0xf7) {
                            multi_byte = 3;
                        } else {
                            tt_assert(c <= 0x7f);
                        }

                    } else {
                        tt_assert(c >= 0x80 && c <= 0xbf);
                        --multi_byte;
                    }
                }

                output += static_cast<std::byte>(c);
            }

            tt_assert(multi_byte == 0);

            open_string = true;
        }
    }

    /** Add a UTF-8 string.
     * It is important that the UTF-8 string is valid.
     *
     * @param value A UTF-8 string.
     */
    void add(std::string const &value) noexcept
    {
        add(std::string_view{value});
    }

    /** Add a UTF-8 string.
     * It is important that the UTF-8 string is valid.
     *
     * @param value A UTF-8 string.
     */
    void add(char const *value) noexcept
    {
        add(std::string_view{value});
    }

    /** Add a datum.
     * @param value A datum.
     */
    void add(datum const &value);

    /** Add a vector of values of the same type.
     * @tparam T Type of the values.
     * @param items A vector of values.
     */
    template<typename T>
    void add(std::vector<T> const &items)
    {
        open_string = false;
        if (std::ssize(items) == 0) {
            output += static_cast<std::byte>(BON8_code_array_empty);
        } else {
            output += static_cast<std::byte>(BON8_code_array);

            for (ttlet &item : items) {
                add(item);
            }

            output += static_cast<std::byte>(BON8_code_eoc);
        }
        open_string = false;
    }

    /** Add a map of key/values pairs.
     * @tparam Key A type convertible to a string_view; a valid UTF-8 string.
     * @tparam Value The type of the Value.
     * @param items The map of key/value pairs.
     */
    template<typename Key, typename Value>
    void add(std::map<Key, Value> const &items)
    {
        using key_type = typename std::remove_cvref_t<decltype(items)>::key_type;

        open_string = false;
        if (std::ssize(items) == 0) {
            output += static_cast<std::byte>(BON8_code_object_empty);

        } else {
            output += static_cast<std::byte>(BON8_code_object);
            for (ttlet &item : items) {
                add(static_cast<std::string_view>(item.first));
                add(item.second);
            }
            output += static_cast<std::byte>(BON8_code_eoc);
        }
        open_string = false;
    }
};

void BON8_encoder::add(datum const &value)
{
    if (auto s = get_if<std::string>(value)) {
        add(*s);
    } else if (auto u = get_if<URL>(value)) {
        add(to_string(*u));
    } else if (auto b = get_if<bool>(value)) {
        add(*b);
    } else if (holds_alternative<nullptr_t>(value)) {
        add(nullptr);
    } else if (auto i = get_if<long long>(value)) {
        add(*i);
    } else if (auto f = get_if<double>(value)) {
        add(*f);
    } else if (auto v = get_if<datum::vector_type>(value)) {
        add(*v);
    } else if (auto m = get_if<datum::map_type>(value)) {
        add(*m);
    } else {
        throw operation_error("Datum value can not be encoded to BON8");
    }
}

/** Count the number of UTF-8-like code units
 * This does not really decode the character, just calculate the size.
 *
 * @param ptr The pointer to the first byte of a UTF-8-like multibyte sequence
 * @param last The pointer beyond the buffer.
 * @return When positive: the number of bytes in the UTF-8 character.
 *         When negative: the number of bytes in the integer.
 */
[[nodiscard]] int BON8_multibyte_count(cbyteptr ptr, cbyteptr last)
{
    ttlet c0 = static_cast<uint8_t>(*ptr);
    int count = c0 <= 0xdf ? 2 : c0 <= 0xef ? 3 : 4;

    tt_parse_check(ptr + count <= last, "Incomplete Multi-byte character at end of buffer");

    ttlet c1 = static_cast<uint8_t>(*(ptr + 1));
    return (c1 < 0x80 || c1 > 0xbf) ? -count : count;
}

/** Decode a 4, or 8 byte signed integer.
 *
 * @param [in,out] ptr The pointer to the first byte of the integer.
 *                     On return this points beyond the integer.
 * @param last The pointer beyond the buffer.
 * @param count The number of bytes used to encode the integer.
 * @return The integer as a datum.
 */
[[nodiscard]] datum decode_BON8_int(cbyteptr &ptr, cbyteptr last, int count)
{
    tt_axiom(count == 4 || count == 8);

    auto u64 = uint64_t{0};
    for (int i = 0; i != count; ++i) {
        tt_parse_check(ptr != last, "Incomplete signed integer at end of buffer");
        u64 <<= 8;
        u64 |= static_cast<uint64_t>(*(ptr++));
    }

    if (count == 4) {
        ttlet u32 = static_cast<uint32_t>(u64);
        ttlet i32 = static_cast<int32_t>(u32);
        return datum{i32};
    } else {
        ttlet i64 = static_cast<int64_t>(u64);
        return datum{i64};
    }
}

[[nodiscard]] datum decode_BON8_float(cbyteptr &ptr, cbyteptr last, int count)
{
    tt_axiom(count == 4 || count == 8);

    auto u64 = uint64_t{0};
    for (int i = 0; i != count; ++i) {
        tt_parse_check(ptr != last, "Incomplete signed integer at end of buffer");
        u64 <<= 8;
        u64 |= static_cast<uint64_t>(*(ptr++));
    }

    if (count == 4) {
        ttlet u32 = static_cast<uint32_t>(u64);
        float f32;
        std::memcpy(&f32, &u32, sizeof(f32));
        return datum{f32};

    } else {
        double f64;
        std::memcpy(&f64, &u64, sizeof(f64));
        return datum{f64};
    }
}

[[nodiscard]] datum decode_BON8_array(cbyteptr &ptr, cbyteptr last)
{
    auto r = datum::vector_type{};

    while (ptr != last) {
        if (*ptr == static_cast<std::byte>(BON8_code_eoc)) {
            ++ptr;
            return datum{std::move(r)};

        } else {
            r.push_back(decode_BON8(ptr, last));
        }
    }
    throw parse_error("Incomplete array at end of buffer");
}

[[nodiscard]] datum decode_BON8_object(cbyteptr &ptr, cbyteptr last)
{
    auto r = datum::map_type{};

    while (ptr != last) {
        if (*ptr == static_cast<std::byte>(BON8_code_eoc)) {
            ++ptr;
            return datum{std::move(r)};

        } else {
            auto key = decode_BON8(ptr, last);
            tt_parse_check(holds_alternative<std::string>(key), "Key in object is not a string");

            auto value = decode_BON8(ptr, last);
            r.emplace(std::move(key), std::move(value));
        }
    }
    throw parse_error("Incomplete array at end of buffer");
}

[[nodiscard]] datum decode_BON8_UTF8_like_int(cbyteptr &ptr, cbyteptr last, int count) noexcept
{
    tt_axiom(count >= 2 && count <= 4);
    tt_axiom(ptr != last);
    ttlet c0 = static_cast<uint8_t>(*(ptr++));

    ttlet mask = int{0b0111'1111} >> count;
    auto value = static_cast<int>(c0) & mask;
    if (count == 2) {
        value -= 2;
    }

    tt_axiom(ptr != last);
    ttlet c1 = static_cast<uint8_t>(*(ptr++));
    ttlet is_positive = c1 <= 0x7f;
    if (is_positive) {
        value <<= 7;
        value |= static_cast<int>(c1);
    } else {
        value <<= 6;
        value |= static_cast<int>(c1 & 0b0011'11111);
    }

    switch (count) {
    case 4:
        tt_axiom(ptr != last);
        value <<= 8;
        value |= static_cast<int>(*(ptr++));
        [[fallthrough]];
    case 3:
        tt_axiom(ptr != last);
        value <<= 8;
        value |= static_cast<int>(*(ptr++));
        [[fallthrough]];
    default:;
    }

    return datum{is_positive ? value : -value};
}

[[nodiscard]] datum decode_BON8(cbyteptr &ptr, cbyteptr last)
{
    std::string str;

    while (ptr != last) {
        ttlet c = static_cast<uint8_t>(*ptr);

        if (c == BON8_code_eot) {
            // End of string found, return the current string.
            ++ptr;
            return datum{str};

        } else if (c <= 0x7f) {
            // ASCII character.
            str += static_cast<char>(*(ptr++));
            continue;

        } else if (c >= 0xc2 && c <= 0xf7) {
            ttlet count = BON8_multibyte_count(ptr, last);
            if (count > 0) {
                // Multibyte UTF-8 character
                for (int i = 0; i != count; ++i) {
                    str += static_cast<char>(*(ptr++));
                }
                continue;

            } else if (std::ssize(str) != 0) {
                // Multibyte integer found, but first return the current string.
                return datum{str};

            } else {
                // Multibyte integer.
                return decode_BON8_UTF8_like_int(ptr, last, -count);
            }

        } else if (std::ssize(str) != 0) {
            // This must be a non-string type, but first return the current string.
            return datum{str};

            // Everything below this, are non-string types.
        } else if (c <= 0xaf) {
            // 1 byte positive integer
            return datum{c - 0x80};

        } else if (c <= 0xb9) {
            // 1 byte negative integer
            return datum{-static_cast<int>(c - 0xb0)};

        } else {
            // This is one of the non-string types.
            switch (c) {
            case BON8_code_null: ++ptr; return datum{nullptr};

            case BON8_code_bool_false: ++ptr; return datum{false};

            case BON8_code_bool_true: ++ptr; return datum{true};

            case BON8_code_float_min_one: ++ptr; return datum{-1.0f};

            case BON8_code_float_zero: ++ptr; return datum{-0.0f};

            case BON8_code_float_one: ++ptr; return datum{1.0f};

            case BON8_code_int32: ++ptr; return decode_BON8_int(ptr, last, 4);

            case BON8_code_int64: ++ptr; return decode_BON8_int(ptr, last, 8);

            case BON8_code_binary32: ++ptr; return decode_BON8_float(ptr, last, 4);

            case BON8_code_binary64: ++ptr; return decode_BON8_float(ptr, last, 8);

            case BON8_code_eoc: throw parse_error("Unexpected end-of-container");

            case BON8_code_array: ++ptr; return decode_BON8_array(ptr, last);

            case BON8_code_object: ++ptr; return decode_BON8_object(ptr, last);

            default: tt_no_default();
            }
        }
    }
    throw parse_error("Unexpected end-of-buffer");
}
} // namespace detail

/** Decode BON8 message from buffer.
 * @param buffer A buffer to a BON8 encoded message.
 * @return The decoded message.
 */
[[nodiscard]] datum decode_BON8(std::span<const std::byte> buffer)
{
    auto *ptr = buffer.data();
    auto *last = ptr + buffer.size();
    return detail::decode_BON8(ptr, last);
}

/** Decode BON8 message from buffer.
 * @param buffer A buffer to a BON8 encoded message.
 * @return The decoded message.
 */
[[nodiscard]] datum decode_BON8(bstring const &buffer)
{
    auto *ptr = buffer.data();
    auto *last = ptr + buffer.size();
    return detail::decode_BON8(ptr, last);
}

/** Decode BON8 message from buffer.
 * @param buffer A buffer to a BON8 encoded message.
 * @return The decoded message.
 */
[[nodiscard]] datum decode_BON8(bstring_view buffer)
{
    auto *ptr = buffer.data();
    auto *last = ptr + buffer.size();
    return detail::decode_BON8(ptr, last);
}

/** Encode a value to a BON8 message.
 * @param value The data to encode
 * @return The encoded message as a byte_string.
 */
[[nodiscard]] bstring encode_BON8(datum const &value)
{
    auto encoder = detail::BON8_encoder{};
    encoder.add(value);
    return encoder.get();
}

} // namespace tt
