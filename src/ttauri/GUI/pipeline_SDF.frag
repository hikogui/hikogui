#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    vec2 window_extent;
    vec2 viewport_scale;
    int subpixel_orientation; // 0:Unknown, 1:BlueRight, 2:BlueLeft, 3:BlueTop, 4:BlueBottom
} pushConstants;

layout(constant_id = 0) const float sdf_max_distance = 1.0;
layout(constant_id = 1) const float atlas_image_width = 1.0;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput in_background_color;
layout(set = 0, binding = 1) uniform sampler in_sampler;
layout(set = 0, binding = 2) uniform texture2D in_textures[16];

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec3 in_texture_coord;
layout(location = 2) in vec4 in_color;
layout(location = 3) in float in_luminance;
layout(location = 4) in float in_lightness;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 out_color;

bool is_clipped()
{
    return greaterThanEqual(gl_FragCoord.xyxy, in_clipping_rectangle) != bvec4(true, true, false, false);
}

float coverage_to_alpha(float coverage, float Y_back, float L_back, float Y_front, float L_front)
{
    if (Y_back == Y_front) {
        return coverage;

    } else {
        float L_target = mix(L_back, L_front, coverage);
        float Y_target = L_target * L_target;
        return (Y_target - Y_back) / (Y_front - Y_back);
    }
}

vec3 coverage_to_alpha(vec3 coverage, float Y_back, float L_back, float Y_front, float L_front)
{
    if (Y_back == Y_front) {
        return coverage;

    } else {
        vec3 L_back3 = vec3(L_back, L_back, L_back);
        vec3 L_front3 = vec3(L_front, L_front, L_front);

        vec3 L_target = mix(L_back3, L_front3, coverage);
        vec3 Y_target = L_target * L_target;
        return (Y_target - Y_back) / (Y_front - Y_back);
    }
}

void main()
{
    if (is_clipped()) {
        discard;
    }

    // The amount of distance and direction covered in 2D inside the texture when
    // stepping one fragment to the right.
    vec2 horizontal_texture_stride = dFdxCoarse(in_texture_coord.xy);
    vec2 vertical_texture_stride = vec2(-horizontal_texture_stride.y, horizontal_texture_stride.x);
    float pixel_distance = length(horizontal_texture_stride);

    float distance_multiplier = sdf_max_distance / (pixel_distance * atlas_image_width);

    float green_radius = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), in_texture_coord.xy).r * distance_multiplier;

    if (green_radius < -0.5) {
        // Fully outside the fragment, early exit.
        discard;

    } else if (green_radius >= 0.5) {
        // Fully inside the fragment.
        out_color = in_color;

    } else {
        // Normal anti-aliasing.
        vec4 background_color = subpassLoad(in_background_color);
        float background_luminance =
            0.2126 * background_color.r + 0.7152 * background_color.g + 0.0722 * background_color.b;
        float background_lightness = sqrt(background_luminance);

        if (pushConstants.subpixel_orientation == 0) {
            // Normal anti-aliasing.
            float coverage = clamp(green_radius + 0.5, 0.0, 1.0);
            float alpha = coverage_to_alpha(
                coverage, background_luminance, background_lightness, in_luminance, in_lightness);

            // Output alpha is always 1.0
            out_color = vec4(mix(background_color.rgb, in_color.rgb, alpha), 1.0);
        
        } else {
            // Subpixel anti-aliasing

            vec2 red_offset;
            vec2 blue_offset;
            switch (pushConstants.subpixel_orientation) {
            case 1: // Red-left, Blue-right
                red_offset = horizontal_texture_stride / -3.0;
                blue_offset = horizontal_texture_stride / 3.0;
                break;
            case 2: // Blue-left, Red-right
                blue_offset = horizontal_texture_stride / -3.0;
                red_offset = horizontal_texture_stride / 3.0;
                break;
            case 3: // Red-bottom, Blue-top
                red_offset = vertical_texture_stride / -3.0;
                blue_offset = vertical_texture_stride / 3.0;
                break;
            case 4: // Blue-bottom, Red-top
                blue_offset = vertical_texture_stride / -3.0;
                red_offset = vertical_texture_stride / 3.0;
                break;
            }

            vec2 red_coord = in_texture_coord.xy + red_offset;
            float red_radius = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), red_coord).r * distance_multiplier;

            vec2 blue_coord = in_texture_coord.xy + blue_offset;
            float blue_radius = texture(sampler2D(in_textures[int(in_texture_coord.z)], in_sampler), blue_coord).r * distance_multiplier;

            vec3 coverage = clamp(vec3(red_radius, green_radius, blue_radius) + 0.5, 0.0, 1.0);
            vec3 alpha = coverage_to_alpha(
                coverage, background_luminance, background_lightness, in_luminance, in_lightness);

            // Output alpha is always 1.0
            out_color = vec4(mix(background_color.rgb, in_color.rgb, alpha), 1.0);
        }
    }
}
