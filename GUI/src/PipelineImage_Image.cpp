// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineImage_ImageLocation.hpp"
#include "TTauri/GUI/PipelineImage_Vertex.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/iaarect.hpp"

namespace TTauri::GUI::PipelineImage {


Image::~Image()
{
    parent->returnPages(pages);
}

iaarect Image::indexToRect(int const pageIndex) const noexcept
{
    let pageWH = ivec{Page::width, Page::height};

    let p1 = ivec::point(
        pageIndex % pageExtent.x(),
        pageIndex / pageExtent.x()
    ) * pageWH;

    // Limit the rectangle to the size of the image.
    let p2 = min(p1 + pageWH, extent);

    return iaarect::p1p2(p1, p2);
}

static std::tuple<vec, vec, bool>calculatePosition(int x, int y, int width, int height, mat transform, aarect clippingRectangle)
{
    auto p = transform * vec::point(x, y);
    return {p, vec{width, height}, clippingRectangle.contains(p)};
}

void Image::calculateVertexPositions(mat transform, aarect clippingRectangle)
{
    tmpVertexPositions.clear();

    let restWidth = extent.x() % Page::width;
    let restHeight = extent.y() % Page::height;
    let lastWidth = restWidth ? restWidth : Page::width;
    let lastHeight = restHeight ? restHeight : Page::height;

    for (int y = 0; y < extent.y(); y += Page::height) {
        for (int x = 0; x < extent.x(); x += Page::width) {
            tmpVertexPositions.push_back(calculatePosition(x, y, Page::width, Page::height, transform, clippingRectangle));
        }
        tmpVertexPositions.push_back(calculatePosition(extent.x(), y, lastWidth, Page::height, transform, clippingRectangle));
    }

    int const y = extent.y();
    for (int x = 0; x < extent.x(); x += Page::width) {
        tmpVertexPositions.push_back(calculatePosition(x, y, Page::width, lastHeight, transform, clippingRectangle));
    }
    tmpVertexPositions.push_back(calculatePosition(extent.x(), y, lastWidth, lastHeight, transform, clippingRectangle));
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
void Image::placePageVertices(vspan<Vertex> &vertices, int const index, aarect clippingRectangle) const {
    let page = pages.at(index);

    if (page.isFullyTransparent()) {
        // Hole in the image does not need to be rendered.
        return;
    }

    let vertexY = index / pageExtent.x();
    let vertexX = index % pageExtent.x();

    let vertexStride = pageExtent.x() + 1;
    let vertexIndex = vertexY * vertexStride + vertexX;

    // Point, Extent, Inside
    let [p1, e1, i1] = tmpVertexPositions[vertexIndex];
    let [p2, e2, i2] = tmpVertexPositions[vertexIndex + 1];
    let [p3, e3, i3] = tmpVertexPositions[vertexIndex + vertexStride];
    let [p4, e4, i4] = tmpVertexPositions[vertexIndex + vertexStride + 1];

    if (!(i1 || i2 || i3 || i4)) {
        // Clipped page.
        return;
    }

    let atlasPosition = DeviceShared::getAtlasPositionFromPage(page);
    let atlasRect = mat::T(vec{atlasPosition.xyz0()}) * aarect{vec{}, e4};

    vertices.emplace_back(p1, atlasRect.corner<0>(), clippingRectangle);
    vertices.emplace_back(p2, atlasRect.corner<1>(), clippingRectangle);
    vertices.emplace_back(p3, atlasRect.corner<2>(), clippingRectangle);
    vertices.emplace_back(p4, atlasRect.corner<3>(), clippingRectangle);
}

/*! Place vertices for this image.
* An image is build out of atlas pages, that need to be individual rendered.
*
* \param position The position (x, y) from the left-top of the window in pixels. Z equals depth.
* \param origin The origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
*/
void Image::placeVertices(vspan<Vertex> &vertices, mat transform, aarect clippingRectangle)
{
    calculateVertexPositions(transform, clippingRectangle);

    for (int index = 0; index < to_signed(pages.size()); index++) {
        placePageVertices(vertices, index, clippingRectangle);
    }
}

}
