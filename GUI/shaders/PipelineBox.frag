#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in vec4 inClippingRectangle;
layout(location = 1) in vec4 inCornerCoordinates;
layout(location = 2) in vec4 inBackgroundColor;
layout(location = 3) in vec4 inBorderColor;
layout(location = 4) in vec4 inCornerShapes;
layout(location = 5) in vec4 inAbsCornerShapes;
layout(location = 6) in float inShadowSize;
layout(location = 7) in float inOneOverShadowSize;
layout(location = 8) in float inBorderStart;
layout(location = 9) in float inBorderEnd;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;


bool isClipped()
{
    return (
        gl_FragCoord.x < inClippingRectangle.x ||
        gl_FragCoord.x > inClippingRectangle.z ||
        gl_FragCoord.y < inClippingRectangle.y ||
        gl_FragCoord.y > inClippingRectangle.w
    );
}

float erf(float x) {
  float s = sign(x);
  float a = abs(x);

  x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
  x *= x;
  return s - s / (x * x);
}

float smoothShadow(float distance, float oneOverShadowSize)
{
    return 0.5 + 0.5 * erf(distance * oneOverShadowSize);
}

float roundedDistance(vec2 distance2D, float size)
{
    float x = size - distance2D.x;
    float y = size - distance2D.y;
    return size - sqrt(x*x + y*y) ;
}

float cutDistance(vec2 distance2D, float size)
{
    return distance2D.x + distance2D.y - size;
}

float cornerDistance(vec2 distance2D)
{
    return min(distance2D.x, distance2D.y);
}

void main() {
    if (isClipped()) {
        discard;
        //outColor = vec4(1.0, 1.0, 0.0, 1.0); return;
    }
        
    vec2 distance2D = vec2(
        min(inCornerCoordinates.x, inCornerCoordinates.z),
        min(inCornerCoordinates.y, inCornerCoordinates.w)
    );

    float distance = cornerDistance(distance2D);
    if (inCornerCoordinates.x < inAbsCornerShapes.x && inCornerCoordinates.y < inAbsCornerShapes.x) {
        // Left bottom corner.
        if (inCornerShapes.x > 0.1) {
            distance = roundedDistance(distance2D, inAbsCornerShapes.x);
        } else if (inCornerShapes.x < -0.1) {
            distance = cutDistance(distance2D, inAbsCornerShapes.x);
        }
    } else if (inCornerCoordinates.z < inAbsCornerShapes.y && inCornerCoordinates.y < inAbsCornerShapes.y) {
        // Right bottom corner.
        if (inCornerShapes.y > 0.1) {
            distance = roundedDistance(distance2D, inAbsCornerShapes.y);
        } else if (inCornerShapes.y < -0.1) {
            distance = cutDistance(distance2D, inAbsCornerShapes.y);
        }
    } else if (inCornerCoordinates.x < inAbsCornerShapes.z && inCornerCoordinates.w < inAbsCornerShapes.z) {
        // Left top corner.
        if (inCornerShapes.z > 0.1) {
            distance = roundedDistance(distance2D, inAbsCornerShapes.z);
        } else if (inCornerShapes.z < -0.1) {
            distance = cutDistance(distance2D, inAbsCornerShapes.z);
        }
    } else if (inCornerCoordinates.z < inAbsCornerShapes.w && inCornerCoordinates.w < inAbsCornerShapes.w) {
        // Right top corner.
        if (inCornerShapes.w > 0.1) {
            distance = roundedDistance(distance2D, inAbsCornerShapes.w);
        } else if (inCornerShapes.w < -0.1) {
            distance = cutDistance(distance2D, inAbsCornerShapes.w);
        }
    }

    float background = clamp(distance - inBorderEnd + 0.5, 0.0, 1.0);
    float border = clamp(distance - inBorderStart + 0.5, 0.0, 1.0);

    float shadow = 0.0;
    if (border < 1.0) {
        // Only calculate the shadow when we are on the border or beyond.
        shadow = clamp(smoothShadow(distance - inShadowSize, inOneOverShadowSize), 0.0, 1.0);
    }

    vec4 shadowColor = vec4(0.0, 0.0, 0.0, shadow);
    vec4 borderColor = inBorderColor * border;
    vec4 backgroundColor = inBackgroundColor * background;

    vec4 tmpColor = borderColor + shadowColor * (1.0 - border);
    outColor = backgroundColor + tmpColor * (1.0 - background);
}
