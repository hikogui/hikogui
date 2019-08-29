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

enum class datum_logical_type {
    Float,
    Integer,
    Boolean,
    Null,
    Undefined,
    Vector,
    Map,
    String,
    URL
};

constexpr datum_logical_type to_datum_logical_type(uint8_t x)
{
    switch (x) {
    case 0b0'0000: return datum_logical_type::Float; // double (+infinite)
    case 0b0'0001: return datum_logical_type::Boolean; // bool
    case 0b0'0010: return datum_logical_type::Null; // nullptr_t
    case 0b0'0011: return datum_logical_type::Undefined; // A value that is expected to be replaced.
    case 0b0'0100: no_default;
    case 0b0'0101: no_default;
    case 0b0'0110: no_default;
    case 0b0'0111: no_default;
    case 0b0'1000: return datum_logical_type::Integer; // int51_t
    case 0b0'1001: return datum_logical_type::Integer; // int51_t
    case 0b0'1010: return datum_logical_type::Integer; // int51_t
    case 0b0'1011: return datum_logical_type::Integer; // int51_t
    case 0b0'1100: return datum_logical_type::Integer; // int51_t
    case 0b0'1101: return datum_logical_type::Integer; // int51_t
    case 0b0'1110: return datum_logical_type::Integer; // int51_t
    case 0b0'1111: return datum_logical_type::Integer; // int51_t

    case 0b1'0000: return datum_logical_type::Float; // double (-infinite)
    case 0b1'0001: return datum_logical_type::String; // char8_t[0]
    case 0b1'0010: return datum_logical_type::String; // char8_t[1]
    case 0b1'0011: return datum_logical_type::String; // char8_t[2]
    case 0b1'0100: return datum_logical_type::String; // char8_t[3]
    case 0b1'0101: return datum_logical_type::String; // char8_t[4]
    case 0b1'0110: return datum_logical_type::String; // char8_t[5]
    case 0b1'0111: return datum_logical_type::String; // char8_t[6]
    case 0b1'1000: return datum_logical_type::String; // std::string*
    case 0b1'1001: return datum_logical_type::URL; // URL*
    case 0b1'1010: return datum_logical_type::Integer; // int64_t*
    case 0b1'1011: return datum_logical_type::Vector; // std::vector<datum>*
    case 0b1'1100: return datum_logical_type::Map; // std::unordered_map<datum,datum>*
    case 0b1'1101: no_default;
    case 0b1'1110: no_default;
    case 0b1'1111: no_default;

    default: no_default;
    }
}

enum class datum_physical_type {
    Float,
    Boolean,
    Null,
    Undefined,
    String,
    Integer,
    VectorPointer,
    MapPointer,
    StringPointer,
    URLPointer,
    IntegerPointer
};

constexpr uint64_t datum_type_id_to_mask(uint8_t x)
{
    return ((uint64_t{x} & 0x10) << 59) | ((uint64_t{x} & 0xf) << 48);
}

constexpr uint8_t datum_physical_type_to_type_id(datum_physical_type x)
{
    switch (x) {
    case datum_physical_type::Float: return 0b0'0000;
    case datum_physical_type::Boolean: return 0b0'0001;
    case datum_physical_type::Null: return 0b0'0010;
    case datum_physical_type::Undefined: return 0b0'0011;
    case datum_physical_type::Integer: return 0b0'1000;
    case datum_physical_type::String: return 0b1'0001;
    case datum_physical_type::StringPointer: return 0b1'1000;
    case datum_physical_type::URLPointer: return 0b1'1001;
    case datum_physical_type::IntegerPointer: return 0b1'1010;
    case datum_physical_type::VectorPointer: return 0b1'1011;
    case datum_physical_type::MapPointer: return 0b1'1100;
    default: no_default;
    }
}

constexpr uint64_t datum_physical_type_to_mask(datum_physical_type x)
{
    return datum_type_id_to_mask(datum_physical_type_to_type_id(x));
}

constexpr uint64_t datum_float_mask = datum_physical_type_to_mask(datum_physical_type::Float);
constexpr uint64_t datum_boolean_mask = datum_physical_type_to_mask(datum_physical_type::Boolean);
constexpr uint64_t datum_null_mask = datum_physical_type_to_mask(datum_physical_type::Null);
constexpr uint64_t datum_undefined_mask = datum_physical_type_to_mask(datum_physical_type::Undefined);
constexpr uint64_t datum_string_mask = datum_physical_type_to_mask(datum_physical_type::String);
constexpr uint64_t datum_integer_mask = datum_physical_type_to_mask(datum_physical_type::Integer);
constexpr uint64_t datum_string_pointer_mask = datum_physical_type_to_mask(datum_physical_type::StringPointer);
constexpr uint64_t datum_url_pointer_mask = datum_physical_type_to_mask(datum_physical_type::URLPointer);
constexpr uint64_t datum_integer_pointer_mask = datum_physical_type_to_mask(datum_physical_type::IntegerPointer);
constexpr uint64_t datum_vector_pointer_mask = datum_physical_type_to_mask(datum_physical_type::VectorPointer);
constexpr uint64_t datum_map_pointer_mask = datum_physical_type_to_mask(datum_physical_type::MapPointer);

