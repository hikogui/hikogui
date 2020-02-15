#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

// In position is in window pixel position, with left-bottom origin.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTextureCoord;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inDistanceMultiplier;

layout(location = 0) out vec3 outTextureCoord;
layout(location = 1) out vec4 outColor;
layout(location = 2) out float outDistanceMultiplier;

vec3 convertToViewport(vec3 windowPosition) {
    float x = windowPosition.x * pushConstants.viewportScale.x - 1.0;
    float y = (pushConstants.windowExtent.y - windowPosition.y) * pushConstants.viewportScale.y - 1.0;
    return vec3(x, y, windowPosition.z);
}

void main() {
    gl_Position = vec4(convertToViewport(inPosition), 1.0);
    outTextureCoord = inTextureCoord;
    outColor = inColor;
    outDistanceMultiplier = inDistanceMultiplier;
}
