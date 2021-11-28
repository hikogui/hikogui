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
layout(location = 1) out vec4 out_blend_factor;;

#include "utils.glsl"

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

/** Get the red and blue subpixel coordinates.
 * @return The red subpixel coordinate in (x,y), the blue subpixel coordinate in (z,w)
 */
vec4 get_red_blue_subpixel_coord(vec4 texture_stride) {
    vec4 rb_offset = get_red_blue_subpixel_offset(texture_stride);
    return in_texture_coord.xyxy + rb_offset;
}

/** Get the green subpixel radius to nearest edge.
 * @return The green subpixel radius
 */
float get_green_subpixel_radius(float distance_multiplier) {
    return texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), in_texture_coord.xy).r * distance_multiplier;
}

/** Get the red and blue subpixel radius to nearest edge.
 * @return The red subpixel radius in x, the blue subpixel radius in y
 */
vec2 get_red_blue_subpixel_radius(float distance_multiplier, vec4 texture_stride) {
    vec4 rb_coords = get_red_blue_subpixel_coord(texture_stride);
    return vec2(
        texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), rb_coords.xy).r * distance_multiplier,
        texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), rb_coords.zw).r * distance_multiplier);
}

/** Get the horizontal and vertical stride in a texture map from one fragment to the next.
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

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }

    // The amount of distance and direction covered in 2D inside the texture when
    // stepping one fragment to the right.
    vec4 texture_stride = get_texture_stride();

    float pixel_distance = length(texture_stride.xy);
    float distance_multiplier = sdf_max_distance / (pixel_distance * atlas_image_width);

    float g_radius = get_green_subpixel_radius(distance_multiplier);

    if (g_radius < -0.5) {
        // Fully outside the fragment, early exit.
        discard;

    } else {
        if (pushConstants.subpixel_orientation == 0 || (in_color_sqrt.a > 0.4 && in_color_sqrt.a < 0.6)) {
            // Normal anti-aliasing.
            float coverage = clamp(g_radius + 0.5, 0.0, 1.0);
            out_color = in_color_rgb * coverage;
            out_blend_factor = vec4(coverage);

        } else {
            // Subpixel anti-aliasing
            vec2 rb_radius = get_red_blue_subpixel_radius(distance_multiplier, texture_stride);
            vec3 rgb_radius = vec3(rb_radius.x, g_radius, rb_radius.y);
            vec3 rgb_coverage = clamp(rgb_radius + 0.5, 0.0, 1.0);

            // XXX use green for alpha now.
            out_color = in_color_rgb * rgb_coverage.g;
            out_blend_factor = vec(rgb_coverage.rgb, rgb_coverage.g)
        }
    }
}

