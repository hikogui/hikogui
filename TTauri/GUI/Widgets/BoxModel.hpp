// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/geometry.hpp"
#include "TTauri/required.hpp"
#include <rhea/linear_expression.hpp>

namespace TTauri::GUI {

struct BoxModel {
    float leftPadding = 0.0;
    float rightPadding = 0.0;
    float bottomPadding = 0.0;
    float topPadding = 0.0;
    float leftMargin = 0.0;
    float bottomMargin = 0.0;
    float rightMargin = 0.0;
    float topMargin = 0.0;

    mutable rhea::variable left;
    mutable rhea::variable bottom;
    mutable rhea::variable width;
    mutable rhea::variable height;

    rhea::linear_expression right() const noexcept {
        return left + width;
    }

    rhea::linear_expression centre() const noexcept {
        return left + width * 0.5;
    }

    rhea::linear_expression top() const noexcept {
        return bottom + height;
    }

    rhea::linear_expression middle() const noexcept {
        return bottom + height * 0.5;
    }

    rhea::linear_expression innerLeft() const noexcept {
        return left + leftPadding;
    }

    rhea::linear_expression innerRight() const noexcept {
        return right() - rightPadding;
    }
    
    rhea::linear_expression innerWidth() const noexcept {
        return width - leftPadding - rightPadding;
    }

    rhea::linear_expression innerCenter() const noexcept {
        return innerLeft() + innerWidth() * 0.5;
    }

    rhea::linear_expression innerBottom() const noexcept {
        return bottom + bottomPadding;
    }

    rhea::linear_expression innerTop() const noexcept {
        return top() - topPadding;
    }

    rhea::linear_expression innerHeight() const noexcept {
        return height - bottomPadding - topPadding;
    }

    rhea::linear_expression innerMiddle() const noexcept {
        return innerBottom() + innerHeight() * 0.5;
    }

    rhea::linear_expression outerLeft() const noexcept {
        return left - leftMargin;
    }

    rhea::linear_expression outerRight() const noexcept {
        return right() + rightMargin;
    }

    rhea::linear_expression outerWidth() const noexcept {
        return width + leftMargin + rightMargin;
    }

    rhea::linear_expression outerCenter() const noexcept {
        return outerLeft() + outerWidth() * 0.5;
    }

    rhea::linear_expression outerBottom() const noexcept {
        return bottom - bottomMargin;
    }

    rhea::linear_expression outerTop() const noexcept {
        return top() + topMargin;
    }

    rhea::linear_expression outerHeight() const noexcept {
        return height + bottomMargin + topMargin;
    }

    rhea::linear_expression outerMiddle() const noexcept {
        return outerBottom() + outerHeight() * 0.5;
    }

    glm::vec2 currentPosition() const noexcept {
        return { left.value(), bottom.value() };
    }

    extent2 currentExtent() const noexcept {
        return { width.value(), height.value() };
    }

    rect2 currentRectangle() const noexcept {
        return { currentPosition(), currentExtent() };
    }

    glm::vec2 currentInnerPosition() const noexcept {
        return { innerLeft().evaluate(), innerBottom().evaluate() };
    }

    extent2 currentInnerExtent() const noexcept {
        return { innerWidth().evaluate(), innerHeight().evaluate() };
    }

    rect2 currentInnerRectangle() const noexcept {
        return { currentInnerPosition(), currentInnerExtent() };
    }

    glm::vec2 currentOuterPosition() const noexcept {
        return { outerLeft().evaluate(), outerBottom().evaluate() };
    }

    extent2 currentOuterExtent() const noexcept {
        return { outerWidth().evaluate(), outerHeight().evaluate() };
    }

    rect2 currentOuterRectangle() const noexcept {
        return { currentOuterPosition(), currentOuterExtent() };
    }

    bool contains(glm::vec2 position) const noexcept {
        return currentRectangle().contains(position);
    }
};

}