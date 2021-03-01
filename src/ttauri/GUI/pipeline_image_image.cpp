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
#include "../numeric_array.hpp"
#include "../aarect.hpp"

namespace tt::pipeline_image {

Image::Image(Image &&other) noexcept :
    parent(other.parent), extent(other.extent), pageExtent(other.pageExtent), pages(std::move(other.pages))
{
    other.parent = nullptr;
}

Image &Image::operator=(Image &&other) noexcept
{
    if (parent) {
        parent->freePages(pages);
    }

    parent = other.parent;
    extent = other.extent;
    pageExtent = other.pageExtent;
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

    auto stagingImage = parent->getStagingPixelMap(extent);
    copy(image, stagingImage);
    parent->updateAtlasWithStagingPixelMap(*this);

    state = State::Uploaded;
}

iaarect Image::indexToRect(int const pageIndex) const noexcept
{
    ttlet pageWH = i32x4{Page::width, Page::height};

    ttlet p0 = i32x4::point(i32x4{pageIndex % pageExtent.x(), pageIndex / pageExtent.x()} * pageWH);

    // Limit the rectangle to the size of the image.
    ttlet p3 = min(p0 + pageWH, i32x4::point(extent));

    return iaarect::p0p3(p0, p3);
}

static std::tuple<f32x4, f32x4, bool>
calculatePosition(int x, int y, int width, int height, matrix3 transform, aarect clippingRectangle)
{
    auto p = transform * f32x4::point(narrow_cast<float>(x), narrow_cast<float>(y));
    return {p, f32x4{narrow_cast<float>(width), narrow_cast<float>(height)}, clippingRectangle.contains(p)};
}

void Image::calculateVertexPositions(matrix3 transform, aarect clippingRectangle)
{
    tmpvertexPositions.clear();

    ttlet restWidth = extent.x() % Page::width;
    ttlet restHeight = extent.y() % Page::height;
    ttlet lastWidth = restWidth ? restWidth : Page::width;
    ttlet lastHeight = restHeight ? restHeight : Page::height;

    for (int y = 0; y < extent.y(); y += Page::height) {
        for (int x = 0; x < extent.x(); x += Page::width) {
            tmpvertexPositions.push_back(calculatePosition(x, y, Page::width, Page::height, transform, clippingRectangle));
        }
        tmpvertexPositions.push_back(calculatePosition(extent.x(), y, lastWidth, Page::height, transform, clippingRectangle));
    }

    int const y = extent.y();
    for (int x = 0; x < extent.x(); x += Page::width) {
        tmpvertexPositions.push_back(calculatePosition(x, y, Page::width, lastHeight, transform, clippingRectangle));
    }
    tmpvertexPositions.push_back(calculatePosition(extent.x(), y, lastWidth, lastHeight, transform, clippingRectangle));
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
void Image::placePageVertices(vspan<vertex> &vertices, int const index, aarect clippingRectangle) const
{
    ttlet page = pages.at(index);

    if (page.isFullyTransparent()) {
        // Hole in the image does not need to be rendered.
        return;
    }

    ttlet vertexY = index / pageExtent.x();
    ttlet vertexX = index % pageExtent.x();

    ttlet vertexStride = pageExtent.x() + 1;
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
    ttlet atlasRect = translate3{f32x4{atlasPosition.xyz0()}} * aarect{e4};

    vertices.emplace_back(p1, atlasRect.corner<0>(), clippingRectangle);
    vertices.emplace_back(p2, atlasRect.corner<1>(), clippingRectangle);
    vertices.emplace_back(p3, atlasRect.corner<2>(), clippingRectangle);
    vertices.emplace_back(p4, atlasRect.corner<3>(), clippingRectangle);
}

/*! Place vertices for this image.
 * An image is build out of atlas pages, that need to be individual rendered.
 *
 * \param position The position (x, y) from the left-top of the window in pixels. Z equals depth.
 * \param origin The origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in
 * radials.
 */
void Image::place_vertices(vspan<vertex> &vertices, aarect clipping_rectangle, matrix3 transform)
{
    calculateVertexPositions(transform, clipping_rectangle);

    for (int index = 0; index < std::ssize(pages); index++) {
        placePageVertices(vertices, index, clipping_rectangle);
    }
}

} // namespace tt::pipeline_image
