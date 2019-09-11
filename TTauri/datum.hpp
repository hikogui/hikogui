// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "os_detect.hpp"
#include "URL.hpp"
#include "wsRGBA.hpp"
#include "memory.hpp"
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
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(datum const &lhs, T const &rhs) { return lhs op datum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(T const &lhs, datum const &rhs) { return datum{lhs} op datum{rhs}; }

#define BI_BOOL_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, bool> operator op(datum const &lhs, T const &rhs) { return lhs op datum{rhs}; }\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, bool> operator op(T const &lhs, datum const &rhs) { return datum{lhs} op datum{rhs}; }

#define MONO_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, datum> operator op(T const &rhs) { return lhs op datum{rhs}; }\

#define MONO_BOOL_OPERATOR_CONVERSION(op)\
    template<typename T> std::enable_if_t<!std::is_same_v<T, datum>, bool> operator op(T const &rhs) { return lhs op datum{rhs}; }\

namespace TTauri {
class datum;
}

namespace std {

template<>
class hash<TTauri::datum> {
public:
    size_t operator()(TTauri::datum const &value) const;
};

}

namespace TTauri {


constexpr uint64_t datum_id_to_mask(uint64_t id) {
    return id << 48;
}

constexpr uint16_t datum_make_id(uint16_t id) {
    return ((id & 0x10) << 11) | (id & 0xf) | 0x7ff0;
}

void swap(datum &lhs, datum &rhs) noexcept;


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
 *  - wsRGBA color.
 *
 * Due to the recursive nature of the datum type (through vector and map)
 * you can serialize your own types by adding conversion constructor and
 * operator to and from the datum on your type.
 *
 * XXX should add pickle and unpickle to datum.
 */
class datum {
    gsl_suppress(bounds.4)
    static constexpr uint64_t make_string(std::string_view str)
    {
        let len = str.size();

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

    static constexpr int64_t minimum_int = 0xfffe'0000'0000'0000LL;
    static constexpr int64_t maximum_int = 0x0007'ffff'ffff'ffffLL;

    static constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    static constexpr uint64_t pointer_mask = 0x0000'ffff'ffff'ffff;

    static constexpr uint16_t phy_boolean_id       = datum_make_id(0b00001);
    static constexpr uint16_t phy_null_id          = datum_make_id(0b00010);
    static constexpr uint16_t phy_undefined_id     = datum_make_id(0b00011);
    static constexpr uint16_t phy_reserved_id0     = datum_make_id(0b00100);
    static constexpr uint16_t phy_reserved_id1     = datum_make_id(0b00101);
    static constexpr uint16_t phy_reserved_id2     = datum_make_id(0b00110);
    static constexpr uint16_t phy_reserved_id3     = datum_make_id(0b00111);
    static constexpr uint16_t phy_integer_id0      = datum_make_id(0b01000);
    static constexpr uint16_t phy_integer_id1      = datum_make_id(0b01001);
    static constexpr uint16_t phy_integer_id2      = datum_make_id(0b01010);
    static constexpr uint16_t phy_integer_id3      = datum_make_id(0b01011);
    static constexpr uint16_t phy_integer_id4      = datum_make_id(0b01100);
    static constexpr uint16_t phy_integer_id5      = datum_make_id(0b01101);
    static constexpr uint16_t phy_integer_id6      = datum_make_id(0b01110);
    static constexpr uint16_t phy_integer_id7      = datum_make_id(0b01111);

    static constexpr uint16_t phy_string_id0       = datum_make_id(0b10001);
    static constexpr uint16_t phy_string_id1       = datum_make_id(0b10010);
    static constexpr uint16_t phy_string_id2       = datum_make_id(0b10011);
    static constexpr uint16_t phy_string_id3       = datum_make_id(0b10100);
    static constexpr uint16_t phy_string_id4       = datum_make_id(0b10101);
    static constexpr uint16_t phy_string_id5       = datum_make_id(0b10110);
    static constexpr uint16_t phy_string_id6       = datum_make_id(0b10111);
    static constexpr uint16_t phy_string_ptr_id    = datum_make_id(0b11000);
    static constexpr uint16_t phy_url_ptr_id       = datum_make_id(0b11001);
    static constexpr uint16_t phy_integer_ptr_id   = datum_make_id(0b11010);
    static constexpr uint16_t phy_vector_ptr_id    = datum_make_id(0b11011);
    static constexpr uint16_t phy_map_ptr_id       = datum_make_id(0b11100);
    static constexpr uint16_t phy_wsrgba_ptr_id    = datum_make_id(0b11101);
    static constexpr uint16_t phy_reserved_ptr_id1 = datum_make_id(0b11110);
    static constexpr uint16_t phy_reserved_ptr_id2 = datum_make_id(0b11111);

    static constexpr uint64_t boolean_mask = datum_id_to_mask(phy_boolean_id);
    static constexpr uint64_t null_mask = datum_id_to_mask(phy_null_id);
    static constexpr uint64_t undefined_mask = datum_id_to_mask(phy_undefined_id);
    static constexpr uint64_t string_mask = datum_id_to_mask(phy_string_id0);
    static constexpr uint64_t character_mask = datum_id_to_mask(phy_string_id1);
    static constexpr uint64_t integer_mask = datum_id_to_mask(phy_integer_id0);
    static constexpr uint64_t string_ptr_mask = datum_id_to_mask(phy_string_ptr_id);
    static constexpr uint64_t url_ptr_mask = datum_id_to_mask(phy_url_ptr_id);
    static constexpr uint64_t integer_ptr_mask = datum_id_to_mask(phy_integer_ptr_id);
    static constexpr uint64_t vector_ptr_mask = datum_id_to_mask(phy_vector_ptr_id);
    static constexpr uint64_t map_ptr_mask = datum_id_to_mask(phy_map_ptr_id);
    static constexpr uint64_t wsrgba_ptr_mask = datum_id_to_mask(phy_wsrgba_ptr_id);

    union {
        double f64;
        uint64_t u64;
    };

    gsl_suppress(type.1)
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

    bool is_phy_pointer() const noexcept {
        return (type_id() & 0xfff8) == 0xfff8;
    }

    bool is_phy_boolean() const noexcept { return type_id() == phy_boolean_id; }
    bool is_phy_null() const noexcept { return type_id() == phy_null_id; }
    bool is_phy_undefined() const noexcept { return type_id() == phy_undefined_id; }
    bool is_phy_string_ptr() const noexcept { return type_id() == phy_string_ptr_id; }
    bool is_phy_url_ptr() const noexcept { return type_id() == phy_url_ptr_id; }
    bool is_phy_integer_ptr() const noexcept { return type_id() == phy_integer_ptr_id; }
    bool is_phy_vector_ptr() const noexcept { return type_id() == phy_vector_ptr_id; }
    bool is_phy_map_ptr() const noexcept { return type_id() == phy_map_ptr_id; }
    bool is_phy_wsrgba_ptr() const noexcept { return type_id() == phy_wsrgba_ptr_id; }

    void delete_pointer() noexcept;
    void copy_pointer(datum const &other) noexcept;

public:
    using vector = std::vector<datum>;
    using map = std::unordered_map<datum,datum>;
    struct undefined {};
    struct null {};

    datum() noexcept : u64(undefined_mask) {}

    ~datum() noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
    }

    datum(datum const &other) noexcept {
        if (ttauri_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            std::memcpy(this, &other, sizeof(*this));
        }
    }

    datum &operator=(datum const &other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        if (ttauri_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            std::memcpy(this, &other, sizeof(*this));
        }
        return *this;
    }

    datum(datum &&other) noexcept : u64(undefined_mask) {
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
    }

    datum &operator=(datum &&other) noexcept {
        swap(*this, other);
        return *this;
    }

    explicit datum(datum::null) noexcept : u64(null_mask) {}
    explicit datum(double value) noexcept : f64(value) { if (value != value) { u64 = undefined_mask; } }
    explicit datum(float value) noexcept : datum(static_cast<double>(value)) {}
    explicit datum(uint64_t value) noexcept : datum(static_cast<int64_t>(value)) {}
    explicit datum(uint32_t value) noexcept : u64(integer_mask | value) {}
    explicit datum(uint16_t value) noexcept : datum(static_cast<uint32_t>(value)) {}
    explicit datum(uint8_t value) noexcept : datum(static_cast<uint32_t>(value)) {}

    gsl_suppress4(type.1,r.11,r.3,con.4)
    explicit datum(int64_t value) noexcept :
        u64(integer_mask | (value & 0x0000ffff'ffffffff)) {
        if (ttauri_unlikely(value < minimum_int || value > maximum_int)) {
            // Overflow.
            auto * const p = new int64_t(value);
            u64 = integer_ptr_mask | reinterpret_cast<uint64_t>(p);
        }
    }

    explicit datum(int32_t value) noexcept : u64(integer_mask | (static_cast<uint64_t>(value) & 0x0000ffff'ffffffff)) {}
    explicit datum(int16_t value) noexcept : datum(static_cast<int32_t>(value)) {}
    explicit datum(int8_t value) noexcept : datum(static_cast<int32_t>(value)) {}
    explicit datum(bool value) noexcept : u64(boolean_mask | static_cast<uint64_t>(value)) {}
    explicit datum(char value) noexcept : u64(character_mask | value) {}
    explicit datum(std::string_view value) noexcept;
    explicit datum(std::string const &value) noexcept : datum(std::string_view(value)) {}
    explicit datum(char const *value) noexcept : datum(std::string_view(value)) {}
    explicit datum(URL const &value) noexcept;
    explicit datum(datum::vector const &value) noexcept;
    explicit datum(datum::map const &value) noexcept;
    explicit datum(wsRGBA const &value) noexcept;

    datum &operator=(double rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        f64 = rhs;
        return *this;
    }
    
    datum &operator=(float rhs) noexcept { return (*this = static_cast<double>(rhs)); }

    datum &operator=(int64_t rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = integer_mask | static_cast<uint64_t>(rhs & 0x0000ffff'ffffffff);
        if (ttauri_unlikely(rhs < minimum_int || rhs > maximum_int)) {
            // Overflow.
            auto * const p = new int64_t(rhs);
            u64 = integer_ptr_mask | reinterpret_cast<uint64_t>(p);
        }
        return *this;
    }

    datum &operator=(int32_t rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = integer_mask | static_cast<uint64_t>(static_cast<int64_t>(rhs) & 0x0000'ffff'ffff'ffff);
        return *this;
    }

    datum &operator=(int16_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }
    datum &operator=(int8_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }

    datum &operator=(uint64_t rhs) noexcept { return (*this = static_cast<int64_t>(rhs)); }

    datum &operator=(uint32_t rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = integer_mask | static_cast<uint64_t>(rhs);
        return *this;
    }

    datum &operator=(uint16_t rhs) noexcept { return (*this =  static_cast<uint32_t>(rhs)); }
    datum &operator=(uint8_t rhs) noexcept { return (*this = static_cast<uint32_t>(rhs)); }

    datum &operator=(bool rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = boolean_mask | static_cast<uint64_t>(rhs);
        return *this;
    }
    
    datum &operator=(char rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = character_mask | static_cast<uint64_t>(rhs);
        return *this;
    }

    datum &operator=(std::string_view rhs);
    datum &operator=(std::string const &rhs) { *this = std::string_view{rhs}; return *this; }
    datum &operator=(char const *rhs) { *this = std::string_view{rhs}; return *this; }
    datum &operator=(URL const &rhs) noexcept;
    datum &operator=(datum::vector const &rhs);
    datum &operator=(datum::map const &rhs);
    datum &operator=(wsRGBA const &rhs);

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
    explicit operator wsRGBA() const;

    bool operator!() const noexcept { return !static_cast<bool>(*this); }
    datum operator~() const;
    datum operator-() const;
    datum &operator[](datum const &rhs);
    datum operator[](datum const &rhs) const;
    datum &append();

    template<typename T>
    std::enable_if_t<!std::is_same_v<T, datum>, datum>
    &operator[](T const &rhs) {
        return (*this)[datum{rhs}];
    }

    template<typename T>
    std::enable_if_t<!std::is_same_v<T, datum>, datum>
    operator[](T const &rhs) const {
        return (*this)[datum{rhs}];
    }

    std::string repr() const noexcept;


    /*! Return ordering of types.
     * Used in less-than comparison between different types.
     */
    int type_order() const noexcept {
        if (is_float() || is_phy_integer_ptr()) {
            // Fold all numeric values into the same group (literal integers).
            return phy_integer_id0;
        } else {
            return type_id();
        }
    }

    datum &get_by_path(std::vector<std::string> const &key);
    datum get_by_path(std::vector<std::string> const &key) const;

    bool is_integer() const noexcept { return is_phy_integer() || is_phy_integer_ptr(); }
    bool is_float() const noexcept { return is_phy_float(); }
    bool is_string() const noexcept { return is_phy_string() || is_phy_string_ptr(); }
    bool is_boolean() const noexcept { return is_phy_boolean(); }
    bool is_null() const noexcept { return is_phy_null(); }
    bool is_undefined() const noexcept { return is_phy_undefined(); }
    bool is_url() const noexcept { return is_phy_url_ptr(); }
    bool is_vector() const noexcept { return is_phy_vector_ptr(); }
    bool is_map() const noexcept { return is_phy_map_ptr(); }
    bool is_wsrgba() const noexcept { return is_phy_wsrgba_ptr(); }
    bool is_numeric() const noexcept { return is_integer() || is_float(); }
    bool is_color() const noexcept { return is_wsrgba(); }

    char const *type_name() const noexcept;

    uint64_t get_unsigned_integer() const noexcept { return u64 & 0x0000ffff'ffffffff; }
    int64_t get_signed_integer() const noexcept { return static_cast<int64_t>(u64 << 16) >> 16; }

    template<typename O>
    O *get_pointer() const {
        // canonical pointers on x86 and ARM.
        return reinterpret_cast<O *>(get_signed_integer()); 
    }

    size_t size() const;
    size_t hash() const noexcept;

    friend bool operator==(datum const &lhs, datum const &rhs) noexcept;
    friend bool operator<(datum const &lhs, datum const &rhs) noexcept;
};

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
template<> inline bool will_cast_to<datum::undefined>(datum const &d) { return d.is_undefined(); }
template<> inline bool will_cast_to<double>(datum const &d) { return d.is_numeric(); }
template<> inline bool will_cast_to<float>(datum const &d) { return will_cast_to<double>(d); }
template<> inline bool will_cast_to<std::string>(datum const &d) { return true; }
template<> inline bool will_cast_to<URL>(datum const &d) { return d.is_url() || d.is_string(); }
template<> inline bool will_cast_to<datum::vector>(datum const &d) { return d.is_vector(); }
template<> inline bool will_cast_to<datum::map>(datum const &d) { return d.is_map(); }
template<> inline bool will_cast_to<wsRGBA>(datum const &d) { return d.is_wsrgba(); }

std::string to_string(datum const &d);
std::ostream &operator<<(std::ostream &os, datum const &d);

inline void swap(datum &lhs, datum &rhs) noexcept {
    memswap(lhs, rhs);
}

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
