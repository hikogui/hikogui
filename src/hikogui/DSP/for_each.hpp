

#pragma once

#include <concepts>

hi_export_module(hikogui.DSP.apply)

hi_export namespace hi { inline v1 {

/** Different implementations of a operation.
 *
 * The following implementations should be part of the specialization:
 *  - (required) `constexpr T operator()(T a, T b) const noexcept`
 *  - (optional) `__m128 operator()(__m128 a, __m128 b) const noexcept`
 *  - (optional) `__m256 operator()(__m256 a, __m256 b) const noexcept`
 *  - (optional) `__m512 operator()(__m512 a, __m512 b) const noexcept`
 *  - (optional) `__m128d operator()(__m128d a, __m128d b) const noexcept`
 *  - (optional) `__m256d operator()(__m256d a, __m256d b) const noexcept`
 *  - (optional) `__m512d operator()(__m512d a, __m512d b) const noexcept`
 *  - (optional) `__m128i operator()(__m128i a, __m128i b) const noexcept`
 *  - (optional) `__m256i operator()(__m256i a, __m256i b) const noexcept`
 *  - (optional) `__m512i operator()(__m512i a, __m512i b) const noexcept`
 *
 * @tparam Operation The name of the operation
 * @tparam T The numeric type to operate on.
 */
template<fixed_string Operation, typename T>
struct dsp_op {
};


template<typename Context, typename T>
concept dsp_apply_argument = std::same_as<Context, T> or std::same_as<Context, T const *> or std::same_as<Context, T *>;;

template<typename Op>
struct dps_op_traits {


};


template<size_t I, typename Array, typename First, typename... Rest>
[[nodiscard]] constexpr void _dsp_regs_init_float(Array &array, First first, Rest... rest) noexcept
{
    if constexpr (not std::is_pointer_v<First>) {
        std::get<I>(array) = first;
    }
    if constexpr (sizeof...(Rest) != 0) {
        return _dsp_regs_init<I + 1>(array, rest...);
    }
}

template<size_t I, typename Array, typename First, typename... Rest>
[[nodiscard]] constexpr void dsp_load(Array &array, First &first, Rest &... rest) noexcept
{
    using reg_type = Array::value_type;

    if constexpr (std::same_as<reg_type, float> and std::same_as<First, float const *>) {
        std::get<I>(array) = *first++;
    } else if constexpr (std::same_as<reg_type, double> and std::same_as<First, double const *>) {
        std::get<I>(array) = *first++;
    } else if constexpr (std::same_as<reg_type, int> and std::same_as<First, int const *>) {
        std::get<I>(array) = *first++;

#if defined(HI_HAS_SSE)
    } else if constexpr (std::same_as<reg_type, __m128> and std::same_as<First, float const *>) {
        std::get<I>(array) = _mm_loadu_ps(first);
        first += 4;
    } else if constexpr (std::same_as<reg_type, __m128> and std::same_as<First, double const *>) {
        std::get<I>(array) = _mm_loadu_pd(first);
        first += 2;
#endif

#if defined(HI_HAS_SSE2)
    } else if constexpr (std::same_as<reg_type, __m128i> and std::same_as<First, int const *>) {
        std::get<I>(array) = _mm_loadu_epi32(first);
        first += 4;
#endif

#if defined(HI_HAS_AVX)
    } else if constexpr (std::same_as<reg_type, __m256> and std::same_as<First, float const *>) {
        std::get<I>(array) = _mm256_loadu_ps(first);
        first += 8;
    } else if constexpr (std::same_as<reg_type, __m256> and std::same_as<First, double const *>) {
        std::get<I>(array) = _mm256_loadu_pd(first);
        first += 4;
#endif

#if defined(HI_HAS_AVX2)
    } else if constexpr (std::same_as<reg_type, __m256i> and std::same_as<First, int const *>) {
        std::get<I>(array) = _mm256_loadu_epi32(first);
        first += 8;
#endif

#if defined(HI_HAS_AVX512F)
    } else if constexpr (std::same_as<reg_type, __m512> and std::same_as<First, float const *>) {
        std::get<I>(array) = _mm512_loadu_ps(first);
        first += 16;
    } else if constexpr (std::same_as<reg_type, __m512> and std::same_as<First, double const *>) {
        std::get<I>(array) = _mm512_loadu_pd(first);
        first += 8;
    } else if constexpr (std::same_as<reg_type, __m512i> and std::same_as<First, int const *>) {
        std::get<I>(array) = _mm512_loadu_epi32(first);
        first += 16;
#endif
    }

    if constexpr (sizeof...(Rest) != 0) {
        return dsp_load<I + 1>(array, rest...);
    }
}

template<size_t I, typename Array, typename First, typename... Rest>
[[nodiscard]] constexpr void dsp_init_scalar(Array &array, First first, Rest... rest) noexcept
{
    using reg_type = Array::value_type;

    if constexpr (std::same_as<reg_type, float> and std::same_as<First, float>) {
        std::get<I>(array) = first;
    } else if constexpr (std::same_as<reg_type, double> and std::same_as<First, double>) {
        std::get<I>(array) = first;
    } else if constexpr (std::same_as<reg_type, int> and std::same_as<First, int>) {
        std::get<I>(array) = first;

#if defined(HI_HAS_SSE)
    } else if constexpr (std::same_as<reg_type, __m128> and std::same_as<First, float>) {
        std::get<I>(array) = _mm_set1_ps(first);
    } else if constexpr (std::same_as<reg_type, __m128> and std::same_as<First, double>) {
        std::get<I>(array) = _mm_set1_pd(first);
#endif

#if defined(HI_HAS_SSE2)
    } else if constexpr (std::same_as<reg_type, __m128i> and std::same_as<First, int>) {
        std::get<I>(array) = _mm_set1_epi32(first);
#endif

#if defined(HI_HAS_AVX)
    } else if constexpr (std::same_as<reg_type, __m256> and std::same_as<First, float>) {
        std::get<I>(array) = _mm256_set1_ps(first);
    } else if constexpr (std::same_as<reg_type, __m256> and std::same_as<First, double>) {
        std::get<I>(array) = _mm256_set1_pd(first);
#endif

#if defined(HI_HAS_AVX2)
    } else if constexpr (std::same_as<reg_type, __m256i> and std::same_as<First, int>) {
        std::get<I>(array) = _mm256_set1_epi32(first);
#endif

#if defined(HI_HAS_AVX512F)
    } else if constexpr (std::same_as<reg_type, __m512> and std::same_as<First, float>) {
        std::get<I>(array) = _mm512_set1_ps(first);
    } else if constexpr (std::same_as<reg_type, __m512> and std::same_as<First, double>) {
        std::get<I>(array) = _mm512_set1_pd(first);
    } else if constexpr (std::same_as<reg_type, __m512i> and std::same_as<First, int>) {
        std::get<I>(array) = _mm512_set1_epi32(first);
#endif
    }

    if constexpr (sizeof...(Rest) != 0) {
        return dsp_init_scalar<I + 1>(array, rest...);
    }
}

template<typename T, typename... Args>
[[nodiscard]] constexpr auto dsp_regs_init(Args... args) noexcept
{
    auto r = std::array<T, sizeof...(Args)>{};
    dsp_init_scalar<0>(r, args...);
    return r;
}

/**
 */
template<typename Operation, typename T, dsp_apply_argument<T>... Args>
constexpr void dsp_for_each(T *r, T *r_last, Args... args) noexcept
{
    using traits = dsp_op_traits<Operation>;
   
    auto T_regs = dsp_regs_init<T>(args...);

    auto r_align = ceil(r, traits::best_alignment());
    inplace_min(r_align, r_last);

    while (r != r_align) {
        *r++ = Operation{}();
    }

#if defined(HI_HAS_AVX512F)


#elif defined(HI_HAS_AVX2)


#elif defined(HI_HAS_SSE)

#else
#endif

    while (r != sse_align) {
        T_regs = dsp_regs_load_and_increment<T>(args...);
        *r++ = Operation{}(T_regs);
    }

    if constexpr (traits{}::sse()) {
    }
#endif

    while (r_first
}


template<>
struct dsp_op<"+", float> {
    constexpr float operator()(float a, float b) const noexcept
    {
        return a + b;
    }

#if HI_HAS_SSE
    __m128 operator()(__m128 a, __m128 b) const noexcept
    {
        return _mm_add_ps(a, b);
    }
#endif

#if HI_HAS_AVX
    __m256 operator()(__m256 a, __m256 b) const noexcept
    {
        return _mm256_add_ps(a, b);
    }
#endif

#if HI_HAS_AVX512F
    __m512 operator()(__m512 a, __m512 b) const noexcept
    {
        return _mm512_add_ps(a, b);
    }
#endif
};

}}
