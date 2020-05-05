#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 outColor;
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

void main() {
    outColor = vec4(subpassLoad(inputColor).rgb, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
