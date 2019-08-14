// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>

namespace TTauri::GUI::PipelineFlat {

struct Vertex;

struct Delegate {
    virtual void pipelineFlatPlaceVertices(gsl::span<Vertex> &vertices, int &offset) = 0;
};

}
