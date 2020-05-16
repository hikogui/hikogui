// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/required.hpp"
#include <rhea/linear_expression.hpp>

namespace TTauri::GUI {

class BoxModel {
private:
    float previousWidth;
    float previousHeight;
public:
    rhea::variable left;
    rhea::variable bottom;
    rhea::variable width;
    rhea::variable height;

    const rhea::linear_expression right = left + width;
    const rhea::linear_expression centre = left + width * 0.5;
    const rhea::linear_expression top = bottom + height;
    const rhea::linear_expression middle = bottom + height * 0.5;

    [[nodiscard]] bool hasResized() const noexcept {
        let currentWidth = width.value();
        let currentHeight = height.value();
        let resized = (previousWidth != currentWidth) || (previousHeight != currentHeight);
        previousWidth = currentWidth;
        previousHeight = currentHeight;
        return resized;
    }

    [[nodiscard]] vec offset() const noexcept {
        return { left.value(), bottom.value() };
    }

    [[nodiscard]] vec extent() const noexcept {
        return { width.value(), height.value() };
    }

    [[nodiscard]] aarect rectangle() const noexcept {
        return { left.value(), bottom.value(), width.value(), height.value() };
    }

    [[nodiscard]] bool contains(vec position) const noexcept {
        return rectangle().contains(position);
    }
};

}
