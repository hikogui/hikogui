// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** logical and.
 */
bvec4 and(bvec4 lhs, bvec4 rhs)
{
    return bvec4(
        lhs.x && rhs.x,
        lhs.y && rhs.y,
        lhs.z && rhs.z,
        lhs.w && rhs.w
    );
}

/** Check if the rectangle contains the point.
 *
 * @param rectangle A axis-aligned rectangle encoded as left-bottom=(x,y), right-top=(z,w)
 * @param point A 2D point
 * @return True is the point is inside the rectangle.
 */
bool contains(vec4 rectangle, vec2 point)
{
    return greaterThanEqual(point.xyxy, rectangle) == bvec4(true, true, false, false);
}

float Kr = 0.2126;
float Kg = 0.7152;
float Kb = 0.0722;
float Ku = Kb / Kg;
float Kv = Kr / Kg;

/** Convert linear-rgb to tYUV.
 * The tYUV format's elements are:
 * .x = Y = linear-luminosity
 * .y = U = B - Y
 * .z = V = R - Y
 */
vec3 rgb_to_tYUV(vec3 rgb)
{
    float R = rgb.r;
    float G = rgb.g;
    float B = rgb.b;

    float Y = Kr * R + Kg * G + Kb * B;
    float U = B - Y;
    float V = R - Y;

    return vec3(Y, U, V);
}

float tYUV_to_R(vec3 yuv)
{
    float Y = yuv.x;
    float V = yuv.z;

    return V + Y;
}

float tYUV_to_G(vec3 yuv)
{
    float Y = yuv.x;
    float U = yuv.y;
    float V = yuv.z;

    return Y - U * Ku - V * Kv;
}

float tYUV_to_B(vec3 yuv)
{
    float Y = yuv.x;
    float U = yuv.y;

    return U + Y;
}

/** Convert tYUV to linear-rgb.
 * The tYUV format's elements are:
 * .x = Y = linear-luminosity
 * .y = U = B - Y
 * .z = V = R - Y
 */
vec3 tYUV_to_rgb(vec3 yuv)
{
    return vec3(tYUV_to_R(yuv), tYUV_to_G(yuv), tYUV_to_B(yuv));
}

/** Convert linear-rgb to tluv.
 * The tYUV format's elements are:
 * .x = L = perceptional-lightness = sqrt(Y)
 * .y = U = B - Y
 * .z = V = R - Y
 */
vec3 rgb_to_tluv(vec3 rgb)
{
    vec3 tmp = rgb_to_tYUV(rgb);
    return vec3(sqrt(tmp.r), tmp.g, tmp.b);
}

vec4 rgb_to_tluv(vec4 rgba)
{
    return vec4(rgb_to_tluv(rgba.rgb), rgba.a);
}

/** Convert tluv to linear-rgb.
 * The tYUV format's elements are:
 * .x = L = perceptional-lightness = sqrt(Y)
 * .y = U = B - Y
 * .z = V = R - Y
 */
vec3 tluv_to_rgb(vec3 luv)
{
    return tYUV_to_rgb(vec3(luv.x * luv.x, luv.y, luv.z));
}

/** Convert a subpixel-triplet of tYUV values to a single rgb value.
 */
vec3 tYUV_to_rgb(vec3 sub_R, vec3 sub_G, vec3 sub_B)
{
    return vec3(tYUV_to_R(sub_R), tYUV_to_G(sub_G), tYUV_to_B(sub_B));
}

/** Convert a subpixel-triplet of tluv values to a single rgb value.
 */
vec3 tluv_to_rgb(vec3 sub_R, vec3 sub_G, vec3 sub_B)
{
    vec3 sub_R_ = vec3(sub_R.x * sub_R.x, sub_R.y, sub_R.z);
    vec3 sub_G_ = vec3(sub_G.x * sub_G.x, sub_G.y, sub_G.z);
    vec3 sub_B_ = vec3(sub_B.x * sub_B.x, sub_B.y, sub_B.z);
    return tYUV_to_rgb(sub_R_, sub_G_, sub_B_);
}
