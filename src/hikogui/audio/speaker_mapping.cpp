// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "speaker_mapping.hpp"

namespace hi::inline v1 {

[[nodiscard]] std::string to_string(speaker_mapping rhs) noexcept
{
    auto r = std::string{};

    if (to_bool(rhs & speaker_mapping::front_left)) {
        r += ",fl";
    }
    if (to_bool(rhs & speaker_mapping::front_right)) {
        r += ",fr";
    }
    if (to_bool(rhs & speaker_mapping::front_center)) {
        r += ",fc";
    }
    if (to_bool(rhs & speaker_mapping::low_frequency)) {
        r += ",lfe";
    }
    if (to_bool(rhs & speaker_mapping::back_left)) {
        r += ",bl";
    }
    if (to_bool(rhs & speaker_mapping::back_right)) {
        r += ",br";
    }
    if (to_bool(rhs & speaker_mapping::front_left_of_center)) {
        r += ",flc";
    }
    if (to_bool(rhs & speaker_mapping::front_right_of_center)) {
        r += ",frc";
    }
    if (to_bool(rhs & speaker_mapping::back_center)) {
        r += ",bc";
    }
    if (to_bool(rhs & speaker_mapping::side_left)) {
        r += ",sl";
    }
    if (to_bool(rhs & speaker_mapping::side_right)) {
        r += ",sr";
    }
    if (to_bool(rhs & speaker_mapping::top_center)) {
        r += ",tc";
    }
    if (to_bool(rhs & speaker_mapping::top_front_left)) {
        r += ",tfl";
    }
    if (to_bool(rhs & speaker_mapping::top_front_center)) {
        r += ",tfc";
    }
    if (to_bool(rhs & speaker_mapping::top_front_right)) {
        r += ",tfr";
    }
    if (to_bool(rhs & speaker_mapping::top_back_left)) {
        r += ",tbl";
    }
    if (to_bool(rhs & speaker_mapping::top_back_center)) {
        r += ",tbc";
    }
    if (to_bool(rhs & speaker_mapping::top_back_right)) {
        r += ",tbr";
    }

    if (r.empty()) {
        r += '[';
    } else {
        // replace the first comma.
        r[0] = '[';
    }

    r += ']';
    return r;
}

} // namespace hi::inline v1
