// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/required.hpp"
#include <rhea/linear_expression.hpp>

namespace TTauri::GUI {

class BoxModel {
public:
    rhea::variable left;
    rhea::variable bottom;
    rhea::variable width;
    rhea::variable height;

    const rhea::linear_expression right = left + width;
    const rhea::linear_expression centre = left + width * 0.5;
    const rhea::linear_expression top = bottom + height;
    const rhea::linear_expression middle = bottom + height * 0.5;

    float leftValue() const noexcept {
        return std::round(numeric_cast<float>(left.value()));
    }

    float bottomValue() const noexcept {
        return std::round(numeric_cast<float>(bottom.value()));
    }

    float widthValue() const noexcept {
        return std::round(numeric_cast<float>(width.value()));
    }

    float heightValue() const noexcept {
        return std::round(numeric_cast<float>(height.value()));
    }

    vec offset() const noexcept {
        return { leftValue(), bottomValue() };
    }

    vec extent() const noexcept {
        return { widthValue(), heightValue() };
    }

    aarect rectangle() const noexcept {
        return { leftValue(), bottomValue(), widthValue(), heightValue() };
    }

    bool contains(vec position) const noexcept {
        return rectangle().contains(position);
    }
};

}
