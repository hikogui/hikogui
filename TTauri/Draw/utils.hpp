
#pragma once

#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <iostream>

namespace TTauri::Draw {

auto const gammaToLinearTable = generate_array<float, 512>([](size_t i) -> float {
    if (i < 256) {
        auto const u = static_cast<float>(i) / 255.0;

        if (u < 0.04045) {
            return u / 12.92;
        } else {
            return std::pow((u + 0.055 / 1.055), 2.4);
        }
    } else {
        return static_cast<float>(i - 256) / 255.0;
    }
});

/*! Cubic root accurate enough for 8 bits.
 */
inline __m128 cbrt_ps(const __m128 x)
{
    auto xInt = _mm_castps_si128(x);

    xInt = _mm_add_epi32(_mm_srai_epi32(xInt, 2), _mm_srai_epi32(xInt, 4));
    xInt = _mm_add_epi32(xInt, _mm_srai_epi32(xInt, 4));
    xInt = _mm_add_epi32(xInt, _mm_srai_epi32(xInt, 8));
    xInt = _mm_add_epi32(xInt, _mm_set1_epi32(0x2a5137a0));

    auto xFloat = _mm_castsi128_ps(xInt);
    xFloat = _mm_mul_ps(
        _mm_set1_ps(1.0/3.0),
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(2.0), xFloat),
            _mm_div_ps(x, _mm_mul_ps(xFloat, xFloat))
        )
    );

    return xFloat;
}

/*! pow(x, 1/2.4)
 */
inline __m128 pow512_ps(const __m128 x)
{
    auto const c = cbrt_ps(x);
    return _mm_mul_ps(c, _mm_sqrt_ps(_mm_sqrt_ps(c)));
}

/*! Add gamma to the full SSE register
 */
inline __m128 sRGB_gamma_ps(const __m128 x)
{
    auto lin_x = _mm_mul_ps(_mm_set1_ps(12.92), x);
    auto pow_x = _mm_sub_ps(_mm_mul_ps(_mm_set1_ps(1.055), pow512_ps(x)), _mm_set1_ps(0.055));

    // x < 0.0031308 ? lin_x : pow_x
    auto const lt = _mm_cmplt_ps(x, _mm_set1_ps(0.0031308));

    lin_x = _mm_and_ps(lt, lin_x);
    pow_x = _mm_andnot_ps(lt, pow_x);
    return _mm_or_ps(lin_x, pow_x);
}

/*! Add gamma to a SSE(A,B,G,R)
 */
inline __m128 sRGBA_gamma_ps(const __m128 x)
{
    auto const gamma_x = sRGB_gamma_ps(x);
    return _mm_blend_ps(gamma_x, x, 1);
}

/*! pack a SSE(A,B,G,R) into a uint32_t(ABGR) == u8vec4(R,G,B,A)
 */
inline void pack_sRGBA_ps(uint32_t *r, const __m128 x)
{
    auto const x255 = _mm_mul_ps(x, _mm_set1_ps(255.0));
    auto const x4_32u = _mm_cvtps_epi32(x255);

    auto const x4_8u = _mm_shuffle_epi8(x4_32u, _mm_set1_epi32(0x00010203));

    _mm_storeu_si32(r, x4_8u);
}

/*! unpack uint32_t(ABGR) == u8vec4(R,G,B,A) to SSE(A,B,G,R) and make linear.
*/
inline __m128 unpack_linear_sRGBA_ps(const uint32_t x)
{
    auto const indices = _mm_set_epi32(
        (x >> 24) + 256, // Alpha
        (x >> 16) & 0xff, // Blue
        (x >> 8) & 0xff, // Green
        x & 0xff // Red
    );
    return _mm_i32gather_ps(gammaToLinearTable.data(), indices, sizeof (float));
}

/*! Composite a over b
 */
inline __m128 compositeOver(const __m128 a, const __m128 b)
{
    auto const a_alpha = _mm_broadcastss_ps(a);
    auto const b_alpha = _mm_broadcastss_ps(b);
    auto const b_inv_alpha = _mm_mul_ps(b_alpha, _mm_sub_ps(_mm_set1_ps(1.0), a_alpha));

    auto const c_alpha = _mm_add_ps(a_alpha, b_inv_alpha);
    auto const c = _mm_div_ps(
        _mm_add_ps(_mm_mul_ps(a, a_alpha), _mm_mul_ps(b, b_inv_alpha)),
        c_alpha
    );
    return _mm_blend_ps(c, c_alpha, 1);
}


}