
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

    //! Number of pages in width and height.
    u16vec2 pageExtent;

    std::vector<uint16_t> pages;

    Image(std::string key, u16vec2 extent, u16vec2 pageExtent, std::vector<uint16_t> pages) :
        key(std::move(key)),
        extent(std::move(extent)),
        pageExtent(std::move(pageExtent)),
        pages(std::move(pages)) {}

    Draw::PixelMap<uint32_t> getPixelMap(u16vec2 extent);
    void transferPixelMapToAtlas();

    /*! Find the image coordinates of a page in the image.
     * \param pageIndex Index in the pages-vector.
     */
    u64rect indexToRect(size_t const pageIndex) const;

    /*! Place vertices for this image.
     * An image is build out of atlas pages, that need to be individual rendered.
     * A page with the value std::numeric_limits<uint16_t>::max() is not rendered.
     *
     * \param position Position (x, y) from the left-top of the window in pixels. Z equals depth.
     * \param origin Origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
     */
    void placeVertices(const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const;
private:
    void placePageVertices(size_t const index, const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const;

};

}