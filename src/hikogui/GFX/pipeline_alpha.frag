#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in float in_alpha;

layout(location = 0) out vec4 out_color;

#include "utils.glsl"

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }
        
    out_color = vec4(0.0, 0.0, 0.0, in_alpha);
}
