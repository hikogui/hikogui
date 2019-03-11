#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inAtlasPosition;
layout(location = 2) in float inAlpha;

layout(location = 0) out vec3 outAtlasPosition;
layout(location = 1) out float outAlpha;

vec3 convertToViewport(vec3 windowPosition) {
    vec2 viewportPosition = (windowPosition.xy * pushConstants.viewportScale) - vec2(1.0, 1.0);
    return vec3(viewportPosition, windowPosition.z);
}

void main() {
    vec3 viewportPosition = convertToViewport(inPosition);

    gl_Position = vec4(viewportPosition, 1.0);
    outAtlasPosition = inAtlasPosition;
    outAlpha = inAlpha;
}
