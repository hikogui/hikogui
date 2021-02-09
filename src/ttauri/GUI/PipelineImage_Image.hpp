// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "PipelineImage_Page.hpp"
#include "../vspan.hpp"
#include "../aarect.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../mat.hpp"
#include <span>
#include <atomic>
#include <string>

namespace tt {

template<typename T> class pixel_map;
class R16G16B16A16SFloat;
};

namespace tt::PipelineImage {

struct Vertex;
struct ImageLocation;
struct DeviceShared;

/** This is a image that is uploaded into the texture atlas.
 */
struct Image {
    enum class State { Uninitialized, Drawing, Uploaded };

    mutable std::atomic<State> state = State::Uninitialized;

    DeviceShared *parent;

    /** The size of the image in pixels.
     */
    i32x4 extent;

    /** This size of the image in pages.
     * This value is used to calculate how many quads need to be drawn.
     */
    i32x4 pageExtent;

    std::vector<Page> pages;

    Image() noexcept :
        parent(nullptr), extent(), pageExtent(), pages() {}

    Image(DeviceShared *parent, i32x4 extent, i32x4 pageExtent, std::vector<Page> &&pages) noexcept :
        parent(parent),
        extent(extent),
        pageExtent(pageExtent),
        pages(std::move(pages)) {}

    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;
    ~Image();

    Image(Image const &other) = delete;
    Image &operator=(Image const &other) = delete;

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

    /** Upload image to atlas.
     */
    void upload(pixel_map<R16G16B16A16SFloat> const &image) noexcept;

private:
    //! Temporary memory used for pre calculating vertices.
    std::vector<std::tuple<f32x4, f32x4, bool>> tmpVertexPositions;

    void calculateVertexPositions(mat transform, aarect clippingRectangle);

    void placePageVertices(vspan<Vertex> &vertices, int const index, aarect clippingRectangle) const;

};

}
