// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "os_detect.hpp"
#include "URL.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <cstdint>
#include <variant>
#include <limits>
#include <type_traits>
#include <ostream>
#include <numeric>

#define BI_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(datum const &lhs, T const &rhs) { return lhs op datum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(T const &lhs, datum const &rhs) { return datum{lhs} op datum{rhs}; }

#define BI_BOOL_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, bool> operator op(datum const &lhs, T const &rhs) { return lhs op datum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, bool> operator op(T const &lhs, datum const &rhs) { return datum{lhs} op datum{rhs}; }

namespace TTauri {
struct datum;
}

namespace std {

template<>
class hash<TTauri::datum> {
public:
    size_t operator()(TTauri::datum const &value) const;
};

}

namespace TTauri {




constexpr int64_t datum_min_int = 0xfffe'0000'0000'0000LL;
constexpr int64_t datum_max_int = 0x0007'ffff'ffff'ffffLL;

template<typename T>
bool holds_alternative(datum const &);

template<typename T>
T get(datum const &);

constexpr uint64_t datum_phy_to_mask(uint64_t id) {
    return 0x7ff0'0000'0000'0000 | ((id & 0x10) << 59) | ((id & 0xf) << 48);
}

/*! A fixed size (64 bits) class for a generic value type.
 * A datum can hold and do calculations with the following types:
 *  - Floating point number (double, without NaN)
 *  - Signed integer number (52 bits)
 *  - Boolean
 *  - Null
 *  - Undefined
 *  - String
 *  - Vector of datum
 *  - Unordered map of datum:datum.
 */
struct datum {
    static constexpr uint64_t make_string(std::string_view str)
    {
        uint64_t len = str.size();

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

    

    static constexpr int phy_to_value(uint8_t x) {
        if (x == phy_float_id0 || x == phy_integer_ptr_id) {
            // Fold all numeric values into a single value.
            return phy_integer_id0;
        } else {
            return x;
        }
    }

    static constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    static constexpr uint64_t pointer_mask = 0x0000'ffff'ffff'ffff;

    static constexpr uint8_t phy_float_id0        = 0b00000;
    static constexpr uint8_t phy_boolean_id       = 0b00001;
    static constexpr uint8_t phy_null_id          = 0b00010;
    static constexpr uint8_t phy_undefined_id     = 0b00011;
    static constexpr uint8_t phy_reserved_id0     = 0b00100;
    static constexpr uint8_t phy_reserved_id1     = 0b00101;
    static constexpr uint8_t phy_reserved_id2     = 0b00110;
    static constexpr uint8_t phy_reserved_id3     = 0b00111;
    static constexpr uint8_t phy_integer_id0      = 0b01000;
    static constexpr uint8_t phy_integer_id1      = 0b01001;
    static constexpr uint8_t phy_integer_id2      = 0b01010;
    static constexpr uint8_t phy_integer_id3      = 0b01011;
    static constexpr uint8_t phy_integer_id4      = 0b01100;
    static constexpr uint8_t phy_integer_id5      = 0b01101;
    static constexpr uint8_t phy_integer_id6      = 0b01110;
    static constexpr uint8_t phy_integer_id7      = 0b01111;
    static constexpr uint8_t phy_float_id1        = 0b10000;
    static constexpr uint8_t phy_string_id0       = 0b10001;
    static constexpr uint8_t phy_string_id1       = 0b10010;
    static constexpr uint8_t phy_string_id2       = 0b10011;
    static constexpr uint8_t phy_string_id3       = 0b10100;
    static constexpr uint8_t phy_string_id4       = 0b10101;
    static constexpr uint8_t phy_string_id5       = 0b10110;
    static constexpr uint8_t phy_string_id6       = 0b10111;
    static constexpr uint8_t phy_string_ptr_id    = 0b11000;
    static constexpr uint8_t phy_url_ptr_id       = 0b11001;
    static constexpr uint8_t phy_integer_ptr_id   = 0b11010;
    static constexpr uint8_t phy_vector_ptr_id    = 0b11011;
    static constexpr uint8_t phy_map_ptr_id       = 0b11100;
    static constexpr uint8_t phy_reserved_ptr_id0 = 0b11101;
    static constexpr uint8_t phy_reserved_ptr_id1 = 0b11110;
    static constexpr uint8_t phy_reserved_ptr_id2 = 0b11111;

    static constexpr uint64_t float_mask = datum_phy_to_mask(phy_float_id0);
    static constexpr uint64_t boolean_mask = datum_phy_to_mask(phy_boolean_id);
    static constexpr uint64_t null_mask = datum_phy_to_mask(phy_null_id);
    static constexpr uint64_t undefined_mask = datum_phy_to_mask(phy_undefined_id);
    static constexpr uint64_t string_mask = datum_phy_to_mask(phy_string_id0);
    static constexpr uint64_t integer_mask = datum_phy_to_mask(phy_integer_id0);
    static constexpr uint64_t string_pointer_mask = datum_phy_to_mask(phy_string_id0);
    static constexpr uint64_t url_pointer_mask = datum_phy_to_mask(phy_url_ptr_id);
    static constexpr uint64_t integer_pointer_mask = datum_phy_to_mask(phy_integer_ptr_id);
    static constexpr uint64_t vector_pointer_mask = datum_phy_to_mask(phy_vector_ptr_id);
    static constexpr uint64_t map_pointer_mask = datum_phy_to_mask(phy_map_ptr_id);


    using vector = std::vector<datum>;
    using map = std::unordered_map<datum,datum>;
    struct undefined {};

    union {
        double f64;
        uint64_t u64;
    };

    datum() noexcept : u64(undefined_mask) {}
    ~datum() noexcept { reset(); }
    datum(datum const &other) noexcept;
    datum &operator=(datum const &other) noexcept;

    datum(datum &&other) noexcept {
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
    }

    datum &operator=(datum &&other) noexcept {
        if (holds_pointer()) {
            reset();
        }
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
        return *this;
    }

    void reset() noexcept;

    explicit datum(double value) noexcept : f64(value) { if (value != value) { u64 = undefined_mask; } }
    explicit datum(float value) noexcept : datum(static_cast<double>(value)) {}
    explicit datum(uint64_t value) noexcept : datum(static_cast<int64_t>(value)) {}
    explicit datum(uint32_t value) noexcept : u64(integer_mask | value) {}
    explicit datum(uint16_t value) noexcept : u64(integer_mask | value) {}
    explicit datum(uint8_t value) noexcept : u64(integer_mask | value) {}
    explicit datum(int64_t value) noexcept;
    explicit datum(int32_t value) noexcept : u64(integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(int16_t value) noexcept : u64(integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(int8_t value) noexcept : u64(integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(bool value) noexcept : u64(boolean_mask | int64_t{value}) {}
    explicit datum(char value) noexcept : u64(string_mask | value) {}
    explicit datum(std::string_view value) noexcept;
    explicit datum(std::string const &value) noexcept : datum(std::string_view(value)) {}
    explicit datum(char const *value) noexcept : datum(std::string_view(value)) {}
    explicit datum(URL const &value) noexcept;
    explicit datum(datum::vector const &value) noexcept;
    explicit datum(datum::map const &value) noexcept;

    datum &operator=(double rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(float rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(uint64_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(uint32_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(uint16_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(uint8_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(int64_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(int32_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(int16_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(int8_t rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(bool rhs) noexcept { return (*this = datum{rhs}); } 
    datum &operator=(char rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(std::string_view rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(std::string const &rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(char const *rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(URL const &rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(datum::vector const &rhs) noexcept { return (*this = datum{rhs}); }
    datum &operator=(datum::map const &rhs) noexcept { return (*this = datum{rhs}); }

    //datum &operator+=(datum const &rhs) { return *this = *this + rhs; }

    explicit operator double() const;
    explicit operator float() const;
    explicit operator uint64_t() const;
    explicit operator uint32_t() const;
    explicit operator uint16_t() const;
    explicit operator uint8_t() const;
    explicit operator int64_t() const;
    explicit operator int32_t() const;
    explicit operator int16_t() const;
    explicit operator int8_t() const;
    explicit operator bool() const noexcept;
    explicit operator char() const;
    explicit operator std::string() const noexcept;
    explicit operator URL() const;
    explicit operator datum::vector() const;
    explicit operator datum::map() const;
    std::string repr() const noexcept;

    uint8_t type_id() const noexcept {
        // We use bit_cast<> on this to get access to the
        // actual bytes for determining the stored type.
        // This gets arround C++ undefined behavour of type-punning
        // through a union.
        uint64_t data;
        std::memcpy(&data, this, sizeof(data));

        auto hi_word = static_cast<uint16_t>(data >> 48);

        if ((hi_word & exponent_mask) != exponent_mask) {
            // If the not all exponent bits are set then this is a normal floating point number.
            return 0;
        }

        hi_word &= ~exponent_mask;
        hi_word |= hi_word >> 11;

        // Get the type, lower 4 bits + the sign bit.
        return static_cast<uint8_t>(hi_word);
    }

    bool is_phy_float() const noexcept { return (type_id() & 0b01111) == phy_float_id0; }
    bool is_phy_boolean() const noexcept { return type_id() == phy_boolean_id; }
    bool is_phy_null() const noexcept { return type_id() == phy_null_id; }
    bool is_phy_undefined() const noexcept { return type_id() == phy_undefined_id; }
    bool is_phy_integer() const noexcept { return (type_id() & 0b11000) == phy_integer_id0; }
    bool is_phy_string() const noexcept { let id = type_id(); return ((id & 0b11000) == 0b10000) && ((id & 0b00111) > 0); }
    bool is_phy_string_ptr() const noexcept { return type_id() == phy_string_ptr_id; }
    bool is_phy_url_ptr() const noexcept { return type_id() == phy_url_ptr_id; }
    bool is_phy_integer_ptr() const noexcept { return type_id() == phy_integer_ptr_id; }
    bool is_phy_vector_ptr() const noexcept { return type_id() == phy_vector_ptr_id; }
    bool is_phy_map_ptr() const noexcept { return type_id() == phy_map_ptr_id; }

    bool is_integer() const noexcept { return is_phy_integer() || is_phy_integer_ptr(); }
    bool is_float() const noexcept { return is_phy_float(); }
    bool is_string() const noexcept { return is_phy_string() || is_phy_string_ptr(); }
    bool is_boolean() const noexcept { return is_phy_boolean(); }
    bool is_null() const noexcept { return is_phy_null(); }
    bool is_undefined() const noexcept { return is_phy_undefined(); }
    bool is_url() const noexcept { return is_phy_url_ptr(); }
    bool is_vector() const noexcept { return is_phy_vector_ptr(); }
    bool is_map() const noexcept { return is_phy_map_ptr(); }
    bool is_numeric() const noexcept { return is_integer() || is_float(); }

    char const *type_name() const noexcept;

    bool holds_pointer() const noexcept { return (type_id() & 0b1'1000) == 0b1'1000; }
    uint64_t get_unsigned_integer() const noexcept { return u64 & 0x0000ffff'ffffffff; }
    int64_t get_signed_integer() const noexcept { return static_cast<int64_t>((u64 << 16) / 65536); }

    template<typename O>
    O *get_pointer() const {
        // canonical pointers on x86 and ARM.
        return reinterpret_cast<O *>(get_signed_integer()); 
    }

    size_t size() const;
    size_t hash() const noexcept;
};

template<typename T> inline bool holds_alternative(datum const &) { return false; }
template<> inline bool holds_alternative<int64_t>(datum const &d) { return d.is_integer(); }
template<> inline bool holds_alternative<int32_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<int16_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<int8_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<uint64_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<uint32_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<uint16_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<uint8_t>(datum const &d) { return holds_alternative<int64_t>(d); }
template<> inline bool holds_alternative<bool>(datum const &d) { return d.is_boolean(); }
template<> inline bool holds_alternative<nullptr_t>(datum const &d) { return d.is_null(); }
template<> inline bool holds_alternative<datum::undefined>(datum const &d) { return d.is_undefined(); }
template<> inline bool holds_alternative<double>(datum const &d) { return d.is_float(); }
template<> inline bool holds_alternative<float>(datum const &d) { return holds_alternative<double>(d); }
template<> inline bool holds_alternative<std::string>(datum const &d) { return d.is_string(); }
template<> inline bool holds_alternative<URL>(datum const &d) { return d.is_url(); }
template<> inline bool holds_alternative<datum::vector>(datum const &d) { return d.is_vector(); }
template<> inline bool holds_alternative<datum::map>(datum const &d) { return d.is_map(); }

template<typename T> inline bool will_cast_to(datum const &) { return false; }
template<> inline bool will_cast_to<int64_t>(datum const &d) { return d.is_numeric(); }
template<> inline bool will_cast_to<int32_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int16_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int8_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint64_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint32_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint16_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint8_t>(datum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<bool>(datum const &d) { return true; }
template<> inline bool will_cast_to<nullptr_t>(datum const &d) { return d.is_null(); }
template<> inline bool will_cast_to<datum::undefined>(datum const &d) { return d.is_undefined(); }
template<> inline bool will_cast_to<double>(datum const &d) { return d.is_numeric(); }
template<> inline bool will_cast_to<float>(datum const &d) { return will_cast_to<double>(d); }
template<> inline bool will_cast_to<std::string>(datum const &d) { return true; }
template<> inline bool will_cast_to<URL>(datum const &d) { return d.is_url() || d.is_string(); }
template<> inline bool will_cast_to<datum::vector>(datum const &d) { return d.is_vector(); }
template<> inline bool will_cast_to<datum::map>(datum const &d) { return d.is_map(); }

template<typename T> T get(datum const &) { TTAURI_THROW(invalid_operation_error("get<{}>()", typeid(T).name())); }

std::ostream &operator<<(std::ostream &os, datum const &d);

bool operator<(datum::map const &lhs, datum::map const &rhs) noexcept;

bool operator==(datum const &lhs, datum const &rhs) noexcept;
bool operator<(datum const &lhs, datum const &rhs) noexcept;
inline bool operator!=(datum const &lhs, datum const &rhs) noexcept { return !(lhs == rhs); }
inline bool operator>(datum const &lhs, datum const &rhs) noexcept { return rhs < lhs; }
inline bool operator<=(datum const &lhs, datum const &rhs) noexcept { return !(rhs < lhs); }
inline bool operator>=(datum const &lhs, datum const &rhs) noexcept { return !(lhs < rhs); }

datum operator+(datum const &lhs, datum const &rhs);
datum operator-(datum const &lhs, datum const &rhs);
datum operator*(datum const &lhs, datum const &rhs);
datum operator/(datum const &lhs, datum const &rhs);
datum operator%(datum const &lhs, datum const &rhs);
datum operator&(datum const &lhs, datum const &rhs);
datum operator|(datum const &lhs, datum const &rhs);
datum operator^(datum const &lhs, datum const &rhs);
datum operator<<(datum const &lhs, datum const &rhs);
datum operator>>(datum const &lhs, datum const &rhs);


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

inline size_t hash<TTauri::datum>::operator()(TTauri::datum const &value) const {
    return value.hash();
}

template<>
class hash<TTauri::datum::vector> {
public:
    size_t operator()(typename TTauri::datum::vector const &value) const {
        return std::accumulate(value.begin(), value.end(), size_t{0}, [](size_t a, auto x) {
            return a ^ hash<TTauri::datum>{}(x);
        });
    }
};

template<>
class hash<TTauri::datum::map> {
public:
    size_t operator()(typename TTauri::datum::map const &value) const {
        return std::accumulate(value.begin(), value.end(), size_t{0}, [](size_t a, auto x) {
            return a ^ hash<TTauri::datum>{}(x.first) ^ hash<TTauri::datum>{}(x.second);
        });
    }
};

}

#undef BI_BOOL_OPERATOR_CONVERSION
#undef BI_OPERATOR_CONVERSION
