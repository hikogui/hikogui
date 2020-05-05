#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout(set = 0, binding = 1) uniform sampler biLinearSampler;
layout(set = 0, binding = 2) uniform texture2D textures[16];

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

    vec3 rgb_coverage = clamp(rgb_radius + 0.45, 0.0, 1.0);
  
    vec3 rgb_alpha = coverage_to_alpha(rgb_coverage, inColor.g > 0.7) * inColor.a;

    // Output alpha is always 1.0
    outColor = vec4(
        inColor.rgb * rgb_alpha + subpassLoad(inputColor).rgb * (1.0 - rgb_alpha),
        1.0
    );
}
