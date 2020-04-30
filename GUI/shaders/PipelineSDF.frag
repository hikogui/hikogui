#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler biLinearSampler;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in flat vec4 inClippingRectangle;
layout(location = 1) in vec3 inTextureCoord;
layout(location = 2) in flat vec4 inColor;
layout(location = 3) in flat float inDistanceMultiplier;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

bool isClipped()
{
    return greaterThanEqual(gl_FragCoord.xyxy, inClippingRectangle) == bvec4(true, false, false, true);
}

void main()
{
    if (isClipped()) {
        discard;
        //outColor = vec4(1.0, 1.0, 0.0, 1.0); return;
    }

    // Distance in screen coordinates from closest edge.
    float distance = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), inTextureCoord.xy).r * inDistanceMultiplier;

    // Mathamatically we should add 0.5 to the distance to get the antialiasing to be centered on the edge of
    // the glyph. However glyphs are designed to be drawn black-on-white, using linear alpha compositing.
    // By using a smaller value we make the glyph slightly thinner, so that our perceptual alpha compositing
    // will draw the font at the correct thickingness.
    float alpha = clamp(distance + 0.42, 0.0, 1.0);

    // Although alpha compositing needs to be done linearilly on colors,
    // the alpha value itself should be calculated perceptually (non-linear).
    // We are using a gamma of 2 because it is fast.
    // This makes dark on light and light on dark text have the same thickness.
    if (inColor.g > 0.75) {
        alpha = alpha * alpha;
    } else if (inColor.g < 0.25) {
        alpha = 1.0 - alpha;
        alpha = alpha * alpha;
        alpha = 1.0 - alpha;
        //alpha = sqrt(alpha);
    }

    if (alpha > 0.0) {
        outColor = inColor * alpha;
    } else {
        discard;
    }
}
