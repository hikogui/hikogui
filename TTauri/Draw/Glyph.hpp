// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "Bezier.hpp"
#include "TTauri/required.hpp"
#include <glm/glm.hpp>

namespace TTauri::Draw {


struct Glyph {
    // Glyph is valid when completely parsed by font parser.
    bool valid = false;

    rect2 boundingBox = {};
    float leftSideBearing = 0.0;
    float rightSideBearing = 0.0;
    float advanceWidth = 0.0;
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

    std::vector<Bezier> getContour(size_t contourNr) const {
        let contourPoints = getPointsOfContour(contourNr);
        return Bezier::getContour(contourPoints);
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