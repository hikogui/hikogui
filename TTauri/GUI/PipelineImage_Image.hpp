
#pragma once

#include "PipelineImage_ImageLocation.hpp"
#include "PipelineImage_Vertex.hpp"
#include "PipelineImage_Page.hpp"
#include "TTauri/Draw/PixelMap.hpp"

#include <gsl/gsl>

namespace TTauri::GUI::PipelineImage {

struct Image {
    struct Error : virtual boost::exception, virtual std::exception {};

    bool drawn = false;
    size_t retainCount = 1;

    std::string key;
    u64extent2 extent;

    //! Number of pages in width and height.
    u64extent2 pageExtent;

    std::vector<Page> pages;

    Image(std::string key, u64extent2 extent, u64extent2 pageExtent, std::vector<Page> pages) :
        key(std::move(key)),
        extent(std::move(extent)),
        pageExtent(std::move(pageExtent)),
        pages(std::move(pages)) {}

    /*! Find the image coordinates of a page in the image.
     * \param pageIndex Index in the pages-vector.
     */
    u64rect2 indexToRect(size_t const pageIndex) const;

    /*! Place vertices for this image.
     * An image is build out of atlas pages, that need to be individual rendered.
     * A page with the value std::numeric_limits<uint16_t>::max() is not rendered.
     *
     * \param position Position (x, y) from the left-top of the window in pixels. Z equals depth.
     * \param origin Origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
     */
    void placeVertices(const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset) const;
private:
    void placePageVertices(size_t const index, const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset) const;

};

}