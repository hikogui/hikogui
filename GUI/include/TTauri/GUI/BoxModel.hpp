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
    mutable __m128d previousExtent;
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
        let currentExtent = _mm_set_pd(width.value(), height.value());
        let resized = _mm_cmpneq_pd(currentExtent, previousExtent);
        let resized_mask = _mm_movemask_pd(resized);
        previousExtent = currentExtent;
        
        return resized_mask != 0;
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
