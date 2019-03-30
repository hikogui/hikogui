#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uvec2 inClippingRectangleOffset;
layout(location = 2) in uvec2 inClippingRectangleExtent;
layout(location = 3) in uvec3 inAtlasPosition;
layout(location = 4) in uint inDepth;
layout(location = 5) in uint inAlpha;

layout(location = 0) out vec3 outAtlasPosition;
layout(location = 1) out float outAlpha;

vec2 convertToViewport(vec2 windowPosition) {
    return (windowPosition.xy * pushConstants.viewportScale) - vec2(1.0, 1.0);
}

void main() {


    vec2 viewportPosition = convertToViewport(inPosition);

    gl_Position = vec4(viewportPosition, 0.0, 1.0);
    outAtlasPosition = inAtlasPosition;
    outAlpha = inAlpha;
}
