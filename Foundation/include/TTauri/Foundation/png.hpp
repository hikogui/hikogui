

#pragma once

#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <nonstd/span>

namespace TTauri {

class png {

public:
    png(nonstd::span<std::byte const> bytes);

    png(URL const &url);

};

};

