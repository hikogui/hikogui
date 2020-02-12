// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/MSD10.hpp"
#include "TTauri/Foundation/SDF8.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/math.hpp"
#include "TTauri/Foundation/bezier.hpp"
#include "TTauri/Foundation/required.hpp"
#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri {

struct BezierPoint;

/*! Bezier Curve
 * A linear, quadratic or cubic bezier curve.
 */
struct BezierCurve {
    enum class Type : uint8_t { None, Linear, Quadratic, Cubic };
    enum class Color : uint8_t { Yellow, Magenta, Cyan, White };

    Type type;
    Color color;
    glm::vec2 P1; //!< First point
    glm::vec2 C1; //!< Control point
    glm::vec2 C2; //!< Control point
    glm::vec2 P2; //!< Last point

    BezierCurve() noexcept = delete;
    BezierCurve(BezierCurve const &other) noexcept = default;
    BezierCurve(BezierCurve &&other) noexcept = default;
    BezierCurve &operator=(BezierCurve const &other) noexcept = default;
    BezierCurve &operator=(BezierCurve &&other) noexcept = default;

    /*! Construct a linear bezier-curve.
     */
    BezierCurve(glm::vec2 const P1, glm::vec2 const P2, Color color=Color::White) noexcept :
        type(Type::Linear), color(color), P1(P1), C1(), C2(), P2(P2) {}

    /*! Construct a quadratic bezier-curve.
     */
    BezierCurve(glm::vec2 const P1, glm::vec2 const C1, glm::vec2 const P2, Color color=Color::White) noexcept :
        type(Type::Quadratic), color(color), P1(P1), C1(C1), C2(C1), P2(P2) {}

    /*! Construct a cubic bezier-curve.
     */
    BezierCurve(glm::vec2 const P1, glm::vec2 const C1, glm::vec2 const C2, glm::vec2 const P2, Color color=Color::White) noexcept :
        type(Type::Cubic), color(color), P1(P1), C1(C1), C2(C2), P2(P2) {}

    /*! Construct a bezier-curve of any type.
    */
    BezierCurve(Type const type, glm::vec2 const P1, glm::vec2 const C1, glm::vec2 const C2, glm::vec2 const P2, Color color=Color::White) noexcept :
        type(type), color(color), P1(P1), C1(C1), C2(C2), P2(P2) {}


    [[nodiscard]] bool has_red() const noexcept {
        return color != Color::Cyan;
    }

    [[nodiscard]] bool has_green() const noexcept {
        return color != Color::Magenta;
    }

    [[nodiscard]] bool has_blue() const noexcept {
        return color != Color::Yellow;
    }

    /*! Return a point on the bezier-curve.
     * Values of `t` beyond 0.0 and 1.0 will find a point extrapolated beyond the
     * bezier segment.
     * 
     * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2).
     * \return the coordinates of the point on the curve.
     */ 
    [[nodiscard]] glm::vec2 pointAt(float const t) const noexcept {
        switch (type) {
        case Type::Linear: return bezierPointAt(P1, P2, t);
        case Type::Quadratic: return bezierPointAt(P1, C1, P2, t);
        case Type::Cubic: return bezierPointAt(P1, C1, C2, P2, t);
        default: no_default;
        }
    }

    /*! Return a tangent on the bezier-curve.
    * Values of `t` beyond 0.0 and 1.0 will find a point extrapolated beyond the
    * bezier segment.
    * 
    * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2).
    * \return the tangent-vector at point t on the curve
    */ 
    [[nodiscard]] glm::vec2 tangentAt(float const t) const noexcept {
        switch (type) {
        case Type::Linear: return bezierTangentAt(P1, P2, t);
        case Type::Quadratic: return bezierTangentAt(P1, C1, P2, t);
        case Type::Cubic: return bezierTangentAt(P1, C1, C2, P2, t);
        default: no_default;
        }
    }

    /*! Return the x values where the curve crosses the y-axis.
     * \param y y-axis.
     * \return 0 to 3, or infinite number of x values.
     */
    [[nodiscard]] results<float,3> solveXByY(float const y) const noexcept {
        switch (type) {
        case Type::Linear: return bezierFindX(P1, P2, y);
        case Type::Quadratic: return bezierFindX(P1, C1, P2, y);
        case Type::Cubic: return bezierFindX(P1, C1, C2, P2, y);
        default: no_default;
        }
    }

