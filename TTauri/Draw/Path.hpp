// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "QBezier.hpp"
#include "Glyph.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace TTauri::Draw {

struct Path {
    std::vector<BezierPoint> points;
    std::vector<size_t> endPoints;

    /*! Return the number of closed sub-paths.
     */
    size_t numberOfSubpaths() const {
        return endPoints.size();
    }

    std::vector<BezierPoint> getBezierPointsOfSubpath(size_t subpathNr) const {
        let begin = points.begin() + (subpathNr == 0 ? 0 : endPoints.at(subpathNr - 1) + 1);
        let end = points.begin() + endPoints.at(subpathNr) + 1;
        return std::vector(begin, end);
    }

    std::vector<QBezier> getQBeziersOfSubpath(size_t subpathNr) const {
        let contourPoints = getBezierPointsOfSubpath(subpathNr);
        return QBezier::getContour(contourPoints);
    }

    std::vector<QBezier> getQBeziers() const {
        std::vector<QBezier> r;
        for (size_t subpathNr = 0; subpathNr < numberOfSubpaths(); subpathNr++) {
            for (let bezier: getQBeziersOfSubpath(subpathNr)) {
                r.push_back(std::move(bezier));
            }
        }
        return r;
    }

    /*! Return true if there is an open sub-path.
     */
    bool hasCurrentPosition() const {
        if (points.size() == 0) {
            return false;
        } else if (endPoints.size() == 0) {
            return true;
        } else {
            return endPoints.back() != (points.size() - 1);
        }
    }

    /*! Get the currentPosition of the open sub-path.
     * Returns {0, 0} when there is no sub-path open.
     */
    glm::vec2 currentPosition() const {
        if (hasCurrentPosition()) {
            return points.back().p;
        } else {
            return {0.0f, 0.0f};
        }
    }

    /*! Close current sub-path.
     * No operation if there is no open sub-path.
     */
    void close() {
        if (hasCurrentPosition()) {
            endPoints.push_back(points.size() - 1);
        }
    }

    /*! Start a new sub-path at position.
     * closes current subpath.
     */
    void moveTo(glm::vec2 position) {
        close();
        points.emplace_back(position, true);
    }

    /*! Start a new sub-path relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(glm::vec2 direction) {
        close();
        points.emplace_back(currentPosition() + direction, true);
    }

    void lineTo(glm::vec2 position) {
        points.emplace_back(position, true);
    }

    void lineRelativeTo(glm::vec2 direction) {
        points.emplace_back(currentPosition() + direction, true);
    }

    void curveTo(glm::vec2 controlPosition, glm::vec2 position) {
        points.emplace_back(controlPosition, false);
        points.emplace_back(position, true);
    }

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void curveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction) {
        let p = currentPosition();
        points.emplace_back(p + controlDirection, false);
        points.emplace_back(p + direction, true);
    }

    /*! Add glyph to path.
     * \param glyph Glyph to draw.
     * \param position The position to draw the origin of the glyph.
     * \param scale how much to scale the glyph by, the original glyph is 1Em high.
     * \param rotation Rotation in radials clock wise.
     */
    void addGlyph(Glyph const &glyph, glm::vec2 position, float scale, float rotation = 0.0f) {
        close();
        let currentNrPoints = points.size();
        for (let point: glyph.points) {
            points.push_back(point.transformFlipY(position, scale, rotation));
        }
        for (let endPoint: glyph.endPoints) {
            endPoints.push_back(currentNrPoints + endPoint);
        }
    }
};

}