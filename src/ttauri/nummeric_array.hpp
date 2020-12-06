
#pragma once

#include "os_detect.hpp"
#include "aligned_array.hpp"

#if TT_PROCESSOR == TT_CPU_X64
#include "detail/f32x4_sse.hpp"
#endif

#include <cstdint>

namespace tt {

template<typename T,int N>
class numeric_array {
public:
    using value_type = T;


    constexpr numeric_array() noexcept : v() {}
    constexpr numeric_array(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array(numeric_array &&rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array &&rhs) noexcept = default;

    [[nodiscard]] constexpr numeric_array(std::initializer_list<T> rhs) noexcept : v()
    {
        auto src = begin(rhs);
        auto dst = begin(v);

        // Copy all values from the initializer list.
        while (src != end(rhs) && dst != end(v)) {
            *(dst++) = *(src++);
        }

        tt_assume(dst != end(v) || src == end(rhs), "Expecting the std:initializer_list size to be <= to the size of the numeric array");

        // Set all other elements to zero
        while (dst != end(v)) {
            *(dst++) = {};
        }
    }

    [[nodiscard]] constexpr numeric_array(aligned_array<T,N> const &rhs) noexcept : v(rhs) {}

    [[nodiscard]] static constexpr numeric_array point(std::initializer_list<T> rhs) noexcept
    {
        auto rhs_ = aligned_array<T,N>{};

        auto src = begin(rhs);
        auto dst = begin(v);

        // Copy all values from the initializer list.
        while (src != end(rhs) && dst != end(v)) {
            *(dst++) = *(src++);
        }

        tt_assume(dst != end(v) || src == end(rhs), "Expecting the std:initializer_list size to be <= to the size of the numeric array");

        if (dst != end(v)) {
            // Set all other elements to zero
            auto dst_last = end
            while (dst != end(v)) {
                *(dst++) = {};
            }

            // Set last element to one.
            rhs_.last() = T{1};
        }

        tt_assume(rhs_.last() != T{}, "Last element of a point shoud be non-zero");
        return numeric_array{std::move(rhs_)};
    }

    [[nodiscard]] constexpr T const &operator[](size_t i) const noexcept
    {
        tt_assume(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T &operator[](size_t i) noexcept
    {
        tt_assume(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T const &x() const noexcept requires (N >= 1)
    {
        return v[0];
    }

    [[nodiscard]] constexpr T const &y() const noexcept requires (N >= 2)
    {
        return v[1];
    }

    [[nodiscard]] constexpr T const &z() const noexcept requires (N >= 3)
    {
        return v[2];
    }

    [[nodiscard]] constexpr T const &w() const noexcept requires (N >= 4)
    {
        return v[3];
    }

    [[nodiscard]] constexpr T &x() noexcept requires (N >= 1)
    {
        return v[0];
    }

    [[nodiscard]] constexpr T &y() noexcept requires (N >= 2)
    {
        return v[1];
    }

    [[nodiscard]] constexpr T &z() noexcept requires (N >= 3)
    {
        return v[2];
    }

    [[nodiscard]] constexpr T &w() noexcept requires (N >= 4)
    {
        return v[3];
    }

    [[nodiscard]] constexpr T const &width() const noexcept requires (N >= 1)
    {
        return v[0];
    }

    [[nodiscard]] constexpr T const &height() const noexcept requires (N >= 2)
    {
        return v[1];
    }

    [[nodiscard]] constexpr T &width() noexcept requires (N >= 1)
    {
        return v[0];
    }

    [[nodiscard]] constexpr T &height() noexcept requires (N >= 2)
    {
        return v[1];
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            // -rhs.v[i] will cause a memory load with msvc.
            r.v[i] = T{} - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array abs(numeric_array const &rhs) noexcept
    {
        auto neg_rhs = -rhs;

        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = rhs.v[i] < T{} ? neg_rhs.v[i] : rhs.v[i] ;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_rcp(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = 1.0f / rhs.v[i];
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array sqrt(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_sqrt(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = std::sqrt(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array rcp_sqrt(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_rcp_sqrt(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = 1.0f / std::sqrt(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array floor(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_floor(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = std::floor(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array ceil(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_ceil(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = std::ceil(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array round(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_round(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (int i = 0; i != N; ++i) {
                r[i] = std::round(rhs.v[i]);
            }
            return r;
        }
    }

    /** Take a dot product.
     * @tparam D Number of dimensions to calculate the dot product over.
     */
    template<size_t D=N>
    [[nodiscard]] friend constexpr T dot(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_dot<D>(rhs.v)};

        } else {
            auto r = T{};
            for (int i = 0; i != D; ++i) {
                r[i] += lhs.v[i] * rhs.v[i];
            }
            return r;
        }
    }

    /** Take a dot product.
     * @tparam D Number of dimensions to calculate the length over.
     */
    template<size_t D=N>
    [[nodiscard]] friend constexpr T length(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_length<D>(rhs.v)};

        } else {
            auto r = T{};
            for (int i = 0; i != D; ++i) {
                r[i] += lhs.v[i] * rhs.v[i];
            }
            return std::sqrt(r);
        }
    }

    /** Take a dot product.
     * @tparam D Number of dimensions to calculate the length over.
     */
    template<size_t D=N>
    [[nodiscard]] friend constexpr T rcp_length(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            return {f32x4_sse_rcp_length<D>(rhs.v)};

        } else {
            auto r = T{};
            for (int i = 0; i != D; ++i) {
                r[i] += lhs.v[i] * rhs.v[i];
            }
            return 1.0f / std::sqrt(r);
        }
    }

    [[nodiscard]] friend constexpr bool operator==(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            // NSVC cannot vectorize comparison.
            return {f32x4_sse_eq(lhs.v, rhs.v)};

        } else {
            auto r = true;
            for (int i = 0; i != N; ++i) {
                r &= (lhs.v[i] == rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr bool operator!=(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        return !(lhs == rhs);
    }


    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] + rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] * rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] % rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array min(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            // std::min() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array max(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            // std::max() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array clamp(numeric_array const &lhs, numeric_array const &low numeric_array const &high) noexcept
    {
        auto r = numeric_array{};
        for (int i = 0; i != N; ++i) {
            // std::clamp() causes vectorization failure with msvc
            r.v[i] =
                lhs.v[i] < low.v[i] ? low.v[i] :
                lhs.v[i] > high.v[i] ? high.v[i] :
                lhs.v[i];
        }
        return r;
    }
    

    template<char I>
    [[nodiscard]] constexpr T const &swizzle_extract() const noexcept
    {
        switch (I) {
        case '0': return T{0};
        case '1': return T{1};

        case 'x': return get<0>(v);
        case 'y': return get<1>(v);
        case 'z': return get<2>(v);
        case 'w': return get<3>(v);

        case 'a': return get<0>(v);
        case 'b': return get<1>(v);
        case 'c': return get<2>(v);
        case 'd': return get<3>(v);
        case 'e': return get<4>(v);
        case 'f': return get<5>(v);
        case 'g': return get<6>(v);
        case 'h': return get<7>(v);
        case 'i': return get<8>(v);
        case 'j': return get<9>(v);
        case 'k': return get<10>(v);
        case 'l': return get<11>(v);
        case 'm': return get<12>(v);
        case 'n': return get<13>(v);
        case 'o': return get<14>(v);
        case 'p': return get<15>(v);
        default: tt_no_default();
        }
    }

    template<char... Elements>
    [[nodiscard]] constexpr numeric_array swizzle() const noexcept
    {
        static_assert(sizeof...(Elements) <= N);

        if (is_f32x4 && is_sse && !std::is_constant_evaluated()) {
            if (sizeof...(Elements) == 4) {
                return {f32x4_sse_swizzle<Elements...>(v)};

            } else if (sizeof...(Elements) == 3) {
                return {f32x4_sse_swizzle<Elements..., '0'>(v)};

            } else if (sizeof...(Elements) == 2) {
                return {f32x4_sse_swizzle<Elements..., '0', '0'>(v)};

            } else if (sizeof...(Elements) == 1) {
                return {f32x4_sse_swizzle<Elements..., '0', '0', '0'>(v)};

            } else {
                return {f32x4_sse_swizzle<'0', '0', '0', '0'>(v)};
            }

        } else {
            auto r = aligned_array<T,N>{};
            swizzle_detail<0,Elements...>(r);
            return r;
        }
    }

#define SWIZZLE(name, ...) \
    [[nodiscard]] auto name() const noexcept \
    { \
        return swizzle<__VA_ARGS__>(); \
    }

#define SWIZZLE_GEN1(name, ...) \
    SWIZZLE(name##0, __VA_ARGS__, '0') \
    SWIZZLE(name##1, __VA_ARGS__, '1') \
    SWIZZLE(name##x, __VA_ARGS__, 'x') \
    SWIZZLE(name##y, __VA_ARGS__, 'y') \
    SWIZZLE(name##z, __VA_ARGS__, 'z') \
    SWIZZLE(name##w, __VA_ARGS__, 'w')

#define SWIZZLE_GEN2(name, ...) \
    SWIZZLE_GEN1(name##0, __VA_ARGS__, '0') \
    SWIZZLE_GEN1(name##1, __VA_ARGS__, '1') \
    SWIZZLE_GEN1(name##x, __VA_ARGS__, 'x') \
    SWIZZLE_GEN1(name##y, __VA_ARGS__, 'y') \
    SWIZZLE_GEN1(name##z, __VA_ARGS__, 'z') \
    SWIZZLE_GEN1(name##w, __VA_ARGS__, 'w')

#define SWIZZLE_GEN3(name, ...) \
    SWIZZLE_GEN2(name##0, __VA_ARGS__, '0') \
    SWIZZLE_GEN2(name##1, __VA_ARGS__, '1') \
    SWIZZLE_GEN2(name##x, __VA_ARGS__, 'x') \
    SWIZZLE_GEN2(name##y, __VA_ARGS__, 'y') \
    SWIZZLE_GEN2(name##z, __VA_ARGS__, 'z') \
    SWIZZLE_GEN2(name##w, __VA_ARGS__, 'w')

    SWIZZLE_GEN3(_0, '0')
    SWIZZLE_GEN3(_1, '1')
    SWIZZLE_GEN3(x, 'x')
    SWIZZLE_GEN3(y, 'y')
    SWIZZLE_GEN3(z, 'z')
    SWIZZLE_GEN3(w, 'w')

#undef SWIZZLE
#undef SWIZZLE_GEN1
#undef SWIZZLE_GEN2
#undef SWIZZLE_GEN3
private:
    aligned_array<T,N> v;

    template<int I, char FirstElement, char... RestElements>
    [[nodiscard]] constexpr void swizzle_detail(aligned_array<T,N> &rhs) const noexcept
    {
        get<I>(rhs) = swizzle_extract<FirstElement>();
        swizzle_detail<I+1,RestElements...>(rhs);
    }

    constexpr bool is_i8x1 = std::is_same_v<T,int8_t> && N == 1;
    constexpr bool is_i8x2 = std::is_same_v<T,int8_t> && N == 2;
    constexpr bool is_i8x4 = std::is_same_v<T,int8_t> && N == 4;
    constexpr bool is_i8x8 = std::is_same_v<T,int8_t> && N == 8;
    constexpr bool is_i8x16 = std::is_same_v<T,int8_t> && N == 16;
    constexpr bool is_i8x32 = std::is_same_v<T,int8_t> && N == 32;
    constexpr bool is_i8x64 = std::is_same_v<T,int8_t> && N == 64;
    constexpr bool is_u8x1 = std::is_same_v<T,uint8_t> && N == 1;
    constexpr bool is_u8x2 = std::is_same_v<T,uint8_t> && N == 2;
    constexpr bool is_u8x4 = std::is_same_v<T,uint8_t> && N == 4;
    constexpr bool is_u8x8 = std::is_same_v<T,uint8_t> && N == 8;
    constexpr bool is_u8x16 = std::is_same_v<T,uint8_t> && N == 16;
    constexpr bool is_u8x32 = std::is_same_v<T,uint8_t> && N == 32;
    constexpr bool is_u8x64 = std::is_same_v<T,uint8_t> && N == 64;

    constexpr bool is_i16x1 = std::is_same_v<T,int16_t> && N == 1;
    constexpr bool is_i16x2 = std::is_same_v<T,int16_t> && N == 2;
    constexpr bool is_i16x4 = std::is_same_v<T,int16_t> && N == 4;
    constexpr bool is_i16x8 = std::is_same_v<T,int16_t> && N == 8;
    constexpr bool is_i16x16 = std::is_same_v<T,int16_t> && N == 16;
    constexpr bool is_i16x32 = std::is_same_v<T,int16_t> && N == 32;
    constexpr bool is_u16x1 = std::is_same_v<T,uint16_t> && N == 1;
    constexpr bool is_u16x2 = std::is_same_v<T,uint16_t> && N == 2;
    constexpr bool is_u16x4 = std::is_same_v<T,uint16_t> && N == 4;
    constexpr bool is_u16x8 = std::is_same_v<T,uint16_t> && N == 8;
    constexpr bool is_u16x16 = std::is_same_v<T,uint16_t> && N == 16;
    constexpr bool is_u16x32 = std::is_same_v<T,uint16_t> && N == 32;

    constexpr bool is_i32x1 = std::is_same_v<T,int32_t> && N == 1;
    constexpr bool is_i32x2 = std::is_same_v<T,int32_t> && N == 2;
    constexpr bool is_i32x4 = std::is_same_v<T,int32_t> && N == 4;
    constexpr bool is_i32x8 = std::is_same_v<T,int32_t> && N == 8;
    constexpr bool is_i32x16 = std::is_same_v<T,int32_t> && N == 16;
    constexpr bool is_u32x1 = std::is_same_v<T,uint32_t> && N == 1;
    constexpr bool is_u32x2 = std::is_same_v<T,uint32_t> && N == 2;
    constexpr bool is_u32x4 = std::is_same_v<T,uint32_t> && N == 4;
    constexpr bool is_u32x8 = std::is_same_v<T,uint32_t> && N == 8;
    constexpr bool is_u32x16 = std::is_same_v<T,uint32_t> && N == 16;
    constexpr bool is_f32x1 = std::is_same_v<T,float> && N == 1;
    constexpr bool is_f32x2 = std::is_same_v<T,float> && N == 2;
    constexpr bool is_f32x4 = std::is_same_v<T,float> && N == 4;
    constexpr bool is_f32x8 = std::is_same_v<T,float> && N == 8;
    constexpr bool is_f32x16 = std::is_same_v<T,float> && N == 16;

    constexpr bool is_i64x1 = std::is_same_v<T,int64_t> && N == 1;
    constexpr bool is_i64x2 = std::is_same_v<T,int64_t> && N == 2;
    constexpr bool is_i64x4 = std::is_same_v<T,int64_t> && N == 4;
    constexpr bool is_i64x8 = std::is_same_v<T,int64_t> && N == 8;
    constexpr bool is_u64x1 = std::is_same_v<T,uint64_t> && N == 1;
    constexpr bool is_u64x2 = std::is_same_v<T,uint64_t> && N == 2;
    constexpr bool is_u64x4 = std::is_same_v<T,uint64_t> && N == 4;
    constexpr bool is_u64x8 = std::is_same_v<T,uint64_t> && N == 8;
    constexpr bool is_f64x1 = std::is_same_v<T,double> && N == 1;
    constexpr bool is_f64x2 = std::is_same_v<T,double> && N == 2;
    constexpr bool is_f64x4 = std::is_same_v<T,double> && N == 4;
    constexpr bool is_f64x8 = std::is_same_v<T,double> && N == 8;

    constexpr bool is_sse = Processor::current == Processor::x64;
};

using i8x1 = numeric_array<int8_t,1>;
using i8x2 = numeric_array<int8_t,2>;
using i8x4 = numeric_array<int8_t,4>;
using i8x8 = numeric_array<int8_t,8>;
using i8x16 = numeric_array<int8_t,16>;
using i8x32 = numeric_array<int8_t,32>;
using i8x64 = numeric_array<int8_t,64>;

using u8x1 = numeric_array<uint8_t,1>;
using u8x2 = numeric_array<uint8_t,2>;
using u8x4 = numeric_array<uint8_t,4>;
using u8x8 = numeric_array<uint8_t,8>;
using u8x16 = numeric_array<uint8_t,16>;
using u8x32 = numeric_array<uint8_t,32>;
using u8x64 = numeric_array<uint8_t,64>;

using i16x1 = numeric_array<int16_t,1>;
using i16x2 = numeric_array<int16_t,2>;
using i16x4 = numeric_array<int16_t,4>;
using i16x8 = numeric_array<int16_t,8>;
using i16x16 = numeric_array<int16_t,16>;
using i16x32 = numeric_array<int16_t,32>;

using u16x1 = numeric_array<uint16_t,1>;
using u16x2 = numeric_array<uint16_t,2>;
using u16x4 = numeric_array<uint16_t,4>;
using u16x8 = numeric_array<uint16_t,8>;
using u16x16 = numeric_array<uint16_t,16>;
using u16x32 = numeric_array<uint16_t,32>;

using i32x1 = numeric_array<int32_t,1>;
using i32x2 = numeric_array<int32_t,2>;
using i32x4 = numeric_array<int32_t,4>;
using i32x8 = numeric_array<int32_t,8>;
using i32x16 = numeric_array<int32_t,16>;

using u32x1 = numeric_array<uint32_t,1>;
using u32x2 = numeric_array<uint32_t,2>;
using u32x4 = numeric_array<uint32_t,4>;
using u32x8 = numeric_array<uint32_t,8>;
using u32x16 = numeric_array<uint32_t,16>;

using f32x1 = numeric_array<float,1>;
using f32x2 = numeric_array<float,2>;
using f32x4 = numeric_array<float,4>;
using f32x8 = numeric_array<float,8>;
using f32x16 = numeric_array<float,16>;

using i64x1 = numeric_array<int64_t,1>;
using i64x2 = numeric_array<int64_t,2>;
using i64x4 = numeric_array<int64_t,4>;
using i64x8 = numeric_array<int64_t,8>;

using u64x1 = numeric_array<uint64_t,1>;
using u64x2 = numeric_array<uint64_t,2>;
using u64x4 = numeric_array<uint64_t,4>;
using u64x8 = numeric_array<uint64_t,8>;

using f64x1 = numeric_array<double,1>;
using f64x2 = numeric_array<double,2>;
using f64x4 = numeric_array<double,4>;
using f64x8 = numeric_array<double,8>;

}