    [[nodiscard]] results<float,3> solveTForNormalsIntersectingPoint(glm::vec2 P) const noexcept {
        switch (type) {
        case Type::Linear: return bezierFindTForNormalsIntersectingPoint(P1, P2, P);
        case Type::Quadratic: return bezierFindTForNormalsIntersectingPoint(P1, C1, P2, P);
        case Type::Cubic: no_default;
        default: no_default;
        }
    }

    struct msdf_result_t {
        float squared_distance;
        float angle;
        float t;

        constexpr msdf_result_t() noexcept :
            squared_distance(std::numeric_limits<float>::max()), angle(0.0f), t(0.0f) {}

        constexpr msdf_result_t(float squared_distance, float angle, float t) noexcept :
            squared_distance(squared_distance), angle(angle), t(t) {}

        [[nodiscard]] friend constexpr bool operator<(msdf_result_t const &lhs, msdf_result_t const &rhs) noexcept {
            if (lhs.squared_distance != rhs.squared_distance) {
                return lhs.squared_distance < rhs.squared_distance;
            } else {
                // Maximize orthogonality.
                return std::abs(lhs.angle) > std::abs(rhs.angle);
            }
        }
    };

    /** Find the distance from the point to the curve.
     */
    [[nodiscard]] std::pair<float,float> sdf_distance(glm::vec2 P) const noexcept {
        auto min_square_distance = std::numeric_limits<float>::max();
        auto min_t = 0.0f;
        auto min_normal = glm::vec2{0.0f, 1.0f};

        let ts = solveTForNormalsIntersectingPoint(P);
        for (auto t: ts) {
            t = std::clamp(t, 0.0f, 1.0f);

            let normal = P - pointAt(t);
            let square_distance = glm::dot(normal, normal);
            if (square_distance < min_square_distance) {
                min_square_distance = square_distance;
                min_t = t;
                min_normal = normal;
            }
        }

        let unit_normal = glm::normalize(min_normal);
        let unit_tangent = glm::normalize(tangentAt(min_t));
        let orthogonality = viktorCross(unit_normal, unit_tangent);

        let tangent = tangentAt(min_t);
        let distance = std::sqrt(min_square_distance);

        let sdistance = viktorCross(tangent, min_normal) < 0.0 ? distance : -distance;

        // Use the original angle, for determining which side of the curve the point is.
        return {sdistance, orthogonality};
    }

    /** Find the distance from the point to the curve.
     */
    [[nodiscard]] msdf_result_t msdf_fast_distance(glm::vec2 P) const noexcept {
        auto min_square_distance = std::numeric_limits<float>::max();
        auto min_clamped_t = 0.0f;
        auto min_t = 0.0f;
        auto min_normal = glm::vec2{0.0f, 1.0f};

        let ts = solveTForNormalsIntersectingPoint(P);
        for (let t: ts) {
            let clamped_t = std::clamp(t, 0.0f, 1.0f);

            let normal = P - pointAt(clamped_t);
            let square_distance = glm::dot(normal, normal);
            if (square_distance < min_square_distance) {
                min_square_distance = square_distance;
                min_clamped_t = clamped_t;
                min_t = t;
                min_normal = normal;
            }
        }

        let unit_normal = glm::normalize(min_normal);
        let unit_tangent = glm::normalize(tangentAt(min_clamped_t));
        auto angle = viktorCross(unit_normal, unit_tangent);

        return {min_square_distance, angle, min_t};
    }

    /** Find the distance from the point to the curve.
    * @return distance to the curve; 0.0 on the curve, positive is inside, negative is outside.
    */
    [[nodiscard]] float signed_pseudo_distance(msdf_result_t result, glm::vec2 P) const noexcept {
        // Use the non-clamped t, to get the distance to the extrapolated curve.
        let distance = glm::length(P - pointAt(result.t));

        // Use the original angle, for determining which side of the curve the point is.
        return result.angle < 0.0f ? -distance : distance;
    }

