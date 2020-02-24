// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
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

    rhea::linear_expression outerLeft(float margin) const noexcept { return left - margin; }
    rhea::linear_expression outerRight(float margin) const noexcept { return right + margin; }
    rhea::linear_expression outerBottom(float margin) const noexcept { return bottom - margin; }
    rhea::linear_expression outerTop(float margin) const noexcept { return top + margin; }
    rhea::linear_expression outerWidth(float margin) const noexcept { return width + (margin * 2.0); }
    rhea::linear_expression outerHeight(float margin) const noexcept { return height + (margin * 2.0); }

    glm::vec2 currentPosition() const noexcept {
        return { left.value(), bottom.value() };
    }

    extent2 currentExtent() const noexcept {
        return { width.value(), height.value() };
    }

    rect2 currentRectangle() const noexcept {
        return { currentPosition(), currentExtent() };
    }

    bool contains(glm::vec2 position) const noexcept {
        return currentRectangle().contains(position);
    }
};

}
