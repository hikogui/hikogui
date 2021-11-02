// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_image_location.hpp"
#include "pipeline_image_vertex.hpp"
#include "gfx_device_vulkan.hpp"
#include "gfx_surface_vulkan.hpp"
#include "../log.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../geometry/translate.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/extent.hpp"

namespace tt::pipeline_image {

image::image(gfx_surface const *surface, size_t width, size_t height) noexcept :
    parent(nullptr), width(width), height(height), pages()
{
    if (surface == nullptr) {
        // During initialization of a widget, the window may not have a surface yet.
        // As it needs to determine the size of the surface based on the size of the containing widgets.
        // Return an empty image.
        return;
    }

    // Like before the surface may not be assigned to a device either.
    // In that case also return an empty image.
    ttlet lock = std::scoped_lock(gfx_system_mutex);
    if (ttlet device = surface->device()) {
        ttlet &vulkan_device = narrow_cast<gfx_device_vulkan &>(*device);
        this->parent = vulkan_device.imagePipeline.get();
        tt_axiom(this->parent);

        ttlet[num_columns, num_rows] = size_in_int_pages();
        this->pages = this->parent->allocate_pages(num_columns * num_rows);
    }
}

image::image(gfx_surface const *surface, pixel_map<sfloat_rgba16> const &pixmap) noexcept :
    image(surface, narrow_cast<size_t>(pixmap.width()), narrow_cast<size_t>(pixmap.height()))
{
    if (parent) {
        ttlet lock = std::scoped_lock(gfx_system_mutex);
        this->upload(pixmap);
    }
}

image::image(image &&other) noexcept :
    state(other.state.exchange(state_type::uninitialized)),
    parent(std::exchange(other.parent, nullptr)),
    width(other.width),
    height(other.height),
    pages(std::move(other.pages))
{
}

image &image::operator=(image &&other) noexcept
{
    tt_return_on_self_assignment(other);

    // If the old image had pages, free them.
    if (parent) {
        parent->free_pages(pages);
    }

    state = other.state.exchange(state_type::uninitialized);
    parent = std::exchange(other.parent, nullptr);
    width = other.width;
    height = other.height;
    pages = std::move(other.pages);
    return *this;
}

image::~image()
{
    if (parent) {
        parent->free_pages(pages);
    }
}

void image::upload(pixel_map<sfloat_rgba16> const &image) noexcept
{
    tt_axiom(parent);
    tt_axiom(image.width() == narrow_cast<ssize_t>(width) and image.height() == narrow_cast<ssize_t>(height));

    ttlet lock = std::scoped_lock(gfx_system_mutex);

    state = state_type::drawing;

    auto stagingimage = parent->get_staging_pixel_map(image.width(), image.height());
    copy(image, stagingimage);
    parent->update_atlas_with_staging_pixel_map(*this);

    state = state_type::uploaded;
}

} // namespace tt::pipeline_image
