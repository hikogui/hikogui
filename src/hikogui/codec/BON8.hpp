// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../byte_string.hpp"
#include "../utility/module.hpp"
#include "../datum.hpp"
#include <cstddef>
#include <string>

hi_warning_push();
// C26429: Symbol '' is never tested for nullness, it can be marked as not_null (f.23)
// False positive reported: https://developercommunity.visualstudio.com/t/C26429-false-positive-on-reference-to-po/10262151
hi_warning_ignore_msvc(26429);

namespace hi::inline v1 {
namespace detail {
constexpr auto BON8_code_array_count0 = uint8_t{0x80};
constexpr auto BON8_code_array_count1 = uint8_t{0x81};
constexpr auto BON8_code_array_count2 = uint8_t{0x82};
constexpr auto BON8_code_array_count3 = uint8_t{0x83};
constexpr auto BON8_code_array_count4 = uint8_t{0x84};
constexpr auto BON8_code_array = uint8_t{0x85};
constexpr auto BON8_code_object_count0 = uint8_t{0x86};
constexpr auto BON8_code_object_count1 = uint8_t{0x87};
constexpr auto BON8_code_object_count2 = uint8_t{0x88};
constexpr auto BON8_code_object_count3 = uint8_t{0x89};
constexpr auto BON8_code_object_count4 = uint8_t{0x8a};
constexpr auto BON8_code_object = uint8_t{0x8b};
constexpr auto BON8_code_int32 = uint8_t{0x8c};
constexpr auto BON8_code_int64 = uint8_t{0x8d};
constexpr auto BON8_code_binary32 = uint8_t{0x8e};
constexpr auto BON8_code_binary64 = uint8_t{0x8f};
constexpr auto BON8_code_positive_s = uint8_t{0x90};
constexpr auto BON8_code_positive_e = uint8_t{0xb7};
constexpr auto BON8_code_negative_s = uint8_t{0xb8};
constexpr auto BON8_code_negative_e = uint8_t{0xc1};

// The last 8 code-units after the extended characters.
constexpr auto BON8_code_bool_false = uint8_t{0xf8};
constexpr auto BON8_code_bool_true = uint8_t{0xf9};
constexpr auto BON8_code_null = uint8_t{0xfa};
constexpr auto BON8_code_float_min_one = uint8_t{0xfb};
constexpr auto BON8_code_float_zero = uint8_t{0xfc};
constexpr auto BON8_code_float_one = uint8_t{0xfd};
constexpr auto BON8_code_eoc = uint8_t{0xfe};
constexpr auto BON8_code_eot = uint8_t{0xff};

/** Decode BON8 message from buffer.
 * @param ptr [in,out] Pointer to start of byte-buffer. After the call
 *            ptr will point one beyond the message.
 * @param last Pointer one beyond the end of the message.
 * @return The decoded message.
 */
[[nodiscard]] datum decode_BON8(cbyteptr& ptr, cbyteptr last);

[[nodiscard]] bstring encode_BON8(datum const& value);

/** BON8 encoder.
 */
class BON8_encoder {
    bool open_string;
    bstring output;

public:
    BON8_encoder() noexcept : open_string(false), output() {}