    /*! Split a cubic bezier-curve into two cubic bezier-curve.
     * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2)
     *        where to split the curve.
     * \return two cubic bezier-curves.
     */
    [[nodiscard]] std::pair<BezierCurve,BezierCurve> cubicSplit(float const t) const noexcept {
        let outerA = BezierCurve{P1, C1};
        let outerBridge = BezierCurve{C1, C2};
        let outerB = BezierCurve{C2, P2};

        let innerA = BezierCurve{outerA.pointAt(t), outerBridge.pointAt(t)};
        let innerB = BezierCurve{outerBridge.pointAt(t), outerB.pointAt(t)};

        let newPoint = BezierCurve{innerA.pointAt(t), innerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), innerA.pointAt(t), newPoint }, { newPoint, innerB.pointAt(t), outerB.pointAt(t), P2 }};
    }

    /*! Split a quadratic bezier-curve into two quadratic bezier-curve.
     * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2)
     *        where to split the curve.
     * \return two quadratic bezier-curves.
     */
    [[nodiscard]] std::pair<BezierCurve,BezierCurve> quadraticSplit(float const t) const noexcept {
        let outerA = BezierCurve{P1, C1};
        let outerB = BezierCurve{C1, P2};

        let newPoint = BezierCurve{outerA.pointAt(t), outerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), newPoint }, { newPoint, outerB.pointAt(t), P2 }};
    }

    /*! Split a linear bezier-curve into two linear bezier-curve.
     * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2)
     *        where to split the curve.
     * \return two linear bezier-curves.
     */
    [[nodiscard]] std::pair<BezierCurve,BezierCurve> linearSplit(float const t) const noexcept {
        let newPoint = pointAt(t);

        return {{ P1, newPoint }, { newPoint, P2 }};
    }

    /*! Split a bezier-curve into two bezier-curve of the same type.
     * \param t a relative distance between 0.0 (point P1) and 1.0 (point P2)
     *        where to split the curve.
     * \return two bezier-curves.
     */
    [[nodiscard]] std::pair<BezierCurve,BezierCurve> split(float const t) const noexcept {
        switch (type) {
        case Type::Linear: return linearSplit(t);
        case Type::Quadratic: return quadraticSplit(t);
        case Type::Cubic: return cubicSplit(t);
        default: no_default;
        }
    }

    /*! Subdivide a bezier-curve until each are flat enough.
     * \param r resulting list of linear segments.
     * \param minimumFlatness minimum amount of flatness of the resulting curve segments.
     */
    void subdivideUntilFlat_impl(std::vector<BezierCurve> &r, float const minimumFlatness) const noexcept {
        if (flatness() >= minimumFlatness) {
            r.push_back(*this);
        } else {
            let [a, b] = split(0.5f);
            a.subdivideUntilFlat_impl(r, minimumFlatness);
            b.subdivideUntilFlat_impl(r, minimumFlatness);
        }
    }

    /*! Subdivide a bezier-curve until each are flat enough.
     * \param tolerance maximum amount of curviness.
     * \return resulting list of curve segments.
     */
    [[nodiscard]] std::vector<BezierCurve> subdivideUntilFlat(float const tolerance) const noexcept {
        std::vector<BezierCurve> r;
        subdivideUntilFlat_impl(r, 1.0f - tolerance);
        return r;
    }

    /*! Return the flatness of a curve.
    * \return 1.0 when completely flat, < 1.0 when curved.
    */
    [[nodiscard]] float flatness() const noexcept {
        switch (type) {
        case Type::Linear: return bezierFlatness(P1, P2);
        case Type::Quadratic: return bezierFlatness(P1, C1, P2);
        case Type::Cubic: return bezierFlatness(P1, C1, C2, P2);
        default: no_default;
        }
    }

    BezierCurve &operator*=(glm::vec2 const rhs) noexcept {
        this->P1 *= rhs;
        this->C1 *= rhs;
        this->C2 *= rhs;
        this->P2 *= rhs;
        return *this;
    }

    BezierCurve &operator+=(glm::vec2 const rhs) noexcept {
        this->P1 += rhs;
        this->C1 += rhs;
        this->C2 += rhs;
        this->P2 += rhs;
        return *this;
    }

    /*! Return a line-segment from a curve at a certain distance.
     * \param offset positive means the parallel line will be on the starboard of the curve.
     * \return line segment offset from the curve.
     */
    [[nodiscard]] BezierCurve toParrallelLine(float const offset) const noexcept {
        auto [newP1, newP2] = parrallelLine(P1, P2, offset);
        return { newP1, newP2 };
    }

    [[nodiscard]] friend bool operator==(BezierCurve const &lhs, BezierCurve const &rhs) noexcept {
        if (lhs.type != rhs.type) {
            return false;
        }
        switch (lhs.type) {
        case BezierCurve::Type::Linear:
            return (lhs.P1 == rhs.P1) && (lhs.P2 == rhs.P2);
        case BezierCurve::Type::Quadratic:
            return (lhs.P1 == rhs.P1) && (lhs.C1 == rhs.C1) && (lhs.P2 == rhs.P2);
        case BezierCurve::Type::Cubic:
            return (lhs.P1 == rhs.P1) && (lhs.C1 == rhs.C1) && (lhs.C2 == rhs.C2) && (lhs.P2 == rhs.P2);
        default:
            no_default;
        }
    }

    [[nodiscard]] friend BezierCurve operator*(glm::mat3x3 const &lhs, BezierCurve const &rhs) noexcept {
        return {
            rhs.type,
            glm::xy(lhs * glm::vec3(rhs.P1, 1.0)),
            glm::xy(lhs * glm::vec3(rhs.C1, 1.0)),
            glm::xy(lhs * glm::vec3(rhs.C2, 1.0)),
            glm::xy(lhs * glm::vec3(rhs.P2, 1.0))
        };
    }

    [[nodiscard]] friend BezierCurve operator*(BezierCurve const &lhs, glm::vec2 const rhs) noexcept {
        return { lhs.type, lhs.P1 * rhs, lhs.C1 * rhs, lhs.C2 * rhs, lhs.P2 * rhs };
    }


    [[nodiscard]] friend BezierCurve operator+(BezierCurve const &lhs, glm::vec2 const rhs) noexcept {
        return { lhs.type, lhs.P1 + rhs, lhs.C1 + rhs, lhs.C2 + rhs, lhs.P2 + rhs };
    }


    /*! Reverse direction of a curve.
     */
    [[nodiscard]] friend BezierCurve operator~(BezierCurve const &rhs) noexcept {
        return { rhs.type, rhs.P2, rhs.C2, rhs.C1, rhs.P1 };
    }
};


