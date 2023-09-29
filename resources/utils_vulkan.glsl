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
 * The full formula to convert coverage to alpha taking into account perceptional
 * uniform lightness between foreground and background colors:
 * ```
 *    F = foreground color
 *    B = background color
 *    T = target color
 *    c = coverage
 *    a = alpha
 *    T = mix(sqrt(F), sqrt(B), c) ^ 2
 *
 *    a = (T - B) / (F - B)       if F != B
 *    a = c                       otherwise
 * ```
 *
 * To simplify this formula and remove the division we fill in the foreground and background
 * with black and white and the other way around:
 * ```
 *    a = c^2                     if F == 1 and B == 0
 *    a = 2c - c^2                if F == 0 and B == 1
 * ```
 *
 * Now we mix based on the foreground color, expecting the background color to mirror.
 * ```
 *    a = mix(2c - c^2, c^2, F^2)   if B^2 == 1 - F^2
 * ```
 *
 * @param coverage The amount of coverage. Elements must be between 0.0 and 1.0
 * @param foreground_sq The sqrt of the foreground. Elements must be between 0.0 and 1.0
 * @return The alpha value for the red, blue, green, alpha color components.
 */
float coverage_to_alpha(float coverage, float sqrt_foreground)
{
    float coverage_sq = coverage * coverage;
    float coverage_2 = coverage + coverage;
    return mix(coverage_2 - coverage_sq, coverage_sq, sqrt_foreground);
}

/** Convert coverage to a perceptional uniform alpha.
 *
 * @see coverage_to_alpha(float, float)
 */
vec4 coverage_to_alpha(vec4 coverage, vec4 sqrt_foreground)
{
    vec4 coverage_sq = coverage * coverage;
    vec4 coverage_2 = coverage + coverage;
    return mix(coverage_2 - coverage_sq, coverage_sq, sqrt_foreground);
}

/** Multiply the alpha with the color.
 *
 * @param color The color+alpha without pre-multiplication.
 * @return The color+alpha where the color is multiplied with the alpha.
 */
vec4 multiply_alpha(vec4 color)
{
    return vec4(color.rgb * color.a, color.a);
}

/** Convert RGB to Y.
 */
float rgb_to_y(vec3 color)
{
    vec3 tmp = color * vec3(0.2126, 0.7152, 0.0722);
    return tmp.r + tmp.g + tmp.b;
}

/** Convert RGB to RGBY.
 */
vec4 rgb_to_rgby(vec3 color)
{
    return vec4(color, rgb_to_y(color));
}

