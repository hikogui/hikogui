// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/Required/memory.hpp"
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

#define BI_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, sdatum> operator op(sdatum const &lhs, T const &rhs) { return lhs op sdatum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, sdatum> operator op(T const &lhs, sdatum const &rhs) { return sdatum{lhs} op sdatum{rhs}; }

#define BI_BOOL_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, bool> operator op(sdatum const &lhs, T const &rhs) { return lhs op sdatum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, bool> operator op(T const &lhs, sdatum const &rhs) { return sdatum{lhs} op sdatum{rhs}; }

#define MONO_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, sdatum> operator op(T const &rhs) { return lhs op sdatum{rhs}; }\

#define MONO_BOOL_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, sdatum>, bool> operator op(T const &rhs) { return lhs op sdatum{rhs}; }\

namespace TTauri {
class sdatum;
}

namespace std {

template<>
class hash<TTauri::sdatum> {
public:
    size_t operator()(TTauri::sdatum const &value) const;
};

}

namespace TTauri {


constexpr uint64_t sdatum_id_to_mask(uint64_t id) {
    return id << 48;
}

constexpr uint16_t sdatum_make_id(uint16_t id) {
    return ((id & 0x10) << 11) | (id & 0xf) | 0x7ff0;
}

void swap(sdatum &lhs, sdatum &rhs) noexcept;


/*! A fixed size (64 bits) class for a generic value type.
 * A sdatum can hold and do calculations with the following types:
 *  - Floating point number (double, without NaN)
 *  - Signed integer number (52 bits)
 *  - Boolean
 *  - Null
 *  - Undefined
 */
class sdatum {
    static constexpr uint64_t make_string(std::string_view str)
    {
        uint64_t len = str.size();

        if (len > 6) {
            len = 6;
        }

        uint64_t x = 0;
        for (uint64_t i = 0; i < len; i++) {
            x <<= 8;
            x |= str[i];
        }
        return (string_mask + (len << 48)) | x;
    }

    static constexpr int64_t minimum_int = 0xfffe'0000'0000'0000LL;
    static constexpr int64_t maximum_int = 0x0007'ffff'ffff'ffffLL;

    static constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    static constexpr uint64_t pointer_mask = 0x0000'ffff'ffff'ffff;

    static constexpr uint16_t phy_boolean_id       = sdatum_make_id(0b00001);
    static constexpr uint16_t phy_null_id          = sdatum_make_id(0b00010);
    static constexpr uint16_t phy_undefined_id     = sdatum_make_id(0b00011);
    static constexpr uint16_t phy_reserved_id0     = sdatum_make_id(0b00100);
    static constexpr uint16_t phy_reserved_id1     = sdatum_make_id(0b00101);
    static constexpr uint16_t phy_reserved_id2     = sdatum_make_id(0b00110);
    static constexpr uint16_t phy_reserved_id3     = sdatum_make_id(0b00111);
    static constexpr uint16_t phy_integer_id0      = sdatum_make_id(0b01000);
    static constexpr uint16_t phy_integer_id1      = sdatum_make_id(0b01001);
    static constexpr uint16_t phy_integer_id2      = sdatum_make_id(0b01010);
    static constexpr uint16_t phy_integer_id3      = sdatum_make_id(0b01011);
    static constexpr uint16_t phy_integer_id4      = sdatum_make_id(0b01100);
    static constexpr uint16_t phy_integer_id5      = sdatum_make_id(0b01101);
    static constexpr uint16_t phy_integer_id6      = sdatum_make_id(0b01110);
    static constexpr uint16_t phy_integer_id7      = sdatum_make_id(0b01111);

    static constexpr uint16_t phy_string_id0       = sdatum_make_id(0b10001);
    static constexpr uint16_t phy_string_id1       = sdatum_make_id(0b10010);
    static constexpr uint16_t phy_string_id2       = sdatum_make_id(0b10011);
    static constexpr uint16_t phy_string_id3       = sdatum_make_id(0b10100);
    static constexpr uint16_t phy_string_id4       = sdatum_make_id(0b10101);
    static constexpr uint16_t phy_string_id5       = sdatum_make_id(0b10110);
    static constexpr uint16_t phy_string_id6       = sdatum_make_id(0b10111);
    static constexpr uint16_t phy_reserved_id4     = sdatum_make_id(0b11000);
    static constexpr uint16_t phy_reserved_id5     = sdatum_make_id(0b11001);
    static constexpr uint16_t phy_reserved_id6     = sdatum_make_id(0b11010);
    static constexpr uint16_t phy_reserved_id7     = sdatum_make_id(0b11011);
    static constexpr uint16_t phy_reserved_id8     = sdatum_make_id(0b11100);
    static constexpr uint16_t phy_reserved_id9     = sdatum_make_id(0b11101);
    static constexpr uint16_t phy_reserved_id10    = sdatum_make_id(0b11110);
    static constexpr uint16_t phy_reserved_id11    = sdatum_make_id(0b11111);

