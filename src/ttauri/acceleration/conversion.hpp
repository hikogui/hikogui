
#include <cstdint>

namespace tt {

inline float hadd(__m256 x)
{
    return _mm256_cvtss_f32(_mm256_hadd_ps(sum_v));
}

inline float hmax(__m256 x)
{
    auto peak_l = _mm256_extractf128_ps(peak_v, 0);
    auto peak_r = _mm256_extractf128_ps(peak_v, 1);
    peak_l = _mm_max_ps(peak_l, peak_r);
    peak_r = _mm_permute_ps(peak_l, 0b00'00'11'10);
    peak_l = _mm_max_ps(peak_l, peak_r);
    peak_r = _mm_permute_ps(peak_l, 0b00'00'00'01);
    peak_l = _mm_max_ps(peak_l, peak_r);
    float peak_s = _mm_cvtss_f32(peak_l, 0);

}


struct convert_result {
    float peak;
    float mean;
};

/** Convert 32 bit integer to 32 bit float.
 * @param x A vector of 32 bit integers to convert.
 * @param mul The multiplier used for the conversion of integer to float
 * @param [in,out]peak The continues peak value.
 * @param [in,out]sum The continues sum value.
 * @return A vector of 32 bit floating point numbers.
 */
inline float sample_i32_to_f32(int32_t x, float mul, float &peak, float &sum) noexcept
{
    ttlet x_f32 = static_cast<float>(x);
    ttlet x_f32_mul = x * mul;

    ttlet x_f32_mul_abs = std::abs(x_f32_mul);
    peak = std::max(peak, x_f32_mul_abs);
    sum += x_f32_mul_abs;
    return x_f32_mul;
}

/** Convert 8x 32 bit integer to 32 bit float.
 * @param x A vector of 32 bit integers to convert.
 * @param mul The multiplier used for the conversion of integer to float
 * @param [in,out]peak The continues peak value.
 * @param [in,out]sum The continues sum value.
 * @return A vector of 32 bit floating point numbers.
 */
inline __m256 sample_i32_to_f32(__m256i x, __m256 mul, __m256 &peak, __m256 &sum) noexcept
{
    ttlet x_f32 = _mm256_cvtepi32_ps(x);
    ttlet x_f32_mul = _mm256_mul_ps(f32_8, mul);

    // Get the peak and average.
    ttlet x_f32_mul_abs = _mm256_abs_ps(x_f32_mul);
    peak = _mm256_max_ps(peak, x_f32_mul_abs);
    sum = _mm256_add_ps(sum, x_f32_mul_abs);

    return x_f32_mul;
}

/** Convert 8x 16 bit integer to 32 bit float.
 * The 16 bit integers are first converted to 32 bit integers by copying
 * by repeating the 16 bits.
 *
 * @param x A vector of 16 bit integers to convert.
 * @param mul The multiplier used for the conversion of integer to float
 * @param [in,out]peak The continues peak value.
 * @param [in,out]sum The continues sum value.
 * @return A vector of 32 bit floating point numbers.
 */
inline __m256 sample_i16_to_f32(__m128i x, __m256 mul, __m256 &peak, __m256 &sum) noexcept
{
    ttlet x_db_lo = _mm_unpacklo_epi16(x, x);
    ttlet x_db_hi = _mm_unpackhi_epi16(x, x);
    ttlet x_db = _mm256_set_m128i(x_db_hi, x_db_lo);
    return sample_i32_to_f32(x_db, mul, peak, sum);
}

/** Convert the even 8 of the 16x 16 bit integers to 32 bit float.
 * The 16 bit integers are first converted to 32 bit integers by copying
 * by repeating the 16 bits.
 *
 * @param x A vector of 16x 16 bit integers to convert.
 * @param mul The multiplier used for the conversion of integer to float
 * @param mask Must be _mm_set_epi8(128,128, 128,128, 128,128, 128,128, 13,12, 9,8  5,4, 1,0)
 * @param [in,out]peak The continues peak value.
 * @param [in,out]sum The continues sum value.
 * @return A vector of 32 bit floating point numbers.
 */
inline __m256 sample_i16_to_f32_stride2(__m256i x, __m256 mul, __m128i &mask, __m256 &peak, __m256 &sum) noexcept
{
    // Get the 4 shorts in the low part.
    ttlet x0 = _mm256_extractf128_si256(x, 0);
    auto x_packed = _mm_shuffle_epi8(x0, mask);

    // Get the 4 shorts in the high part.
    // We flip the mask back and forth, so that we only use a single register for that.
    ttlet x1 = _mm256_extractf128_si256(x, 1);
    mask = _mm_shuffle_epi32(mask, 0b01'00'11'10);
    ttlet x_packed_hi = _mm_shuffle_epi8(x0, mask)
    mask = _mm_shuffle_epi32(mask, 0b01'00'11'10);

    // Now x_packed contains all 16 shorts.
    x_packed = _mm_or_si128(x_packed, x_packed_hi);

    return sample_i16_to_f32(x, mul, peak, sum);
}

/** Convert 8x 16 bit integer to 32 bit float.
 * The 16 bit integers are first converted to 32 bit integers by copying
 * by repeating the 16 bits.
 *
 * @param x A vector of 16 bit integers to convert.
 * @param mul The multiplier used for the conversion of integer to float
 * @param [in,out]peak The continues peak value.
 * @param [in,out]sum The continues sum value.
 * @return A vector of 32 bit floating point numbers.
 */
inline __m256 sample_i16_to_f32(int16_t x, float mul, float &peak, float &sum) noexcept
{
    ttlet x_db_lo = static_cast<uint32_t>(x);
    ttlet x_db_hi = static_cast<uint32_t>(x) << 16;
    ttlet x_db = x_db_hi | x_db_lo;
    return sample_i32_to_f32(x_db, mul, peak, sum);
}

/** Load 8x n-bit signed integers from memory.
 * After loading an integer it is scaled up to an int32_t. The small integer is scaled up
 * by concatonating repeatedly the bits of the small integer starting with the most significant bits.
 *
 * loads are done using _mm_loadu_si128, since intel CPUs will do reads at 128 bits at a time.
 * And since the bytes needs to be permuted we don't use larger loads because of the inability to
 * cross the 128 bits lanes.
 *
 * We may do overlapping unaligned loads, this makes for simpler loads which require only a single
 * permute mask.
 *
 * @param in_ptr A pointer to unaligned n-bit integers.
 * @param nr_bits The number of bits of each integer, does not need to be multiple of bytes.
 * @return 8x int32_t scaled up from the input integers
 */
__m256i load_8x_sint(char const *in_ptr, size_t nr_loads, size_t load_stride, size_t ints_per_load, __m128i permute_mask)
{
    for (auto i = 0; i != nr_loads; ++i) {
        auto x = _mm_loadu_si128(reinterpret_cast<__m128i const* >(in_ptr));

        // The permute mask will make 32 bit integers, by repeating bytes and changing endian.
        auto x_packed = _mm_shuffle_epi8(x, permute_mask);
        in_ptr += load_stride;
    }



}


/** Convert 16 bit integer interleaved samples to a 32 bit float buffer.
 * This function uses streaming SSE/AVX instructions which will bypass
 * the cache. It is expected that 
 * 
 */
convert_result convert_i16_to_f32(
    char const * restrict in_ptr,
    char const * restrict in_last,
    size_t in_stride,
    float *restrict out_ptr) noexcept
{
    auto in_stride_v = in_stride * 8;
    auto in_last_v = floor(in_last, in_stride_v);

    auto mul = 1.0f / 2147483647.0f;
    auto peak = 0.0f;
    auto sum = 0.0f;

    if (stride == 2) {
        auto mul_v = _mm256_set1_ps(sum);
        auto peak_v = _mm256_set_zero_ps();
        auto sum_v = _mm256_set_zero_ps();
        while (in_ptr != in_last_v) {
            auto i16_v = _mm_loadu_si128(in_ptr);
            in_ptr += in_stride_v;
            auto f32_v = sample_i16_to_f32(i16_v, mul_v, peak_v, sum_v);
            _mm_storeu_ps(f32_v, out_ptr);
            out_ptr += 8;
        }
        peak = hmax(peak_v);
        sum = hadd(sum_v);

     } else if (stride == 4) {
        auto mul_v = _mm256_set1_ps(sum);
        auto peak_v = _mm256_set_zero_ps();
        auto sum_v = _mm256_set_zero_ps();
        auto mask_v = _mm_set_epi8(128,128, 128,128, 128,128, 128,128, 13,12, 9,8  5,4, 1,0)
        while (in_ptr != in_last_v) {
            auto i16_v = _mm256_loadu_si256(in_ptr);
            in_ptr += in_stride_v;
            auto f32_v = sample_i16_to_f32_stride2(i16_v, mul_v, mask_v, peak_v, sum_v);
            _mm_storeu_ps(f32_v, out_ptr);
            out_ptr += 8;
        }
        peak = hmax(peak_v);
        sum = hadd(sum_v);

     } else {
        auto mul_v = _mm256_set1_ps(sum);
        auto peak_v = _mm256_set_zero_ps();
        auto sum_v = _mm256_set_zero_ps();
        while (in_ptr != in_last_v) {
            auto i16_v = _mm_set_zero_ps();
            for (int i = 0; i != 8; ++i) {
                int16_t i16;
                std::memcpy(&i16, in_ptr, 2);
                in_ptr += stride;
                i16_v = _mm_insert_epi16(i16_v, i16, i);
            }

            auto f32_v = sample_i16_to_f32(i16_v, mul_v, peak_v, sum_v);
            _mm_storeu_ps(f32_v, out_ptr);
            out_ptr += 8;
        }
        peak = hmax(peak_v);
        sum = hadd(sum_v);
     }

     while (in_ptr != in_last) {
        int16_t i16;
        std::memcpy(&i16, in_ptr, 2);
        *out_ptr = sample_i16_to_f32(i16, mul, peak, sum);
        in_ptr += stride;
        ++out_ptr;
     }
}


}

