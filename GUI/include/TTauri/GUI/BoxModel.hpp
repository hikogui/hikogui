// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
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

    vec currentBegin(float depth=0.0f) const noexcept {
        return vec::point(left.value(), bottom.value(), depth);
    }

    vec currentEnd(float depth=0.0f) const noexcept {
        return vec::point(right.evaluate(), top.evaluate(), depth);
    }

    vec currentOffset(float depth=0.0f) const noexcept {
        return { left.value(), bottom.value(), depth };
    }

    vec currentExtent() const noexcept {
        return { width.value(), height.value() };
    }

    rect currentRectangle() const noexcept {
        return {left.value(), bottom.value(), width.value(), height.value()};
    }

    bool contains(vec position) const noexcept {
        return currentRectangle().contains(position);
    }
};

}
