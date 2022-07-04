#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
} constants;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_clipping_rectangle;
layout(location = 2) in float in_alpha;

layout(location = 0) out flat vec4 out_clipping_rectangle;
layout(location = 1) out flat float out_alpha;

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
    gl_Position = convert_position_to_viewport(in_position.xyz);
    out_clipping_rectangle = convert_clipping_rectangle_to_screen(in_clipping_rectangle);
    out_alpha = in_alpha;
}