constexpr uint64_t datum_make_string(std::string_view str)
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
    return (datum_string_mask + (len << 48)) | x;
}

constexpr datum_physical_type to_datum_physical_type(uint8_t x)
{
    switch (x) {
    case 0b0'0000: return datum_physical_type::Float; // double (+infinite)
    case 0b0'0001: return datum_physical_type::Boolean; // bool
    case 0b0'0010: return datum_physical_type::Null; // nullptr_t
    case 0b0'0011: return datum_physical_type::Undefined; // A value that is expected to be replaced.
    case 0b0'0100: no_default;
    case 0b0'0101: no_default;
    case 0b0'0110: no_default;
    case 0b0'0111: no_default;
    case 0b0'1000: return datum_physical_type::Integer; // int51_t
    case 0b0'1001: return datum_physical_type::Integer; // int51_t
    case 0b0'1010: return datum_physical_type::Integer; // int51_t
    case 0b0'1011: return datum_physical_type::Integer; // int51_t
    case 0b0'1100: return datum_physical_type::Integer; // int51_t
    case 0b0'1101: return datum_physical_type::Integer; // int51_t
    case 0b0'1110: return datum_physical_type::Integer; // int51_t
    case 0b0'1111: return datum_physical_type::Integer; // int51_t

    case 0b1'0000: return datum_physical_type::Float; // double (-infinite)
    case 0b1'0001: return datum_physical_type::String; // char8_t[0]
    case 0b1'0010: return datum_physical_type::String; // char8_t[1]
    case 0b1'0011: return datum_physical_type::String; // char8_t[2]
    case 0b1'0100: return datum_physical_type::String; // char8_t[3]
    case 0b1'0101: return datum_physical_type::String; // char8_t[4]
    case 0b1'0110: return datum_physical_type::String; // char8_t[5]
    case 0b1'0111: return datum_physical_type::String; // char8_t[6]
    case 0b1'1000: return datum_physical_type::StringPointer; // std::string*
    case 0b1'1001: return datum_physical_type::URLPointer; // URL*
    case 0b1'1010: return datum_physical_type::IntegerPointer; // int64_t*
    case 0b1'1011: return datum_physical_type::VectorPointer; // std::vector<datum>*
    case 0b1'1100: return datum_physical_type::MapPointer; // std::unordered_map<datum,datum>*
    case 0b1'1101: no_default;
    case 0b1'1110: no_default;
    case 0b1'1111: no_default;
    default: no_default;
    }
}

constexpr int64_t datum_min_int = 0xfffe'0000'0000'0000LL;
constexpr int64_t datum_max_int = 0x0007'ffff'ffff'ffffLL;

template<typename T>
bool holds_alternative(datum const &);

