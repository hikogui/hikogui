#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
} constants;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_clipping_rectangle;
layout(location = 2) in vec4 in_edge_distances;
layout(location = 3) in vec4 in_corner_radii;
layout(location = 4) in vec4 in_fill_color;
layout(location = 5) in vec4 in_border_color;
layout(location = 6) in float in_border_width;

layout(location = 0) out flat vec4 out_clipping_rectangle;
layout(location = 1) out vec4 out_edge_distances;
layout(location = 2) out vec4 out_fill_color;
layout(location = 3) out vec4 out_border_color;
layout(location = 4) out float out_border_sqrt_y;
layout(location = 5) out flat vec4 out_corner_radii;
layout(location = 6) out flat float out_border_start;
layout(location = 7) out flat float out_border_end;

#include "utils.glsl"

vec4 convert_position_to_viewport(vec3 window_position)
{
    float x = window_position.x * constants.viewport_scale.x - 1.0;
    float y = (constants.window_extent.y - window_position.y) * constants.viewport_scale.y - 1.0;
    // Get reverse-z to work, where window_position.z = 0.0 is far.
    return vec4(x, y, 1.0 - window_position.z * 0.01, 1.0);
}

vec4 convert_clipping_rectangle_to_screen(vec4 clipping_rectangle)
{
    return vec4(
        clipping_rectangle.x,
        constants.window_extent.y - clipping_rectangle.w,
        clipping_rectangle.z,
        constants.window_extent.y - clipping_rectangle.y
    );
}

void main()
{
    float border_start = 1.0;
    float border_middle = border_start + in_border_width * 0.5;
    float border_end = border_start + in_border_width;

    vec4 fill_color = multiply_alpha(in_fill_color);
    vec4 border_color = multiply_alpha(in_border_color);

    gl_Position = convert_position_to_viewport(in_position.xyz);
    out_clipping_rectangle = convert_clipping_rectangle_to_screen(in_clipping_rectangle);
    out_edge_distances = in_edge_distances;
    out_fill_color = fill_color;
    out_border_color = border_color;
    out_border_sqrt_y = sqrt(clamp(rgb_to_y(border_color.rgb), 0.0, 1.0));
    out_corner_radii = in_corner_radii + border_middle;
    out_border_start = border_start;
    out_border_end = border_end;
}
