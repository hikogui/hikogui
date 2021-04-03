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

float Kr = 0.2126;
float Kg = 0.7152;
float Kb = 0.0722;
float inv_Kr = Kr / Kg;
float inv_Kb = Kb / Kg;

vec3 RGB_to_PrYPb(vec3 x)
{
    float Y = Kr * x.r + Kg * x.g + Kb * x.b;
    float Pr = x.r - Y;
    float Pb = x.b - Y;
    return vec3(Pr, Y, Pb);
}

vec3 PrYPb_to_RGB(vec3 x)
{
    float R = x.r + x.g;
    float G = x.g - x.r * inv_Kr - x.b * inv_Kb;
    float B = x.b + x.g;

    return vec3(R, G, B);
}

vec3 RGB_to_PrLPb(vec3 x)
{
    vec3 tmp = RGB_to_PrYPb(x);
    return vec3(tmp.r, sqrt(tmp.g), tmp.b);
}

vec3 PrLPb_to_RGB(vec3 x)
{
    return PrYPb_to_RGB(vec3(x.r, x.g * x.g, x.b));
}

vec3 PrYPb_to_RGB_subpixel(vec3 sub_R, vec3 sub_G, vec3 sub_B)
{
    float R = sub_R.r + sub_R.g;
    float G = sub_G.g - sub_G.r * inv_Kr - sub_G.b * inv_Kb;
    float B = sub_B.b + sub_B.g;

    return vec3(R, G, B);
}

vec3 PrLPb_to_RGB_subpixel(vec3 sub_R, vec3 sub_G, vec3 sub_B)
{
    vec3 sub_R_ = vec3(sub_R.r, sub_R.g * sub_R.g, sub_R.b);
    vec3 sub_G_ = vec3(sub_G.r, sub_G.g * sub_G.g, sub_G.b);
    vec3 sub_B_ = vec3(sub_B.r, sub_B.g * sub_B.g, sub_B.b);
    return PrYPb_to_RGB_subpixel(sub_R_, sub_G_, sub_B_);
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

    } else if (in_color.a == 1.0 && green_radius >= 0.5) {
        // Fully inside the fragment.
        out_color = vec4(in_color.rgb, 1.0);

    } else {
        vec4 background_RGBA = subpassLoad(in_background_color);
        vec3 background_PrLPb = RGB_to_PrLPb(background_RGBA.rgb);
        vec3 foreground_PrLPb = RGB_to_PrLPb(in_color.rgb);

        if (pushConstants.subpixel_orientation == 0) {
            // Normal anti-aliasing.
            float coverage = clamp(green_radius + 0.5, 0.0, 1.0) * in_color.a;

            vec3 composit_PrLPb = mix(background_PrLPb, foreground_PrLPb, coverage);
            out_color = vec4(PrLPb_to_RGB(composit_PrLPb), 1.0);
        
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

            vec3 coverage = clamp(vec3(red_radius, green_radius, blue_radius) + 0.5, 0.0, 1.0) * in_color.a;

            vec3 R_composit_PrLPb = mix(background_PrLPb, foreground_PrLPb, coverage.r);
            vec3 G_composit_PrLPb = mix(background_PrLPb, foreground_PrLPb, coverage.g);
            vec3 B_composit_PrLPb = mix(background_PrLPb, foreground_PrLPb, coverage.b);

            vec3 composit_RGB = PrLPb_to_RGB_subpixel(R_composit_PrLPb, G_composit_PrLPb, B_composit_PrLPb);

            // Output alpha is always 1.0
            out_color = vec4(composit_RGB, 1.0);
        }
    }
}
