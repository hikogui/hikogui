// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "URL.hpp"
#include "decimal.hpp"
#include "memory.hpp"
#include "type_traits.hpp"
#include "throw_exception.hpp"
#include "math.hpp"
#include "algorithm.hpp"
#include "byte_string.hpp"
#include <date/date.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <cstdint>
#include <variant>
#include <limits>
#include <type_traits>
#include <typeinfo>
#include <ostream>
#include <numeric>
#include <string_view>
#include <cmath>


namespace tt {
template<bool HasLargeObjects>
class datum_impl;

}

namespace std {

template<bool HasLargeObjects>
class hash<tt::datum_impl<HasLargeObjects>> {
public:
    size_t operator()(tt::datum_impl<HasLargeObjects> const &value) const;
};

}

namespace tt {

enum class datum_type_t {
    Null,
    Undefined,
    Break,
    Continue,
    Boolean,
    Integer,
    Decimal,
    Float,
    String,
    URL,
    Map,
    Vector,
    YearMonthDay,
    Bytes,
};

inline std::ostream &operator<<(std::ostream &lhs, datum_type_t rhs)
{
    switch (rhs) {
    case datum_type_t::Null: lhs << "Null"; break;
    case datum_type_t::Break: lhs << "Break"; break;
    case datum_type_t::Continue: lhs << "Continue"; break;
    case datum_type_t::Undefined: lhs << "Undefined"; break;
    case datum_type_t::Boolean: lhs << "Boolean"; break;
    case datum_type_t::Integer: lhs << "Integer"; break;
    case datum_type_t::Decimal: lhs << "Decimal"; break;
    case datum_type_t::Float: lhs << "Float"; break;
    case datum_type_t::String: lhs << "String"; break;
    case datum_type_t::URL: lhs << "URL"; break;
    case datum_type_t::Map: lhs << "Map"; break;
    case datum_type_t::Vector: lhs << "Vector"; break;
    case datum_type_t::YearMonthDay: lhs << "YearMonthDay"; break;
    case datum_type_t::Bytes: lhs << "Bytes"; break;
    default: tt_no_default;
    }
    return lhs;
}

/** A fixed size (64 bits) class for a generic value type.
 * A datum can hold and do calculations with the following types:
 *  - Floating point number (double, without NaN)
 *  - Signed integer number (52 bits)
 *  - Boolean
 *  - Null
 *  - Break
 *  - Continue
 *  - Undefined
 *  - String
 *  - Vector of datum
 *  - Unordered map of datum:datum.
 *  - YearMonthDay.
 *  - Bytes.
 *
 * Due to the recursive nature of the datum type (through vector and map)
 * you can serialize your own types by adding conversion constructor and
 * operator to and from the datum on your type.
 *
 * @param HasLargeObjects true when the datum will manage memory for large objects 
 */
template<bool HasLargeObjects>
class datum_impl {
private:
    /** Encode 0 to 6 UTF-8 code units in to a uint64_t.
     *
     * @param str String to encode into an uint64_t
     * @return Encoded string, or zero if `str` did not fit.
     */
    static constexpr uint64_t make_string(std::string_view str) {
        ttlet len = str.size();

        if (len > 6) {
            return 0;
        }

        uint64_t x = 0;
        for (uint64_t i = 0; i < len; i++) {
            x <<= 8;
            x |= str[i];
        }
        return (string_mask + (len << 48)) | x;
    }

    /** Encode a pointer into a uint64_t.
     *
     * @param mask A mask signifying the type of the pointer.
     * @param ptr Pointer to encode.
     * @return Encoded pointer.
     */
    static uint64_t make_pointer(uint64_t mask, void *ptr) {
        return mask | (reinterpret_cast<uint64_t>(ptr) & pointer_mask);
    }

    /** Convert an type_id to a mask.
     *
     * @param id Type id
     * @return Encoded type id.
     */
    static constexpr uint64_t id_to_mask(uint64_t id) {
        return id << 48;
    }

    /** Make an id from a 5 bit integer.
     * Encodes the 5 bit integer into the top 16 bits of a floating
     * point NaN. The msb is encoded into the sign-bit the other 4 bits
     * in bits 52:49.
     *
     * @param id 5 bit integer.
     * @return 16-bit type id.
     */
    static constexpr uint16_t make_id(uint16_t id) {
        return ((id & 0x10) << 11) | (id & 0xf) | 0x7ff0;
    }

    /// Lowest integer that can be encoded into datum's storage.
    static constexpr int64_t minimum_int = 0xfffc'0000'0000'0000LL;
    /// Highest integer that can be encoded into datum's storage.
    static constexpr int64_t maximum_int = 0x0003'ffff'ffff'ffffLL;

    static constexpr int64_t minimum_mantissa = 0xffff'ff80'0000'0000LL;
    static constexpr int64_t maximum_mantissa = 0x0000'007f'ffff'ffffLL;

    static constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    static constexpr uint64_t pointer_mask = 0x0000'ffff'ffff'ffff;

    static constexpr uint64_t small_undefined = 0;
    static constexpr uint64_t small_null = 1;
    static constexpr uint64_t small_true = 2;
    static constexpr uint64_t small_false = 3;
    static constexpr uint64_t small_break = 4;
    static constexpr uint64_t small_continue = 5;

    static constexpr uint16_t phy_small_id         = make_id(0b00001);
    static constexpr uint16_t phy_decimal_id       = make_id(0b00010);
    static constexpr uint16_t phy_ymd_id           = make_id(0b00011);
    static constexpr uint16_t phy_reserved_id0     = make_id(0b00100);
    static constexpr uint16_t phy_reserved_id1     = make_id(0b00101);
    static constexpr uint16_t phy_reserved_id2     = make_id(0b00110);
    static constexpr uint16_t phy_reserved_id3     = make_id(0b00111);
    static constexpr uint16_t phy_integer_id0      = make_id(0b01000);
    static constexpr uint16_t phy_integer_id1      = make_id(0b01001);
    static constexpr uint16_t phy_integer_id2      = make_id(0b01010);
    static constexpr uint16_t phy_integer_id3      = make_id(0b01011);
    static constexpr uint16_t phy_integer_id4      = make_id(0b01100);
    static constexpr uint16_t phy_integer_id5      = make_id(0b01101);
    static constexpr uint16_t phy_integer_id6      = make_id(0b01110);
    static constexpr uint16_t phy_integer_id7      = make_id(0b01111);

    static constexpr uint16_t phy_string_id0       = make_id(0b10001);
    static constexpr uint16_t phy_string_id1       = make_id(0b10010);
    static constexpr uint16_t phy_string_id2       = make_id(0b10011);
    static constexpr uint16_t phy_string_id3       = make_id(0b10100);
    static constexpr uint16_t phy_string_id4       = make_id(0b10101);
    static constexpr uint16_t phy_string_id5       = make_id(0b10110);
    static constexpr uint16_t phy_string_id6       = make_id(0b10111);
    static constexpr uint16_t phy_string_ptr_id    = make_id(0b11000);
    static constexpr uint16_t phy_url_ptr_id       = make_id(0b11001);
    static constexpr uint16_t phy_integer_ptr_id   = make_id(0b11010);
    static constexpr uint16_t phy_vector_ptr_id    = make_id(0b11011);
    static constexpr uint16_t phy_map_ptr_id       = make_id(0b11100);
    static constexpr uint16_t phy_decimal_ptr_id   = make_id(0b11101);
    static constexpr uint16_t phy_bytes_ptr_id     = make_id(0b11110);
    static constexpr uint16_t phy_reserved_ptr_id0 = make_id(0b11111);

