#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inClippingRectangle;
layout(location = 3) in vec4 inColor;

layout(location = 0) out flat vec4 outClippingRectangle;
layout(location = 2) out vec4 outColor;

vec4 convertPositionToViewport(vec3 windowPosition)
{
    float x = windowPosition.x * pushConstants.viewportScale.x - 1.0;
    float y = (pushConstants.windowExtent.y - windowPosition.y) * pushConstants.viewportScale.y - 1.0;
    // Get reverse-z to work, where windowsPosition.z = 0.0 is far.
    return vec4(x, y, 1.0 - windowPosition.z, 1.0);
}

vec4 convertClippingRectangleToScreen(vec4 clippingRectangle)
{
    return vec4(
        clippingRectangle.x,
        pushConstants.windowExtent.y - clippingRectangle.w,
        clippingRectangle.z,
        pushConstants.windowExtent.y - clippingRectangle.y
    );
}

void main() {
    gl_Position = convertPositionToViewport(inPosition);
    outClippingRectangle = convertClippingRectangleToScreen(inClippingRectangle);
    outColor = inColor;
}
