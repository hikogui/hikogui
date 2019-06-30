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

    rhea::linear_expression right() const {
        return left + width;
    }

    rhea::linear_expression centre() const {
        return left + width * 0.5;
    }

    rhea::linear_expression top() const {
        return bottom + height;
    }

    rhea::linear_expression middle() const {
        return bottom + height * 0.5;
    }

    rhea::linear_expression innerLeft() const {
        return left + leftPadding;
    }

    rhea::linear_expression innerRight() const {
        return right() - rightPadding;
    }
    
    rhea::linear_expression innerWidth() const {
        return width - leftPadding - rightPadding;
    }

    rhea::linear_expression innerCenter() const {
        return innerLeft() + innerWidth() * 0.5;
    }

    rhea::linear_expression innerBottom() const {
        return bottom + bottomPadding;
    }

    rhea::linear_expression innerTop() const {
        return top() - topPadding;
    }

    rhea::linear_expression innerHeight() const {
        return height - bottomPadding - topPadding;
    }

    rhea::linear_expression innerMiddle() const {
        return innerBottom() + innerHeight() * 0.5;
    }

    rhea::linear_expression outerLeft() const {
        return left - leftMargin;
    }

    rhea::linear_expression outerRight() const {
        return right() + rightMargin;
    }

    rhea::linear_expression outerWidth() const {
        return width + leftMargin + rightMargin;
    }

    rhea::linear_expression outerCenter() const {
        return outerLeft() + outerWidth() * 0.5;
    }

    rhea::linear_expression outerBottom() const {
        return bottom - bottomMargin;
    }

    rhea::linear_expression outerTop() const {
        return top() + topMargin;
    }

    rhea::linear_expression outerHeight() const {
        return height + bottomMargin + topMargin;
    }

    rhea::linear_expression outerMiddle() const {
        return outerBottom() + outerHeight() * 0.5;
    }

    glm::vec2 currentPosition() const {
        return { left.value(), bottom.value() };
    }

    extent2 currentExtent() const {
        return { width.value(), height.value() };
    }

    rect2 currentRectangle() const {
        return { currentPosition(), currentExtent() };
    }

    glm::vec2 currentInnerPosition() const {
        return { innerLeft().evaluate(), innerBottom().evaluate() };
    }

    extent2 currentInnerExtent() const {
        return { innerWidth().evaluate(), innerHeight().evaluate() };
    }

    rect2 currentInnerRectangle() const {
        return { currentInnerPosition(), currentInnerExtent() };
    }

    glm::vec2 currentOuterPosition() const {
        return { outerLeft().evaluate(), outerBottom().evaluate() };
    }

    extent2 currentOuterExtent() const {
        return { outerWidth().evaluate(), outerHeight().evaluate() };
    }

    rect2 currentOuterRectangle() const {
        return { currentOuterPosition(), currentOuterExtent() };
    }

    bool contains(glm::vec2 position) const {
        return currentRectangle().contains(position);
    }
};

}