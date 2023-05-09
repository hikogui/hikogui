

#pragma once

#include "subpixel_orientation.hpp"
#include "pipeline_box_vertex.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "pipeline_alpha_vertex.hpp"
#include "../geometry/module.hpp"
#include "../vector_span.hpp"
#include <cstdlib>

namespace hi { inline namespace v1 {
class gfx_device;

struct gfx_draw_context {
    gfx_device *device = nullptr;
    vector_span<pipeline_box::vertex> box_vertices;
    vector_span<pipeline_image::vertex> image_vertices;
    vector_span<pipeline_SDF::vertex> sdf_vertices;
    vector_span<pipeline_alpha::vertex> alpha_vertices;
    aarectanglei scissor_rectangle = {};
    size_t frame_buffer_index = 0;
    hi::subpixel_orientation subpixel_orientation = hi::subpixel_orientation::unknown;

    constexpr gfx_draw_context(gfx_draw_context const&) noexcept = delete;
    constexpr gfx_draw_context(gfx_draw_context&&) noexcept = default;
    constexpr gfx_draw_context& operator=(gfx_draw_context const&) noexcept = delete;
    constexpr gfx_draw_context& operator=(gfx_draw_context&&) noexcept = default;

    constexpr gfx_draw_context(
        gfx_device& device,
        std::span<pipeline_box::vertex> box_vertices,
        std::span<pipeline_image::vertex> image_vertices,
        std::span<pipeline_SDF::vertex> sdf_vertices,
        std::span<pipeline_alpha::vertex> alpha_vertices,
        size_t frame_buffer_index,
        aarectanglei scissor_rectangle) noexcept :
        device(std::addressof(device)),
        box_vertices(box_vertices),
        image_vertices(image_vertices),
        sdf_vertices(sdf_vertices),
        alpha_vertices(alpha_vertices),
        frame_buffer_index(frame_buffer_index),
        scissor_rectangle(scissor_rectangle)
    {
    }
};

}} // namespace hi::v1
