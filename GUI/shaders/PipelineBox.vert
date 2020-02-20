#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inClippingRectangle;
layout(location = 2) in vec4 inCornerCoordinates;
layout(location = 3) in vec4 inBackgroundColor;
layout(location = 4) in vec4 inBorderColor;
layout(location = 5) in vec4 inCornerShapes;
layout(location = 6) in float inBorderSize;
layout(location = 7) in float inShadowSize;

layout(location = 0) out vec4 outClippingRectangle;
layout(location = 1) out vec4 outCornerCoordinates;
layout(location = 2) out vec4 outBackgroundColor;
layout(location = 3) out vec4 outBorderColor;
layout(location = 4) out vec4 outCornerShapes;
layout(location = 5) out vec4 outAbsCornerShapes;
layout(location = 6) out float outShadowSize;
layout(location = 7) out float outOneOverShadowSize;
layout(location = 8) out float outBorderStart;
layout(location = 9) out float outBorderEnd;

vec4 convertPositionToViewport(vec3 windowPosition)
{
    float x = windowPosition.x * pushConstants.viewportScale.x - 1.0;
    float y = (pushConstants.windowExtent.y - windowPosition.y) * pushConstants.viewportScale.y - 1.0;
    return vec4(x, y, windowPosition.z, 1.0);
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
    float shadowStart = 1.0;
    float shadowEnd = shadowStart + inShadowSize;
    float borderStart = shadowEnd;
    float borderMiddle = borderStart + inBorderSize * 0.5;
    float borderEnd = borderStart + inBorderSize;

    gl_Position = convertPositionToViewport(inPosition);
    outClippingRectangle = convertClippingRectangleToScreen(inClippingRectangle);
    outCornerCoordinates = inCornerCoordinates;
    outBackgroundColor = inBackgroundColor;
    outBorderColor = inBorderColor;
    outCornerShapes = inCornerShapes;
    outAbsCornerShapes = abs(inCornerShapes) + vec4(borderMiddle, borderMiddle, borderMiddle, borderMiddle);
    outShadowSize = inShadowSize;
    outOneOverShadowSize = 1.0 / inShadowSize;
    outBorderStart = borderStart;
    outBorderEnd = borderEnd;
}
