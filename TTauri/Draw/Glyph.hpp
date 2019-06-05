// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "QBezier.hpp"
#include "TTauri/all.hpp"

#include <glm/glm.hpp>

namespace TTauri::Draw {




struct Glyph {
    // Glyph is valid when completely parsed by font parser.
    bool valid;

    rect2 boundingBox;
    float leftSideBearing;
    float rightSideBearing;
    float advanceWidth;
    size_t numberOfGraphemes = 1;
    size_t useMetricsOfGlyph = std::numeric_limits<size_t>::max();

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

    void addSubGlyph(Glyph const &other, glm::mat2x2 scale, glm::vec2 offset) {
        for (let endPoint: other.endPoints) {
            endPoints.push_back(endPoint + points.size());
        }
        for (let point: other.points) {
            points.push_back(point * scale + offset);
        }
    }
};


}