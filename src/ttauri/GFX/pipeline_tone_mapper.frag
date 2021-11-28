#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_color;
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput in_attachment;

void main() {
    out_color = subpassLoad(in_attachment);
}

