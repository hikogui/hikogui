// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri {

enum class datum_type {
    FloatingPoint,
    ShortInteger,
    ShortString,
    Boolean,
    Null,
    Undefined,
    Vector,
    UnorderedMap,
    String,
    URL,
    LongInteger
};

constexpr datum_type to_datum_type(uint8_t x)
{
    switch (x) {
    case 0b0'0000: return FloatingPoint; // double (+infinite)
    case 0b0'0001: return Boolean; // bool
    case 0b0'0010: return Null; // nullptr_t
    case 0b0'0011: return Undefined; // A value that is expected to be replaced.
    case 0b0'0100: no_default;
    case 0b0'0101: no_default;
    case 0b0'0110: return ShortString; // char7_t[7] null terminated.
    case 0b0'0111: return ShortString; // char7_t[7] null terminated.
    case 0b0'1000: return ShortInteger; // int51_t
    case 0b0'1001: return ShortInteger; // int51_t
    case 0b0'1010: return ShortInteger; // int51_t
    case 0b0'1011: return ShortInteger; // int51_t
    case 0b0'1100: return ShortInteger; // int51_t
    case 0b0'1101: return ShortInteger; // int51_t
    case 0b0'1110: return ShortInteger; // int51_t
    case 0b0'1111: return ShortInteger; // int51_t

    case 0b1'0000: return FloatingPoint; // double (-infinite)
    case 0b1'0001: return String; // std::string*
    case 0b1'0010: return URL; // URL*
    case 0b1'0011: return LongInteger; // int64_t*
    case 0b1'0100: return Vector; // std::vector<datum>*
    case 0b1'0101: return UnorderedMap; // std::unordered_map<datum,datum>*
    case 0b1'0110: no_default;
    case 0b1'0111: no_default;
    case 0b1'1000: no_default;
    case 0b1'1001: no_default;
    case 0b1'1010: no_default;
    case 0b1'1011: no_default;
    case 0b1'1100: no_default;
    case 0b1'1101: no_default;
    case 0b1'1110: no_default;
    case 0b1'1111: no_default;

    default: no_default;
    }
}

inline double datum_nan = std::nan("1");

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
    union {
        double      f64;
        intptr_t    p64;
        int64_t     i64;
        uint64_t    u64;
    };

    explicit datum(double value) : f64(std::isnan(value) ? datum_nan : value) {}

    datum_type type() const {
        constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
        constexpr uint16_t type_mask = 0b0000'0000'0000'1111;

        // We use bit_cast<> on this to get access to the
        // actual bytes for determining the stored type.
        // This gets arround C++ undefined behavour of type-punning
        // through a union.
        std::byte bytes[sizeof(datum)];
        std::memcpy(bytes, this, sizeof(bytes));

        let ms_word = (uint16_t{bytes[7]} << 8) | uint16_t{bytes[6]};

        if (ms_word & exponent_mask != exponent_mask) {
            return datum_type::FloatingPoint;
        }

        // Get the type, lower 4 bits + the sign bit.
        let type = static_cast<uint8_t>((ms_word & type_mask) | (ms_word >> 11));
        return to_datum_type(type);
    }


};

}