    /** Return a byte_string of the encoded object.
     */
    bstring const& get() noexcept
    {
        if (open_string) {
            output += static_cast<std::byte>(BON8_code_eot);
            open_string = false;
        }
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

        } else if (value <= -33818507) {
            output += static_cast<std::byte>(BON8_code_int32);
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(value >> (24 - i * 8));
            }

        } else if (value <= -264075) {
            value = -(value + 264075);
            output += static_cast<std::byte>(0xf0 + (value >> 22 & 0x07));
            output += static_cast<std::byte>(0xc0 + (value >> 16 & 0x3f));
            output += static_cast<std::byte>(value >> 8);
            output += static_cast<std::byte>(value);

        } else if (value <= -1931) {
            value = -(value + 1931);
            output += static_cast<std::byte>(0xe0 + (value >> 14 & 0x0f));
            output += static_cast<std::byte>(0xc0 + (value >> 8 & 0x3f));
            output += static_cast<std::byte>(value);

        } else if (value <= -11) {
            value = -(value + 11);
            output += static_cast<std::byte>(0xc2 + (value >> 6 & 0x1f));
            output += static_cast<std::byte>(0xc0 + (value & 0x3f));

        } else if (value <= -1) {
            value = -(value + 1);
            output += static_cast<std::byte>(BON8_code_negative_s + value);

        } else if (value <= 39) {
            output += static_cast<std::byte>(BON8_code_positive_s + value);

        } else if (value <= 3879) {
            value -= 40;
            output += static_cast<std::byte>(0xc2 + (value >> 7 & 0x1f));
            output += static_cast<std::byte>(value & 0x7f);

        } else if (value <= 528167) {
            value -= 3880;
            output += static_cast<std::byte>(0xe0 + (value >> 15 & 0x0f));
            output += static_cast<std::byte>(value >> 8 & 0x7f);
            output += static_cast<std::byte>(value);

        } else if (value <= 67637031) {
            value -= 528168;
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

        hilet f32 = static_cast<float>(value);
        hilet f32_64 = static_cast<double>(f32);

        if (value == -1.0) {
            output += static_cast<std::byte>(BON8_code_float_min_one);

        } else if (value == 0.0 and not std::signbit(value)) {
            output += static_cast<std::byte>(BON8_code_float_zero);

        } else if (value == 1.0) {
            output += static_cast<std::byte>(BON8_code_float_one);

        } else if (f32_64 == value) {
            // After conversion to 32-bit float, precession was not reduced.
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
        return add(static_cast<double>(value));
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

        if (value.empty()) {
            output += static_cast<std::byte>(BON8_code_eot);
            open_string = false;

        } else {
            int multi_byte = 0;

            for (hilet _c : value) {
                hilet c = truncate<uint8_t>(_c);

#ifndef NDEBUG
                if (multi_byte == 0) {
                    if (c >= 0xc2 and c <= 0xdf) {
                        multi_byte = 1;
                    } else if (c >= 0xe0 and c <= 0xef) {
                        multi_byte = 2;
                    } else if (c >= 0xf0 and c <= 0xf7) {
                        multi_byte = 3;
                    } else {
                        hi_assert(c <= 0x7f);
                    }

                } else {
                    hi_assert(c >= 0x80 and c <= 0xbf);
                    --multi_byte;
                }
#endif

                output += static_cast<std::byte>(c);
            }
            hi_assert(multi_byte == 0);

            open_string = true;
        }
    }

    /** Add a UTF-8 string.
     * It is important that the UTF-8 string is valid.
     *
     * @param value A UTF-8 string.
     */
    void add(std::string const& value) noexcept
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
    void add(datum const& value);

    /** Add a vector of values of the same type.
     * @tparam T Type of the values.
     * @param items A vector of values.
     */
    template<typename T>
    void add(std::vector<T> const& items)
    {
        open_string = false;
        if (size(items) <= 4) {
            output += static_cast<std::byte>(BON8_code_array_count0 + size(items));
        } else {
            output += static_cast<std::byte>(BON8_code_array);
        }

        for (hilet& item : items) {
            add(item);
        }

        if (size(items) > 4) {
            output += static_cast<std::byte>(BON8_code_eoc);
            open_string = false;
        }
    }

    /** Add a map of key/values pairs.
     * @tparam Key A type convertible to a string_view; a valid UTF-8 string.
     * @tparam Value The type of the Value.
     * @param items The map of key/value pairs.
     */
    template<typename Key, typename Value>
    void add(std::map<Key, Value> const& items)
    {
        using key_type = typename std::remove_cvref_t<decltype(items)>::key_type;

        open_string = false;
        if (size(items) <= 4) {
            output += static_cast<std::byte>(BON8_code_object_count0 + size(items));
        } else {
            output += static_cast<std::byte>(BON8_code_object);
        }

        for (hilet& item : items) {
            if (auto *s = get_if<std::string>(item.first)) {
                add(*s);
            } else {
                throw operation_error("BON8 object keys must be strings");
            }
            add(item.second);
        }

        if (size(items) > 4) {
            output += static_cast<std::byte>(BON8_code_eoc);
            open_string = false;
        }
    }
};

void BON8_encoder::add(datum const& value)
{
    if (auto s = get_if<std::string>(value)) {
        add(*s);
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
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);

    hilet c0 = static_cast<uint8_t>(*ptr);
    hilet count = c0 <= 0xdf ? 2 : c0 <= 0xef ? 3 : 4;

    hi_check(ptr + count <= last, "Incomplete Multi-byte character at end of buffer");

    hilet c1 = static_cast<uint8_t>(*(ptr + 1));
    return (c1 < 0x80 or c1 > 0xbf) ? -count : count;
}

/** Decode a 4, or 8 byte signed integer.
 *
 * @param[in,out] ptr The pointer to the first byte of the integer.
 *                     On return this points beyond the integer.
 * @param last The pointer beyond the buffer.
 * @param count The number of bytes used to encode the integer.
 * @return The integer as a datum.
 */
[[nodiscard]] datum decode_BON8_int(cbyteptr& ptr, cbyteptr last, int count)
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);
    hi_assert(count == 4 || count == 8);

