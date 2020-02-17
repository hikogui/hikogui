// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>

namespace TTauri::GUI::PipelineSDF {

struct Vertex;

struct Delegate {
    virtual void pipelineSDFPlaceVertices(gsl::span<Vertex> &vertices, ssize_t &offset) = 0;
};

}
