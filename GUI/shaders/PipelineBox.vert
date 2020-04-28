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
layout(location = 5) in vec4 inCornerRadiiAndShapes;
layout(location = 6) in float inBorderSize;
layout(location = 7) in float inShadowSize;

layout(location = 0) out flat vec4 outClippingRectangle;
layout(location = 1) out vec4 outCornerCoordinates;
layout(location = 2) out flat vec4 outBackgroundColor;
layout(location = 3) out flat vec4 outBorderColor;
layout(location = 4) out flat uvec4 outCornerShapes;
layout(location = 5) out flat vec4 outCornerRadii;
layout(location = 6) out flat float outShadowSize;
layout(location = 7) out flat float outOneOverShadowSize;
layout(location = 8) out flat float outBorderStart;
layout(location = 9) out flat float outBorderEnd;

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
    float shadowStart = 1.0;
    float shadowEnd = shadowStart + inShadowSize;
    float borderStart = shadowEnd;
    float borderMiddle = borderStart + inBorderSize * 0.5;
    float borderEnd = borderStart + inBorderSize;

    gl_Position = convertPositionToViewport(inPosition);
    outClippingRectangle = convertClippingRectangleToScreen(inClippingRectangle);
    outCornerCoordinates = inCornerCoordinates;
    outBackgroundColor = vec4(inBackgroundColor.rgb * inBackgroundColor.a, inBackgroundColor.a);
    outBorderColor = vec4(inBorderColor.rgb * inBorderColor.a, inBorderColor.a);
    outCornerShapes = ivec4(
        (inCornerRadiiAndShapes.x > 0.1) ? 1 : (inCornerRadiiAndShapes.x < -0.1) ? 2 : 0,
        (inCornerRadiiAndShapes.y > 0.1) ? 1 : (inCornerRadiiAndShapes.y < -0.1) ? 2 : 0,
        (inCornerRadiiAndShapes.z > 0.1) ? 1 : (inCornerRadiiAndShapes.z < -0.1) ? 2 : 0,
        (inCornerRadiiAndShapes.w > 0.1) ? 1 : (inCornerRadiiAndShapes.w < -0.1) ? 2 : 0
    );
    outCornerRadii = abs(inCornerRadiiAndShapes) + vec4(borderMiddle, borderMiddle, borderMiddle, borderMiddle);
    outShadowSize = inShadowSize;
    outOneOverShadowSize = 1.0 / inShadowSize;
    outBorderStart = borderStart;
    outBorderEnd = borderEnd;
}
