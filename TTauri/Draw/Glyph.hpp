// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "QBezier.hpp"
#include "PixelMap.hpp"
#include "TTauri/geometry.hpp"

#include <glm/glm.hpp>

namespace TTauri::Draw {




struct Glyph {
    rect2 boundingBox;
    float leftSideBearing;
    float rightSideBearing;
    float advanceWidth;

    std::vector<BezierPoint> points;
    std::vector<size_t> endPoints;

    size_t nrContours() {
        return endPoints.size();
    }

    std::vector<BezierPoint> getPointsOfContour(size_t contourNr) {
        let begin = points.begin() + (contourNr == 0 ? 0 : endPoints.at(contourNr - 1));
        let end = points.begin() + endPoints.at(contourNr);
        return std::vector(begin, end);
    }

    std::vector<QBezier> getContour(size_t contourNr) {
        let contourPoints = getPointsOfContour(contourNr);
        QBezier::getContour(contourPoints);
    }
};


}