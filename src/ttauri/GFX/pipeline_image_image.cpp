// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_image_location.hpp"
#include "pipeline_image_vertex.hpp"
#include "../log.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../geometry/translate.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/extent.hpp"

namespace tt::pipeline_image {

image::image(image &&other) noexcept :
    parent(std::exchange(other.parent, nullptr)), width(other.width), height(other.height), pages(std::move(other.pages))
{
}

image &image::operator=(image &&other) noexcept
{
    tt_return_on_self_assignment(other);

    // If the old image had pages, free them.
    if (parent) {
        parent->free_pages(pages);
    }

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

    state = state_type::drawing;

    auto stagingimage = parent->get_staging_pixel_map(image.width(), image.height());
    copy(image, stagingimage);
    parent->update_atlas_with_staging_pixel_map(*this);

    state = state_type::uploaded;
}

} // namespace tt::pipeline_image
