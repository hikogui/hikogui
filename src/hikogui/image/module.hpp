// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pixmap.hpp"
#include "pixmap_view.hpp"
#include "sdf_r8.hpp"
#include "sfloat_rg32.hpp"
#include "sfloat_rgb32.hpp"
#include "sfloat_rgba16.hpp"
#include "sfloat_rgba32.hpp"
#include "sfloat_rgba32x4.hpp"
#include "sint_abgr8_pack.hpp"
#include "snorm_r8.hpp"
#include "srgb_abgr8_pack.hpp"
#include "uint_abgr8_pack.hpp"
#include "unorm_a2bgr10_pack.hpp"

namespace hi {
inline namespace v1 {
/**
\defgroup image Pixel-map image types.

Pixel formats
-------------

  Prefix     | Description
 :---------- |:----------------------------------------------------------------------
  sfloat_    | Maps a floating-point number identifically.
  snorm_     | Maps the maximums of an signed-integer linearily to -1.0 - 1.0.
  unorm_     | Maps the maximums of an unsigned-integer linearily to 0.0 - 1.0.
  int_       | Maps an signed-integer identicially.
  uint_      | Maps an unsigned-integer identically.
  srgb_      | Maps a unsigned-integer from sRGB values to linear values in the range 0.0 - 1.0. Alpha is mapped like unorm.




  HikoGUI type         | Vulkan format                      | Comment
 :-------------------- |:---------------------------------- |:--------------------------------
  `sfloat_rg32`        | VK_FORMAT_R32G32_SFLOAT            |
  `sfloat_rgb32`       | VK_FORMAT_R32G32B32_SFLOAT         |
  `sfloat_rgba16`      | VK_FORMAT_R16G16B16A16_SFLOAT      | The default HikoGUI image format.
  `sfloat_rgba32`      | VK_FORMAT_R32G32B32A32_SFLOAT      |
  `sfloat_rgba32x4`    | 4 x VK_FORMAT_R32G32B32A32_SFLOAT  | Used for 4x4 matrix
  `snorm_r8`           | VK_FORMAT_R8_SNORM                 |
  `int_abgr8_pack`     | VK_FORMAT_A8B8G8R8_SINT_PACK32     |
  `uint_abgr8_pack`    | VK_FORMAT_A8B8G8R8_UINT_PACK32     |
  `srgb_abgr8_pack`    | VK_FORMAT_A8B8G8R8_SRGB_PACK32     |
  `unorm_a2bgr10_pack` | VK_FORMAT_A2R10G10B10_UNORM_PACK32 |
  `sdf_r8`             | VK_FORMAT_R8_SNORM                 | To store signed-distance-field.



*/
}}