    static constexpr uint64_t small_mask = id_to_mask(phy_small_id);
    static constexpr uint64_t undefined_mask = small_mask | small_undefined;
    static constexpr uint64_t null_mask = small_mask | small_null;
    static constexpr uint64_t true_mask = small_mask | small_true;
    static constexpr uint64_t false_mask = small_mask | small_false;
    static constexpr uint64_t break_mask = small_mask | small_break;
    static constexpr uint64_t continue_mask = small_mask | small_continue;
    static constexpr uint64_t string_mask = id_to_mask(phy_string_id0);
    static constexpr uint64_t character_mask = id_to_mask(phy_string_id1);
    static constexpr uint64_t integer_mask = id_to_mask(phy_integer_id0);
    static constexpr uint64_t decimal_mask = id_to_mask(phy_decimal_id);
    static constexpr uint64_t ymd_mask = id_to_mask(phy_ymd_id);
    static constexpr uint64_t string_ptr_mask = id_to_mask(phy_string_ptr_id);
    static constexpr uint64_t url_ptr_mask = id_to_mask(phy_url_ptr_id);
    static constexpr uint64_t integer_ptr_mask = id_to_mask(phy_integer_ptr_id);
    static constexpr uint64_t vector_ptr_mask = id_to_mask(phy_vector_ptr_id);
    static constexpr uint64_t map_ptr_mask = id_to_mask(phy_map_ptr_id);
    static constexpr uint64_t decimal_ptr_mask = id_to_mask(phy_decimal_ptr_id);
    static constexpr uint64_t bytes_ptr_mask = id_to_mask(phy_bytes_ptr_id);

    union {
        double f64;
        uint64_t u64;
    };

    /** Extract the type_id from datum's storage.
     * This function will get the most significant 16 bits from
     * the datum's storage. It uses std::memcpy() to make sure
     * there is no undefined behavior, due to not knowing
     * ahead of time if a double or uint64_t was stored.
     *
     * @return The type_id of the stored object.
     */
    uint16_t type_id() const noexcept {
        uint64_t data;
        std::memcpy(&data, this, sizeof(data));
        return static_cast<uint16_t>(data >> 48);
    }

    bool is_phy_float() const noexcept {
        ttlet id = type_id();
        return (id & 0x7ff0) != 0x7ff0 || (id & 0x000f) == 0;
    }

    bool is_phy_integer() const noexcept {
        return (type_id() & 0xfff8) == 0x7ff8;
    }

    bool is_phy_string() const noexcept {
        ttlet id = type_id();
        return (id & 0xfff8) == 0xfff0 && (id & 0x0007) > 0;
    }

    bool is_phy_decimal() const noexcept {
        return type_id() == phy_decimal_id;
    }

    bool is_phy_ymd() const noexcept {
        return type_id() == phy_ymd_id;
    }

    bool is_phy_small() const noexcept {
        return type_id() == phy_small_id;
    }

    bool is_phy_pointer() const noexcept {
        return HasLargeObjects && (type_id() & 0xfff8) == 0xfff8;
    }

    bool is_phy_string_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_string_ptr_id;
    }

    bool is_phy_url_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_url_ptr_id;
    }

    bool is_phy_integer_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_integer_ptr_id;
    }

    bool is_phy_vector_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_vector_ptr_id;
    }

    bool is_phy_map_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_map_ptr_id;
    }

    bool is_phy_decimal_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_decimal_ptr_id;
    }

    bool is_phy_bytes_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_bytes_ptr_id;
    }

    /** Extract the 48 bit unsigned integer from datum's storage.
     */
    uint64_t get_unsigned_integer() const noexcept {
        return (u64 << 16) >> 16;
    }

    /** Extract the 48 bit signed integer from datum's storage and sign extent to 64 bit.
     */
    int64_t get_signed_integer() const noexcept {
        return static_cast<int64_t>(u64 << 16) >> 16;
    }

    /** Extract a pointer for an existing object from datum's storage.
     * Canonical pointers on x86 and ARM are at most 48 bit and are sign extended to 64 bit.
     * Since the pointer is stored as a 48 bit integer, this function will launder it.
     */
    template<typename O>
    O *get_pointer() const {
        return std::launder(reinterpret_cast<O *>(get_signed_integer()));
    }

    /** Delete the object that the datum is pointing to.
     * This function should only be called on a datum that holds a pointer.
     */
    void delete_pointer() noexcept {
        if constexpr (HasLargeObjects) {
            switch (type_id()) {
            case phy_integer_ptr_id: delete get_pointer<int64_t>(); break;
            case phy_string_ptr_id: delete get_pointer<std::string>(); break;
            case phy_url_ptr_id: delete get_pointer<URL>(); break;
            case phy_vector_ptr_id: delete get_pointer<datum_impl::vector>(); break;
            case phy_map_ptr_id: delete get_pointer<datum_impl::map>(); break;
            case phy_decimal_ptr_id: delete get_pointer<decimal>(); break;
            case phy_bytes_ptr_id: delete get_pointer<bstring>(); break;
            default: tt_no_default;
            }
        }
    }

    /** Copy the object pointed to by the other datum into this datum.
     * Other datum must point to an object. This datum must not point to an object.
     *
     * @param other The other datum which holds a pointer to an object.
     */
    void copy_pointer(datum_impl const &other) noexcept {
        if constexpr (HasLargeObjects) {
            switch (other.type_id()) {
            case phy_integer_ptr_id: {
                auto * const p = new int64_t(*other.get_pointer<int64_t>());
                u64 = make_pointer(integer_ptr_mask, p);
            } break;

            case phy_string_ptr_id: {
                auto * const p = new std::string(*other.get_pointer<std::string>());
                u64 = make_pointer(string_ptr_mask, p);
            } break;

            case phy_url_ptr_id: {
                auto * const p = new URL(*other.get_pointer<URL>());
                u64 = make_pointer(url_ptr_mask, p);
            } break;

            case phy_vector_ptr_id: {
                auto * const p = new datum_impl::vector(*other.get_pointer<datum_impl::vector>());
                u64 = make_pointer(vector_ptr_mask, p);
            } break;

            case phy_map_ptr_id: {
                auto * const p = new datum_impl::map(*other.get_pointer<datum_impl::map>());
                u64 = make_pointer(map_ptr_mask, p);
            } break;

            case phy_decimal_ptr_id: {
                auto* const p = new decimal(*other.get_pointer<decimal>());
                u64 = make_pointer(decimal_ptr_mask, p);
            } break;

            case phy_bytes_ptr_id: {
                auto* const p = new bstring(*other.get_pointer<bstring>());
                u64 = make_pointer(bytes_ptr_mask, p);
            } break;

            default:
                tt_no_default;
            }
        }
    }

    
