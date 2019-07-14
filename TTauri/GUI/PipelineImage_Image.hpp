// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/geometry.hpp"
#include <boost/exception/exception.hpp>
#include <gsl/gsl>
#include <atomic>
#include <string>

namespace TTauri::GUI::PipelineImage {

struct Page;
struct Vertex;
struct ImageLocation;
struct DeviceShared;

struct Image {
    enum class State { Uninitialized, Drawing, Uploaded };

    struct Error : virtual boost::exception, virtual std::exception {};

    mutable std::atomic<State> state = State::Uninitialized;

    DeviceShared *parent;

    std::string key;
    u64extent2 extent;

    //! Number of pages in width and height.
    u64extent2 pageExtent;

    std::vector<Page> pages;

    Image(DeviceShared *parent, std::string key, u64extent2 extent, u64extent2 pageExtent, std::vector<Page> pages) :
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
    void placeVertices(const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset);

private:
    //! Temporary memory used for pre calculating vertices.
    std::vector<std::tuple<glm::vec2, u64extent2, bool>> tmpVertexPositions;

    void calculateVertexPositions(const ImageLocation &location);

    void placePageVertices(size_t const index, const ImageLocation &location, gsl::span<Vertex> &vertices, size_t &offset) const;

};

}
