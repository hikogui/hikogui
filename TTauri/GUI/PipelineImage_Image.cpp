// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineImage_Image.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineImage_ImageLocation.hpp"
#include "PipelineImage_Vertex.hpp"
#include "TTauri/logging.hpp"
#include "TTauri/required.hpp"
#include <glm/gtx/rotate_vector.hpp>

namespace TTauri::GUI::PipelineImage {


Image::~Image()
{
    parent->returnPages(pages);
}

u64rect2 Image::indexToRect(size_t const pageIndex) const
{
    let indexY = pageIndex / pageExtent.x;
    let indexX = pageIndex % pageExtent.x;

    let left = indexX * Page::width;
    let top = indexY * Page::height;
    let right = left + Page::width;
    let bottom = top + Page::height;
    let rightOverflow = right - std::min(right, static_cast<size_t>(extent.x));
    let bottomOverflow = bottom - std::min(bottom, static_cast<size_t>(extent.y));
    let width = Page::width - rightOverflow;
    let height = Page::height - bottomOverflow;

    return {{left, top}, {width, height}};
}

static bool inside(glm::vec2 point, rect2 clip)
{
    return (
        (point.x >= clip.offset.x) &&
        (point.x <= (clip.offset.x + clip.extent.width())) &&
        (point.y >= clip.offset.y) &&
        (point.y <= (clip.offset.y + clip.extent.height()))
    );
}

static bool anyInside(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, rect2 clip)
{
    return inside(p1, clip) || inside(p2, clip) || inside(p3, clip) || inside(p4, clip);
}

static std::tuple<glm::vec2, u64extent2, bool>calculatePosition(size_t x, size_t y, size_t width, size_t height, const ImageLocation &location)
{
    auto p = glm::vec2{x, y};
    p -= location.origin;
    p = p * location.scale;
    p = glm::rotate(p, location.rotation);
    p += location.position;

    return {p, {width, height}, inside(p, location.clippingRectangle)};
}

void Image::calculateVertexPositions(const ImageLocation &location)
{
    tmpVertexPositions.clear();

    let restWidth = extent.width() % Page::width;
    let restHeight = extent.height() % Page::height;
    let lastWidth = restWidth ? restWidth : Page::width;
    let lastHeight = restHeight ? restHeight : Page::height;

    for (size_t y = 0; y < extent.height(); y += Page::height) {
        for (size_t x = 0; x < extent.width(); x += Page::width) {
            tmpVertexPositions.push_back(calculatePosition(x, y, Page::width, Page::height, location));
        }
        tmpVertexPositions.push_back(calculatePosition(extent.width(), y, lastWidth, Page::height, location));
    }

    size_t const y = extent.height();
    for (size_t x = 0; x < extent.width(); x += Page::width) {
        tmpVertexPositions.push_back(calculatePosition(x, y, Page::width, lastHeight, location));
    }
    tmpVertexPositions.push_back(calculatePosition(extent.width(), y, lastWidth, lastHeight, location));
}

void Image::placePageVertices(size_t const index, const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset) const {
    let page = pages.at(index);

    if (page.isFullyTransparent()) {
        // Hole in the image does not need to be rendered.
        return;
    }

    let vertexY = index / pageExtent.width();
    let vertexX = index % pageExtent.width();

    let vertexStride = pageExtent.width() + 1;
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

    vertices[offset++] = {location, p1, atlasPosition};
    vertices[offset++] = {location, p2, {atlasPosition.x + e2.width(), atlasPosition.y, atlasPosition.z}};
    vertices[offset++] = {location, p3, {atlasPosition.x, atlasPosition.y + e3.height(), atlasPosition.z}};
    vertices[offset++] = {location, p4, {atlasPosition.x + e4.width(), atlasPosition.y + e4.height(), atlasPosition.z}};
}

/*! Place vertices for this image.
* An image is build out of atlas pages, that need to be individual rendered.
*
* \param position Position (x, y) from the left-top of the window in pixels. Z equals depth.
* \param origin Origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
*/
void Image::placeVertices(const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset)
{
    calculateVertexPositions(location);

    if (offset + pages.size() * 4 > static_cast<size_t>(vertices.size())) {
        LOG_FATAL("vertices don't fit");
        abort();
    }

    for (size_t index = 0; index < pages.size(); index++) {
        placePageVertices(index, location, vertices, offset);
    }
}

}
