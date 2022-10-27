// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "paged_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_vertex.hpp"
#include "gfx_device_vulkan.hpp"
#include "gfx_surface_vulkan.hpp"
#include "../log.hpp"
#include "../utility.hpp"
#include "../cast.hpp"
#include "../geometry/translate.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/extent.hpp"
#include "../codec/png.hpp"

namespace hi::inline v1 {

paged_image::paged_image(gfx_surface const *surface, std::size_t width, std::size_t height) noexcept :
    device(nullptr), width(width), height(height), pages()
{
    if (surface == nullptr) {
        // During initialization of a widget, the window may not have a surface yet.
        // As it needs to determine the size of the surface based on the size of the containing widgets.
        // Return an empty image.
        return;
    }

    // Like before the surface may not be assigned to a device either.
    // In that case also return an empty image.
    hilet lock = std::scoped_lock(gfx_system_mutex);
    if ((this->device = surface->device()) != nullptr) {
        hilet &vulkan_device = down_cast<gfx_device_vulkan &>(*device);

        hilet[num_columns, num_rows] = size_in_int_pages();
        this->pages = vulkan_device.image_pipeline->allocate_pages(num_columns * num_rows);
    }
}

paged_image::paged_image(gfx_surface const *surface, pixel_map<sfloat_rgba16> const &image) noexcept :
    paged_image(surface, narrow_cast<std::size_t>(image.width()), narrow_cast<std::size_t>(image.height()))
{
    if (this->device) {
        hilet lock = std::scoped_lock(gfx_system_mutex);
        this->upload(image);
    }
}

paged_image::paged_image(gfx_surface const *surface, png const &image) noexcept :
    paged_image(surface, narrow_cast<std::size_t>(image.width()), narrow_cast<std::size_t>(image.height()))
{
    if (this->device) {
        hilet lock = std::scoped_lock(gfx_system_mutex);
        this->upload(image);
    }
}

paged_image::paged_image(paged_image &&other) noexcept :
    state(other.state.exchange(state_type::uninitialized)),
    device(std::exchange(other.device, nullptr)),
    width(other.width),
    height(other.height),
    pages(std::move(other.pages))
{
}

paged_image &paged_image::operator=(paged_image &&other) noexcept
{
    hi_return_on_self_assignment(other);

    // If the old image had pages, free them.
    if (hilet vulkan_device = down_cast<gfx_device_vulkan *>(device)) {
        vulkan_device->image_pipeline->free_pages(pages);
    }

    state = other.state.exchange(state_type::uninitialized);
    device = std::exchange(other.device, nullptr);
    width = other.width;
    height = other.height;
    pages = std::move(other.pages);
    return *this;
}

paged_image::~paged_image()
{
    if (hilet vulkan_device = down_cast<gfx_device_vulkan *>(device)) {
        vulkan_device->image_pipeline->free_pages(pages);
    }
}

void paged_image::upload(png const &image) noexcept
{
    hi_assert(image.width() == width and image.height() == height);

    if (hilet vulkan_device = down_cast<gfx_device_vulkan *>(device)) {
        hilet lock = std::scoped_lock(gfx_system_mutex);

        state = state_type::drawing;

        auto staging_image = vulkan_device->image_pipeline->get_staging_pixel_map(image.width(), image.height());
        image.decode_image(staging_image);
        vulkan_device->image_pipeline->update_atlas_with_staging_pixel_map(*this);

        state = state_type::uploaded;
    }
}

void paged_image::upload(pixel_map<sfloat_rgba16> const &image) noexcept
{
    hi_assert(image.width() == width and image.height() == height);

    if (hilet vulkan_device = down_cast<gfx_device_vulkan *>(device)) {
        hilet lock = std::scoped_lock(gfx_system_mutex);

        state = state_type::drawing;

        auto staging_image = vulkan_device->image_pipeline->get_staging_pixel_map(image.width(), image.height());
        copy(image, staging_image);
        vulkan_device->image_pipeline->update_atlas_with_staging_pixel_map(*this);

        state = state_type::uploaded;
    }
}

} // namespace hi::inline v1
