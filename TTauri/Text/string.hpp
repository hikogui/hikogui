// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "format.hpp"
#include <string>

namespace TTauri::Text {

/*! Format a string.
 *
 * This function will also handle formatting of numbers, such as adding a sign and zero padding.
 *
 * \param value The string to be formatted.
 * \param negative When true we are trying to format a negative number.
 * \param decimal_seperator_position The position of the decimal seperator in a floating/fixed point number. Maybe -1 to denote the decimal
 *        seperator be on the right of the to formated string.
 */
inline std::string to_string(std::string value, parameter_t const &param, bool negative=false, int decimal_seperator_position=-1)
{
    required_assert(decimal_position >= -1);

    std::string r;
    r.reserve(std::max(param.width, value.size() + 1));
 
    auto size = value.size(); 
    if (negative || param.has_plus_sign) {
        // The sign makes value larger, thus less space for left padding.
        size++;

        let sign = negative ? param.min_sign : param.plus_sign;
        if (param.has_sign_left) {
            // Add the sign before the padding.
            r.push_back(sign);
        } else {
            // Add the sign after padding.
            value.insert(value.begin(), sign);
        }
    }

    let [left_padding, right_padding] = calculate_padding(param, size, decimal_seperator_position);
  
    // Add left padding. 
    for (int i = 0; i < left_paddig; i++) {
        r.push_back(param.left_pad_character);
    }

    r += value;
    
    // Add right padding. 
    for (int i = 0; i < right_paddig; i++) {
        r.push_back(param.right_pad_character);
    }
    
    return r;
}


}

