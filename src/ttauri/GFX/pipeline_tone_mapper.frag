#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    float saturation;
} pushConstants;

layout(location = 0) out vec4 out_color;
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput in_attachment;

#include "utils.glsl"

void main() {
    vec4 original_color = subpassLoad(in_attachment);
    
    vec4 saturated_color = original_color;
    if (pushConstants.saturation < 1.0 && original_color.a <= 1.0) {
        float y = rgb_to_y(original_color.rgb);
        vec4 greyscale_color = vec4(y, y, y, original_color.a);
        saturated_color = mix(greyscale_color, original_color, pushConstants.saturation);
    }

    out_color = saturated_color;
}
