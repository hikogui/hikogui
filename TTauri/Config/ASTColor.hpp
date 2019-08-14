// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "TTauri/wsRGBA.hpp"
#include <boost/format.hpp>
#include <glm/glm.hpp>

namespace TTauri::Config {

struct ASTColor : ASTExpression {
    //! The color value, stored in CIE XYZ linear color space.
    wsRGBA value;

    ASTColor(Location location, uint32_t value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        return to_string(value);
    }

    universal_value execute(ExecutionContext &context) const noexcept override { 
        return value;
    } 

};

}
