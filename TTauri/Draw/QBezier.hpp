// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/math.hpp"

#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri::Draw {

/*! Quadratic Bezier Curve
*/
struct QBezier {
    struct ClosestPoint {
        //! Pointer to the segment tested.
        QBezier *segment;

        //! Closest position on the curve compared to the given point.
        float t;

        //! Closest position on the extended-curve compared to the given point.
        float pseudoT;

        //! Vector from the curve to the given point.
        glm::vec2 ray;

        //! Vector from the extended-curve to the given point.
        glm::vec2 pseudoRay;

        //! The squared length of the ray.
        float distance;

        //! The squared length of the pseudoRay.
        float pseudoDistance;

        //! The sin(angle) between the ray and the direction of the curve.
        float orthogality;

        //! If the point is inside positive or outside negative the curve.
        float inside;

        ClosestPoint() :
            segment(nullptr),
            t(0.0f), pseudoT(0.0f),
            ray({0.0f, 0.0f}), pseudoRay({0.0f, 0.0f}),
            distance(std::numeric_limits<float>::infinity()), pseudoDistance(std::numeric_limits<float>::infinity()),
            orthogality(0.0f), inside(1.0f) {}

        ClosestPoint(QBezier *segment, float t, glm::vec2 ray, float distance, float pseudoT, glm::vec2 pseudoRay, float pseudoDistance, float orthogality, float inside) :
            segment(segment),
            t(t), pseudoT(pseudoT),
            ray(ray), pseudoRay(pseudoRay),
            distance(distance), pseudoDistance(pseudoDistance),
            orthogality(orthogality), inside(inside) {}

        bool operator<(const ClosestPoint &other) const {
            return
                (distance < other.distance) || (
                    (distance == other.distance) &&
                    (orthogality > other.orthogality)
                );
        }
    };

    glm::vec2 P0; //!< First point
    glm::vec2 P1; //!< Control point
    glm::vec2 P2; //!< Last point
    uint8_t color;

    // Distance prepare values.
    glm::vec2 pv1;
    glm::vec2 pv2;
    float a;
    float b;
    float twoPv1DotPv1;

    void transform(glm::mat3x3 M) {
        P0 = (glm::vec3(P0, 1.0) * M).xy();
        P1 = (glm::vec3(P1, 1.0) * M).xy();
        P2 = (glm::vec3(P2, 1.0) * M).xy();
    }

    void prepare() {
        pv1 = P1 - P0;
        pv2 = P2 - 2.0f*P1 + P0;

        a = glm::dot(pv2, pv2);
        b = 3.0f * glm::dot(pv1, pv2);

        twoPv1DotPv1 = glm::dot(2.0f*pv1, pv1);
    }

    /*! Calculate the direction at position t (inside {0,1}) on the bezier curve.
     * prepare() must be called first.
     */
    glm::vec2 directionAt(float t) {
        return 2.0f*t*pv2 + 2.0f*pv1;
    }

    /*! Get the point at position t (inside {0,1}) on the bezier curve.
     * prepare() must be called first.
     */
    glm::vec2 pointAt(float t) {
        return P0 + 2.0f*t*pv1 * t*t*pv2;
    }

    /*! Get the shortest ray to the bezier curve within the endpoints.
     */
    glm::vec2 rayAt(glm::vec2 P, float t) {
        return P - pointAt(t);
    }

    /*!
     * \return t, ray, distance, clampedT, clampedRay, clampedDistance
     */
    std::tuple<float,glm::vec2,float,float,glm::vec2,float> closestPointAt(glm::vec2 P, float t) {
        auto const clampedT = std::clamp(t, 0.0f, 1.0f);
        auto const clampedRay = rayAt(P, clampedT);
        auto const clampedDistance = glm::dot(clampedRay, clampedRay);

        glm::vec2 ray;
        float distance;
        if (clampedT == t) {
            ray = clampedRay;
            distance = clampedDistance;
        } else {
            ray = rayAt(P, t);
            distance = glm::dot(ray, ray);
        }
        return { t, ray, distance, clampedT, clampedRay, clampedDistance };
    }

    /*! Get the shortest distance from the point to the bezier curve
     * as if the bezier curve was extended to infinity.
     * prepare() must be called first.
     *
     * \return t, ray, distance, clampedT, clampedRay, clampedDistance, clampedDirection
     */
    ClosestPoint closestPoint(glm::vec2 P) {
        auto const pv = P - P0;

        auto const c = twoPv1DotPv1 - glm::dot(pv2, pv);
        auto const d = glm::dot(pv1, pv);

        auto const [t0, t1, t2] = solveCubic(a, b, c, d);

        auto [closestT, closestRay, closestDistance, closestClampedT, closestClampedRay, closestClampedDistance] = closestPointAt(P, t0);

        if (t1 != t0) {
            auto const [t, ray, distance, clampedT, clampedRay, clampedDistance] = closestPointAt(P, t1);
            if (clampedDistance < closestClampedDistance || (clampedDistance == closestClampedDistance && distance < closestDistance)) {
                closestT = t;
                closestRay = ray;
                closestDistance = distance;
                closestClampedT = clampedT;
                closestClampedRay = clampedRay;
                closestClampedDistance = clampedDistance;
            }
        }

        if (t2 != t0 && t2 != t1) {
            auto const [t, ray, distance, clampedT, clampedRay, clampedDistance] = closestPointAt(P, t2);
            if (clampedDistance < closestClampedDistance || (clampedDistance == closestClampedDistance && distance < closestDistance)) {
                closestT = t;
                closestRay = ray;
                closestDistance = distance;
                closestClampedT = clampedT;
                closestClampedRay = clampedRay;
                closestClampedDistance = clampedDistance;
            }
        }

        auto const direction = glm::normalize(directionAt(closestClampedT));
        auto const ray = glm::normalize(closestClampedRay);
        auto const orthogality = viktorCross(ray, direction);
        auto const inside = glm::dot(ray, direction);

        return {
            this,
            closestT,
            closestRay,
            closestDistance,
            closestClampedT,
            closestClampedRay,
            closestClampedDistance,
            orthogality,
            inside
        };
    }

};

}