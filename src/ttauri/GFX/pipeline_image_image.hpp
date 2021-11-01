// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_image_page.hpp"
#include "../vspan.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include "../memory.hpp"
#include <cstdlib>
#include <span>
#include <atomic>
#include <string>
#include <vector>

namespace tt {

template<typename T>
class pixel_map;
class sfloat_rgba16;
}; // namespace tt

namespace tt::pipeline_image {

struct vertex;
struct ImageLocation;
struct device_shared;

/** This is a image that is uploaded into the texture atlas.
 */
struct image {
    enum class state_type { uninitialized, drawing, uploaded };

    mutable std::atomic<state_type> state = state_type::uninitialized;

    device_shared *parent = nullptr;
    size_t width;
    size_t height;
    std::vector<page> pages;

    constexpr image(device_shared *parent, size_t width, size_t height, std::vector<page> &&pages) noexcept :
        parent(parent), width(width), height(height), pages(std::move(pages))
    {
    }

    ~image();
    constexpr image() noexcept = default;
    image(image &&other) noexcept;
    image &operator=(image &&other) noexcept;
    image(image const &other) = delete;
    image &operator=(image const &other) = delete;

    [[nodiscard]] constexpr std::pair<size_t, size_t> size_in_int_pages() const noexcept
    {
        return {ceil(width, page::size) / page::size, ceil(height, page::size) / page::size};
    }

    [[nodiscard]] constexpr extent2 size_in_float_pages() const noexcept
    {
        constexpr auto page_size = f32x4{narrow_cast<float>(page::size), narrow_cast<float>(page::size)};
        auto size = f32x4{i32x4{narrow_cast<int32_t>(width),narrow_cast<int32_t>(height), 1, 1}};
        return extent2{size / page_size};
    }

    /** Upload image to atlas.
     */
    void upload(pixel_map<sfloat_rgba16> const &image) noexcept;
};

} // namespace tt::pipeline_image
