// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/extent.hpp"
#include <cstdlib>
#include <span>
#include <atomic>
#include <string>
#include <vector>

namespace tt {

template<typename T>
class pixel_map;
class sfloat_rgba16;
inline namespace v1 {
class gfx_surface;
class gfx_device;

/** This is a image that is uploaded into the texture atlas.
 */
struct paged_image {
    enum class state_type { uninitialized, drawing, uploaded };

    static constexpr size_t page_size = 64;
    static constexpr size_t page_border = 1;

    mutable std::atomic<state_type> state = state_type::uninitialized;
    gfx_device *device = nullptr;
    size_t width;
    size_t height;
    std::vector<size_t> pages;

    ~paged_image();
    constexpr paged_image() noexcept = default;
    paged_image(paged_image &&other) noexcept;
    paged_image &operator=(paged_image &&other) noexcept;
    paged_image(paged_image const &other) = delete;
    paged_image &operator=(paged_image const &other) = delete;

    paged_image(gfx_surface const *surface, size_t width, size_t height) noexcept;
    paged_image(gfx_surface const *surface, pixel_map<sfloat_rgba16> const &pixmap) noexcept;

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return device != nullptr;
    }

    [[nodiscard]] constexpr std::pair<size_t, size_t> size_in_int_pages() const noexcept
    {
        ttlet num_columns = (width + page_size - 1) / page_size;
        ttlet num_rows = (height + page_size - 1) / page_size;
        return {num_columns, num_rows};
    }

    [[nodiscard]] constexpr extent2 size_in_float_pages() const noexcept
    {
        constexpr auto page_size_ = f32x4{narrow_cast<float>(page_size), narrow_cast<float>(page_size)};
        auto size = f32x4{i32x4{narrow_cast<int32_t>(width),narrow_cast<int32_t>(height), 1, 1}};
        return extent2{size / page_size_};
    }

    /** Upload image to atlas.
     */
    void upload(pixel_map<sfloat_rgba16> const &image) noexcept;
};

}
} // namespace tt::pipeline_image
