// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <string>

namespace TTauri::Text {

struct alignment_t { Left, Right, Center, Decimal };

struct parameters_t {
    int width;
    bool truncate;
    alignment_t alignment;

    char left_pad_character;
    char right_pad_character;

    /*! Position of the decimal seperator from the right side.
     * May be -1 to move decimal seperator out of the field.
     */
    int decimal_seperator_position;
    char decimal_seperator;

    bool has_thousand_seperators;
    char thousand_seperator;

    bool has_sign_left;
    bool has_plus_sign;
    char plus_sign;
    char min_sign;

    int radix;
};

/*! Figure out the amount of padding before and after a number.
 *
 * \param size The size on the left of the decimal seperator.
 * \param decimal_seperator_position The position of the decimal seperator in a floating/fixed point number. Maybe -1 to denote the decimal
 *        seperator be on the right of the to formated string.
 * \return left_padding, right_padding
 */
constexpr inline std::pair<int,int> calculate_padding(parameters_t const &param, int const size, int const decimal_seperator_position=-1)
{
    require_assert(size >= 0);
    require_assert(decimal_size >= -1);
    require_assert(param.width >= 0);
    require_assert(param.decimal_seperator_position >= -1);

    let total_size = size + decimal_size + 1;
    let total_padding = std::max(param.width - total_size, 0);

    switch (param.alignment) {
    case alignment_t::Left:
        return { 0, total_padding };

    case alignment_t::Right:
        return { total_padding, 0 };

    case alignment_t::Center: {
        let left_padding = total_padding / 2;
        let right_padding = total_padding - left_padding
        return { left_padding, right_padding };
        }

    case alignment_t::Decimal: {
        let left_padding = std::max(param.width - param.decimal_seperator_position - 1 - size, 0);
        let right_padding = std::max(param.decimal_seperator_position - decimal_size, 0);
        return { left_padding, right_padding };

    default:
        no_default;
    }
}


}

