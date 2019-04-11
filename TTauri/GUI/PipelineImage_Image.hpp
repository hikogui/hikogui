
#pragma once

#include "PipelineImage.hpp"
#include "TTauri/Draw/PixelMap.hpp"

namespace TTauri::GUI {

struct PipelineImage::Image {
    struct Error : virtual boost::exception, virtual std::exception {};

    bool drawn = false;
    size_t retainCount = 1;

    std::string key;
    u16vec2 extent;

    //! Number of slices in width and height.
    u16vec2 sliceExtent;

    std::vector<uint16_t> slices;

    Image(std::string key, u16vec2 extent, u16vec2 sliceExtent, std::vector<uint16_t> slices) :
        key(std::move(key)),
        extent(std::move(extent)),
        sliceExtent(std::move(sliceExtent)),
        slices(std::move(slices)) {}

    Draw::PixelMap<uint32_t> getPixelMap(u16vec2 extent);
    void transferPixelMapToAtlas();

    /*! Find the image coordinates of a slice in the image.
     * \param sliceIndex Index in the slices-vector.
     */
    u64rect indexToRect(size_t const sliceIndex) const;

    /*! Place vertices for this image.
     * An image is build out of atlas slices, that need to be individual rendered.
     * A slice with the value std::numeric_limits<uint16_t>::max() is not rendered.
     *
     * \param position Position (x, y) from the left-top of the window in pixels. Z equals depth.
     * \param origin Origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
     */
    void placeVertices(const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const;
private:
    void placeSliceVertices(size_t const index, const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const;

};

}