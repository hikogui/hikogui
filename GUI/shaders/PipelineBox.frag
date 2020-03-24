#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) in flat vec4 inClippingRectangle;
layout(location = 1) in vec4 inCornerCoordinates;
layout(location = 2) in flat vec4 inBackgroundColor;
layout(location = 3) in flat vec4 inBorderColor;
layout(location = 4) in flat uvec4 inCornerShapes;
layout(location = 5) in flat vec4 inCornerRadii;
layout(location = 6) in flat float inShadowSize;
layout(location = 7) in flat float inOneOverShadowSize;
layout(location = 8) in flat float inBorderStart;
layout(location = 9) in flat float inBorderEnd;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;


bool isClipped()
{
    return greaterThanEqual(gl_FragCoord.xyxy, inClippingRectangle) == bvec4(true, false, false, true);
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

float roundedDistance(vec2 distance2D, float cornerRadius)
{
    float x = cornerRadius - distance2D.x;
    float y = cornerRadius - distance2D.y;
    return cornerRadius - sqrt(x*x + y*y) ;
}

float cutDistance(vec2 distance2D, float cornerRadius)
{
    return distance2D.x + distance2D.y - cornerRadius;
}

float edgeDistance(vec2 distance2D)
{
    return min(distance2D.x, distance2D.y);
}

float cornerDistance(vec2 distance2D, float cornerRadius, uint shape)
{
    switch (shape) {
    case 1: return roundedDistance(distance2D, cornerRadius);
    case 2: return cutDistance(distance2D, cornerRadius);
    default: return edgeDistance(distance2D);
    }
}

bool insideBottomLeftCorner(float maximumDistanceFromCorner)
{
    return inCornerCoordinates.x < maximumDistanceFromCorner && inCornerCoordinates.y < maximumDistanceFromCorner;
}

bool insideBottomRightCorner(float maximumDistanceFromCorner)
{
    return inCornerCoordinates.z < maximumDistanceFromCorner && inCornerCoordinates.y < maximumDistanceFromCorner;
}

bool insideTopLeftCorner(float maximumDistanceFromCorner)
{
    return inCornerCoordinates.x < maximumDistanceFromCorner && inCornerCoordinates.w < maximumDistanceFromCorner;
}

bool insideTopRightCorner(float maximumDistanceFromCorner)
{
    return inCornerCoordinates.z < maximumDistanceFromCorner && inCornerCoordinates.w < maximumDistanceFromCorner;
}

float shapeAdjustedDistance(vec2 distance2D)
{
    if (insideBottomLeftCorner(inCornerRadii.x)) {
        return cornerDistance(distance2D, inCornerRadii.x, inCornerShapes.x);
    } else if (insideBottomRightCorner(inCornerRadii.y)) {
        return cornerDistance(distance2D, inCornerRadii.y, inCornerShapes.y);
    } else if (insideTopLeftCorner(inCornerRadii.z)) {
        return cornerDistance(distance2D, inCornerRadii.z, inCornerShapes.z);
    } else if (insideTopRightCorner(inCornerRadii.w)) {
        return cornerDistance(distance2D, inCornerRadii.w, inCornerShapes.w);
    } else {
        return edgeDistance(distance2D);
    }
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

    float distance = shapeAdjustedDistance(distance2D);
    
    float background = clamp(distance - inBorderEnd + 0.5, 0.0, 1.0);
    float border = clamp(distance - inBorderStart + 0.5, 0.0, 1.0);

    float shadow = 0.0;
    if (border < 1.0 && inShadowSize >= 1.0) {
        // Only calculate the shadow when we are on the border or beyond.
        shadow = clamp(smoothShadow(distance - inShadowSize, inOneOverShadowSize), 0.0, 1.0);
    }

    vec4 shadowColor = vec4(0.0, 0.0, 0.0, shadow);
    vec4 borderColor = inBorderColor * border;
    vec4 backgroundColor = inBackgroundColor * background;

    vec4 tmpColor = borderColor + shadowColor * (1.0 - border);
    outColor = backgroundColor + tmpColor * (1.0 - background);
}
