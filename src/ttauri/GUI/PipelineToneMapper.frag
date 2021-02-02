#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput sdfInputColor;

void main() {
    vec4 sdf = subpassLoad(sdfInputColor);

    if (sdf.a == 1.0) {
        outColor = sdf;
    } else {
        outColor = vec4(subpassLoad(inputColor).rgb, 1.0);
    }
    //outColor = subpassLoad(inputColor);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
