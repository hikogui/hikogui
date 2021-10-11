#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
    int subpixel_orientation;
} pushConstants;

// In position is in window pixel position, with left-bottom origin.
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_clipping_rectangle;
layout(location = 2) in vec3 in_texture_coord;
layout(location = 3) in vec4 in_color_rgb;

layout(location = 0) out flat vec4 out_clipping_rectangle;
layout(location = 1) out noperspective vec3 out_texture_coord;
layout(location = 2) out noperspective vec4 out_color_luv;

#include "utils.glsl"

vec4 convert_position_to_viewport(vec3 window_position)
{
    float x = window_position.x * pushConstants.viewport_scale.x - 1.0;
    float y = (pushConstants.window_extent.y - window_position.y) * pushConstants.viewport_scale.y - 1.0;
    // Get reverse-z to work, where windows_position.z = 0.0 is far.
    return vec4(x, y, 1.0 - window_position.z * 0.01, 1.0);
}

vec4 convert_clipping_rectangle_to_screen(vec4 clipping_rectangle)
{
    return vec4(
        clipping_rectangle.x,
        pushConstants.window_extent.y - clipping_rectangle.w,
        clipping_rectangle.z,
        pushConstants.window_extent.y - clipping_rectangle.y
    );
}

void main() {
    gl_Position = convert_position_to_viewport(in_position);
    out_clipping_rectangle = convert_clipping_rectangle_to_screen(in_clipping_rectangle);
    out_texture_coord = in_texture_coord;

    // Do not pre-multiply the alpha due to subpixel compositing. 
    out_color_luv = rgb_to_tluv(in_color_rgb);
}