/*! Make a contour of Bezier curves from a list of points.
 * The contour is also colorized to be used for creating multichannel-signed-distance-fields.
 *
 * \param first The first point in a list
 * \param first The last point in a list
 */
[[nodiscard]] std::vector<BezierCurve> makeContourFromPoints(
    std::vector<BezierPoint>::const_iterator first,
    std::vector<BezierPoint>::const_iterator last) noexcept;

/*! Inverse a contour.
 * Reverse the direction of the whole contour, turning it inside out.
 * This is useful for creating a stroke, by inverting the inner offset contour.
 * \param contour contour to reverse.
 * \return the reversed contour.
 */
[[nodiscard]] std::vector<BezierCurve> makeInverseContour(std::vector<BezierCurve> const &contour) noexcept;

/*! Make a contour of Bezier curves from another contour of Bezier curves at a offset.
 * Make a new contour made out of line-segments offset from the original curve. After
 * offsetting the line segment the line segments are properly cut or extended to
 * cover all intersections and gaps.
 *
 * \param contour a list of bezier curve segments forming a closed contour.
 * \param offset positive means the parallel contour will be on the starboard side of the given contour. 
 * \param lineJoinStyle how the gaps between line segments are joined together.
 * \param tolerance to how curved the new contour should look.
 */
[[nodiscard]] std::vector<BezierCurve> makeParrallelContour(
    std::vector<BezierCurve> const &contour,
    float offset,
    LineJoinStyle lineJoinStyle,
    float tolerance) noexcept;

/** Fill a linear gray scale image by filling a curve with anti-aliasing.
 * @param image An alpha-channel image to make opaque where pixel is inside the contours
 * @param curves All curves of path, in no particular order.
 */
void fill(PixelMap<uint8_t>& image, std::vector<BezierCurve> const& curves) noexcept;

/** Fill a multi-channel signed distance field image from the given contour.
 * @param image An multichannel-signed-distance-field which show distance toward the closest curve
 * @param curves All curves of path, in no particular order.
 */
void fill(PixelMap<MSD10> &image, std::vector<BezierCurve> const &curves) noexcept;

/** Fill a signed distance field image from the given contour.
* @param image An signed-distance-field which show distance toward the closest curve
* @param curves All curves of path, in no particular order.
*/
void fill(PixelMap<SDF8> &image, std::vector<BezierCurve> const &curves) noexcept;

}
