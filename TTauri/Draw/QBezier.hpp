// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/math.hpp"
#include "TTauri/geometry.hpp"

#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri::Draw {

/*! Quadratic Bezier Curve
*/
struct QBezier {
    glm::vec2 P0; //!< First point
    glm::vec2 P1; //!< Control point
    glm::vec2 P2; //!< Last point

    QBezier(glm::vec2 P0, glm::vec2 P1, glm::vec2 P2) : P0(P0), P1(P1), P2(P2) {}

    void transform(glm::mat3x3 M) {
        P0 = (glm::vec3(P0, 1.0) * M).xy();
        P1 = (glm::vec3(P1, 1.0) * M).xy();
        P2 = (glm::vec3(P2, 1.0) * M).xy();
    }
};

}