template<typename T>
T get(datum const &);

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
    using vector = std::vector<datum>;
    using map = std::unordered_map<datum,datum>;
    struct undefined {};

    union {
        double f64;
        uint64_t u64;
    };

    datum() noexcept : u64(datum_undefined_mask) {}
    datum(datum const &other) noexcept;
    datum(datum &&other) noexcept;
    datum &operator=(datum const &other) noexcept;
    datum &operator=(datum &&other) noexcept;
    ~datum() noexcept { reset(); }

    void reset() noexcept;

    explicit datum(double value) noexcept : f64(value) { if (value != value) { u64 = datum_undefined_mask; } }
    explicit datum(float value) noexcept : datum(static_cast<double>(value)) {}
    explicit datum(uint64_t value) noexcept : datum(static_cast<int64_t>(value)) {}
    explicit datum(uint32_t value) noexcept : u64(datum_integer_mask | value) {}
    explicit datum(uint16_t value) noexcept : u64(datum_integer_mask | value) {}
    explicit datum(uint8_t value) noexcept : u64(datum_integer_mask | value) {}
    explicit datum(int64_t value) noexcept;
    explicit datum(int32_t value) noexcept : u64(datum_integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(int16_t value) noexcept : u64(datum_integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(int8_t value) noexcept : u64(datum_integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit datum(bool value) noexcept : u64(datum_boolean_mask | int64_t{value}) {}
    explicit datum(char value) noexcept : u64(datum_string_mask | value) {}
    explicit datum(std::string_view value) noexcept;
    explicit datum(std::string const &value) noexcept : datum(std::string_view(value)) {}
    explicit datum(char const *value) noexcept : datum(std::string_view(value)) {}
    explicit datum(URL const &value) noexcept;
    explicit datum(datum::vector const &value) noexcept;
    explicit datum(datum::map const &value) noexcept;

    datum &operator=(double value) noexcept { return (*this = datum{value}); }
    datum &operator=(float value) noexcept { return (*this = datum{value}); }
    datum &operator=(uint64_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(uint32_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(uint16_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(uint8_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(int64_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(int32_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(int16_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(int8_t value) noexcept { return (*this = datum{value}); }
    datum &operator=(bool value) noexcept { return (*this = datum{value}); } 
    datum &operator=(char value) noexcept { return (*this = datum{value}); }
    datum &operator=(std::string_view value) noexcept { return (*this = datum{value}); }
    datum &operator=(std::string const &value) noexcept { return (*this = datum{value}); }
    datum &operator=(char const *value) noexcept { return (*this = datum{value}); }
    datum &operator=(URL const &value) noexcept { return (*this = datum{value}); }
    datum &operator=(datum::vector const &value) noexcept { return (*this = datum{value}); }
    datum &operator=(datum::map const &value) noexcept { return (*this = datum{value}); }

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

    uint8_t type_id() const noexcept;
    datum_physical_type physical_type() const noexcept { return to_datum_physical_type(type_id()); }
    datum_logical_type logical_type() const noexcept { return to_datum_logical_type(type_id()); }
    bool is_integer() const noexcept { return logical_type() == datum_logical_type::Integer; }
    bool is_float() const noexcept { return logical_type() == datum_logical_type::Float; }
    bool is_string() const noexcept { return logical_type() == datum_logical_type::String; }
    bool is_boolean() const noexcept { return logical_type() == datum_logical_type::Boolean; }
    bool is_null() const noexcept { return logical_type() == datum_logical_type::Null; }
    bool is_undefined() const noexcept { return logical_type() == datum_logical_type::Undefined; }
    bool is_url() const noexcept { return logical_type() == datum_logical_type::URL; }
    bool is_vector() const noexcept { return logical_type() == datum_logical_type::Vector; }
    bool is_map() const noexcept { return logical_type() == datum_logical_type::Map; }
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
template<> inline bool holds_alternative<int64_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<int32_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<int16_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<int8_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<uint64_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<uint32_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<uint16_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<uint8_t>(datum const &d) { return d.logical_type() == datum_logical_type::Integer; }
template<> inline bool holds_alternative<bool>(datum const &d) { return d.logical_type() == datum_logical_type::Boolean; }
template<> inline bool holds_alternative<nullptr_t>(datum const &d) { return d.logical_type() == datum_logical_type::Null; }
template<> inline bool holds_alternative<datum::undefined>(datum const &d) { return d.logical_type() == datum_logical_type::Null; }
template<> inline bool holds_alternative<double>(datum const &d) { return d.logical_type() == datum_logical_type::Float; }
template<> inline bool holds_alternative<float>(datum const &d) { return d.logical_type() == datum_logical_type::Float; }
template<> inline bool holds_alternative<std::string>(datum const &d) { return d.logical_type() == datum_logical_type::String; }
template<> inline bool holds_alternative<URL>(datum const &d) { return d.logical_type() == datum_logical_type::URL; }
template<> inline bool holds_alternative<datum::vector>(datum const &d) { return d.logical_type() == datum_logical_type::Vector; }
template<> inline bool holds_alternative<datum::map>(datum const &d) { return d.logical_type() == datum_logical_type::Map; }


template<typename T> T get(datum const &) { TTAURI_THROW(invalid_operation_error("get<{}>()", typeid(T).name())); }

bool operator<(datum::map const &lhs, datum::map const &rhs) noexcept;

bool operator==(datum const &lhs, datum const &rhs) noexcept;
bool operator<(datum const &lhs, datum const &rhs) noexcept;

datum operator+(datum const &lhs, datum const &rhs);

#define BI_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(datum const &lhs, T const &rhs) { return lhs op datum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(T const &lhs, datum const &rhs) { return datum{lhs} op datum{rhs}; }

BI_OPERATOR_CONVERSION(==)
BI_OPERATOR_CONVERSION(<)
BI_OPERATOR_CONVERSION(+)

#undef BI_OPERATOR_CONVERSION
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

