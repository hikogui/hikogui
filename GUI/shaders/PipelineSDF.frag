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
    }

    float pixel_distance = fwidth(inTextureCoord.x);
    float sub_pixel_distance = pixel_distance / 3.0;
    vec2 sub_pixel_offset = vec2(sub_pixel_distance * 0.333333, 0.0);

    vec2 green_coordinate = inTextureCoord.xy;
    float green_radius = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), green_coordinate).r * inDistanceMultiplier;

    if (green_radius < -0.7071067811865476) {
        // Fully outside the fragment, early exit.
        discard;
    } // It is very rare to be fully inside, so don't test for this.

    vec2 red_coordinate = inTextureCoord.xy - sub_pixel_offset;
    float red_radius   = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), red_coordinate).r * inDistanceMultiplier;

    vec2 blue_coordinate = inTextureCoord.xy + sub_pixel_offset;
    float blue_radius  = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), blue_coordinate).r * inDistanceMultiplier;

    vec3 rgb_radius = vec3(red_radius, green_radius, blue_radius);
    vec3 rgb_coverage = clamp((rgb_radius * 1) + 0.5, 0.0, 1.0);
  
    if (inColor.g > 0.5) {
        rgb_coverage = rgb_coverage * rgb_coverage;
    } else {
        rgb_coverage = 1.0 - rgb_coverage;
        rgb_coverage = rgb_coverage * rgb_coverage;
        rgb_coverage = 1.0 - rgb_coverage;
    }

    // Turn off sub-pixel rendering.
    //rgb_coverage = vec3(rgb_coverage.g, rgb_coverage.g, rgb_coverage.g);

    vec3 color = inColor.rgb * rgb_coverage;
    float alpha = (rgb_coverage.r + rgb_coverage.g + rgb_coverage.b) / 3.0;
    outColor = vec4(color.rgb, alpha);

    // Although alpha compositing needs to be done linearilly on colors,
    // the alpha value itself should be calculated perceptually (non-linear).
    // We are using a gamma of 2 because it is fast.
    // This makes dark on light and light on dark text have the same thickness.
    //float alpha;
    //if (inColor.g > 0.5) {
    //    alpha = green_coverage * green_coverage;
    //} else {
    //    alpha = 1.0 - green_coverage;
    //    alpha = 1.0 - (alpha * alpha);
    //}
//
    //outColor = inColor * alpha;
}
