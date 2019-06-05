#pragma once

#include "alignment.hpp"
#include "BezierPoint.hpp"
#include "QBezier.hpp"
#include "Glyphs.hpp"
#include "Font.hpp"
#include "SubpixelMask.hpp"
#include "TTauri/all.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace TTauri::Draw {

template<typename T> struct PixelMap;

void renderMask(PixelMap<uint8_t>& mask, std::vector<QBezier> const& curves);


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

    /*! Draw an circular arc.
     * The arc is drawn from the current position to the position given
     * in this method. A positive arc is drawn counter-clockwise.
     *
     * \param radius postive radius means positive arc, negative radius is a negative arc.
     * \param position end position of the arc.
     */
    void arcTo(float radius, glm::vec2 position) {
        let r = std::abs(radius);
        let P0 = currentPosition();
        let P2 = position;
        let Pm = midpoint(P0, P2);

        let Vm2 = P2 - Pm;

        // Calculate the half angle between vectors P0 - C and P2 - C.
        let alpha = std::asin(glm::length(Vm2) / r);
        
        // Find P1 by extending a normal at Pm away from C.
        let P1 = Pm + normal(Vm2) * std::sin(alpha) * -radius;

        curveTo(P1, P2);
    }

    /*! Draw a rectangle.
     * \param rect the offset and size of the rectangle.
     * \param corner radius of <bottom-left, bottom-right, top-right, top-left>
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(rect2 rect, glm::vec4 corners={0.0f, 0.0f, 0.0f, 0.0f}) {
        glm::vec4 radii = glm::abs(corners);

        let blc = rect.offset;
        let brc = rect.offset + glm::vec2{rect.extent.x, 0.0f};
        let trc = rect.offset + rect.extent;
        let tlc = rect.offset + glm::vec2{0.0f, rect.extent.y};

        let blc1 = blc + glm::vec2{0.0f, radii.x};
        let blc2 = blc + glm::vec2{radii.x, 0.0f};
        let brc1 = brc + glm::vec2{-radii.y, 0.0f};
        let brc2 = brc + glm::vec2{0.0f, radii.y};
        let trc1 = trc + glm::vec2{0.0f, -radii.z};
        let trc2 = trc + glm::vec2{-radii.z, 0.0f};
        let tlc1 = tlc + glm::vec2{radii.w, 0.0f};
        let tlc2 = tlc + glm::vec2{0.0, -radii.w};

        moveTo(blc1);
        if (corners.x > 0.0) {
            arcTo(radii.x, blc2);
        } else if (corners.x < 0.0) {
            lineTo(blc2);
        }

        lineTo(brc1);
        if (corners.y > 0.0) {
            arcTo(radii.y, brc2);
        } else if (corners.y < 0.0) {
            lineTo(blc2);
        }

        lineTo(trc1);
        if (corners.z > 0.0) {
            arcTo(radii.z, trc2);
        } else if (corners.z < 0.0) {
            lineTo(trc2);
        }

        lineTo(tlc1);
        if (corners.w > 0.0) {
            arcTo(radii.w, tlc2);
        } else if (corners.w < 0.0) {
            lineTo(tlc2);
        }

        close();
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
            points.push_back(point.transform(position, scale, rotation));
        }
        for (let endPoint: glyph.endPoints) {
            endPoints.push_back(currentNrPoints + endPoint);
        }
    }

    void addText(std::string const &text, Font const &font, glm::vec2 position, float scale, float rotation = 0.0f, HorizontalAlignment alignment=HorizontalAlignment::Left) {
        Glyphs glyphs = font.getGlyphs(text);

        auto glyphPosition = glyphs.getStartPosition(position, scale, rotation, alignment);
        for (size_t i = 0; i < glyphs.size(); i++) {
            let glyph = glyphs.at(i);
            addGlyph(glyph, glyphPosition, scale, rotation);
            glyphPosition += glyphs.glyphAdvanceVector(i, scale, rotation);
        }
    }

    /*! Render path in subpixel mask.
     * The rendering done is additive and saturates the pixel value, therefor
     * you can do multiple path renders into the same mask.
     * 
     * \param mask pixelmap to be modified.
     */
    void render(SubpixelMask& mask) const {
        return mask.render(getQBeziers());
    }

    void render(PixelMap<uint64_t>& pixels, Color_sRGBLinear color, SubpixelMask::Orientation subpixelOrientation) const {
        auto mask = SubpixelMask(pixels.width * 3, pixels.height);
        mask.clear();
        mask.render(getQBeziers());
        mask.filter(subpixelOrientation);

        composit(pixels, color, mask);
    }
};



}