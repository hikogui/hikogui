#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

void main() {
    return subpassLoad(inputColor);
}

