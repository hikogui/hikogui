// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil.hpp"

namespace tt {

class image_stencil: public stencil {
public:
    using super = stencil;

    image_stencil(alignment alignment) : super(alignment) {}

};

}
