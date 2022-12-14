// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "speaker_mapping_win32.hpp"
#include "../check.hpp"

namespace hi::inline v1 {

[[nodiscard]] speaker_mapping speaker_mapping_from_win32(DWORD from)
{
    auto r = speaker_mapping{0};

    constexpr DWORD valid_mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
        SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER |
        SPEAKER_BACK_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_TOP_CENTER | SPEAKER_TOP_FRONT_LEFT |
        SPEAKER_TOP_FRONT_CENTER | SPEAKER_TOP_FRONT_RIGHT | SPEAKER_TOP_BACK_LEFT | SPEAKER_TOP_BACK_CENTER |
        SPEAKER_TOP_BACK_RIGHT;

    hi_parse_check((from & ~valid_mask) == 0, "Unknown speaker locations");

    if (from & SPEAKER_FRONT_LEFT) {
        r |= speaker_mapping::front_left;
    }
    if (from & SPEAKER_FRONT_RIGHT) {
        r |= speaker_mapping::front_right;
    }
    if (from & SPEAKER_FRONT_CENTER) {
        r |= speaker_mapping::front_center;
    }
    if (from & SPEAKER_LOW_FREQUENCY) {
        r |= speaker_mapping::low_frequency;
    }
    if (from & SPEAKER_BACK_LEFT) {
        r |= speaker_mapping::back_left;
    }
    if (from & SPEAKER_BACK_RIGHT) {
        r |= speaker_mapping::back_right;
    }
    if (from & SPEAKER_FRONT_LEFT_OF_CENTER) {
        r |= speaker_mapping::front_left_of_center;
    }
    if (from & SPEAKER_FRONT_RIGHT_OF_CENTER) {
        r |= speaker_mapping::front_right_of_center;
    }
    if (from & SPEAKER_BACK_CENTER) {
        r |= speaker_mapping::back_center;
    }
    if (from & SPEAKER_SIDE_LEFT) {
        r |= speaker_mapping::side_left;
    }
    if (from & SPEAKER_SIDE_RIGHT) {
        r |= speaker_mapping::side_right;
    }
    if (from & SPEAKER_TOP_CENTER) {
        r |= speaker_mapping::top_center;
    }
    if (from & SPEAKER_TOP_FRONT_LEFT) {
        r |= speaker_mapping::top_front_left;
    }
    if (from & SPEAKER_TOP_FRONT_CENTER) {
        r |= speaker_mapping::top_front_center;
    }
    if (from & SPEAKER_TOP_FRONT_RIGHT) {
        r |= speaker_mapping::top_front_right;
    }
    if (from & SPEAKER_TOP_BACK_LEFT) {
        r |= speaker_mapping::top_back_left;
    }
    if (from & SPEAKER_TOP_BACK_CENTER) {
        r |= speaker_mapping::top_back_center;
    }
    if (from & SPEAKER_TOP_BACK_RIGHT) {
        r |= speaker_mapping::top_back_right;
    }

    return r;
}

[[nodiscard]] DWORD speaker_mapping_to_win32(speaker_mapping from) noexcept
{
    auto r = DWORD{0};

    if (to_bool(from & speaker_mapping::front_left)) {
        r |= SPEAKER_FRONT_LEFT;
    }
    if (to_bool(from & speaker_mapping::front_right)) {
        r |= SPEAKER_FRONT_RIGHT;
    }
    if (to_bool(from & speaker_mapping::front_center)) {
        r |= SPEAKER_FRONT_CENTER;
    }
    if (to_bool(from & speaker_mapping::low_frequency)) {
        r |= SPEAKER_LOW_FREQUENCY;
    }
    if (to_bool(from & speaker_mapping::back_left)) {
        r |= SPEAKER_BACK_LEFT;
    }
    if (to_bool(from & speaker_mapping::back_right)) {
        r |= SPEAKER_BACK_RIGHT;
    }
    if (to_bool(from & speaker_mapping::front_left_of_center)) {
        r |= SPEAKER_FRONT_LEFT_OF_CENTER;
    }
    if (to_bool(from & speaker_mapping::front_right_of_center)) {
        r |= SPEAKER_FRONT_RIGHT_OF_CENTER;
    }
    if (to_bool(from & speaker_mapping::back_center)) {
        r |= SPEAKER_BACK_CENTER;
    }
    if (to_bool(from & speaker_mapping::side_left)) {
        r |= SPEAKER_SIDE_LEFT;
    }
    if (to_bool(from & speaker_mapping::side_right)) {
        r |= SPEAKER_SIDE_RIGHT;
    }
    if (to_bool(from & speaker_mapping::top_center)) {
        r |= SPEAKER_TOP_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_front_left)) {
        r |= SPEAKER_TOP_FRONT_LEFT;
    }
    if (to_bool(from & speaker_mapping::top_front_center)) {
        r |= SPEAKER_TOP_FRONT_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_front_right)) {
        r |= SPEAKER_TOP_FRONT_RIGHT;
    }
    if (to_bool(from & speaker_mapping::top_back_left)) {
        r |= SPEAKER_TOP_BACK_LEFT;
    }
    if (to_bool(from & speaker_mapping::top_back_center)) {
        r |= SPEAKER_TOP_BACK_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_back_right)) {
        r |= SPEAKER_TOP_BACK_RIGHT;
    }

    return r;
}

} // namespace hi::inline v1
