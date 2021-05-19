// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_channel_mapping.hpp"
#include <windows.h>
#include <ks.h>
#include <ksmedia.h>

namespace tt {

[[nodiscard]] audio_channel_mapping audio_channel_mapping_from_win32(uint32_t from) noexcept
{
    auto r = audio_channel_mapping{0};

    if (from & SPEAKER_FRONT_LEFT) {
        r |= audio_channel_mapping::front_left;
    }
    if (from & SPEAKER_FRONT_RIGHT) {
        r |= audio_channel_mapping::front_right;
    }
    if (from & SPEAKER_FRONT_CENTER) {
        r |= audio_channel_mapping::front_center;
    }
    if (from & SPEAKER_LOW_FREQUENCY) {
        r |= audio_channel_mapping::low_frequency;
    }
    if (from & SPEAKER_BACK_LEFT) {
        r |= audio_channel_mapping::back_left;
    }
    if (from & SPEAKER_BACK_RIGHT) {
        r |= audio_channel_mapping::back_right;
    }
    if (from & SPEAKER_FRONT_LEFT_OF_CENTER) {
        r |= audio_channel_mapping::front_left_of_center;
    }
    if (from & SPEAKER_FRONT_RIGHT_OF_CENTER) {
        r |= audio_channel_mapping::front_right_of_center;
    }
    if (from & SPEAKER_BACK_CENTER) {
        r |= audio_channel_mapping::back_center;
    }
    if (from & SPEAKER_SIDE_LEFT) {
        r |= audio_channel_mapping::side_left;
    }
    if (from & SPEAKER_SIDE_RIGHT) {
        r |= audio_channel_mapping::side_right;
    }
    if (from & SPEAKER_TOP_CENTER) {
        r |= audio_channel_mapping::top_center;
    }
    if (from & SPEAKER_TOP_FRONT_LEFT) {
        r |= audio_channel_mapping::top_front_left;
    }
    if (from & SPEAKER_TOP_FRONT_CENTER) {
        r |= audio_channel_mapping::top_front_center;
    }
    if (from & SPEAKER_TOP_FRONT_RIGHT) {
        r |= audio_channel_mapping::top_front_right;
    }
    if (from & SPEAKER_TOP_BACK_LEFT) {
        r |= audio_channel_mapping::top_back_left;
    }
    if (from & SPEAKER_TOP_BACK_CENTER) {
        r |= audio_channel_mapping::top_back_center;
    }
    if (from & SPEAKER_TOP_BACK_RIGHT) {
        r |= audio_channel_mapping::top_back_right;
    }

    return r;
}


[[nodiscard]] uint32_t audio_channel_mapping_to_win32(audio_channel_mapping from) noexcept
{
    auto r = uint32_t{0};

    if (to_bool(from & audio_channel_mapping::front_left)) {
        r |= SPEAKER_FRONT_LEFT;
    }
    if (to_bool(from & audio_channel_mapping::front_right)) {
        r |= SPEAKER_FRONT_RIGHT;
    }
    if (to_bool(from & audio_channel_mapping::front_center)) {
        r |= SPEAKER_FRONT_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::low_frequency)) {
        r |= SPEAKER_LOW_FREQUENCY;
    }
    if (to_bool(from & audio_channel_mapping::back_left)) {
        r |= SPEAKER_BACK_LEFT;
    }
    if (to_bool(from & audio_channel_mapping::back_right)) {
        r |= SPEAKER_BACK_RIGHT;
    }
    if (to_bool(from & audio_channel_mapping::front_left_of_center)) {
        r |= SPEAKER_FRONT_LEFT_OF_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::front_right_of_center)) {
        r |= SPEAKER_FRONT_RIGHT_OF_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::back_center)) {
        r |= SPEAKER_BACK_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::side_left)) {
        r |= SPEAKER_SIDE_LEFT;
    }
    if (to_bool(from & audio_channel_mapping::side_right)) {
        r |= SPEAKER_SIDE_RIGHT;
    }
    if (to_bool(from & audio_channel_mapping::top_center)) {
        r |= SPEAKER_TOP_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::top_front_left)) {
        r |= SPEAKER_TOP_FRONT_LEFT;
    }
    if (to_bool(from & audio_channel_mapping::top_front_center)) {
        r |= SPEAKER_TOP_FRONT_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::top_front_right)) {
        r |= SPEAKER_TOP_FRONT_RIGHT;
    }
    if (to_bool(from & audio_channel_mapping::top_back_left)) {
        r |= SPEAKER_TOP_BACK_LEFT;
    }
    if (to_bool(from & audio_channel_mapping::top_back_center)) {
        r |= SPEAKER_TOP_BACK_CENTER;
    }
    if (to_bool(from & audio_channel_mapping::top_back_right)) {
        r |= SPEAKER_TOP_BACK_RIGHT;
    }

    return r;
}


}