public:
    using vector = std::vector<datum_impl>;
    using map = std::unordered_map<datum_impl,datum_impl>;
    struct undefined {};
    struct null {};
    struct _continue {};
    struct _break {};

    datum_impl() noexcept : u64(undefined_mask) {}

    ~datum_impl() noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
    }

    datum_impl(datum_impl const &other) noexcept {
        if (tt_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            // We do a memcpy, because we don't know the type in the union.
            std::memcpy(this, &other, sizeof(*this));
        }
    }

    datum_impl &operator=(datum_impl const &other) noexcept {
        if (this != &other) {
            if (tt_unlikely(is_phy_pointer())) {
                delete_pointer();
            }
            if (tt_unlikely(other.is_phy_pointer())) {
                copy_pointer(other);
            } else {
                // We do a memcpy, because we don't know the type in the union.
                std::memcpy(this, &other, sizeof(*this));
            }
        }
        return *this;
    }

    datum_impl(datum_impl &&other) noexcept : u64(undefined_mask) {
        // We do a memcpy, because we don't know the type in the union.
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
    }

    datum_impl &operator=(datum_impl &&other) noexcept {
        if (this != &other) {
            // We do a memcpy, because we don't know the type in the union.
            std::memcpy(this, &other, sizeof(*this));
        }
        other.u64 = undefined_mask;
        return *this;
    }

    datum_impl(datum_impl::undefined) noexcept : u64(undefined_mask) {}
    datum_impl(datum_impl::null) noexcept : u64(null_mask) {}
    datum_impl(datum_impl::_break) noexcept : u64(break_mask) {}
    datum_impl(datum_impl::_continue) noexcept : u64(continue_mask) {}

    datum_impl(double value) noexcept : f64(value) {
        if (value != value) {
            u64 = undefined_mask;
        }
    }
    datum_impl(float value) noexcept : datum_impl(static_cast<double>(value)) {}

    datum_impl(decimal value) noexcept {
        long long m = value.mantissa();

        if (tt_unlikely(m < minimum_mantissa || m > maximum_mantissa)) {
            if constexpr (HasLargeObjects) {
                auto* const p = new decimal(value);
                u64 = make_pointer(decimal_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Constructing decimal {} to datum", value);
            }
        } else {
            int e = value.exponent();

            u64 =
                decimal_mask |
                static_cast<uint8_t>(e) |
                ((static_cast<uint64_t>(m) << 24) >> 16);
        }
    }

    datum_impl(date::year_month_day const &ymd) noexcept :
        u64(
            ymd_mask | ((
                (static_cast<uint64_t>(static_cast<int>(ymd.year())) << 9) |
                (static_cast<uint64_t>(static_cast<unsigned>(ymd.month())) << 5) |
                static_cast<uint64_t>(static_cast<unsigned>(ymd.day()))
            ) & 0x0000ffff'ffffffff)
        ) {}

    datum_impl(unsigned long long value) noexcept : u64(integer_mask | value) {
        if (tt_unlikely(value > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new uint64_t(value);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Constructing datum from integer {}, larger than {}", value, maximum_int);
            }
        }
    }
    datum_impl(unsigned long value) noexcept : datum_impl(static_cast<unsigned long long>(value)) {}
    datum_impl(unsigned int value) noexcept : datum_impl(static_cast<unsigned long long>(value)) {}
    datum_impl(unsigned short value) noexcept : datum_impl(static_cast<unsigned long long>(value)) {}
    datum_impl(unsigned char value) noexcept : datum_impl(static_cast<unsigned long long>(value)) {}

    datum_impl(signed long long value) noexcept :
        u64(integer_mask | (static_cast<uint64_t>(value) & 0x0007ffff'ffffffff))
    {
        if (tt_unlikely(value < minimum_int || value > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new int64_t(value);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Constructing integer {} to datum, outside {} and {}", value, minimum_int, maximum_int);
            }
        }
    }
    datum_impl(signed long value) noexcept : datum_impl(static_cast<signed long long>(value)) {}
    datum_impl(signed int value) noexcept : datum_impl(static_cast<signed long long>(value)) {}
    datum_impl(signed short value) noexcept : datum_impl(static_cast<signed long long>(value)) {}
    datum_impl(signed char value) noexcept : datum_impl(static_cast<signed long long>(value)) {}

    datum_impl(bool value) noexcept : u64(value ? true_mask : false_mask) {}
    datum_impl(char value) noexcept : u64(character_mask | value) {}

    datum_impl(std::string_view value) noexcept : u64(make_string(value)) {
        if (value.size() > 6) {
            if constexpr (HasLargeObjects) {
                auto * const p = new std::string(value);
                u64 = make_pointer(string_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Constructing string {} to datum, larger than 6 characters", value);
            }
        }
    }

    datum_impl(std::string const &value) noexcept : datum_impl(std::string_view(value)) {}
    datum_impl(char const *value) noexcept : datum_impl(std::string_view(value)) {}

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(URL const &value) noexcept {
        auto * const p = new URL(value);
        u64 = make_pointer(url_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(URL &&value) noexcept {
        auto * const p = new URL(std::move(value));
        u64 = make_pointer(url_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::vector const &value) noexcept {
        auto * const p = new datum_impl::vector(value);
        u64 = make_pointer(vector_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::vector &&value) noexcept {
        auto * const p = new datum_impl::vector(std::move(value));
        u64 = make_pointer(vector_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::map const &value) noexcept {
        auto * const p = new datum_impl::map(value);
        u64 = make_pointer(map_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::map &&value) noexcept {
        auto * const p = new datum_impl::map(std::move(value));
        u64 = make_pointer(map_ptr_mask, p);
    }

    datum_impl &operator=(datum_impl::undefined rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = undefined_mask;
        return *this;
    }

    datum_impl &operator=(datum_impl::null rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = null_mask;
        return *this;
    }

    datum_impl &operator=(datum_impl::_break rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = break_mask;
        return *this;
    }

    datum_impl &operator=(datum_impl::_continue rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = continue_mask;
        return *this;
    }

    datum_impl &operator=(double rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        if (rhs == rhs) {
            f64 = rhs;
        } else {
            u64 = undefined_mask;
        }
        return *this;
    }
    datum_impl& operator=(float rhs) noexcept { return *this = static_cast<double>(rhs); }


    datum_impl& operator=(decimal rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        long long m = rhs.mantissa();
        if (tt_unlikely(m < minimum_mantissa || m > maximum_mantissa)) {
            if constexpr (HasLargeObjects) {
                auto* const p = new decimal(rhs);
                u64 = make_pointer(decimal_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Constructing decimal {} to datum", rhs);
            }
        } else {
            int e = rhs.exponent();

            u64 =
                decimal_mask |
                static_cast<uint8_t>(e) |
                ((static_cast<uint64_t>(m) << 24) >> 16);
        }
        return *this;
    }

    datum_impl &operator=(date::year_month_day const & ymd) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = ymd_mask | ((
            (static_cast<uint64_t>(static_cast<int>(ymd.year())) << 9) |
            (static_cast<uint64_t>(static_cast<unsigned>(ymd.month())) << 5) |
            static_cast<uint64_t>(static_cast<unsigned>(ymd.day()))
        ) & 0x0000ffff'ffffffff);
        return *this;
    }

    datum_impl &operator=(unsigned long long rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = integer_mask | static_cast<uint64_t>(rhs);
        if (tt_unlikely(rhs > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new uint64_t(rhs);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Assigning integer {} to datum, larger than {}", rhs, maximum_int);
            }
        }
        return *this;
    }
    datum_impl& operator=(unsigned long rhs) noexcept { return *this = static_cast<unsigned long long>(rhs); }
    datum_impl& operator=(unsigned int rhs) noexcept { return *this = static_cast<unsigned long long>(rhs); }
    datum_impl& operator=(unsigned short rhs) noexcept { return *this = static_cast<unsigned long long>(rhs); }
    datum_impl& operator=(unsigned char rhs) noexcept { return *this = static_cast<unsigned long long>(rhs); }

    datum_impl &operator=(signed long long rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = integer_mask | (static_cast<uint64_t>(rhs) & 0x0007ffff'ffffffff);
        if (tt_unlikely(rhs < minimum_int || rhs > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new int64_t(rhs);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Assigning integer {} to datum, outside {} and {}", rhs, minimum_int, maximum_int);
            }
        }

        return *this;
    }
    datum_impl& operator=(signed long rhs) noexcept { return *this = static_cast<signed long long>(rhs); }
    datum_impl& operator=(signed int rhs) noexcept { return *this = static_cast<signed long long>(rhs); }
    datum_impl& operator=(signed short rhs) noexcept { return *this = static_cast<signed long long>(rhs); }
    datum_impl& operator=(signed char rhs) noexcept { return *this = static_cast<signed long long>(rhs); }

    datum_impl &operator=(bool rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = rhs ? true_mask : false_mask;
        return *this;
    }
    
    datum_impl &operator=(char rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = character_mask | static_cast<uint64_t>(rhs);
        return *this;
    }

    datum_impl &operator=(std::string_view rhs) {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = make_string(rhs);
        if (rhs.size() > 6) {
            if constexpr (HasLargeObjects) {
                auto * const p = new std::string(rhs);
                u64 = make_pointer(string_ptr_mask, p);
            } else {
                TTAURI_THROW_MATH_ERROR("Assigning string {} to datum, larger than 6 characters", rhs);
            }
        }
        return *this;
    }

    datum_impl &operator=(std::string const &rhs) {
        *this = std::string_view{rhs};
        return *this;
    }

    datum_impl &operator=(char const *rhs) {
        *this = std::string_view{rhs};
        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(URL const &rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new URL(rhs);
        u64 = make_pointer(url_ptr_mask, p);
        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(URL &&rhs) noexcept {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new URL(std::move(rhs));
        u64 = make_pointer(url_ptr_mask, p);
        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::vector const &rhs) {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::vector(rhs);
        u64 = make_pointer(vector_ptr_mask, p);

        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::vector &&rhs) {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::vector(std::move(rhs));
        u64 = make_pointer(vector_ptr_mask, p);

        return *this;
    }


    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::map const &rhs) {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::map(rhs);
        u64 = make_pointer(map_ptr_mask, p);

        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::map &&rhs) {
        if (tt_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::map(std::move(rhs));
        u64 = make_pointer(map_ptr_mask, p);

        return *this;
    }

    explicit operator double() const {
        if (is_phy_float()) {
            return f64;
        } else if (is_decimal()) {
            return static_cast<double>(static_cast<decimal>(*this));
        } else if (is_phy_integer()) {
            return static_cast<double>(get_signed_integer());
        } else if (is_phy_integer_ptr()) {
            return static_cast<double>(*get_pointer<int64_t>());
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a double", this->repr(), this->type_name());
        }
    }

    explicit operator float() const {
        return static_cast<float>(static_cast<double>(*this));
    }

    explicit operator decimal() const {
        if (is_phy_decimal()) {
            uint64_t v = get_unsigned_integer();
            int e = static_cast<int8_t>(v);
            long long m = static_cast<int64_t>(v << 16) >> 24;
            return decimal{e, m};
        } else if (is_phy_decimal_ptr()) {
            return *get_pointer<decimal>();
        } else if (is_phy_integer() || is_phy_integer_ptr()) {
            return decimal{static_cast<signed long long>(*this)};
        } else if (is_phy_float()) {
            return decimal{static_cast<double>(*this)};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a decimal", this->repr(), this->type_name());
        }
    }

    explicit operator date::year_month_day() const {
        if (is_phy_ymd()) {
            ttlet u = get_unsigned_integer();
            ttlet i = get_signed_integer();
            ttlet day = static_cast<unsigned>(u & 0x1f);
            ttlet month = static_cast<unsigned>((u >> 5) & 0xf);
            ttlet year = static_cast<signed>(i >> 9);

            if (day == 0) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a year-month-day", this->repr(), this->type_name());
            }
            if (month == 0) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a year-month-day", this->repr(), this->type_name());
            }
            return date::year_month_day{date::year{year}, date::month{month}, date::day{day}};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a year-month-day", this->repr(), this->type_name());
        }
    }

    explicit operator signed long long () const {
        if (is_phy_integer()) {
            return get_signed_integer();
        } else if (is_phy_integer_ptr()) {
            return *get_pointer<signed long long>();
        } else if (is_phy_float()) {
            return static_cast<signed long long>(f64);
        } else if (is_phy_small()) {
            switch (get_unsigned_integer()) {
            case small_true: return 1;
            case small_false: return 0;
            }
        }
        TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed long long", this->repr(), this->type_name());
    }

    explicit operator signed long () const {
        ttlet v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed long>::min() || v > std::numeric_limits<signed long>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed long", this->repr(), this->type_name());
        }
        return static_cast<signed long>(v);
    }

    explicit operator signed int () const {
        ttlet v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed int>::min() || v > std::numeric_limits<signed int>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed int", this->repr(), this->type_name());
        }
        return static_cast<signed int>(v);
    }

    explicit operator signed short () const {
        ttlet v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed short>::min() || v > std::numeric_limits<signed short>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed short", this->repr(), this->type_name());
        }
        return static_cast<signed short>(v);
    }

    explicit operator signed char () const {
        ttlet v = static_cast<int64_t>(*this);
        if (v < std::numeric_limits<signed char>::min() || v > std::numeric_limits<signed char>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed char", this->repr(), this->type_name());
        }
        return static_cast<signed char>(v);
    }

    explicit operator unsigned long long () const {
        ttlet v = static_cast<signed long long>(*this);
        return static_cast<unsigned long long>(v);
    }

    explicit operator unsigned long () const {
        ttlet v = static_cast<unsigned long long>(*this);
        if ( v > std::numeric_limits<unsigned long>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned long", this->repr(), this->type_name());
        }
        return static_cast<unsigned long>(v);
    }

    explicit operator unsigned int () const {
        ttlet v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned int>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned int", this->repr(), this->type_name());
        }
        return static_cast<unsigned int>(v);
    }

    explicit operator unsigned short () const {
        ttlet v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned short>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned short", this->repr(), this->type_name());
        }
        return static_cast<unsigned short>(v);
    }

    explicit operator unsigned char () const {
        ttlet v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned char>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned char", this->repr(), this->type_name());
        }
        return static_cast<unsigned char>(v);
    }

    explicit operator bool() const noexcept {
        switch (type_id()) {
        case phy_small_id: return get_unsigned_integer() == small_true;
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7: return static_cast<int64_t>(*this) != 0;
        case phy_decimal_id: return static_cast<decimal>(*this) != 0;
        case phy_ymd_id: return true;
        case phy_integer_ptr_id: return *get_pointer<int64_t>() != 0;
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return this->size() > 0;
        case phy_url_ptr_id: return true;
        case phy_vector_ptr_id: return this->size() > 0;
        case phy_map_ptr_id: return this->size() > 0;
        case phy_decimal_ptr_id: return static_cast<decimal>(*this) != 0;
        case phy_bytes_ptr_id: return this->size() > 0;
        default:
            if (tt_likely(is_phy_float())) {
                return static_cast<double>(*this) != 0.0;
            } else {
                tt_no_default;
            };
        }
    }

    explicit operator char() const {
        if (is_phy_string() && size() == 1) {
            return u64 & 0xff;
        } else if (is_phy_string_ptr() && size() == 1) {
            return get_pointer<std::string>()->at(0);
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a char", this->repr(), this->type_name());
        }
    }

    explicit operator bstring() const {
        if (is_bytes()) {
            if constexpr (HasLargeObjects) {
                return *get_pointer<bstring>();
            } else {
                tt_no_default;
            }
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to bytes", this->repr(), this->type_name());
        }
    }

    explicit operator std::string() const noexcept {
        switch (type_id()) {
        case phy_small_id:
            switch (get_unsigned_integer()) {
            case small_undefined: return "undefined";
            case small_null: return "null";
            case small_true: return "true";
            case small_false: return "false";
            case small_break: return "break";
            case small_continue: return "continue";
            default: tt_no_default;
            }

        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7: return fmt::format("{}", static_cast<int64_t>(*this));
        case phy_decimal_id: return fmt::format("{}", static_cast<decimal>(*this));
        case phy_ymd_id: return fmt::format("{}", static_cast<date::year_month_day>(*this));
        case phy_integer_ptr_id:
            if constexpr (HasLargeObjects) {
                return fmt::format("{}", static_cast<int64_t>(*this));
            } else {
                tt_no_default;
            }

        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6: {
                ttlet length = size();
                char buffer[6];
                for (int i = 0; i < length; i++) {
                    buffer[i] = (u64 >> ((length - i - 1) * 8)) & 0xff;
                }
                return std::string(buffer, length);
            }

        case phy_string_ptr_id:
            if constexpr (HasLargeObjects) {
                return *get_pointer<std::string>();
            } else {
                tt_no_default;
            }

        case phy_url_ptr_id:
            if constexpr (HasLargeObjects) {
                return get_pointer<URL>()->string();
            } else {
                tt_no_default;
            }

        case phy_decimal_ptr_id:
            if constexpr (HasLargeObjects) {
                return fmt::format("{}", static_cast<decimal>(*this));
            } else {
                tt_no_default;
            }

        case phy_vector_ptr_id:
            if constexpr (HasLargeObjects) {
                std::string r = "[";
                auto count = 0;
                for (auto i = vector_begin(); i != vector_end(); i++) {
                    if (count++ > 0) {
                        r += ", ";
                    }
                    r += i->repr();
                }
                r += "]";
                return r;
            } else {
                tt_no_default;
            }

        case phy_map_ptr_id:
            if constexpr (HasLargeObjects) {
                std::vector<std::pair<datum_impl,datum_impl>> items;
                items.reserve(size());
                std::copy(map_begin(), map_end(), std::back_inserter(items));
                std::sort(items.begin(), items.end(), [](auto &a, auto &b) {
                    return a.first < b.first;
                });

                std::string r = "{";
                auto count = 0;
                for (auto &item: items) {
                    if (count++ > 0) {
                        r += ", ";
                    }
                    r += item.first.repr();
                    r += ": ";
                    r += item.second.repr();
                }
                r += "}";
                return r;
            } else {
                tt_no_default;
            }

        case phy_bytes_ptr_id:
            if constexpr (HasLargeObjects) {
                return to_pretty_string(*get_pointer<bstring>());
            } else {
                tt_no_default;
            }

        default:
            if (is_phy_float()) {
                auto str = fmt::format("{:g}", static_cast<double>(*this));
                if (str.find('.') == str.npos) {
                    str += ".0";
                }
                return str;
            } else {
                tt_no_default;
            }
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator URL() const {
        if (is_string()) {
            return URL{static_cast<std::string>(*this)};
        } else if (is_url()) {
            return *get_pointer<URL>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a URL", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator datum_impl::vector() const {
        if (is_vector()) {
            return *get_pointer<datum_impl::vector>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a Vector", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator datum_impl::map() const {
        if (is_map()) {
            return *get_pointer<datum_impl::map>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a Map", this->repr(), this->type_name());
        }
    }

    /** Index into a datum::map or datum::vector.
     * This datum must hold a vector, map or undefined.
     * When this datum holds undefined it is treated as if datum holds an empty map.
     * When this datum holds a vector, the index must be datum holding an integer.
     *
     * @param rhs An index into the map or vector.
     */
    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator[](datum_impl const &rhs) {
        if (is_undefined()) {
            // When accessing a name on an undefined it means we need replace it with an empty map.
            auto *p = new datum_impl::map();
            u64 = map_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_map()) {
            auto &m = *get_pointer<datum_impl::map>();
            auto [i, did_insert] = m.try_emplace(rhs);
            return i->second;

        } else if (is_vector() && rhs.is_integer()) {
            auto index = static_cast<int64_t>(rhs);
            auto &v = *get_pointer<datum_impl::vector>();

            if (index < 0) {
                index = std::ssize(v) + index;
            }

            if (index < 0 || index >= std::ssize(v)) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Index {} out of range to access value in vector of size {}", index, std::ssize(v));
            } else {
                return v[index];
            }
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot index value of type {} with {} of type {}", type_name(), rhs.repr(), rhs.type_name());
        }
    }

    /** Index into a datum::map or datum::vector.
     * This datum must hold a vector, map or undefined.
     * When this datum holds undefined it is treated as if datum holds an empty map.
     * When this datum holds a vector, the index must be datum holding an integer.
     *
     * @param rhs An index into the map or vector.
     */
    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl operator[](datum_impl const &rhs) const {
        if (is_map()) {
            ttlet &m = *get_pointer<datum_impl::map>();
            ttlet i = m.find(rhs);
            if (i == m.cend()) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Could not find key {} in map of size {}", rhs.repr(), std::ssize(m));
            }
            return i->second;

        } else if (is_vector() && rhs.is_integer()) {
            auto index = static_cast<int64_t>(rhs);
            ttlet &v = *get_pointer<datum_impl::vector>();

            if (index < 0) {
                index = std::ssize(v) + index;
            }

            if (index < 0 || index >= std::ssize(v)) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Index {} out of range to access value in vector of size {}", index, std::ssize(v));
            } else {
                return v[index];
            }
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot index value of type {} with {} of type {}", type_name(), rhs.repr(), rhs.type_name());
        }
    }

    /** Check if an index is contained in a datum.
     * This datum must hold a vector, map or undefined.
     * When this datum holds undefined it is treated as if datum holds an empty map.
     * When this datum holds a vector, the index must be datum holding an integer.
     *
     * @param rhs An index into the map or vector.
     * @return true if the index is in the map or vector.
     */
    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    bool contains(datum_impl const &rhs) const noexcept {
        if (is_map()) {
            ttlet &m = *get_pointer<datum_impl::map>();
            ttlet i = m.find(rhs);
            return i != m.cend();

        } else if (is_vector() && rhs.is_integer()) {
            auto index = static_cast<int64_t>(rhs);
            ttlet &v = *get_pointer<datum_impl::vector>();

            if (index < 0) {
                index = std::ssize(v) + index;
            }

            return index >= 0 && index < std::ssize(v);

        } else {
            return false;
        }
    }

    /** Append and return a reference to a datum holding undefined to this datum.
     * This datum holding a undefined will be treated as if it is holding an empty vector.
     * This datum must hold a vector.
     * @return a reference to datum holding a vector.
     */
    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &append() {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->emplace_back();
            return v->back();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    template<typename... Args>
    void emplace_back(Args &&... args) {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->emplace_back(std::forward<Args>(args)...);

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    template<typename Arg>
    void push_back(Arg &&arg) {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->push_back(std::forward<Arg>(arg));

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    void pop_back() {
        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->pop_back();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot pop_back() onto type {}", type_name());
        }
    }

    datum_impl year() const {
        if (is_ymd()) {
            return {static_cast<signed>(static_cast<date::year_month_day>(*this).year())};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot get year() from type {}", type_name());
        }
    }

    datum_impl quarter() const {
        if (is_ymd()) {
            auto month = static_cast<unsigned>(static_cast<date::year_month_day>(*this).month());
            return {((month-1) / 3) + 1};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot get month() from type {}", type_name());
        }
    }

    datum_impl month() const {
        if (is_ymd()) {
            return {static_cast<unsigned>(static_cast<date::year_month_day>(*this).month())};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot get month() from type {}", type_name());
        }
    }

    datum_impl day() const {
        if (is_ymd()) {
            return {static_cast<unsigned>(static_cast<date::year_month_day>(*this).day())};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot get day() from type {}", type_name());
        }
    }

    datum_impl const &front() const {
        if (is_vector()) {
            ttlet *v = get_pointer<datum_impl::vector>();
            return v->front();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot front() onto type {}", type_name());
        }
    }

    datum_impl &front() {
        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            return v->front();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot front() onto type {}", type_name());
        }
    }

    datum_impl const &back() const {
        if (is_vector()) {
            ttlet *v = get_pointer<datum_impl::vector>();
            return v->back();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot back() onto type {}", type_name());
        }
    }

    datum_impl &back() {
        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            return v->back();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot back() onto type {}", type_name());
        }
    }


    std::string repr() const noexcept {
        switch (type_id()) {
        case phy_small_id:
            switch (get_unsigned_integer()) {
            case small_undefined: return "undefined";
            case small_null: return "null";
            case small_true: return "true";
            case small_false: return "false";
            case small_break: return "break";
            case small_continue: return "continue";
            default: tt_no_default;
            }
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return static_cast<std::string>(*this);
        case phy_decimal_id:
        case phy_decimal_ptr_id: return static_cast<std::string>(*this);
        case phy_ymd_id: return static_cast<std::string>(*this);
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return fmt::format("\"{}\"", static_cast<std::string>(*this));
        case phy_url_ptr_id: return fmt::format("<URL {}>", static_cast<std::string>(*this));
        case phy_vector_ptr_id: return static_cast<std::string>(*this);
        case phy_map_ptr_id: return static_cast<std::string>(*this);
        case phy_bytes_ptr_id: return static_cast<std::string>(*this);
        default:
            if (tt_likely(is_phy_float())) {
                return static_cast<std::string>(*this);
            } else {
                tt_no_default;
            }
        }
    }

    /*! Return ordering of types.
     * Used in less-than comparison between different types.
     */
    int type_order() const noexcept {
        if (is_numeric()) {
            // Fold all numeric values into the same group (literal integers).
            return phy_integer_id0;
        } else {
            return type_id();
        }
    }

    datum_impl &get_by_path(std::vector<std::string> const &key) {
        if (key.size() > 0 && is_map()) {
            ttlet index = key.at(0);
            auto &next = (*this)[index];
            ttlet next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0 && is_vector()) {
            size_t const index = std::stoll(key.at(0));
            auto &next = (*this)[index];
            ttlet next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("type {} does not support get() with '{}'", type_name(), key.at(0));
        } else {
            return *this;
        }
    }

    datum_impl get_by_path(std::vector<std::string> const &key) const {
        if (key.size() > 0 && is_map()) {
            ttlet index = key.at(0);
            ttlet next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0 && is_vector()) {
            size_t const index = std::stoll(key.at(0));
            ttlet next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("type {} does not support get() with '{}'", type_name(), key.at(0));
        } else {
            return *this;
        }
    }

    bool is_integer() const noexcept { return is_phy_integer() || is_phy_integer_ptr(); }
    bool is_decimal() const noexcept { return is_phy_decimal() || is_phy_decimal_ptr(); }
    bool is_ymd() const noexcept { return is_phy_ymd(); }
    bool is_float() const noexcept { return is_phy_float(); }
    bool is_string() const noexcept { return is_phy_string() || is_phy_string_ptr(); }
    bool is_bytes() const noexcept { return is_phy_bytes_ptr(); }

    bool is_bool() const noexcept {
        if (is_phy_small()) {
            ttlet tmp = get_unsigned_integer();
            return tmp == small_true || tmp == small_false;
        } else {
            return false;
        }
    }

    bool is_null() const noexcept {
        return is_phy_small() && get_unsigned_integer() == small_null;
    }

    bool is_undefined() const noexcept {
        return is_phy_small() && get_unsigned_integer() == small_undefined;
    }

    bool is_break() const noexcept {
        return is_phy_small() && get_unsigned_integer() == small_break;
    }

    bool is_continue() const noexcept {
        return is_phy_small() && get_unsigned_integer() == small_continue;
    }    bool is_url() const noexcept { return is_phy_url_ptr(); }

    bool is_vector() const noexcept { return is_phy_vector_ptr(); }
    bool is_map() const noexcept { return is_phy_map_ptr(); }
    bool is_numeric() const noexcept { return is_integer() || is_decimal() ||  is_float(); }

    datum_type_t type() const noexcept {
        switch (type_id()) {
        case phy_small_id:
            switch (get_unsigned_integer()) {
            case small_undefined: return datum_type_t::Undefined;
            case small_null: return datum_type_t::Null;
            case small_false: return datum_type_t::Boolean;
            case small_true: return datum_type_t::Boolean;
            case small_break: return datum_type_t::Break;
            case small_continue: return datum_type_t::Continue;
            default: tt_no_default;
            }
      
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return datum_type_t::Integer;
        case phy_decimal_id:
        case phy_decimal_ptr_id: return datum_type_t::Decimal;
        case phy_ymd_id: return datum_type_t::YearMonthDay;
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return datum_type_t::String;
        case phy_url_ptr_id: return datum_type_t::URL;
        case phy_vector_ptr_id: return datum_type_t::Vector;
        case phy_map_ptr_id: return datum_type_t::Map;
        case phy_bytes_ptr_id: return datum_type_t::Bytes;
        default:
            if (tt_likely(is_phy_float())) {
                return datum_type_t::Float;
            } else {
                tt_no_default;
            }
        }
    }

    char const *type_name() const noexcept{
        switch (type_id()) {
        case phy_small_id:
            switch (get_unsigned_integer()) {
            case small_undefined: return "Undefined";
            case small_null: return "Null";
            case small_false: return "Boolean";
            case small_true: return "Boolean";
            case small_break: return "Break";
            case small_continue: return "Continue";
            default: tt_no_default;
            }

        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return "Integer";
        case phy_decimal_id:
        case phy_decimal_ptr_id: return "Decimal";
        case phy_ymd_id: return "YearMonthDay";
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return "String";
        case phy_url_ptr_id: return "URL";
        case phy_vector_ptr_id: return "Vector";
        case phy_map_ptr_id: return "Map";
        case phy_bytes_ptr_id: return "Bytes";
        default:
            if (tt_likely(is_phy_float())) {
                return "Float";
            } else {
                tt_no_default;
            }
        }
    }

    size_t size() const{
        switch (type_id()) {
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6: return ((u64 & 0xffff'0000'0000'0000ULL) - string_mask) >> 48;
        case phy_string_ptr_id: return get_pointer<std::string>()->size();
        case phy_vector_ptr_id: return get_pointer<datum_impl::vector>()->size();
        case phy_map_ptr_id: return get_pointer<datum_impl::map>()->size();
        case phy_bytes_ptr_id: return get_pointer<bstring>()->size();
        default: TTAURI_THROW_INVALID_OPERATION_ERROR("Can't get size of value {} of type {}.", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename map::const_iterator map_begin() const noexcept {
        if (is_phy_map_ptr()) {
            return get_pointer<datum_impl::map>()->begin();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("map_begin() expect datum to be a map, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename map::const_iterator map_end() const noexcept {
        if (is_phy_map_ptr()) {
            return get_pointer<datum_impl::map>()->end();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("map_end() expect datum to be a map, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename vector::const_iterator vector_begin() const noexcept {
        if (is_phy_vector_ptr()) {
            return get_pointer<datum_impl::vector>()->begin();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("vector_begin() expect datum to be a vector, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename vector::const_iterator vector_end() const noexcept{
        if (is_phy_vector_ptr()) {
            return get_pointer<datum_impl::vector>()->end();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("vector_end() expect datum to be a vector, but it is a {}.", this->type_name());
        }
    }

    size_t hash() const noexcept{
        if (is_phy_float()) {
            return std::hash<double>{}(f64);
        } else if (tt_unlikely(is_phy_pointer())) {
            switch (type_id()) {
            case phy_string_ptr_id:
                return std::hash<std::string>{}(*get_pointer<std::string>());
            case phy_url_ptr_id:
                return std::hash<URL>{}(*get_pointer<URL>());
            case phy_vector_ptr_id:
                return std::accumulate(vector_begin(), vector_end(), size_t{0}, [](size_t a, auto x) {
                    return a ^ x.hash();
                    });
            case phy_map_ptr_id:
                return std::accumulate(map_begin(), map_end(), size_t{0}, [](size_t a, auto x) {
                    return a ^ (x.first.hash() ^ x.second.hash());
                    });
            case phy_decimal_ptr_id:
                return std::hash<decimal>{}(*get_pointer<decimal>());
            case phy_bytes_ptr_id:
                return std::hash<bstring>{}(*get_pointer<bstring>());
            default: tt_no_default;
            }
        } else {
            return std::hash<uint64_t>{}(u64);
        }
    }

    datum_impl &operator++() {
        if (!this->is_numeric()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't increment '++' value {} of type {}",
                this->repr(), this->type_name()
            );
        }

        return *this += 1;
    }

    datum_impl &operator--() {
        if (!this->is_numeric()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't increment '--' value {} of type {}",
                this->repr(), this->type_name()
            );
        }

        return *this -= 1;
    }

    datum_impl operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    datum_impl operator--(int) {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    datum_impl &operator+=(datum_impl const &rhs) {
        if (this->is_vector()) {
            this->push_back(rhs);
        } else {
            *this = *this + rhs;
        }
        return *this;
    }

    datum_impl &operator-=(datum_impl const &rhs) {
        return *this = *this - rhs;
    }

    datum_impl &operator*=(datum_impl const &rhs) {
        return *this = *this * rhs;
    }

    datum_impl &operator/=(datum_impl const &rhs) {
        return *this = *this / rhs;
    }

    datum_impl &operator%=(datum_impl const &rhs) {
        return *this = *this % rhs;
    }

    datum_impl &operator<<=(datum_impl const &rhs) {
        return *this = *this << rhs;
    }

    datum_impl &operator>>=(datum_impl const &rhs) {
        return *this = *this >> rhs;
    }

    datum_impl &operator&=(datum_impl const &rhs) {
        return *this = *this & rhs;
    }

    datum_impl &operator|=(datum_impl const &rhs) {
        return *this = *this | rhs;
    }

    datum_impl &operator^=(datum_impl const &rhs) {
        return *this = *this ^ rhs;
    }

    friend datum_impl operator~(datum_impl const &rhs) {
        if (rhs.is_integer()) {
            return datum_impl{~static_cast<int64_t>(rhs)};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't bit-wise negate '~' value {} of type {}",
                rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator-(datum_impl const &rhs) {
        if (rhs.is_integer()) {
            return datum_impl{-static_cast<int64_t>(rhs)};
        } else if (rhs.is_decimal()) {
             return datum_impl{-static_cast<decimal>(rhs)};
        } else if (rhs.is_float()) {
            return datum_impl{-static_cast<double>(rhs)};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't arithmetic negate '-' value {} of type {}",
                rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator+(datum_impl const &rhs) {
        if (rhs.is_numeric()) {
            return rhs;
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't arithmetic posgate '+' value {} of type {}",
                rhs.repr(), rhs.type_name()
            );
        }
    }

    friend bool operator==(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        switch (lhs.type_id()) {
        case datum_impl::phy_small_id:
            return rhs.is_phy_small() && lhs.get_unsigned_integer() == rhs.get_unsigned_integer();
        case datum_impl::phy_integer_id0:
        case datum_impl::phy_integer_id1:
        case datum_impl::phy_integer_id2:
        case datum_impl::phy_integer_id3:
        case datum_impl::phy_integer_id4:
        case datum_impl::phy_integer_id5:
        case datum_impl::phy_integer_id6:
        case datum_impl::phy_integer_id7:
        case datum_impl::phy_integer_ptr_id:
            return (
                (rhs.is_float() && static_cast<double>(lhs) == static_cast<double>(rhs)) ||
                (rhs.is_decimal() && static_cast<decimal>(lhs) == static_cast<decimal>(rhs)) ||
                (rhs.is_integer() && static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs))
            );
        case datum_impl::phy_decimal_id:
        case datum_impl::phy_decimal_ptr_id:
            return (
                (rhs.is_float() && static_cast<double>(lhs) == static_cast<double>(rhs)) ||
                (rhs.is_decimal() && static_cast<decimal>(lhs) == static_cast<decimal>(rhs)) ||
                (rhs.is_integer() && static_cast<decimal>(lhs) == static_cast<decimal>(rhs))
            );
        case datum_impl::phy_ymd_id:
            return rhs.is_ymd() && lhs.get_unsigned_integer() == rhs.get_unsigned_integer();
        case datum_impl::phy_string_id0:
        case datum_impl::phy_string_id1:
        case datum_impl::phy_string_id2:
        case datum_impl::phy_string_id3:
        case datum_impl::phy_string_id4:
        case datum_impl::phy_string_id5:
        case datum_impl::phy_string_id6:
        case datum_impl::phy_string_ptr_id:
            return (
                (rhs.is_string() && static_cast<std::string>(lhs) == static_cast<std::string>(rhs)) ||
                (rhs.is_url() && static_cast<URL>(lhs) == static_cast<URL>(rhs))
                );
        case datum_impl::phy_url_ptr_id:
            return (rhs.is_url() || rhs.is_string()) && static_cast<URL>(lhs) == static_cast<URL>(rhs);
        case datum_impl::phy_vector_ptr_id:
            return rhs.is_vector() && *lhs.get_pointer<datum_impl::vector>() == *rhs.get_pointer<datum_impl::vector>();
        case datum_impl::phy_map_ptr_id:
            return rhs.is_map() && *lhs.get_pointer<datum_impl::map>() == *rhs.get_pointer<datum_impl::map>();
        case datum_impl::phy_bytes_ptr_id:
            return
                (rhs.is_bytes() && static_cast<bstring>(lhs) == static_cast<bstring>(rhs));
        default:
            if (lhs.is_phy_float()) {
                return rhs.is_numeric() && static_cast<double>(lhs) == static_cast<double>(rhs);
            } else {
                tt_no_default;
            }
        }
    }

    friend bool operator<(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        switch (lhs.type_id()) {
        case datum_impl::phy_small_id:
            if (lhs.is_bool() && rhs.is_bool()) {
                return static_cast<bool>(lhs) < static_cast<bool>(rhs);
            } else {
                return lhs.get_unsigned_integer() < rhs.get_unsigned_integer();
            }
        case datum_impl::phy_integer_id0:
        case datum_impl::phy_integer_id1:
        case datum_impl::phy_integer_id2:
        case datum_impl::phy_integer_id3:
        case datum_impl::phy_integer_id4:
        case datum_impl::phy_integer_id5:
        case datum_impl::phy_integer_id6:
        case datum_impl::phy_integer_id7:
        case datum_impl::phy_integer_ptr_id:
            if (rhs.is_float()) {
                return static_cast<double>(lhs) < static_cast<double>(rhs);
            } else if (rhs.is_decimal()) {
                return static_cast<decimal>(lhs) < static_cast<decimal>(rhs);
            } else if (rhs.is_integer()) {
                return static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_decimal_id:
        case datum_impl::phy_decimal_ptr_id:
            if (rhs.is_float()) {
                return static_cast<double>(lhs) < static_cast<double>(rhs);
            } else if (rhs.is_decimal()) {
                return static_cast<decimal>(lhs) < static_cast<decimal>(rhs);
            } else if (rhs.is_integer()) {
                return static_cast<decimal>(lhs) < static_cast<decimal>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_ymd_id:
            if (rhs.is_ymd()) {
                return static_cast<date::year_month_day>(lhs) < static_cast<date::year_month_day>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_string_id0:
        case datum_impl::phy_string_id1:
        case datum_impl::phy_string_id2:
        case datum_impl::phy_string_id3:
        case datum_impl::phy_string_id4:
        case datum_impl::phy_string_id5:
        case datum_impl::phy_string_id6:
        case datum_impl::phy_string_ptr_id:
            if (rhs.is_string()) {
                return static_cast<std::string>(lhs) < static_cast<std::string>(rhs);
            } else if (rhs.is_url()) {
                return static_cast<URL>(lhs) < static_cast<URL>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_url_ptr_id:
            if (rhs.is_url() || rhs.is_string()) {
                return static_cast<URL>(lhs) < static_cast<URL>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_vector_ptr_id:
            if (rhs.is_vector()) {
                return *lhs.get_pointer<datum_impl::vector>() < *rhs.get_pointer<datum_impl::vector>();
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_map_ptr_id:
            if (rhs.is_map()) {
                return *lhs.get_pointer<datum_impl::map>() < *rhs.get_pointer<datum_impl::map>();
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_bytes_ptr_id:
            if (rhs.is_bytes()) {
                return static_cast<bstring>(lhs) < static_cast<bstring>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        default:
            if (lhs.is_phy_float()) {
                if (rhs.is_numeric()) {
                    return static_cast<double>(lhs) < static_cast<double>(rhs);
                } else {
                    return lhs.type_order() < rhs.type_order();
                }
            } else {
                tt_no_default;
            }
        }
    }

    friend bool operator!=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator>(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(lhs < rhs);
    }

    friend datum_impl operator+(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_float() || rhs.is_float()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ lhs_ + rhs_ };

        } else if (lhs.is_decimal() || rhs.is_decimal()) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return datum_impl{lhs_ + rhs_};

        } else if (lhs.is_integer() || rhs.is_integer()) {
            ttlet lhs_ = static_cast<long long int>(lhs);
            ttlet rhs_ = static_cast<long long int>(rhs);
            return datum_impl{ lhs_ + rhs_ };

        } else if (lhs.is_string() && rhs.is_string()) {
            ttlet lhs_ = static_cast<std::string>(lhs);
            ttlet rhs_ = static_cast<std::string>(rhs);
            return datum_impl{std::move(lhs_ + rhs_)};

        } else if (lhs.is_vector() && rhs.is_vector()) {
            auto lhs_ = static_cast<datum_impl::vector>(lhs);
            ttlet &rhs_ = *(rhs.get_pointer<datum_impl::vector>());
            std::copy(rhs_.begin(), rhs_.end(), std::back_inserter(lhs_));
            return datum_impl{std::move(lhs_)};

        } else if (lhs.is_map() && rhs.is_map()) {
            ttlet &lhs_ = *(lhs.get_pointer<datum_impl::map>());
            auto rhs_ = static_cast<datum_impl::map>(rhs);
            for (ttlet &item: lhs_) {
                rhs_.try_emplace(item.first, item.second);
            }
            return datum_impl{std::move(rhs_)};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't add '+' value {} of type {} to value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator-(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_float() || rhs.is_float()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ lhs_ - rhs_ };

        } else if (lhs.is_decimal() || rhs.is_decimal()) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return datum_impl{ lhs_ - rhs_ };

        } else if (lhs.is_integer() || rhs.is_integer()) {
            ttlet lhs_ = static_cast<long long int>(lhs);
            ttlet rhs_ = static_cast<long long int>(rhs);
            return datum_impl{ lhs_ - rhs_ };

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't subtract '-' value {} of type {} from value {} of type {}",
                rhs.repr(), rhs.type_name(), lhs.repr(), lhs.type_name()
            );
        }
    }

    friend datum_impl operator*(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_float() || rhs.is_float()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ lhs_ * rhs_ };

        } else if (lhs.is_decimal() || rhs.is_decimal()) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return datum_impl{ lhs_ * rhs_ };

        } else if (lhs.is_integer() || rhs.is_integer()) {
            ttlet lhs_ = static_cast<long long int>(lhs);
            ttlet rhs_ = static_cast<long long int>(rhs);
            return datum_impl{ lhs_ * rhs_ };

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't multiply '*' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator/(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_float() || rhs.is_float()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ lhs_ / rhs_ };

        } else if (lhs.is_decimal() || rhs.is_decimal()) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return datum_impl{ lhs_ / rhs_ };

        } else if (lhs.is_integer() || rhs.is_integer()) {
            ttlet lhs_ = static_cast<long long int>(lhs);
            ttlet rhs_ = static_cast<long long int>(rhs);
            return datum_impl{ lhs_ / rhs_ };

        } else if (lhs.is_url() && (rhs.is_url() || rhs.is_string())) {
            ttlet lhs_ = static_cast<URL>(lhs);
            ttlet rhs_ = static_cast<URL>(rhs);
            return datum_impl{ lhs_ / rhs_ };

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't divide '/' value {} of type {} by value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator%(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_float() || rhs.is_float()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ fmod(lhs_, rhs_) };

        } else if (lhs.is_decimal() || rhs.is_decimal()) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return datum_impl{ lhs_ % rhs_ };

        } else if (lhs.is_integer() || rhs.is_integer()) {
            ttlet lhs_ = static_cast<long long int>(lhs);
            ttlet rhs_ = static_cast<long long int>(rhs);
            return datum_impl{ lhs_ % rhs_ };

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't take modulo '%' value {} of type {} by value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator<<(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            ttlet lhs_ = static_cast<uint64_t>(lhs);
            ttlet rhs_ = static_cast<int64_t>(rhs);
            if (rhs_ < -63) {
                return datum_impl{0};
            } else if (rhs_ < 0) {
                // Pretend this is a unsigned shift right.
                return datum_impl{lhs_ >> -rhs_};
            } else if (rhs_ == 0) {
                return lhs;
            } else if (rhs_ > 63) {
                return datum_impl{0};
            } else {
                return datum_impl{lhs_ << rhs_};
            }

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't logical shift-left '<<' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator>>(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            ttlet lhs_ = static_cast<uint64_t>(lhs);
            ttlet rhs_ = static_cast<int64_t>(rhs);
            if (rhs_ < -63) {
                return datum_impl{0};
            } else if (rhs_ < 0) {
                return datum_impl{lhs_ << -rhs_};
            } else if (rhs_ == 0) {
                return lhs;
            } else if (rhs_ > 63) {
                return (lhs_ >= 0) ? datum_impl{0} : datum_impl{-1};
            } else {
                return datum_impl{static_cast<int64_t>(lhs_) >> rhs_};
            }

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't arithmetic shift-right '>>' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator&(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            ttlet lhs_ = static_cast<uint64_t>(lhs);
            ttlet rhs_ = static_cast<uint64_t>(rhs);
            return datum_impl{lhs_ & rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't AND '&' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator|(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            ttlet lhs_ = static_cast<uint64_t>(lhs);
            ttlet rhs_ = static_cast<uint64_t>(rhs);
            return datum_impl{lhs_ | rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't OR '|' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator^(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            ttlet lhs_ = static_cast<uint64_t>(lhs);
            ttlet rhs_ = static_cast<uint64_t>(rhs);
            return datum_impl{lhs_ ^ rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't XOR '^' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend std::string to_string(datum_impl const &d) {
        return static_cast<std::string>(d);
    }

    friend std::ostream &operator<<(std::ostream &os, datum_impl const &d) {
        return os << static_cast<std::string>(d);
    }

    friend void swap(datum_impl &lhs, datum_impl &rhs) noexcept {
        memswap(lhs, rhs);
    }

    friend datum_impl pow(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_numeric() || rhs.is_numeric()) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            return datum_impl{ std::pow(lhs_, rhs_) };

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't raise to a power '**' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    /** Merge two datums together, such that the second will override values on the first.
     * This will merge map-datums together by recursively deep merging matching items.
     *
     * @param lhs First datum.
     * @param rhs Second datum that will override the first datum.
     * @return 
     */
    friend datum_impl deep_merge(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        datum_impl result;

        if (lhs.is_map() && rhs.is_map()) {
            result = lhs;

            auto result_map = result.get_pointer<datum_impl::map>();
            for (auto rhs_i = rhs.map_begin(); rhs_i != rhs.map_end(); rhs_i++) {
                auto result_i = result_map->find(rhs_i->first);
                if (result_i == result_map->end()) {
                    result_map->insert(*rhs_i);
                } else {
                    result_i->second = deep_merge(result_i->second, rhs_i->second);
                }
            }

        } else if (lhs.is_vector() && rhs.is_vector()) {
            result = lhs;

            auto result_vector = result.get_pointer<datum_impl::vector>();
            for (auto rhs_i = rhs.vector_begin(); rhs_i != rhs.vector_end(); rhs_i++) {
                result_vector->push_back(*rhs_i);
            }

        } else {
            result = rhs;
        }

        return result;
    }
};

template<typename T, bool HasLargeObjects>
inline bool will_cast_to(datum_impl<HasLargeObjects> const &rhs) {
    if constexpr (std::is_same_v<T, bool>) {
        return true;
    } else if constexpr (std::is_same_v<T, char>) {
        return rhs.is_string();
    } else if constexpr (std::is_arithmetic_v<T>) {
        return rhs.is_numeric();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::undefined>) {
        return rhs.is_undefined();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::null>) {
        return rhs.is_null();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::_break>) {
        return rhs.is_break();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::_continue>) {
        return rhs.is_continue();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::vector>) {
        return rhs.is_vector();
    } else if constexpr (std::is_same_v<T, typename datum_impl<HasLargeObjects>::map>) {
        return rhs.is_map();
    } else if constexpr (std::is_same_v<T, URL>) {
        return rhs.is_url() || rhs.is_string();
    } else if constexpr (std::is_same_v<T, std::string>) {
        return true;
    } else {
        return false;
    }
}

template<bool HasLargeObjects>
bool operator<(typename datum_impl<HasLargeObjects>::map const &lhs, typename datum_impl<HasLargeObjects>::map const &rhs) noexcept {
    auto lhs_keys = transform<datum_impl<HasLargeObjects>::vector>(lhs, [](auto x) { return x.first; });
    auto rhs_keys = transform<datum_impl<HasLargeObjects>::vector>(lhs, [](auto x) { return x.first; });

    if (lhs_keys == rhs_keys) {
        for (ttlet &k: lhs_keys) {
            if (lhs.at(k) == rhs.at(k)) {
                continue;
            } else if (lhs.at(k) < rhs.at(k)) {
                return true;
            } else {
                return false;
            }
        }
    } else {
        return lhs_keys < rhs_keys;
    }
    return false;
}

using datum = datum_impl<true>;
using sdatum = datum_impl<false>;

}

namespace std {

template<bool HasLargeObjects>
inline size_t hash<tt::datum_impl<HasLargeObjects>>::operator()(tt::datum_impl<HasLargeObjects> const &value) const {
    return value.hash();
}



}

