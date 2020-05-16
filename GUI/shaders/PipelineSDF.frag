#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
    int subpixelOrientation; // 0:Unknown, 1:BlueRight, 2:BlueLeft, 3:BlueTop, 4:BlueBottom
} pushConstants;

layout(constant_id = 0) const float SDFmaxDistance = 1.0;
layout(constant_id = 1) const float atlastImageWidth = 1.0;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout(set = 0, binding = 1) uniform sampler biLinearSampler;
layout(set = 0, binding = 2) uniform texture2D textures[16];

layout(location = 0) in flat vec4 inClippingRectangle;
layout(location = 1) in vec3 inTextureCoord;
layout(location = 2) in flat vec4 inColor;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

bool isClipped()
{
    return greaterThanEqual(gl_FragCoord.xyxy, inClippingRectangle) == bvec4(true, false, false, true);
}

// Use a perceptional curve of gamma 2.0.
vec3 coverage_to_alpha(vec3 x, bool light_to_dark)
{
    if (light_to_dark) {
        return x * x;
    } else {
        x = 1.0 - x;
        return 1.0 - (x * x);
    }
}

// Use a perceptional curve of gamma 2.0.
float coverage_to_alpha(float x, bool light_to_dark)
{
    if (light_to_dark) {
        return x * x;
    } else {
        x = 1.0 - x;
        return 1.0 - (x * x);
    }
}

void main()
{
    if (isClipped()) {
        discard;
    }

    // The amount of distance and direction covered in 2D inside the texture when
    // stepping one fragment to the right.
    vec2 horizontalTextureStride = dFdxFine(inTextureCoord.xy);
    vec2 verticalTextureStride = vec2(-horizontalTextureStride.y, horizontalTextureStride.x);
    float pixelDistance = length(horizontalTextureStride);

    float distanceMultiplier = SDFmaxDistance / (pixelDistance * atlastImageWidth);

    float greenRadius = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), inTextureCoord.xy).r * distanceMultiplier;

    if (greenRadius < -0.7071067811865476) {
        // Fully outside the fragment, early exit.
        discard;
        //outColor = vec4(1.0, 1.0, 0.0, 1.0);
        //return;

    } else if (pushConstants.subpixelOrientation == 0) {
        // Normal anti-aliasing.
        float coverage = clamp(greenRadius + 0.5, 0.0, 1.0);
        float alpha = coverage_to_alpha(coverage, inColor.g > 0.7) * inColor.a;

        // Output alpha is always 1.0
        outColor = vec4(
            inColor.rgb * alpha + subpassLoad(inputColor).rgb * (1.0 - alpha),
            1.0 
        );

    } else {
        vec2 redOffset;
        vec2 blueOffset;

        switch (pushConstants.subpixelOrientation) {
        case 1: // Red-left, Blue-right
            redOffset = horizontalTextureStride / -3.0;
            blueOffset = horizontalTextureStride / 3.0;
            break;
        case 2: // Blue-left, Red-right
            blueOffset = horizontalTextureStride / -3.0;
            redOffset = horizontalTextureStride / 3.0;
            break;
        case 3: // Red-bottom, Blue-top
            redOffset = verticalTextureStride / -3.0;
            blueOffset = verticalTextureStride / 3.0;
            break;
        case 4: // Blue-bottom, Red-top
            blueOffset = verticalTextureStride / -3.0;
            redOffset = verticalTextureStride / 3.0;
            break;
        }

        // Subpixel anti-aliasing
        vec2 redCoordinate = inTextureCoord.xy + redOffset;
        float redRadius   = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), redCoordinate).r * distanceMultiplier;

        vec2 blueCoordinate = inTextureCoord.xy + blueOffset;
        float blueRadius  = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), blueCoordinate).r * distanceMultiplier;

        vec3 RGBCoverage = clamp(vec3(redRadius, greenRadius, blueRadius) + 0.5, 0.0, 1.0);
        vec3 RGBAlpha = coverage_to_alpha(RGBCoverage, inColor.g > 0.7) * inColor.a;

        // Output alpha is always 1.0
        outColor = vec4(
            inColor.rgb * RGBAlpha + subpassLoad(inputColor).rgb * (1.0 - RGBAlpha),
            1.0 
        );
    }
}