    static constexpr uint64_t boolean_mask = sdatum_id_to_mask(phy_boolean_id);
    static constexpr uint64_t null_mask = sdatum_id_to_mask(phy_null_id);
    static constexpr uint64_t undefined_mask = sdatum_id_to_mask(phy_undefined_id);
    static constexpr uint64_t string_mask = sdatum_id_to_mask(phy_string_id0);
    static constexpr uint64_t character_mask = sdatum_id_to_mask(phy_string_id1);
    static constexpr uint64_t integer_mask = sdatum_id_to_mask(phy_integer_id0);

    union {
        double f64;
        uint64_t u64;
    };

    uint16_t type_id() const noexcept {
        // We use memcpy on this to get access to the
        // actual bytes for determining the stored type.
        // This gets arround C++ undefined behavour of type-punning
        // through a union.
        // Technically you should memcpy to std::bytes[] but the
        // implementation was really slow compared to this, since
        // the native-endian presentation of the uint64_t destination
        // helps a lot.

        // The following is implemented as a simple direct uint16_t
        // memory load.
        uint64_t data;
        std::memcpy(&data, this, sizeof(data));
        return static_cast<uint16_t>(data >> 48);
    }

    bool is_phy_float() const noexcept {
        let id = type_id();
        return (id & 0x7ff0) != 0x7ff0 || (id & 0x000f) == 0;
    }

    bool is_phy_integer() const noexcept {
        return (type_id() & 0xfff8) == 0x7ff8;
    }

    bool is_phy_string() const noexcept {
        let id = type_id();
        return (id & 0xfff8) == 0xfff0 && (id & 0x0007) > 0;
    }

    bool is_phy_boolean() const noexcept { return type_id() == phy_boolean_id; }
    bool is_phy_null() const noexcept { return type_id() == phy_null_id; }
    bool is_phy_undefined() const noexcept { return type_id() == phy_undefined_id; }

public:
    struct undefined {};
    struct null {};

    force_inline sdatum() noexcept : u64(undefined_mask) {}

