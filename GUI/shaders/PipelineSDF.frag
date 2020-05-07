#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
    vec2 blueOffset;
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

    float pixelDistance = fwidth(inTextureCoord.x);
    float distanceMultiplier = SDFmaxDistance / (pixelDistance * atlastImageWidth);

    float greenRadius = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), inTextureCoord.xy).r * distanceMultiplier;

    if (greenRadius < -0.7071067811865476) {
        // Fully outside the fragment, early exit.
        discard;

    } else if (pushConstants.blueOffset == vec2(0.0, 0.0)) {
        // Normal anti-aliasing.
        float coverage = clamp(greenRadius + 0.5, 0.0, 1.0);
        float alpha = coverage_to_alpha(coverage, inColor.g > 0.7) * inColor.a;

        // Output alpha is always 1.0
        outColor = vec4(
            inColor.rgb * alpha + subpassLoad(inputColor).rgb * (1.0 - alpha),
            1.0 
        );

    } else {
        vec2 bluePixelOffset = pixelDistance * pushConstants.blueOffset;

        // Subpixel anti-aliasing
        vec2 redCoordinate = inTextureCoord.xy - bluePixelOffset;
        float redRadius   = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), redCoordinate).r * distanceMultiplier;

        vec2 blueCoordinate = inTextureCoord.xy + bluePixelOffset;
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
