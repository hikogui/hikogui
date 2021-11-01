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
        return {narrow_cast<float>(width) / page::size, narrow_cast<float>(height) / page::size};
    }

    /** Get the page size, for the given page index.
     *
     * @return For a full page {page::size, page::size}, for partial pages at the right and top edge smaller.
     */
    [[nodiscard]] constexpr extent2 page_size(size_t page_index) const noexcept
    {
        constexpr auto page_size = f32x4{page::size, page::size};

        ttlet page_width = (width + page::size - 1) / page::size;
        ttlet image_size = f32x4{width, height};
        ttlet page_xy = f32x4{page_index % page_width, page_index / page_width};

        return extent2{min(image_size - page_xy * page_size, page_size)};
    }

    /** Upload image to atlas.
     */
    void upload(pixel_map<sfloat_rgba16> const &image) noexcept;
};

} // namespace tt::pipeline_image
