#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
    int subpixel_orientation; // 0:Unknown, 1:BlueRight, 2:BlueLeft, 3:BlueTop, 4:BlueBottom
} pushConstants;

layout(constant_id = 0) const float sdf_max_distance = 1.0;
layout(constant_id = 1) const float atlas_image_width = 1.0;

layout(set = 0, binding = 0) uniform sampler in_sampler;
layout(set = 0, binding = 1) uniform texture2D in_textures[16];

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec3 in_texture_coord;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec4 in_color_sqrt;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_blend_factor;

#include "utils.glsl"

/** Get the horizontal and vertical stride in a texture map from one fragment to the next.
 *
 * @return The horizontal texture stride in (x,y) and the vertical texture stride in (z,w).
 */
vec4 get_texture_stride()
{
    // The amount of distance and direction covered in 2D inside the texture when
    // stepping one fragment to the right.
    vec2 horizontal_texture_stride = dFdxCoarse(in_texture_coord.xy);
    vec2 vertical_texture_stride = vec2(-horizontal_texture_stride.y, horizontal_texture_stride.x);
    return vec4(horizontal_texture_stride, vertical_texture_stride);
}

/** Get the red and blue subpixel offsets.
 * @return The red subpixel offset in (x,y), the blue subpixel offset in (z,w)
 */
vec4 get_red_blue_subpixel_offset(vec4 texture_stride) {
    switch (pushConstants.subpixel_orientation) {
    case 1: // Red-left, Blue-right
        return vec4(
            texture_stride.xy / -3.0,
            texture_stride.xy / 3.0);

    case 2: // Blue-left, Red-right
        return vec4(
            texture_stride.xy / 3.0,
            texture_stride.xy / -3.0);

    case 3: // Red-bottom, Blue-top
        return vec4(
            texture_stride.zw / -3.0,
            texture_stride.zw / 3.0);

    case 4: // Blue-bottom, Red-top
        return vec4(
            texture_stride.zw / 3.0,
            texture_stride.zw / -3.0);

    default:
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
}

/** Get the distance from the sub-pixels to the nearest edge.
 *
 * @return Distances to the edge from the center of the red, green and blue sub-pixels.
 */
vec3 get_subpixel_to_edge_distances()
{
    float green_distance = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), in_texture_coord.xy).r;

    vec3 distances = vec3(green_distance, green_distance, green_distance);
    vec4 texture_stride = get_texture_stride();
    if (pushConstants.subpixel_orientation != 0) {
        vec4 rb_coords = in_texture_coord.xyxy + get_red_blue_subpixel_offset(texture_stride);

        distances.r = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), rb_coords.xy).r;
        distances.b = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), rb_coords.zw).r;
    }

    float pixel_distance = length(texture_stride.xy);
    float distance_multiplier = sdf_max_distance / (pixel_distance * atlas_image_width);
    return distances * distance_multiplier;
}

/** Convert coverage to a perceptional uniform alpha.
 *
 * This function takes into account the lightness of the full pixel, then
 * determines based on this if the background is either black or white, or
 * if linear conversion of coverage to alpha is needed.
 *
 * On black and white background we measure the target lightness of each sub-pixel
 * then convert to target luminosity and eventually the alpha value.
 *
 * The alpha-component of the return value is calculated based on the full pixel
 * lightness and from the green sub-pixel coverage.
 *
 * @param coverage The amount of coverage on the red, green and blue sub-pixels.
 * @return The alpha value for the red, blue, green, alpha color components.
 */
vec4 alpha_from_coverage(vec4 coverage)
{
    if (in_color_sqrt.a > 0.6) {
        // The full formula to anti-alias perceptional uniform light text on a black background:
        //     F = in_color_sqrt
        //     c = coverage
        //     alpha = (F * c)^2 / F^2
        //
        // Simplified, removing the division:
        //     alpha = c^2

        return coverage * coverage;

    } else if (in_color_sqrt.a < 0.4) {
        // The full formula to anti-alias perceptional uniform dark text on a white background:
        //     F = in_color_sqrt
        //     c = coverage
        //     alpha = (((1 - c) + (F * c))^2 - 1) / (F^2 - 1)
        //
        // We can use the following formula to approximate, which also removes the division:
        //     alpha = c(1 + K - Kc)
        //     K = 0.7 * F^2 - 1.7 * F + 1

        vec4 K = 0.7 * (in_color_sqrt * in_color_sqrt) - 1.7 * in_color_sqrt + 1.0;
        return coverage * (1.0 + K - K * coverage);

    } else {
        // The brightness of the foreground and background are similar.
        // Use the coverage value directly.
        return coverage;
    }
}

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }

    vec3 distances = get_subpixel_to_edge_distances();
    vec3 coverage = clamp(distances + 0.5, 0.0, 1.0);

    vec4 alpha = alpha_from_coverage(coverage.rgbg);

    // Properly set both the out_color.a and the out_blend_factor, this makes this
    // shader compatible with or without the dualSrcBlend feature.
    out_color = vec4(in_color * alpha);
    out_blend_factor = in_color.a * alpha;
}
