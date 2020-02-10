// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/A2B10G10R10UNorm.hpp"

namespace TTauri {

/** A pixel of a multi-channel signed distance field.
 * https://github.com/Chlumsky/msdfgen
 * https://github.com/Chlumsky/msdfgen/files/3050967/thesis.pdf
 *
 * Since multichannel distance field require 3 channels, and
 * Vulkan textures want to use 4 bytes per pixel, we use the
 * A2B10G10R10_UNorm format to trade the unused alpha channel
 * for extra precision of R, G & B.
 */
struct MSD10 : public A2B10G10R10UNorm {
    /** Max distance in pixels represented by a channel.
     */
    constexpr static float max_distance = 4.0f;

    // Multiplier to fit a signed distance in a range between 0.0 and 1.0.
    constexpr static float from_multiplier = (max_distance * 2.0f);
    constexpr static float to_multiplier = 1.0f / from_multiplier;

    MSD10() noexcept = default;
    MSD10(MSD10 const &other) noexcept = default;
    MSD10(MSD10 &&other) noexcept = default;
    MSD10 &operator=(MSD10 const &other) noexcept = default;
    MSD10 &operator=(MSD10 &&other) noexcept = default;
    ~MSD10() = default;

    MSD10(glm::vec3 const &rhs) noexcept :
        A2B10G10R10UNorm(rhs * to_multiplier + 0.5f) {}

    MSD10(float r, float g, float b) noexcept :
        MSD10(glm::vec3{r, g, b}) {}

    MSD10 &operator=(glm::vec3 const &rhs) noexcept {
        A2B10G10R10UNorm::operator=(rhs * to_multiplier + 0.5f);
        return *this;
    }

    operator glm::vec3 () const noexcept {
        return ((A2B10G10R10UNorm::operator glm::vec3()) - 0.5f) * from_multiplier;
    }
};

}