    explicit sdatum(sdatum::null) noexcept : u64(null_mask) {}
    explicit sdatum(double value) noexcept : f64(value) { if (value != value) { u64 = undefined_mask; } }
    explicit sdatum(float value) noexcept : sdatum(static_cast<double>(value)) {}
    explicit sdatum(uint64_t value) noexcept : sdatum(static_cast<int64_t>(value)) {}
    explicit sdatum(uint32_t value) noexcept : u64(integer_mask | value) {}
    explicit sdatum(uint16_t value) noexcept : sdatum(static_cast<uint32_t>(value)) {}
    explicit sdatum(uint8_t value) noexcept : sdatum(static_cast<uint32_t>(value)) {}
    explicit sdatum(int64_t value) noexcept : u64(integer_mask | (static_cast<uint64_t>(value) & 0x0000ffff'ffffffff)) {}

    explicit sdatum(int32_t value) noexcept : u64(integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit sdatum(int16_t value) noexcept : sdatum(static_cast<int32_t>(value)) {}
    explicit sdatum(int8_t value) noexcept : sdatum(static_cast<int32_t>(value)) {}
    explicit sdatum(bool value) noexcept : u64(boolean_mask | int64_t{value}) {}
    explicit sdatum(char value) noexcept : u64(character_mask | value) {}
    explicit sdatum(std::string_view value) noexcept : u64(make_string(value)) {}
    explicit sdatum(std::string const &value) noexcept : sdatum(std::string_view(value)) {}
    explicit sdatum(char const *value) noexcept : sdatum(std::string_view(value)) {}

    sdatum &operator=(double rhs) noexcept { f64 = rhs; return *this; }
    sdatum &operator=(float rhs) noexcept { return (*this = static_cast<double>(rhs)); }
    sdatum &operator=(uint64_t rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(uint32_t rhs) noexcept { u64 = integer_mask | uint64_t{rhs}; return *this; }
    sdatum &operator=(uint16_t rhs) noexcept { return (*this =  static_cast<uint32_t>(rhs)); }
    sdatum &operator=(uint8_t rhs) noexcept { return (*this = static_cast<uint32_t>(rhs)); }
    sdatum &operator=(int64_t rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(int32_t rhs) noexcept { u64 = integer_mask | (int64_t(rhs) & 0x0000'ffff'ffff'ffff); return *this; }
    sdatum &operator=(int16_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }
    sdatum &operator=(int8_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }
    sdatum &operator=(bool rhs) noexcept { u64 = boolean_mask | int64_t{rhs}; return *this; }
    sdatum &operator=(char rhs) noexcept { u64 = character_mask | static_cast<uint64_t>(rhs); return *this; }
    sdatum &operator=(std::string_view rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(std::string const &rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(char const *rhs) noexcept { return (*this = sdatum{rhs}); }

    //sdatum &operator+=(sdatum const &rhs) { return *this = *this + rhs; }

    explicit operator double() const noexcept {
        if (is_phy_float()) {
            return f64;
        } else if (is_phy_integer()) {
            return static_cast<double>(get_signed_integer());
        } else {
            no_default;
        }
    }

    explicit operator float() const noexcept {
        return static_cast<float>(static_cast<double>(*this));
    }

    explicit operator int64_t() const noexcept {
        if (is_phy_integer()) {
            return get_signed_integer();
        } else if (is_phy_float()) {
            return static_cast<int64_t>(f64);
        } else if (is_phy_boolean()) {
            return get_unsigned_integer() > 0 ? 1 : 0;
        } else {
            no_default;
        }
    }

    explicit operator int32_t() const noexcept {
        let v = static_cast<int64_t>(*this);
        return static_cast<int32_t>(v);
    }

    explicit operator int16_t() const noexcept {
        let v = static_cast<int64_t>(*this);
        return static_cast<int16_t>(v);
    }

    explicit operator int8_t() const noexcept {
        let v = static_cast<int64_t>(*this);
        return static_cast<int8_t>(v);
    }

    explicit operator uint64_t() const noexcept {
        let v = static_cast<int64_t>(*this);
        return static_cast<uint64_t>(v);
    }

    explicit operator uint32_t() const noexcept {
        let v = static_cast<uint64_t>(*this);
        return static_cast<uint32_t>(v);
    }

    explicit operator uint16_t() const noexcept {
        let v = static_cast<uint64_t>(*this);
        return static_cast<uint16_t>(v);
    }

    explicit operator uint8_t() const noexcept {
        let v = static_cast<uint64_t>(*this);
        return static_cast<uint8_t>(v);
    }

    explicit operator bool() const noexcept {
        switch (type_id()) {
        case phy_boolean_id: return get_unsigned_integer() > 0;
        case phy_null_id: return false;
        case phy_undefined_id: return false;
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7: return static_cast<int64_t>(*this) != 0;
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        default:
            if (ttauri_likely(is_phy_float())) {
                return static_cast<double>(*this) != 0.0;
            } else {
                no_default;
            };
        }
    }

    explicit operator char() const noexcept {
        if (is_phy_string() && size() == 1) {
            return u64 & 0xff;
        } else {
            no_default;
        }
    }

    explicit operator std::string() const noexcept;

    bool operator!() const noexcept { return !static_cast<bool>(*this); }

    sdatum operator~() const noexcept {
        if (is_integer()) {
            return sdatum{~static_cast<int64_t>(*this)};
        } else {
            no_default;
        }
    }

    sdatum operator-() const noexcept {
        if (is_integer()) {
            return sdatum{-static_cast<int64_t>(*this)};
        } else if (is_float()) {
            return sdatum{-static_cast<double>(*this)};
        } else {
            no_default;
        }
    }

    std::string repr() const noexcept;

    /*! Return ordering of types.
     * Used in less-than comparison between different types.
     */
    int type_order() const noexcept {
        if (is_float()) {
            // Fold all numeric values into the same group (literal integers).
            return phy_integer_id0;
        } else {
            return type_id();
        }
    }

    bool is_integer() const noexcept { return is_phy_integer(); }
    bool is_float() const noexcept { return is_phy_float(); }
    bool is_string() const noexcept { return is_phy_string(); }
    bool is_boolean() const noexcept { return is_phy_boolean(); }
    bool is_null() const noexcept { return is_phy_null(); }
    bool is_undefined() const noexcept { return is_phy_undefined(); }
    bool is_numeric() const noexcept { return is_integer() || is_float(); }

    char const *type_name() const noexcept;

    uint64_t get_unsigned_integer() const noexcept { return u64 & 0x0000ffff'ffffffff; }
    int64_t get_signed_integer() const noexcept { return static_cast<int64_t>(u64 << 16) >> 16; }

    template<typename O>
    O *get_pointer() const noexcept {
        // canonical pointers on x86 and ARM.
        return reinterpret_cast<O *>(get_signed_integer()); 
    }

    size_t size() const noexcept;

    size_t hash() const noexcept
    {
        if (is_phy_float()) {
            return std::hash<double>{}(f64);
        } else {
            return std::hash<uint64_t>{}(u64);
        }
    }

    friend bool operator==(sdatum const &lhs, sdatum const &rhs) noexcept;
    friend bool operator<(sdatum const &lhs, sdatum const &rhs) noexcept;
};

template<typename T> inline bool will_cast_to(sdatum const &) noexcept { return false; }
template<> inline bool will_cast_to<int64_t>(sdatum const &d) noexcept { return d.is_numeric(); }
template<> inline bool will_cast_to<int32_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int16_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int8_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint64_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint32_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint16_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint8_t>(sdatum const &d) noexcept { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<bool>(sdatum const &d) noexcept { return true; }
template<> inline bool will_cast_to<sdatum::undefined>(sdatum const &d) noexcept { return d.is_undefined(); }
template<> inline bool will_cast_to<double>(sdatum const &d) noexcept { return d.is_numeric(); }
template<> inline bool will_cast_to<float>(sdatum const &d) noexcept { return will_cast_to<double>(d); }
template<> inline bool will_cast_to<std::string>(sdatum const &d) noexcept { return true; }

std::string to_string(sdatum const &d) noexcept;
std::ostream &operator<<(std::ostream &os, sdatum const &d);

inline bool operator==(sdatum const &lhs, sdatum const &rhs) noexcept
{
    switch (lhs.type_id()) {
    case sdatum::phy_boolean_id:
        return rhs.is_boolean() && static_cast<bool>(lhs) == static_cast<bool>(rhs);
    case sdatum::phy_null_id:
        return rhs.is_null();
    case sdatum::phy_undefined_id:
        return rhs.is_undefined();
    case sdatum::phy_integer_id0:
    case sdatum::phy_integer_id1:
    case sdatum::phy_integer_id2:
    case sdatum::phy_integer_id3:
    case sdatum::phy_integer_id4:
    case sdatum::phy_integer_id5:
    case sdatum::phy_integer_id6:
    case sdatum::phy_integer_id7:
        return (
            (rhs.is_float() && static_cast<double>(lhs) == static_cast<double>(rhs)) ||
            (rhs.is_integer() && static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs))
            );
    case sdatum::phy_string_id0:
    case sdatum::phy_string_id1:
    case sdatum::phy_string_id2:
    case sdatum::phy_string_id3:
    case sdatum::phy_string_id4:
    case sdatum::phy_string_id5:
    case sdatum::phy_string_id6:
        return rhs.is_string() && static_cast<std::string>(lhs) == static_cast<std::string>(rhs);
    default:
        if (lhs.is_phy_float()) {
            return rhs.is_numeric() && static_cast<double>(lhs) == static_cast<double>(rhs);
        } else {
            no_default;
        }
    }
}

inline bool operator<(sdatum const &lhs, sdatum const &rhs) noexcept
{
    switch (lhs.type_id()) {
    case sdatum::phy_boolean_id:
        if (rhs.is_boolean()) {
            return static_cast<bool>(lhs) < static_cast<bool>(rhs);
        } else {
            return lhs.type_order() < rhs.type_order();
        }
    case sdatum::phy_null_id:
        return lhs.type_order() < rhs.type_order();
    case sdatum::phy_undefined_id:
        return lhs.type_order() < rhs.type_order();
    case sdatum::phy_integer_id0:
    case sdatum::phy_integer_id1:
    case sdatum::phy_integer_id2:
    case sdatum::phy_integer_id3:
    case sdatum::phy_integer_id4:
    case sdatum::phy_integer_id5:
    case sdatum::phy_integer_id6:
    case sdatum::phy_integer_id7:
        if (rhs.is_float()) {
            return static_cast<double>(lhs) < static_cast<double>(rhs);
        } else if (rhs.is_integer()) {
            return static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
        } else {
            return lhs.type_order() < rhs.type_order();
        }
    case sdatum::phy_string_id0:
    case sdatum::phy_string_id1:
    case sdatum::phy_string_id2:
    case sdatum::phy_string_id3:
    case sdatum::phy_string_id4:
    case sdatum::phy_string_id5:
    case sdatum::phy_string_id6:
        if (rhs.is_string()) {
            return static_cast<std::string>(lhs) < static_cast<std::string>(rhs);
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
            no_default;
        }
    }
}

inline sdatum operator+(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return sdatum{lhs_ + rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return sdatum{lhs_ + rhs_};

    } else if (lhs.is_string() && rhs.is_string()) {
        let lhs_ = static_cast<std::string>(lhs);
        let rhs_ = static_cast<std::string>(rhs);
        return sdatum{std::move(lhs_ + rhs_)};

    } else {
        no_default;
    }
}

inline sdatum operator-(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return sdatum{lhs_ - rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return sdatum{lhs_ - rhs_};

    } else {
        no_default;
    }
}

inline sdatum operator*(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return sdatum{lhs_ * rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return sdatum{lhs_ * rhs_};

    } else {
        no_default;
    }
}

inline sdatum operator/(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return sdatum{lhs_ / rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return sdatum{lhs_ / rhs_};

    } else {
        no_default;
    }
}

inline sdatum operator%(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return sdatum{lhs_ % rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return sdatum{std::fmod(lhs_,rhs_)};

    } else {
        no_default;
    }
}

inline sdatum operator<<(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<uint64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        if (rhs_ < -63) {
            return sdatum{0};
        } else if (rhs_ < 0) {
            // Pretent this is a unsigned shift right.
            return sdatum{lhs_ >> -rhs_};
        } else if (rhs_ == 0) {
            return lhs;
        } else if (rhs_ > 63) {
            return sdatum{0};
        } else {
            return sdatum{lhs_ << rhs_};
        }

    } else {
        no_default;
    }
}

inline sdatum operator>>(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<uint64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        if (rhs_ < -63) {
            return sdatum{0};
        } else if (rhs_ < 0) {
            return sdatum{lhs_ << -rhs_};
        } else if (rhs_ == 0) {
            return lhs;
        } else if (rhs_ > 63) {
            return (lhs_ >= 0) ? sdatum{0} : sdatum{-1};
        } else {
            return sdatum{static_cast<int64_t>(lhs_) >> rhs_};
        }

    } else {
        no_default;
    }
}

inline sdatum operator&(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<uint64_t>(lhs);
        let rhs_ = static_cast<uint64_t>(rhs);
        return sdatum{lhs_ & rhs_};

    } else {
        no_default;
    }
}

inline sdatum operator|(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<uint64_t>(lhs);
        let rhs_ = static_cast<uint64_t>(rhs);
        return sdatum{lhs_ | rhs_};

    } else {
        no_default;
    }
}

inline sdatum operator^(sdatum const &lhs, sdatum const &rhs) noexcept
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<uint64_t>(lhs);
        let rhs_ = static_cast<uint64_t>(rhs);
        return sdatum{lhs_ ^ rhs_};

    } else {
        no_default;
    }
}

BI_BOOL_OPERATOR_CONVERSION(==)
BI_BOOL_OPERATOR_CONVERSION(<)
BI_BOOL_OPERATOR_CONVERSION(!=)
BI_BOOL_OPERATOR_CONVERSION(>)
BI_BOOL_OPERATOR_CONVERSION(<=)
BI_BOOL_OPERATOR_CONVERSION(>=)
    
BI_OPERATOR_CONVERSION(+)
BI_OPERATOR_CONVERSION(-)
BI_OPERATOR_CONVERSION(*)
BI_OPERATOR_CONVERSION(/)
BI_OPERATOR_CONVERSION(%)
BI_OPERATOR_CONVERSION(&)
BI_OPERATOR_CONVERSION(|)
BI_OPERATOR_CONVERSION(^)
BI_OPERATOR_CONVERSION(<<)
BI_OPERATOR_CONVERSION(>>)

}

namespace std {

inline size_t hash<TTauri::sdatum>::operator()(TTauri::sdatum const &value) const {
    return value.hash();
}

}

#undef BI_BOOL_OPERATOR_CONVERSION
#undef BI_OPERATOR_CONVERSION
#undef MONO_OPERATOR_CONVERSION
#undef MONO_BOOL_OPERATOR_CONVERSION