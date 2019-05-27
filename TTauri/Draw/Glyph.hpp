// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "QBezier.hpp"
#include "TTauri/all.hpp"

#include <glm/glm.hpp>

namespace TTauri::Draw {




struct Glyph {
    rect2 boundingBox;
    float leftSideBearing;
    float rightSideBearing;
    float advanceWidth;
    size_t numberOfGraphemes;

    std::vector<BezierPoint> points;
    std::vector<size_t> endPoints;

    size_t nrContours() const {
        return endPoints.size();
    }

    float getAdvanceForGrapheme(size_t index) const {
        return (advanceWidth / numberOfGraphemes) * index;
    }

    std::vector<BezierPoint> getPointsOfContour(size_t contourNr) const {
        let begin = points.begin() + (contourNr == 0 ? 0 : endPoints.at(contourNr - 1));
        let end = points.begin() + endPoints.at(contourNr);
        return std::vector(begin, end);
    }

    std::vector<QBezier> getContour(size_t contourNr) const {
        let contourPoints = getPointsOfContour(contourNr);
        return QBezier::getContour(contourPoints);
    }
};


}