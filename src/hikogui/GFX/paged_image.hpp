// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/extent.hpp"
#include <cstdlib>
#include <span>
#include <atomic>
#include <string>
#include <vector>

namespace hi::inline v1 {

template<typename T>
class pixel_map;
class png;
class sfloat_rgba16;
class gfx_surface;
class gfx_device;

/** This is a image that is uploaded into the texture atlas.
 */
struct paged_image {
    enum class state_type { uninitialized, drawing, uploaded };

    static constexpr std::size_t page_size = 62; // 64x64 including a 1 pixel border.

    mutable std::atomic<state_type> state = state_type::uninitialized;
    gfx_device *device = nullptr;
    std::size_t width;
    std::size_t height;
    std::vector<std::size_t> pages;

    ~paged_image();
    constexpr paged_image() noexcept = default;
    paged_image(paged_image &&other) noexcept;
    paged_image &operator=(paged_image &&other) noexcept;
    paged_image(paged_image const &other) = delete;
    paged_image &operator=(paged_image const &other) = delete;

    paged_image(gfx_surface const *surface, std::size_t width, std::size_t height) noexcept;
    paged_image(gfx_surface const *surface, pixel_map<sfloat_rgba16> const &image) noexcept;
    paged_image(gfx_surface const *surface, png const &image) noexcept;

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return device != nullptr;
    }

    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return extent2{narrow_cast<float>(width), narrow_cast<float>(height)};
    }

    [[nodiscard]] constexpr std::pair<std::size_t, std::size_t> size_in_int_pages() const noexcept
    {
        hilet num_columns = (width + page_size - 1) / page_size;
        hilet num_rows = (height + page_size - 1) / page_size;
        return {num_columns, num_rows};
    }

    [[nodiscard]] constexpr extent2 size_in_float_pages() const noexcept
    {
        constexpr auto page_size_ = f32x4{narrow_cast<float>(page_size), narrow_cast<float>(page_size)};
        auto size = f32x4{i32x4{narrow_cast<int32_t>(width), narrow_cast<int32_t>(height), 1, 1}};
        return extent2{size / page_size_};
    }

    /** Upload image to atlas.
     */
    void upload(pixel_map<sfloat_rgba16> const &image) noexcept;

    /** Upload image to atlas.
     */
    void upload(png const &image) noexcept;
};

} // namespace hi::inline v1
