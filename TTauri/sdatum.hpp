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
 *  - String
 *  - Vector of sdatum
 *  - Unordered map of sdatum:sdatum.
 *  - wsRGBA color.
 *
 * Due to the recursive nature of the sdatum type (through vector and map)
 * you can serialize your own types by adding conversion constructor and
 * operator to and from the sdatum on your type.
 *
 * XXX should add pickle and unpickle to sdatum.
 */
class sdatum {
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
    static constexpr uint16_t phy_string_ptr_id    = sdatum_make_id(0b11000);
    static constexpr uint16_t phy_url_ptr_id       = sdatum_make_id(0b11001);
    static constexpr uint16_t phy_integer_ptr_id   = sdatum_make_id(0b11010);
    static constexpr uint16_t phy_vector_ptr_id    = sdatum_make_id(0b11011);
    static constexpr uint16_t phy_map_ptr_id       = sdatum_make_id(0b11100);
    static constexpr uint16_t phy_wsrgba_ptr_id    = sdatum_make_id(0b11101);
    static constexpr uint16_t phy_reserved_ptr_id1 = sdatum_make_id(0b11110);
    static constexpr uint16_t phy_reserved_ptr_id2 = sdatum_make_id(0b11111);

    static constexpr uint64_t boolean_mask = sdatum_id_to_mask(phy_boolean_id);
    static constexpr uint64_t null_mask = sdatum_id_to_mask(phy_null_id);
    static constexpr uint64_t undefined_mask = sdatum_id_to_mask(phy_undefined_id);
    static constexpr uint64_t string_mask = sdatum_id_to_mask(phy_string_id0);
    static constexpr uint64_t character_mask = sdatum_id_to_mask(phy_string_id1);
    static constexpr uint64_t integer_mask = sdatum_id_to_mask(phy_integer_id0);
    static constexpr uint64_t string_ptr_mask = sdatum_id_to_mask(phy_string_ptr_id);
    static constexpr uint64_t url_ptr_mask = sdatum_id_to_mask(phy_url_ptr_id);
    static constexpr uint64_t integer_ptr_mask = sdatum_id_to_mask(phy_integer_ptr_id);
    static constexpr uint64_t vector_ptr_mask = sdatum_id_to_mask(phy_vector_ptr_id);
    static constexpr uint64_t map_ptr_mask = sdatum_id_to_mask(phy_map_ptr_id);
    static constexpr uint64_t wsrgba_ptr_mask = sdatum_id_to_mask(phy_wsrgba_ptr_id);

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
    void copy_pointer(sdatum const &other) noexcept;

public:
    using vector = std::vector<sdatum>;
    using map = std::unordered_map<sdatum,sdatum>;
    struct undefined {};
    struct null {};

    sdatum() noexcept : u64(undefined_mask) {}

