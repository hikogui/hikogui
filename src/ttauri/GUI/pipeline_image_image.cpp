// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_image_location.hpp"
#include "pipeline_image_vertex.hpp"
#include "../logger.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../geometry/translate.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"

namespace tt::pipeline_image {

Image::Image(Image &&other) noexcept :
    parent(other.parent),
    width_in_px(other.width_in_px),
    height_in_px(other.height_in_px),
    width_in_pages(other.width_in_pages),
    height_in_pages(other.height_in_pages),
    pages(std::move(other.pages))
{
    other.parent = nullptr;
}

Image &Image::operator=(Image &&other) noexcept
{
    if (parent) {
        parent->freePages(pages);
    }

    parent = other.parent;
    width_in_px = other.width_in_px;
    height_in_px = other.height_in_px;
    width_in_pages = other.width_in_pages;
    height_in_pages = other.height_in_pages;
    pages = std::move(other.pages);
    other.parent = nullptr;
    return *this;
}

Image::~Image()
{
    if (parent) {
        parent->freePages(pages);
    }
}

void Image::upload(pixel_map<sfloat_rgba16> const &image) noexcept
{
    tt_axiom(parent);

    state = State::Drawing;

    auto stagingImage = parent->getStagingPixelMap(width_in_px, height_in_px);
    copy(image, stagingImage);
    parent->updateAtlasWithStagingPixelMap(*this);

    state = State::Uploaded;
}

aarectangle Image::index_to_rect(size_t page_index) const noexcept
{
    ttlet left = (page_index % width_in_pages) * Page::width;
    ttlet bottom = (page_index / width_in_pages) * Page::height;
    ttlet right = std::min(left + Page::width, width_in_px);
    ttlet top = std::min(bottom + Page::height, height_in_px);

    ttlet p0 = point2{narrow_cast<float>(left), narrow_cast<float>(bottom)};
    ttlet p3 = point2{narrow_cast<float>(right), narrow_cast<float>(top)};
    return aarectangle{p0, p3};
}

static std::tuple<point3, extent2, bool>
calculatePosition(size_t x, size_t y, size_t width, size_t height, matrix3 transform, aarectangle clippingRectangle)
{
    auto p = transform * point2(narrow_cast<float>(x), narrow_cast<float>(y));
    auto e = extent2{narrow_cast<float>(width), narrow_cast<float>(height)};
    auto i = clippingRectangle.contains(point2{p});
    return {p, e, i};
}

void Image::calculateVertexPositions(matrix3 transform, aarectangle clippingRectangle)
{
    tmpvertexPositions.clear();

    ttlet restWidth = width_in_px % Page::width;
    ttlet restHeight = height_in_px % Page::height;
    ttlet lastWidth = restWidth ? restWidth : Page::width;
    ttlet lastHeight = restHeight ? restHeight : Page::height;

    for (auto y = 0_uz; y < height_in_px; y += Page::height) {
        for (auto x = 0_uz; x < width_in_px; x += Page::width) {
            tmpvertexPositions.push_back(calculatePosition(x, y, Page::width, Page::height, transform, clippingRectangle));
        }
        tmpvertexPositions.push_back(calculatePosition(width_in_px, y, lastWidth, Page::height, transform, clippingRectangle));
    }

    auto y = height_in_px;
    for (auto x = 0_uz; x < width_in_px; x += Page::width) {
        tmpvertexPositions.push_back(calculatePosition(x, y, Page::width, lastHeight, transform, clippingRectangle));
    }
    tmpvertexPositions.push_back(calculatePosition(width_in_px, y, lastWidth, lastHeight, transform, clippingRectangle));
}

/** Places vertices.
 *
 * This is the format of a quad.
 *
 *    2 <-- 3
 *    | \   ^
 *    |  \  |
 *    v   \ |
 *    0 --> 1
 */
void Image::placePageVertices(vspan<vertex> &vertices, size_t index, aarectangle clippingRectangle) const
{
    ttlet page = pages.at(index);

    if (page.isFullyTransparent()) {
        // Hole in the image does not need to be rendered.
        return;
    }

    ttlet vertexY = index / width_in_pages;
    ttlet vertexX = index % width_in_pages;

    ttlet vertexStride = width_in_pages + 1;
    ttlet vertexIndex = vertexY * vertexStride + vertexX;

    // Point, Extent, Inside
    ttlet[p1, e1, i1] = tmpvertexPositions[vertexIndex];
    ttlet[p2, e2, i2] = tmpvertexPositions[vertexIndex + 1];
    ttlet[p3, e3, i3] = tmpvertexPositions[vertexIndex + vertexStride];
    ttlet[p4, e4, i4] = tmpvertexPositions[vertexIndex + vertexStride + 1];

    if (!(i1 || i2 || i3 || i4)) {
        // Clipped page.
        return;
    }

    ttlet atlasPosition = device_shared::getAtlasPositionFromPage(page);
    ttlet atlasPosition_ = point3{
        narrow_cast<float>(atlasPosition.x()), narrow_cast<float>(atlasPosition.y()), narrow_cast<float>(atlasPosition.z())};
    ttlet atlasRect = translate3{atlasPosition_} * aarectangle{e4};

    vertices.emplace_back(p1, get<0>(atlasRect), clippingRectangle);
    vertices.emplace_back(p2, get<1>(atlasRect), clippingRectangle);
    vertices.emplace_back(p3, get<2>(atlasRect), clippingRectangle);
    vertices.emplace_back(p4, get<3>(atlasRect), clippingRectangle);
}

/*! Place vertices for this image.
 * An image is build out of atlas pages, that need to be individual rendered.
 *
 * \param position The position (x, y) from the left-top of the window in pixels. Z equals depth.
 * \param origin The origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in
 * radials.
 */
void Image::place_vertices(vspan<vertex> &vertices, aarectangle clipping_rectangle, matrix3 transform)
{
    calculateVertexPositions(transform, clipping_rectangle);

    for (int index = 0; index < std::ssize(pages); index++) {
        placePageVertices(vertices, index, clipping_rectangle);
    }
}

} // namespace tt::pipeline_image