    auto u64 = uint64_t{0};
    for (int i = 0; i != count; ++i) {
        hi_check(ptr != last, "Incomplete signed integer at end of buffer");
        u64 <<= 8;
        u64 |= static_cast<uint64_t>(*(ptr++));
    }

    if (count == 4) {
        hilet u32 = truncate<uint32_t>(u64);
        hilet i32 = truncate<int32_t>(u32);
        return datum{i32};
    } else {
        hilet i64 = truncate<int64_t>(u64);
        return datum{i64};
    }
}

[[nodiscard]] datum decode_BON8_float(cbyteptr& ptr, cbyteptr last, int count)
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);
    hi_assert(count == 4 || count == 8);

    auto u64 = uint64_t{0};
    for (int i = 0; i != count; ++i) {
        hi_check(ptr != last, "Incomplete signed integer at end of buffer");
        u64 <<= 8;
        u64 |= static_cast<uint64_t>(*(ptr++));
    }

    if (count == 4) {
        hilet u32 = truncate<uint32_t>(u64);
        float f32;
        std::memcpy(&f32, &u32, sizeof(f32));
        return datum{f32};

    } else {
        double f64;
        std::memcpy(&f64, &u64, sizeof(f64));
        return datum{f64};
    }
}

[[nodiscard]] datum decode_BON8_array(cbyteptr& ptr, cbyteptr last)
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);

    auto r = datum::make_vector();
    auto& vector = get<datum::vector_type>(r);

    while (ptr != last) {
        if (*ptr == static_cast<std::byte>(BON8_code_eoc)) {
            ++ptr;
            return r;

        } else {
            vector.push_back(decode_BON8(ptr, last));
        }
    }
    throw parse_error("Incomplete array at end of buffer");
}

[[nodiscard]] datum decode_BON8_array(cbyteptr& ptr, cbyteptr last, std::size_t count)
{
    auto r = datum::make_vector();
    auto& vector = get<datum::vector_type>(r);

    while (count--) {
        vector.push_back(decode_BON8(ptr, last));
    }
    return r;
}

[[nodiscard]] datum decode_BON8_object(cbyteptr& ptr, cbyteptr last)
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);

    auto r = datum::make_map();
    auto& map = get<datum::map_type>(r);

    while (ptr != last) {
        if (*ptr == static_cast<std::byte>(BON8_code_eoc)) {
            ++ptr;
            return r;

        } else {
            auto key = decode_BON8(ptr, last);
            hi_check(holds_alternative<std::string>(key), "Key in object is not a string");

            auto value = decode_BON8(ptr, last);
            map.emplace(std::move(key), std::move(value));
        }
    }
    throw parse_error("Incomplete object at end of buffer");
}

[[nodiscard]] datum decode_BON8_object(cbyteptr& ptr, cbyteptr last, std::size_t count)
{
    auto r = datum::make_map();
    auto& map = get<datum::map_type>(r);

    while (count--) {
        auto key = decode_BON8(ptr, last);
        hi_check(holds_alternative<std::string>(key), "Key in object is not a string");

        auto value = decode_BON8(ptr, last);
        map.emplace(std::move(key), std::move(value));
    }
    return r;
}

