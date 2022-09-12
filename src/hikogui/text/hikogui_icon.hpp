// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

enum class hikogui_icon : char32_t {
    MinimizeWindow = 0xf301,
    MaximizeWindowMS = 0xf302,
    RestoreWindowMS = 0xf303,
    CloseWindow = 0xf304,
    RestoreWindowMacOS = 0xf305,
    MaximizeWindowMacOS = 0xf306,

    // Standard surround configurations
    none_0_0 = 0xf3c0,
    mono_1_0 = 0xf3c1,
    mono_1_1 = 0xf3c2,
    stereo_2_0 = 0xf3c3,
    stereo_2_1 = 0xf3c4,
    stereo_3_0 = 0xf3c5,
    stereo_3_1 = 0xf3c6,
    surround_3_0 = 0xf3c7,
    surround_3_1 = 0xf3c8,
    surround_4_0 = 0xf3c9,
    surround_4_1 = 0xf3ca,
    surround_5_0 = 0xf3cb,
    surround_5_1 = 0xf3cc,
    surround_7_0 = 0xf3cd,
    surround_7_1 = 0xf3ce,
    surround_9_0 = 0xf3cf,
    surround_9_1 = 0xf3d0,
    surround_11_0 = 0xf3d1,
    surround_11_1 = 0xf3d2,

    // Surround sound with side speakers instead of left/right back speakers.
    surround_side_5_0 = 0xf3d3,
    surround_side_5_1 = 0xf3d4,
    surround_side_6_0 = 0xf3d5,
    surround_side_6_1 = 0xf3d6,
    surround_side_7_0 = 0xf3d7,
    surround_side_7_1 = 0xf3d8,

    // Surround sound with extra front speakers.
    surround_wide_6_0 = 0xf3d9,
    surround_wide_6_1 = 0xf3da,
    surround_wide_7_0 = 0xf3db,
    surround_wide_7_1 = 0xf3dc,

    // Music configuration
    quad_4_0 = 0xf3dd,
    quad_4_1 = 0xf3de,
    quad_side_4_0 = 0xf3df,
    quad_side_4_1 = 0xf3e0,
    hexagonal_6_0 = 0xf3e1,
    hexagonal_6_1 = 0xf3e2,
    octagonal_8_0 = 0xf3e3,
    octagonal_8_1 = 0xf3e4,

    // Surround sound with extra top speakers.
    surround_atmos_5_1_4 = 0xf3e5,
    surround_atmos_7_1_4 = 0xf3e6,
};

}
