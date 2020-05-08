// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/iaarect.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/mat.hpp"
#include <gsl/gsl>
#include <atomic>
#include <string>

namespace TTauri::GUI::PipelineImage {

struct Page;
struct Vertex;
struct ImageLocation;
struct DeviceShared;

/** This is a image that is uploaded into the texture atlas.
 */
struct Image {
    enum class State { Uninitialized, Drawing, Uploaded };

    mutable std::atomic<State> state = State::Uninitialized;

    DeviceShared *parent;

    std::string key;

    /** The size of the image in pixels.
     */
    ivec extent;

    /** This size of the image in pages.
     * This value is used to calculate how many quads need to be drawn.
     */
    ivec pageExtent;

    std::vector<Page> pages;

    Image(DeviceShared *parent, std::string key, ivec extent, ivec pageExtent, std::vector<Page> pages) noexcept :
        parent(parent),
        key(std::move(key)),
        extent(std::move(extent)),
        pageExtent(std::move(pageExtent)),
        pages(std::move(pages)) {}

    ~Image();

    Image() = delete;
    Image(Image const &other) = delete;
    Image(Image &&other) = delete;
    Image &operator=(Image const &other) = delete;
    Image &operator=(Image &&other) = delete;

    /** Find the image coordinates of a page in the image.
     * @param pageIndex Index in the pages-vector.
     * @return The rectangle within the image representing a quad to be drawn.
     *         This rectangle is already size-adjusted for the quads on the edge.
     */
    iaarect indexToRect(int const pageIndex) const noexcept;

    /*! Place vertices for this image.
     * An image is build out of atlas pages, that need to be individual rendered.
     * A page with the value std::numeric_limits<uint16_t>::max() is not rendered.
     *     */
    void placeVertices(vspan<Vertex> &vertices, mat transform, aarect clippingRectangle);

private:
    //! Temporary memory used for pre calculating vertices.
    std::vector<std::tuple<vec, vec, bool>> tmpVertexPositions;

    void calculateVertexPositions(mat transform, aarect clippingRectangle);

    void placePageVertices(vspan<Vertex> &vertices, int const index, aarect clippingRectangle) const;

};

}
