// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "TTauri/all.hpp"
#include <boost/format.hpp>
#include <glm/glm.hpp>

namespace TTauri::Config {

struct ASTColor : ASTExpression {
    //! The color value, stored in CIE XYZ linear color space.
    sRGBA value;

    ASTColor(Location location, uint32_t value) : ASTExpression(location), value(value) {}

    std::string str() const override {
        return value.string();
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}
