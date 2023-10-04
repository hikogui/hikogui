#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec4 in_blend_factor;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_blend_factor;

#include "utils_vulkan.glsl"

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }
        
    out_color = in_color;
    out_blend_factor = in_blend_factor;
}
