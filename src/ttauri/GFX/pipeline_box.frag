#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec4 in_edge_distances;
layout(location = 2) in vec4 in_fill_color;
layout(location = 3) in vec4 in_border_color;
layout(location = 4) in float in_border_sqrt_y;
layout(location = 5) in flat vec4 in_corner_radii;
layout(location = 6) in flat float in_border_start;
layout(location = 7) in flat float in_border_end;

layout(location = 0) out vec4 out_color;

#include "utils.glsl"


float distance_from_box_outline()
{
    bvec4 in_corner = and(lessThan(in_edge_distances.xzxz, in_corner_radii), lessThan(in_edge_distances.yyww, in_corner_radii));

    if (any(in_corner)) {
        vec2 coordinate;
        vec2 radius;

        if (in_corner.x) {
            coordinate = in_edge_distances.xy;
            radius = in_corner_radii.xx;
        } else if (in_corner.y) {
            coordinate = in_edge_distances.zy;
            radius = in_corner_radii.yy;
        } else if (in_corner.z) {
            coordinate = in_edge_distances.xw;
            radius = in_corner_radii.zz;
        } else {
            coordinate = in_edge_distances.zw;
            radius = in_corner_radii.ww;
        }

        return radius.x - distance(radius, coordinate);

    } else {
        // The shortest distance to any of the edge distances.
        vec2 tmp = min(in_edge_distances.xy, in_edge_distances.zw);
        return min(tmp.x, tmp.y);
    }
}

void main()
{
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }
        
    float distance = distance_from_box_outline();
    
    float border_coverage = clamp(distance - in_border_start + 0.5, 0.0, 1.0);
    if (border_coverage == 0.0) {
        // Don't update depth beyond the border.
        discard;
    }

    float fill_coverage = clamp(in_border_end - distance + 0.5, 0.0, 1.0);
    
    // Both the border_coverage and fill_coverage modulate the amount of border_color
    // that is visible. So convert both coverages to alpha using the border lightness.
    float border_alpha = coverage_to_alpha(border_coverage, in_border_sqrt_y);
    float fill_alpha = coverage_to_alpha(fill_coverage, in_border_sqrt_y);

    // Adjust transparency of the border color by how much it is overlapping with the fill.
    vec4 border_color = in_border_color * fill_alpha;

    // combine the border on top of the fill.
    //vec4 combined_color = in_fill_color * (1.0 - border_color.a) + border_color;
    vec4 combined_color = fma(in_fill_color, vec4(1.0 - border_color.a), border_color);

    // Adjust transparency of the combined color by how much it is overlapping with the background.
    out_color = combined_color * border_alpha;
}
