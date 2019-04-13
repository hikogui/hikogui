
#pragma once

#include "PipelineImage_Vertex.hpp"
#include <boost/exception/all.hpp>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineImage {

struct Delegate {
    struct Error : virtual boost::exception, virtual std::exception {};

    virtual void pipelineImagePlaceVertices(gsl::span<Vertex> &vertices, size_t &offset) = 0;
};

}
