// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "string.hpp"

namespace TTauri::Text {

inline std::string to_string(uint64_t value, int radix, bool has_thousands, char thousand_seperator)
{
    required_assert(radix >= 2 && radix <= 16);

    std::string r;
    r.reserve(21); // maximum for decimal without thousands.
    let front_i = r.begin();

    int digit_count = 0;
    do {
        let digit = value % radix;
        value /= radix;
        
        if (has_thousands)
            if ((digit_count > 0) && (digit_count % 3 == 0)) {
                r.insert(r.begin(), thousand_seperator);
            }
            digit_count++;
        }

        switch (digit) {
        case 0: r.insert(front_i, '0'); break;
        case 1: r.insert(front_i, '1'); break;
        case 2: r.insert(front_i, '2'); break;
        case 3: r.insert(front_i, '3'); break;
        case 4: r.insert(front_i, '4'); break;
        case 5: r.insert(front_i, '5'); break;
        case 6: r.insert(front_i, '6'); break;
        case 7: r.insert(front_i, '7'); break;
        case 8: r.insert(front_i, '8'); break;
        case 9: r.insert(front_i, '9'); break;
        case 10: r.insert(front_i, 'a'); break;
        case 11: r.insert(front_i, 'b'); break;
        case 12: r.insert(front_i, 'c'); break;
        case 13: r.insert(front_i, 'd'); break;
        case 14: r.insert(front_i, 'e'); break;
        case 15: r.insert(front_i, 'f'); break;
        default: no_default;
        }
    } while (value);

    return r;
}

inline std::string to_string(uint64_t value, parameters_t const &param)
{
    let digits = to_string(value, param.has_thousand_seperators, param.thousand_seperator);

    std::string r;
    r.reserve(std::max(param.width, digits.size()));

    return to_string(digits, param);
}
   
inline std::string to_string(uint32_t value, parameters_t const &param) { return to_string(static_cast<uint64_t>(value), param); }
inline std::string to_string(uint16_t value, parameters_t const &param) { return to_string(static_cast<uint64_t>(value), param); }
inline std::string to_string(uint8_t value, parameters_t const &param) { return to_string(static_cast<uint64_t>(value), param); }


inline std::string to_string(int64_t value, parameters_t const &param)
{
    if (value >= 0) {
        return to_string(static_cast<uint64_t>(value), param);    
    }

    let positive_value = (value == std::numeric_limits<int64>::min) ?
        static_cast<uint64_t>(std::numeric_limits<int64>::min) + 1 :
        static_cast<uint64_t>(-value);

    let digits = to_string(positive_value, param.has_thousand_seperators, param.thousand_seperator);

    std::string r;
    r.reserve(std::max(param.width, digits.size() + 1));

    return to_string(digits, param, true);
}

inline std::string to_string(int32_t value, parameters_t const &param) { return to_string(static_cast<int64_t>(value), param); }
inline std::string to_string(int16_t value, parameters_t const &param) { return to_string(static_cast<int64_t>(value), param); }
inline std::string to_string(int8_t value, parameters_t const &param) { return to_string(static_cast<int64_t>(value), param); }

}
