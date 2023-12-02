

#pragam once

namespace hi { inline namespace v1 {

/** Multiply two float arrays into another array.
 *
 * @param a A pointer the first array.
 * @param b A pointer the second array.
 * @param o A pointer the output array.
 * @param n Number of elements in each array.
 */
constexpr void dsp_mul(float const *a, float const *b, float *o, size_t n) noexcept
{
    if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX)
        for (hilet a_end = a + floor(n, 8); a != a_end; a += 8, b += 8, o += 8) {
            hilet a_ = _mm256_loadu_ps(a);
            hilet b_ = _mm256_loadu_ps(b);
            hilet o_ = _mm256_mul_ps(a_, b_);
            _mm256_storeu_ps(o, o_);
        }

#elif defined(HI_HAS_SSE)
        for (hilet a_end = a + floor(n, 4); a != a_end; a += 4, b += 4, o += 4) {
            hilet a_ = _mm_loadu_ps(a);
            hilet b_ = _mm_loadu_ps(b);
            hilet o_ = _mm_mul_ps(a_, b_);
            _mm_storeu_ps(o, o_);
        }
#endif
    }

    for (hilet a_end = a + n; a != a_end; ++a, ++b, ++o) {
        *o = *a * *b;
    }
}

/** Multiply a float array with a scalar into another array.
 *
 * @param a A pointer the first array.
 * @param b The second file.
 * @param o A pointer the output array.
 * @param n Number of elements in each array.
 */
constexpr void dsp_mul(float const *a, float b, float *o, size_t n) noexcept
{
    if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX)
        hilet b_ = _mm256_set1_ps(b);
        for (hilet a_end = a + floor(n, 8); a != a_end; a += 8, o += 8) {
            hilet a_ = _mm256_loadu_ps(a);
            hilet o_ = _mm256_mul_ps(a_, b_);
            _mm256_storeu_ps(o, o_);
        }

#elif defined(HI_HAS_SSE)
        hilet b_ = _mm_set1_ps(b);
        for (hilet a_end = a + floor(n, 4); a != a_end; a += 4, o += 4) {
            hilet a_ = _mm_loadu_ps(a);
            hilet o_ = _mm_mul_ps(a_, b_);
            _mm_storeu_ps(o, o_);
        }
#endif
    }

    for (hilet a_end = a + n; a != a_end; ++a, ++o) {
        *o = *a * b;
    }
}


/** Multiply two float arrays and accumulate into another array.
 *
 * @param a A pointer the first array.
 * @param b A pointer the second array.
 * @param o A pointer the output array.
 * @param n Number of elements in each array.
 */
constexpr void dsp_mul_acc(float const *a, float const *b, float *o, size_t n) noexcept
{
    if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX)
        for (hilet a_end = a + floor(n, 8); a != a_end; a += 8, b += 8, o += 8) {
            hilet a_ = _mm256_loadu_ps(a);
            hilet b_ = _mm256_loadu_ps(b);
            hilet o_ = _mm256_mul_ps(a_, b_);
            _mm256_storeu_ps(o, _mm256_add_ps(_mm256_loadu_ps(o), o_));
        }

#elif defined(HI_HAS_SSE)
        for (hilet a_end = a + floor(n, 4); a != a_end; a += 4, b += 4, o += 4) {
            hilet a_ = _mm_loadu_ps(a);
            hilet b_ = _mm_loadu_ps(b);
            hilet o_ = _mm_mul_ps(a_, b_);
            _mm_storeu_ps(o, _mm_add_ps(_mm_loadu_ps(o), o_));
        }
#endif
    }

    for (hilet a_end = a + n; a != a_end; ++a, ++b, ++o) {
        *o = *o + *a * *b;
    }
}

/** Multiply a float array with a scalar and accumulate into another array.
 *
 * @param a A pointer the first array.
 * @param b The second file.
 * @param o A pointer the output array.
 * @param n Number of elements in each array.
 */
constexpr void dsp_mul_acc(float const *a, float b, float *o, size_t n) noexcept
{
    if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX)
        hilet b_ = _mm256_set1_ps(b);
        for (hilet a_end = a + floor(n, 8); a != a_end; a += 8, o += 8) {
            hilet a_ = _mm256_loadu_ps(a);
            hilet o_ = _mm256_mul_ps(a_, b_);
            _mm256_storeu_ps(o, _mm256_add_ps(_mm256_loadu_ps(o), o_));
        }

#elif defined(HI_HAS_SSE)
        hilet b_ = _mm_set1_ps(b);
        for (hilet a_end = a + floor(n, 4); a != a_end; a += 4, o += 4) {
            hilet a_ = _mm_loadu_ps(a);
            hilet o_ = _mm_mul_ps(a_, b_);
            _mm_storeu_ps(o, _mm_add_ps(_mm_loadu_ps(o), o_));
        }
#endif
    }

    for (hilet a_end = a + n; a != a_end; ++a, ++o) {
        *o = *o + *a * b;
    }
}


}}
