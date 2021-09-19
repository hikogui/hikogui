#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in flat vec4 in_clipping_rectangle;
layout(location = 1) in vec4 in_edge_distances;
layout(location = 2) in vec4 in_fill_color;
layout(location = 3) in vec4 in_border_color;
layout(location = 4) in flat vec4 in_corner_radii;
layout(location = 5) in flat float in_border_start;
layout(location = 6) in flat float in_border_end;

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

        vec2 distance_from_center = radius - coordinate;
        vec2 distance_from_center_squared = distance_from_center * distance_from_center;

        return radius.x - sqrt(distance_from_center_squared.x + distance_from_center_squared.y);

    } else {
        // The shortest distance to any of the edge distances.
        vec2 tmp = min(in_edge_distances.xy, in_edge_distances.zw);
        return min(tmp.x, tmp.y);
    }
}

void main() {
    if (!contains(in_clipping_rectangle, gl_FragCoord.xy)) {
        discard;
    }
        
    float distance = distance_from_box_outline();
    
    float fill_coverage = clamp(distance - in_border_end + 0.5, 0.0, 1.0);
    if (fill_coverage == 1.0) {
        out_color = in_fill_color;

    } else {
        float border_coverage = clamp(distance - in_border_start + 0.5, 0.0, 1.0);

        vec4 border_color = in_border_color * border_coverage;
        vec4 fill_color = in_fill_color * fill_coverage;

        out_color = fill_color + border_color * (1.0 - fill_coverage);
    }
}
