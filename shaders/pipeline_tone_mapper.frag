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
    
    vec4 color = original_color;

    if (original_color.a < 0.0) {
        // Alpha below 0.0 is reserved.
        discard;

    } else if (original_color.a < 1.0) {
        // Punch a hole in the user interface.
        // The alpha was set by the pipeline_alpha shader, which left the color alone,
        // make sure the color gets pre-multiplied by the alpha before passing to the vulkan-alpha-blending.
        color = vec4(original_color.rgb * original_color.a, original_color.a);

        // Clamp colors with sRGB range.
        out_color = clamp(color, vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0));

    } else if (original_color.a == 1.0) {
        // Normal user-interface.
        if (pushConstants.saturation < 1.0) {
            // Desaturate the color of the user-interface, used when the window becomes inactive.
            float y = rgb_to_y(original_color.rgb);
            vec4 greyscale_color = vec4(y, y, y, original_color.a);
            color = mix(greyscale_color, original_color, pushConstants.saturation);
        }

        // Clamp colors with sRGB range.
        out_color = clamp(color, vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0));

    } else {
        // Alpha values above 1.0 are reserved.
        discard;
    }
}
