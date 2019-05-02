// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "TTauri/Color.hpp"
#include <boost/format.hpp>
#include <glm/glm.hpp>

namespace TTauri::Config {

struct ASTColor : ASTExpression {
    //! The color value, stored in CIE XYZ linear color space.
    Color_XYZ value;

    ASTColor(Location location, uint32_t value) : ASTExpression(location), value(color_cast<Color_XYZ>(Color_sRGB(value))) {}

    std::string str() const override {
        return color_cast<Color_sRGB>(value).str();
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}