[[nodiscard]] long long decode_BON8_UTF8_like_int(cbyteptr& ptr, cbyteptr last, int count) noexcept
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);
    hi_assert(count >= 2 && count <= 4);
    hi_assert(ptr != last);
    hilet c0 = static_cast<uint8_t>(*(ptr++));

    hilet mask = uint8_t{0b0111'1111} >> count;
    auto value = static_cast<long long>(c0 & mask);
    if (count == 2) {
        // The two byte sequence starts with 0xc2, leaving only 30 entries in the first byte.
        value -= 2;
    }

    // The second byte determines the sign, and adds 6 or 7 bits to the number.
    hi_assert(ptr != last);
    hilet c1 = static_cast<uint8_t>(*(ptr++));
    hilet is_positive = c1 <= 0x7f;
    if (is_positive) {
        value <<= 7;
        value |= static_cast<long long>(c1);
    } else {
        value <<= 6;
        value |= static_cast<long long>(c1 & 0b0011'1111);
    }

    switch (count) {
    case 4:
        hi_assert(ptr != last);
        value <<= 8;
        value |= static_cast<int>(*(ptr++));
        [[fallthrough]];
    case 3:
        hi_assert(ptr != last);
        value <<= 8;
        value |= static_cast<int>(*(ptr++));
        [[fallthrough]];
    default:;
    }

    if (is_positive) {
        switch (count) {
        case 2:
            return value + 40;
        case 3:
            return value + 3880;
        case 4:
            return value + 528168;
        default:
            hi_no_default();
        }

    } else {
        switch (count) {
        case 2:
            return -(value + 11);
        case 3:
            return -(value + 1931);
        case 4:
            return -(value + 264075);
        default:
            hi_no_default();
        }
    }
}

[[nodiscard]] datum decode_BON8(cbyteptr& ptr, cbyteptr last)
{
    hi_assert_not_null(ptr);
    hi_assert_not_null(last);

    std::string str;

    while (ptr != last) {
        hilet c = static_cast<uint8_t>(*ptr);

        if (c == BON8_code_eot) {
            // End of string found, return the current string.
            ++ptr;
            return datum{str};

        } else if (c <= 0x7f) {
            // ASCII character.
            str += static_cast<char>(*(ptr++));
            continue;

        } else if (c >= 0xc2 && c <= 0xf7) {
            hilet count = BON8_multibyte_count(ptr, last);
            if (count > 0) {
                // Multibyte UTF-8 code-point, The count includes the first code-unit.
                for (int i = 0; i != count; ++i) {
                    str += static_cast<char>(*(ptr++));
                }
                continue;

            } else if (not str.empty()) {
                // Multibyte integer found, but first return the current string.
                return datum{str};

            } else {
                // Multibyte integer, the first code-unit includes part of the integer.
                return datum{decode_BON8_UTF8_like_int(ptr, last, -count)};
            }

        } else if (not str.empty()) {
            // This must be a non-string type, but first return the current string.
            return datum{str};

        } else {
            // This is one of the non-string types.
            ++ptr;
            switch (c) {
            case BON8_code_null:
                return datum{nullptr};
            case BON8_code_bool_false:
                return datum{false};
            case BON8_code_bool_true:
                return datum{true};
            case BON8_code_float_min_one:
                return datum{-1.0f};
            case BON8_code_float_zero:
                return datum{0.0f};
            case BON8_code_float_one:
                return datum{1.0f};
            case BON8_code_int32:
                return decode_BON8_int(ptr, last, 4);
            case BON8_code_int64:
                return decode_BON8_int(ptr, last, 8);
            case BON8_code_binary32:
                return decode_BON8_float(ptr, last, 4);
            case BON8_code_binary64:
                return decode_BON8_float(ptr, last, 8);
            case BON8_code_array_count0:
                return datum::make_vector();
            case BON8_code_array_count1:
                return decode_BON8_array(ptr, last, 1);
            case BON8_code_array_count2:
                return decode_BON8_array(ptr, last, 2);
            case BON8_code_array_count3:
                return decode_BON8_array(ptr, last, 3);
            case BON8_code_array_count4:
                return decode_BON8_array(ptr, last, 4);
            case BON8_code_array:
                return decode_BON8_array(ptr, last);
            case BON8_code_object_count0:
                return datum::make_map();
            case BON8_code_object_count1:
                return decode_BON8_object(ptr, last, 1);
            case BON8_code_object_count2:
                return decode_BON8_object(ptr, last, 2);
            case BON8_code_object_count3:
                return decode_BON8_object(ptr, last, 3);
            case BON8_code_object_count4:
                return decode_BON8_object(ptr, last, 4);
            case BON8_code_object:
                return decode_BON8_object(ptr, last);
            case BON8_code_eoc:
                throw parse_error("Unexpected end-of-container");
            case BON8_code_eot:
                throw parse_error("Unexpected end-of-text");
            default:
                // Everything below this, are non-string types.
                if (c >= BON8_code_positive_s and c <= BON8_code_positive_e) {
                    return datum{c - BON8_code_positive_s};

                } else if (c >= BON8_code_negative_s and c <= BON8_code_negative_e) {
                    return datum{~truncate<int>(c - BON8_code_negative_s)};

                } else {
                    hi_no_default();
                }
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
[[nodiscard]] datum decode_BON8(bstring const& buffer)
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
[[nodiscard]] bstring encode_BON8(datum const& value)
{
    auto encoder = detail::BON8_encoder{};
    encoder.add(value);
    return encoder.get();
}

} // namespace hi::inline v1

hi_warning_pop();
