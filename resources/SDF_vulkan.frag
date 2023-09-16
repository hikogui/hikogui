#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
    vec2 red_subpixel_orientation;
    vec2 blue_subpixel_orientation;
    bool has_subpixels;
} pushConstants;

layout(constant_id = 0) const float sdf_max_distance = 1.0;
layout(constant_id = 1) const float atlas_image_width = 1.0;

layout(set = 0, binding = 0) uniform sampler in_sampler;
layout(set = 0, binding = 1) uniform texture2D in_textures[128];

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec3 in_texture_coord;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec4 in_color_sqrt_rgby;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_blend_factor;

#include "utils_vulkan.glsl"

/** Get the horizontal and vertical stride in a texture map from one fragment to the next.
 *
 * @return The horizontal texture stride in (x,y) and the vertical texture stride in (z,w).
 */
vec4 get_texture_stride()
{
    // We are calculating both the horizontal and vertical stride to handle non-uniform scaling,
    // shearing and perspective transforms.

    // The amount of distance and direction covered in 2D inside the texture when
    // stepping one fragment to the right and stepping one fragment up.
    vec2 horizontal_texture_stride = dFdxCoarse(in_texture_coord.xy);
    vec2 vertical_texture_stride = dFdyCoarse(in_texture_coord.xy);
    return vec4(horizontal_texture_stride, vertical_texture_stride);
}

vec2 green_coord(vec4 texture_stride, vec2 coord)
{
    return coord;
}

vec2 red_coord(vec4 texture_stride, vec2 coord)
{
    vec4 tmp = texture_stride * pushConstants.red_subpixel_orientation.xxyy;
    return coord + tmp.xy + tmp.zw;
}

vec2 blue_coord(vec4 texture_stride, vec2 coord)
{
    vec4 tmp = texture_stride * pushConstants.blue_subpixel_orientation.xxyy;
    return coord + tmp.xy + tmp.zw;
}

/** Get the distance from the sub-pixels to the nearest edge.
 *
 * @return Distances to the edge from the center of the red, green and blue sub-pixels.
 */
vec3 get_subpixel_to_edge_distances()
{
    int image_nr = int(in_texture_coord.z);
    vec2 image_coord = in_texture_coord.xy;
    vec4 texture_stride = get_texture_stride();

    float green_distance = texture(sampler2D(in_textures[image_nr], in_sampler), green_coord(texture_stride, image_coord)).r;

    vec3 distances = vec3(green_distance, green_distance, green_distance);
    if (pushConstants.has_subpixels) {
        distances.r = texture(sampler2D(in_textures[image_nr], in_sampler), red_coord(texture_stride, image_coord)).r;
        distances.b = texture(sampler2D(in_textures[image_nr], in_sampler), blue_coord(texture_stride, image_coord)).r;
    }

    float pixel_distance = length(texture_stride.xy);
    float distance_multiplier = sdf_max_distance / (pixel_distance * atlas_image_width);
    return distances * distance_multiplier;
}

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }

    vec3 distances = get_subpixel_to_edge_distances();
    vec3 coverage = clamp(distances + 0.5, 0.0, 1.0);
    if (coverage == vec3(0.0, 0.0, 0.0)) {
        discard;
    }

    vec4 alpha = coverage_to_alpha(coverage.rgbg, in_color_sqrt_rgby);

    // Properly set both the out_color.a and the out_blend_factor, this makes this
    // shader compatible with or without the dualSrcBlend feature.
    out_color = vec4(in_color * alpha);
    out_blend_factor = in_color.a * alpha;
}