    ~sdatum() noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
    }

    sdatum(sdatum const &other) noexcept {
        if (ttauri_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            std::memcpy(this, &other, sizeof(*this));
        }
    }

    sdatum &operator=(sdatum const &other) noexcept {
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

    sdatum(sdatum &&other) noexcept : u64(undefined_mask) {
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
    }

    sdatum &operator=(sdatum &&other) noexcept {
        swap(*this, other);
        return *this;
    }

    explicit sdatum(sdatum::null) noexcept : u64(null_mask) {}
    explicit sdatum(double value) noexcept : f64(value) { if (value != value) { u64 = undefined_mask; } }
    explicit sdatum(float value) noexcept : sdatum(static_cast<double>(value)) {}
    explicit sdatum(uint64_t value) noexcept : sdatum(static_cast<int64_t>(value)) {}
    explicit sdatum(uint32_t value) noexcept : u64(integer_mask | value) {}
    explicit sdatum(uint16_t value) noexcept : sdatum(static_cast<uint32_t>(value)) {}
    explicit sdatum(uint8_t value) noexcept : sdatum(static_cast<uint32_t>(value)) {}

    explicit sdatum(int64_t value) noexcept :
        u64(integer_mask | (value & 0x0000ffff'ffffffff)) {
        if (ttauri_unlikely(value < minimum_int || value > maximum_int)) {
            // Overflow.
            auto p = new int64_t(value);
            u64 = integer_ptr_mask | reinterpret_cast<uint64_t>(p);
        }
    }

    explicit sdatum(int32_t value) noexcept : u64(integer_mask | (int64_t{value} & 0x0000ffff'ffffffff)) {}
    explicit sdatum(int16_t value) noexcept : sdatum(static_cast<int32_t>(value)) {}
    explicit sdatum(int8_t value) noexcept : sdatum(static_cast<int32_t>(value)) {}
    explicit sdatum(bool value) noexcept : u64(boolean_mask | int64_t{value}) {}
    explicit sdatum(char value) noexcept : u64(character_mask | value) {}
    explicit sdatum(std::string_view value) noexcept;
    explicit sdatum(std::string const &value) noexcept : sdatum(std::string_view(value)) {}
    explicit sdatum(char const *value) noexcept : sdatum(std::string_view(value)) {}
    explicit sdatum(URL const &value) noexcept;
    explicit sdatum(sdatum::vector const &value) noexcept;
    explicit sdatum(sdatum::map const &value) noexcept;
    explicit sdatum(wsRGBA const &value) noexcept;

    sdatum &operator=(double rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        f64 = rhs;
        return *this;
    }
    
    sdatum &operator=(float rhs) noexcept { return (*this = static_cast<double>(rhs)); }
    sdatum &operator=(uint64_t rhs) noexcept { return (*this = sdatum{rhs}); }

    sdatum &operator=(uint32_t rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = integer_mask | uint64_t{rhs};
        return *this;
    }

    sdatum &operator=(uint16_t rhs) noexcept { return (*this =  static_cast<uint32_t>(rhs)); }
    sdatum &operator=(uint8_t rhs) noexcept { return (*this = static_cast<uint32_t>(rhs)); }
    sdatum &operator=(int64_t rhs) noexcept { return (*this = sdatum{rhs}); }

    sdatum &operator=(int32_t rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = integer_mask | (int64_t(rhs) & 0x0000'ffff'ffff'ffff);
        return *this;
    }

    sdatum &operator=(int16_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }
    sdatum &operator=(int8_t rhs) noexcept { return (*this = static_cast<int32_t>(rhs)); }

    sdatum &operator=(bool rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = boolean_mask | int64_t{rhs};
        return *this;
    }
    
    sdatum &operator=(char rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = character_mask | static_cast<uint64_t>(rhs);
        return *this;
    }

    sdatum &operator=(std::string_view rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(std::string const &rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(char const *rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(URL const &rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(sdatum::vector const &rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(sdatum::map const &rhs) noexcept { return (*this = sdatum{rhs}); }
    sdatum &operator=(wsRGBA const &rhs) noexcept { return (*this = sdatum{rhs}); }

    //sdatum &operator+=(sdatum const &rhs) { return *this = *this + rhs; }

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
    explicit operator sdatum::vector() const;
    explicit operator sdatum::map() const;
    explicit operator wsRGBA() const;

    bool operator!() const noexcept { return !static_cast<bool>(*this); }
    sdatum operator~() const;
    sdatum operator-() const;
    sdatum &operator[](sdatum const &rhs);
    sdatum operator[](sdatum const &rhs) const;
    sdatum &append();

    template<typename T>
    std::enable_if_t<!std::is_same_v<T, sdatum>, sdatum>
    &operator[](T const &rhs) {
        return (*this)[sdatum{rhs}];
    }

    template<typename T>
    std::enable_if_t<!std::is_same_v<T, sdatum>, sdatum>
    operator[](T const &rhs) const {
        return (*this)[sdatum{rhs}];
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

    sdatum &get_by_path(std::vector<std::string> const &key);
    sdatum get_by_path(std::vector<std::string> const &key) const;

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

    friend bool operator==(sdatum const &lhs, sdatum const &rhs) noexcept;
    friend bool operator<(sdatum const &lhs, sdatum const &rhs) noexcept;
};

template<typename T> inline bool will_cast_to(sdatum const &) { return false; }
template<> inline bool will_cast_to<int64_t>(sdatum const &d) { return d.is_numeric(); }
template<> inline bool will_cast_to<int32_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int16_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<int8_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint64_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint32_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint16_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<uint8_t>(sdatum const &d) { return will_cast_to<int64_t>(d); }
template<> inline bool will_cast_to<bool>(sdatum const &d) { return true; }
template<> inline bool will_cast_to<sdatum::undefined>(sdatum const &d) { return d.is_undefined(); }
template<> inline bool will_cast_to<double>(sdatum const &d) { return d.is_numeric(); }
template<> inline bool will_cast_to<float>(sdatum const &d) { return will_cast_to<double>(d); }
template<> inline bool will_cast_to<std::string>(sdatum const &d) { return true; }
template<> inline bool will_cast_to<URL>(sdatum const &d) { return d.is_url() || d.is_string(); }
template<> inline bool will_cast_to<sdatum::vector>(sdatum const &d) { return d.is_vector(); }
template<> inline bool will_cast_to<sdatum::map>(sdatum const &d) { return d.is_map(); }
template<> inline bool will_cast_to<wsRGBA>(sdatum const &d) { return d.is_wsrgba(); }

std::string to_string(sdatum const &d);
std::ostream &operator<<(std::ostream &os, sdatum const &d);

inline void swap(sdatum &lhs, sdatum &rhs) noexcept {
    memswap(lhs, rhs);
}

bool operator<(sdatum::map const &lhs, sdatum::map const &rhs) noexcept;

bool operator==(sdatum const &lhs, sdatum const &rhs) noexcept;
bool operator<(sdatum const &lhs, sdatum const &rhs) noexcept;
inline bool operator!=(sdatum const &lhs, sdatum const &rhs) noexcept { return !(lhs == rhs); }
inline bool operator>(sdatum const &lhs, sdatum const &rhs) noexcept { return rhs < lhs; }
inline bool operator<=(sdatum const &lhs, sdatum const &rhs) noexcept { return !(rhs < lhs); }
inline bool operator>=(sdatum const &lhs, sdatum const &rhs) noexcept { return !(lhs < rhs); }

sdatum operator+(sdatum const &lhs, sdatum const &rhs);
sdatum operator-(sdatum const &lhs, sdatum const &rhs);
sdatum operator*(sdatum const &lhs, sdatum const &rhs);
sdatum operator/(sdatum const &lhs, sdatum const &rhs);
sdatum operator%(sdatum const &lhs, sdatum const &rhs);
sdatum operator&(sdatum const &lhs, sdatum const &rhs);
sdatum operator|(sdatum const &lhs, sdatum const &rhs);
sdatum operator^(sdatum const &lhs, sdatum const &rhs);
sdatum operator<<(sdatum const &lhs, sdatum const &rhs);
sdatum operator>>(sdatum const &lhs, sdatum const &rhs);


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

template<>
class hash<TTauri::sdatum::vector> {
public:
    size_t operator()(typename TTauri::sdatum::vector const &value) const {
        return std::accumulate(value.begin(), value.end(), size_t{0}, [](size_t a, auto x) {
            return a ^ hash<TTauri::sdatum>{}(x);
        });
    }
};

template<>
class hash<TTauri::sdatum::map> {
public:
    size_t operator()(typename TTauri::sdatum::map const &value) const {
        return std::accumulate(value.begin(), value.end(), size_t{0}, [](size_t a, auto x) {
            return a ^ hash<TTauri::sdatum>{}(x.first) ^ hash<TTauri::sdatum>{}(x.second);
        });
    }
};

}

#undef BI_BOOL_OPERATOR_CONVERSION
#undef BI_OPERATOR_CONVERSION
