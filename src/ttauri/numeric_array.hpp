
#pragma once

#include "os_detect.hpp"
#include "aligned_array.hpp"
#include "concepts.hpp"
#include "cast.hpp"
#include "type_traits.hpp"
#if TT_PROCESSOR == TT_CPU_X64
#include "detail/f32x4_sse.hpp"
#endif

#include <cstdint>
#include <ostream>
#include <string>
#include <array>
#include <type_traits>
#include <concepts>
#include <bit>

namespace tt {

template<arithmetic T, ssize_t N>
class numeric_array {
public:
    static_assert(N >= 0);

    using container_type = aligned_array<T, N>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer = typename container_type::pointer;
    using const_pointer = typename container_type::const_pointer;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    constexpr numeric_array() noexcept = default;
    constexpr numeric_array(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array(numeric_array &&rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array &&rhs) noexcept = default;

    [[nodiscard]] constexpr numeric_array(std::initializer_list<T> rhs) noexcept : v()
    {
        auto src = std::begin(rhs);
        auto dst = std::begin(v);

        // Copy all values from the initializer list.
        while (src != std::end(rhs) && dst != std::end(v)) {
            *(dst++) = *(src++);
        }

        tt_axiom(
            dst != std::end(v) || src == std::end(rhs),
            "Expecting the std:initializer_list size to be <= to the size of the numeric array");

        // Set all other elements to zero
        while (dst != std::end(v)) {
            *(dst++) = {};
        }
    }

    [[nodiscard]] constexpr numeric_array(T const &first) noexcept requires (N == 1) :
        numeric_array({first})
    {
    }

    template<arithmetic... Rest>
    requires(sizeof...(Rest) + 2 <= N) [[nodiscard]] constexpr numeric_array(T const &first, T const &second, Rest const &... rest) noexcept :
        numeric_array({first, second, narrow_cast<T>(rest)...})
    {
    }

    [[nodiscard]] explicit constexpr numeric_array(aligned_array<T, N> const &rhs) noexcept : v(rhs) {}

    [[nodiscard]] static constexpr numeric_array broadcast(T rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r[i] = rhs;
        }
        return r;
    }

    /** Get a point at the origin.
     */
    [[nodiscard]] static constexpr numeric_array point() noexcept
    {
        auto r = numeric_array{};
        r.back() = T{1};
        return r;
    }

    /** Convert vector to point.
     */
    [[nodiscard]] static constexpr numeric_array point(numeric_array rhs) noexcept
    {
        rhs.back() = T{1};
        return rhs;
    }

    [[nodiscard]] static constexpr numeric_array point(std::initializer_list<T> rhs) noexcept
    {
        auto r = numeric_array(rhs);

        if (std::size(rhs) < N) {
            r.v.back() = T{1};
        }

        tt_axiom(r.v.back() != T{}, "Last element of a point should be non-zero");
        return r;
    }

    template<arithmetic... Rest>
    [[nodiscard]] static constexpr numeric_array point(T const &first, Rest const &... rest) noexcept
        requires(sizeof...(Rest) < N)
    {
        return point({first, narrow_cast<T>(rest)...});
    }

    [[nodiscard]] static constexpr numeric_array color(std::initializer_list<T> rhs) noexcept
    {
        return point(rhs);
    }

    template<arithmetic... Rest>
    [[nodiscard]] static constexpr numeric_array color(T const &first, Rest const &... rest) noexcept
        requires(sizeof...(Rest) < N)
    {
        return color({first, narrow_cast<T>(rest)...});
    }

    template<arithmetic U, ssize_t M>
    [[nodiscard]] explicit constexpr numeric_array(std::array<U, M> const &rhs) noexcept : v()
    {
        auto src = std::begin(rhs);
        auto dst = std::begin(v);
        auto src_last = std::end(rhs);
        auto dst_last = std::end(v);

        while (src != src_last && dst != dst_last) {
            *(dst++) = narrow_cast<T>(*(src++));
        }
        while (dst != dst_last) {
            *(dst++) = T{};
        }
        while (src != src_last) {
            tt_axiom(*(src++) == U{});
        }
    }

    template<arithmetic U, ssize_t M>
    [[nodiscard]] explicit constexpr numeric_array(numeric_array<U, M> const &rhs) noexcept : v()
    {
        auto src = std::begin(rhs);
        auto dst = std::begin(v);
        auto src_last = std::end(rhs);
        auto dst_last = std::end(v);

        while (src != src_last && dst != dst_last) {
            *(dst++) = narrow_cast<T>(*(src++));
        }
        while (dst != dst_last) {
            *(dst++) = T{};
        }
        while (src != src_last) {
            tt_axiom(*(src++) == U{});
        }
    }

    [[nodiscard]] explicit numeric_array(__m128 const &rhs) noexcept requires(N == 4 && std::is_same_v<T, float> && has_sse) :
        v(rhs)
    {
    }

    [[nodiscard]] explicit numeric_array(__m128d const &rhs) noexcept requires(N == 2 && std::is_same_v<T, double> && has_sse) :
        v(rhs)
    {
    }

    [[nodiscard]] explicit numeric_array(__m128i const &rhs) noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 16 && has_sse) :
        v(rhs)
    {
    }

    [[nodiscard]] explicit numeric_array(__m256 const &rhs) noexcept requires(N == 8 && std::is_same_v<T, float> && has_sse) :
        v(rhs)
    {
    }

    [[nodiscard]] explicit numeric_array(__m256d const &rhs) noexcept requires(N == 4 && std::is_same_v<T, double> && has_sse) :
        v(rhs)
    {
    }

    [[nodiscard]] explicit numeric_array(__m256i const &rhs) noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 32 && has_sse) :
        v(rhs)
    {
    }

    template<arithmetic U, ssize_t M>
    [[nodiscard]] explicit constexpr operator std::array<U, M>() const noexcept
    {
        auto r = std::array<U, M>{};

        auto src = std::begin(v);
        auto dst = std::begin(r);
        auto src_last = std::end(v);
        auto dst_last = std::end(r);

        while (src != src_last && dst != dst_last) {
            *(dst++) = narrow_cast<U>(*(src++));
        }
        while (dst != dst_last) {
            *(dst++) = U{};
        }

        // Check if the rest of the data are zeros, or one in homogeneous coordinates.
        while (src != src_last) {
            tt_axiom(*src == T{} || *src == T{1});
            ++src;
        }

        return r;
    }

    template<arithmetic U, ssize_t M>
    [[nodiscard]] explicit constexpr operator numeric_array<U, M>() const noexcept
    {
        auto r = std::array<U, M>{};

        auto src = std::begin(v);
        auto dst = std::begin(r);
        auto src_last = std::end(v);
        auto dst_last = std::end(r);

        while (src != src_last && dst != dst_last) {
            *(dst++) = narrow_cast<U>(*(src++));
        }
        while (dst != dst_last) {
            *(dst++) = U{};
        }
        while (src != src_last) {
            tt_axiom(*(src++) == T{});
        }

        return r;
    }

    [[nodiscard]] explicit operator __m128() const noexcept requires(N == 4 && std::is_same_v<T, float> && has_sse)
    {
        return static_cast<__m128>(v);
    }

    [[nodiscard]] explicit operator __m128d() const noexcept requires(N == 2 && std::is_same_v<T, double> && has_sse)
    {
        return static_cast<__m128d>(v);
    }

    [[nodiscard]] explicit operator __m128i() const noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 16 && has_sse)
    {
        return static_cast<__m128i>(v);
    }

    [[nodiscard]] explicit operator __m256() const noexcept requires(N == 8 && std::is_same_v<T, float> && has_sse)
    {
        return static_cast<__m256>(v);
    }

    [[nodiscard]] explicit operator __m256d() const noexcept requires(N == 4 && std::is_same_v<T, double> && has_sse)
    {
        return static_cast<__m256d>(v);
    }

    [[nodiscard]] explicit operator __m256i() const noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 32 && has_sse)
    {
        return static_cast<__m256i>(v);
    }

    [[nodiscard]] constexpr T const &operator[](ssize_t i) const noexcept
    {
        tt_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T &operator[](ssize_t i) noexcept
    {
        tt_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr reference front() noexcept
    {
        return v.front();
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        return v.front();
    }

    [[nodiscard]] constexpr reference back() noexcept
    {
        return v.back();
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        return v.back();
    }

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return v.data();
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return v.data();
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return v.begin();
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return v.begin();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return v.cbegin();
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return v.end();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return v.end();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return v.cend();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return v.empty();
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return v.size();
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return v.max_size();
    }

    constexpr bool is_point() const noexcept
    {
        return v.back() != T{};
    }

    constexpr bool is_vector() const noexcept
    {
        return v.back() == T{};
    }

    constexpr bool is_opaque() const noexcept
    {
        return a() == T{1};
    }

    constexpr bool is_transparent() const noexcept
    {
        return a() == T{0};
    }

    [[nodiscard]] constexpr T const &x() const noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T const &y() const noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T const &z() const noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    [[nodiscard]] constexpr T const &w() const noexcept requires(N >= 4)
    {
        return get<3>(v);
    }

    [[nodiscard]] constexpr T &x() noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T &y() noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T &z() noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    [[nodiscard]] constexpr T &w() noexcept requires(N >= 4)
    {
        return get<3>(v);
    }

    [[nodiscard]] constexpr T const &r() const noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T const &g() const noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T const &b() const noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    [[nodiscard]] constexpr T const &a() const noexcept requires(N >= 4)
    {
        return get<3>(v);
    }

    [[nodiscard]] constexpr T &r() noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T &g() noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T &b() noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    [[nodiscard]] constexpr T &a() noexcept requires(N >= 4)
    {
        return get<3>(v);
    }

    [[nodiscard]] constexpr T const &width() const noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T const &height() const noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T const &depth() const noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    [[nodiscard]] constexpr T &width() noexcept requires(N >= 1)
    {
        return get<0>(v);
    }

    [[nodiscard]] constexpr T &height() noexcept requires(N >= 2)
    {
        return get<1>(v);
    }

    [[nodiscard]] constexpr T &depth() noexcept requires(N >= 3)
    {
        return get<2>(v);
    }

    constexpr numeric_array &operator+=(numeric_array const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] += rhs.v[i];
        }
        return *this;
    }

    constexpr numeric_array &operator+=(T const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] += rhs;
        }
        return *this;
    }

    constexpr numeric_array &operator-=(numeric_array const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] -= rhs.v[i];
        }
        return *this;
    }

    constexpr numeric_array &operator-=(T const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] -= rhs;
        }
        return *this;
    }

    constexpr numeric_array &operator*=(numeric_array const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] *= rhs.v[i];
        }
        return *this;
    }

    constexpr numeric_array &operator*=(T const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] *= rhs;
        }
        return *this;
    }

    constexpr numeric_array &operator/=(numeric_array const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] /= rhs.v[i];
        }
        return *this;
    }

    constexpr numeric_array &operator/=(T const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] /= rhs;
        }
        return *this;
    }

    constexpr numeric_array &operator%=(numeric_array const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] %= rhs.v[i];
        }
        return *this;
    }

    constexpr numeric_array &operator%=(T const &rhs) noexcept
    {
        for (ssize_t i = 0; i != N; ++i) {
            v[i] %= rhs;
        }
        return *this;
    }

    constexpr static ssize_t get_zero = -1;
    constexpr static ssize_t get_one = -2;

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<ssize_t I>
    [[nodiscard]] friend constexpr T &get(numeric_array &rhs) noexcept
    {
        static_assert(I >= 0 && I < N, "Index out of bounds");
        return get<I>(rhs.v);
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array,
     *           or the special indices: -1 yields a literal 0, -2 yields a literal 1.
     */
    template<ssize_t I>
    [[nodiscard]] friend constexpr T get(numeric_array &&rhs) noexcept
    {
        static_assert(I >= -2 && I < N, "Index out of bounds");
        if constexpr (I == get_zero) {
            return T{0};
        } else if constexpr (I == get_one) {
            return T{1};
        } else {
            return get<I>(rhs.v);
        }
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array,
     *           or the special indices: -1 yields a literal 0, -2 yields a literal 1.
     */
    template<ssize_t I>
    [[nodiscard]] friend constexpr T get(numeric_array const &rhs) noexcept
    {
        static_assert(I >= -2 && I < N, "Index out of bounds");
        if constexpr (I == get_zero) {
            return T{0};
        } else if constexpr (I == get_one) {
            return T{1};
        } else {
            return get<I>(rhs.v);
        }
    }

    /** Negate individual elements.
     *
     * @tparam NegateElement A list of which element to negate.
     */
    template<bool... NegateElement>
    [[nodiscard]] friend constexpr numeric_array neg(numeric_array rhs) noexcept
    {
        neg_detail<0, NegateElement...>(rhs);
        return rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            // -rhs.v[i] will cause a memory load with msvc.
            r.v[i] = T{} - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array abs(numeric_array const &rhs) noexcept
    {
        auto neg_rhs = -rhs;

        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = rhs.v[i] < T{} ? neg_rhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_rcp(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = 1.0f / rhs.v[i];
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array sqrt(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_sqrt(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = std::sqrt(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array rcp_sqrt(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_rcp_sqrt(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = 1.0f / std::sqrt(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array floor(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_floor(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = std::floor(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array ceil(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_ceil(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = std::ceil(rhs.v[i]);
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array round(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_round(rhs.v)};

        } else {
            auto r = numeric_array{};
            for (ssize_t i = 0; i != N; ++i) {
                r[i] = std::round(rhs.v[i]);
            }
            return r;
        }
    }

    /** Take a dot product.
     * @tparam D Number of dimensions to calculate the dot product over.
     */
    template<ssize_t D>
    [[nodiscard]] friend constexpr T dot(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_dot<D>(lhs.v, rhs.v);

        } else {
            auto r = T{};
            for (ssize_t i = 0; i != D; ++i) {
                r += lhs.v[i] * rhs.v[i];
            }
            return r;
        }
    }

    /** Take the length of the vector
     * @tparam D Number of dimensions to calculate the length over.
     */
    template<ssize_t D>
    [[nodiscard]] friend constexpr T hypot(numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_hypot<D>(rhs.v);

        } else {
            auto r = T{};
            for (ssize_t i = 0; i != D; ++i) {
                r += rhs.v[i] * rhs.v[i];
            }
            return std::sqrt(r);
        }
    }

    template<ssize_t D>
    [[deprecated]] [[nodiscard]] friend constexpr T length(numeric_array const &rhs) noexcept
    {
        return hypot<D>(rhs);
    }

    /** Take the squared length of the vector
     * @tparam D Number of dimensions to calculate the length over.
     */
    template<ssize_t D>
    [[nodiscard]] friend constexpr T hypot_squared(numeric_array const &rhs) noexcept
    {
        return dot<D>(rhs, rhs);
    }

    template<ssize_t D>
    [[deprecated]] [[nodiscard]] friend constexpr T length_squared(numeric_array const &rhs) noexcept
    {
        return hypot_squared<D>(rhs);
    }

    /** Take a reciprocal of the length.
     * @tparam D Number of dimensions to calculate the length over.
     */
    template<ssize_t D>
    [[nodiscard]] friend constexpr T rcp_hypot(numeric_array const &rhs) noexcept
    {
        static_assert(D <= N);
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_rcp_hypot<D>(rhs.v);

        } else {
            auto r = T{};
            for (ssize_t i = 0; i != D; ++i) {
                r += rhs.v[i] * rhs.v[i];
            }
            return 1.0f / std::sqrt(r);
        }
    }

    template<ssize_t D>
    [[deprecated]] [[nodiscard]] friend constexpr T rcp_length(numeric_array const &rhs) noexcept
    {
        return rcp_hypot<D>(rhs);
    }

    template<ssize_t D>
    [[nodiscard]] friend constexpr numeric_array normalize(numeric_array const &rhs) noexcept
    {
        tt_axiom(rhs.is_vector());
        return rhs * rcp_length<D>(rhs);
    }

    [[nodiscard]] friend constexpr unsigned int eq(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_eq_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] == rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr unsigned int ne(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_ne_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] != rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr unsigned int lt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_lt_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] < rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr unsigned int gt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_gt_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] > rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr unsigned int le(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_le_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] <= rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr unsigned int ge(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_ge_mask(lhs.v, rhs.v);
        } else {
            unsigned int r = 0;
            for (ssize_t i = 0; i != N; ++i) {
                r |= static_cast<unsigned int>(lhs.v[i] >= rhs.v[i]) << i;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr bool operator==(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && has_sse) {
                // MSVC cannot vectorize comparison.
                return f32x4_sse_eq(lhs.v, rhs.v);
            }
        }

        auto r = true;
        for (ssize_t i = 0; i != N; ++i) {
            r &= (lhs.v[i] == rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr bool operator!=(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] + rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const &lhs, T const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] + rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(T const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs + rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array hadd(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_hadd(lhs.v, rhs.v)};

        } else {
            tt_axiom(N % 2 == 0);

            auto r = numeric_array{};

            ssize_t src_i = 0;
            ssize_t dst_i = 0;
            while (src_i != N) {
                auto tmp = lhs[src_i++];
                tmp += lhs[src_i++];
                r.v[dst_i++] = tmp;
            }

            src_i = 0;
            while (src_i != N) {
                auto tmp = rhs[src_i++];
                tmp += rhs[src_i++];
                r.v[dst_i++] = tmp;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array hsub(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_sse_hsub(lhs.v, rhs.v)};

        } else {
            tt_axiom(N % 2 == 0);

            auto r = numeric_array{};

            ssize_t src_i = 0;
            ssize_t dst_i = 0;
            while (src_i != N) {
                auto tmp = lhs[src_i++];
                tmp -= lhs[src_i++];
                r.v[dst_i++] = tmp;
            }

            src_i = 0;
            while (src_i != N) {
                auto tmp = rhs[src_i++];
                tmp -= rhs[src_i++];
                r.v[dst_i++] = tmp;
            }
            return r;
        }
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, T const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] - rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(T const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] * rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, T const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] * rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(T const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs * rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const &lhs, T const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] / rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(T const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] % rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const &lhs, T const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] % rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(T const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            r.v[i] = lhs % rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array min(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            // std::min() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array max(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            // std::max() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array
    clamp(numeric_array const &lhs, numeric_array const &low, numeric_array const &high) noexcept
    {
        auto r = numeric_array{};
        for (ssize_t i = 0; i != N; ++i) {
            // std::clamp() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] < low.v[i] ? low.v[i] : lhs.v[i] > high.v[i] ? high.v[i] : lhs.v[i];
        }
        return r;
    }

    /** Calculate the 2D normal on a 2D vector.
     */
    template<int D>
    [[nodiscard]] friend constexpr numeric_array normal(numeric_array const &rhs) noexcept requires(N >= 2)
    {
        static_assert(D == 2, "Normal on a single vector can only be calculated in two dimensions");
        tt_axiom(rhs.z() == 0.0f && rhs.is_vector());
        return normalize<D>(numeric_array{-rhs.y(), rhs.x()});
    }

    /** Calculate the cross-product between two 2D vectors.
     */
    template<int D>
    [[nodiscard]] friend constexpr float viktor_cross(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N >= 2)
    {
        static_assert(D == 2, "A cross-product must be done in two dimensions");

        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            return f32x4_sse_viktor_cross(lhs.v, rhs.v);

        } else {
            return lhs.x() * rhs.y() - lhs.y() * rhs.x();
        }
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    template<int D>
    [[nodiscard]] friend numeric_array cross(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        static_assert(D == 3 || D == 4, "A cross-product must be done in three or four dimensions");

        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && has_sse) {
                return numeric_array{f32x4_sse_cross(lhs.v, rhs.v)};
            }
        }

        return numeric_array{
            lhs.y() * rhs.z() - lhs.z() * rhs.y(),
            lhs.z() * rhs.x() - lhs.x() * rhs.z(),
            lhs.x() * rhs.y() - lhs.y() * rhs.x(),
            0.0f};
    }
    /** Find a point at the midpoint between two points.
     */
    [[nodiscard]] friend constexpr numeric_array midpoint(numeric_array const &p1, numeric_array const &p2) noexcept
    {
        tt_axiom(p1.is_point());
        tt_axiom(p2.is_point());
        return (p1 + p2) * 0.5f;
    }

    /** Find the point on the other side and at the same distance of an anchor-point.
     */
    [[nodiscard]] friend constexpr numeric_array reflect_point(numeric_array const &p, numeric_array const anchor) noexcept
    {
        tt_axiom(p.is_point());
        tt_axiom(anchor.is_point());
        return anchor - (p - anchor);
    }

    template<typename... Columns>
    [[nodiscard]] friend constexpr std::array<numeric_array, N> transpose(Columns const &... columns) noexcept
    {
        static_assert(sizeof...(Columns) == N, "Can only transpose square matrices");

        auto r = std::array<numeric_array, N>{};

        if (is_f32x4 && has_sse && !std::is_constant_evaluated()) {
            auto tmp = f32x4_sse_transpose(columns.v...);
            for (int i = 0; i != N; ++i) {
                r[i] = numeric_array{tmp[i]};
            }

        } else {
            transpose_detail<0, Columns...>(columns..., r);
        }

        return r;
    }

    [[nodiscard]] friend numeric_array desaturate(numeric_array const &color, T brightness) noexcept
        requires(N == 4 && std::is_floating_point_v<T>)
    {
        // Use luminance ratios and change the brightness.
        // luminance ratios according to BT.709.
        ttlet RGB0 = color * numeric_array::color({T{0.2126}, T{0.7152}, T{0.0722}, T{0.0}}) * brightness;
        ttlet tmp = hadd(RGB0, RGB0);
        ttlet luminance = hadd(tmp, tmp);
        return luminance.xxx0() + color._000w();
    }

    [[nodiscard]] friend numeric_array composit(numeric_array const &under, numeric_array const &over) noexcept
        requires(N == 4 && std::is_floating_point_v<T>)
    {
        if (over.is_transparent()) {
            return under;
        }
        if (over.is_opaque()) {
            return over;
        }

        ttlet over_alpha = over.wwww();
        ttlet under_alpha = under.wwww();

        ttlet over_color = over.xyz1();
        ttlet under_color = under.xyz1();

        ttlet output_color = over_color * over_alpha + under_color * under_alpha * (1.0 - over_alpha);

        return output_color / output_color.www1();
    }

    [[nodiscard]] friend std::string to_string(numeric_array const &rhs) noexcept
    {
        auto r = std::string{};

        r += '(';
        for (ssize_t i = 0; i != N; ++i) {
            if (i != 0) {
                r += "; ";
            }
            r += fmt::format("{}", rhs[i]);
        }
        r += ')';
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, numeric_array const &rhs)
    {
        return lhs << to_string(rhs);
    }

    /** swizzle around the elements of the numeric array.
     *
     * @tparam Elements a list of indices pointing to an element in this array.
     *         Or the special indices -1: literal zero, -2: literal one.
     * @return A new array with the elements ordered based on the Elements list.
     *         The elements at the end of the array are set to zero.
     */
    template<ssize_t... Elements>
    [[nodiscard]] constexpr numeric_array swizzle() const
    {
        static_assert(sizeof...(Elements) <= N);

        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && has_sse) {
                return numeric_array{f32x4_sse_swizzle<Elements...>(v)};
            }
        }

        auto r = numeric_array{};
        swizzle_detail<0, Elements...>(r);
        return r;
    }

#define SWIZZLE(swizzle_name, D, ...) \
    [[nodiscard]] numeric_array swizzle_name() const noexcept requires(D == N) \
    { \
        return swizzle<__VA_ARGS__>(); \
    }

#define SWIZZLE_4D_GEN1(name, ...) \
    SWIZZLE(name##0, 4, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 4, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 4, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 4, __VA_ARGS__, 1) \
    SWIZZLE(name##z, 4, __VA_ARGS__, 2) \
    SWIZZLE(name##w, 4, __VA_ARGS__, 3)

#define SWIZZLE_4D_GEN2(name, ...) \
    SWIZZLE_4D_GEN1(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_4D_GEN1(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_4D_GEN1(name##x, __VA_ARGS__, 0) \
    SWIZZLE_4D_GEN1(name##y, __VA_ARGS__, 1) \
    SWIZZLE_4D_GEN1(name##z, __VA_ARGS__, 2) \
    SWIZZLE_4D_GEN1(name##w, __VA_ARGS__, 3)

#define SWIZZLE_4D_GEN3(name, ...) \
    SWIZZLE_4D_GEN2(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_4D_GEN2(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_4D_GEN2(name##x, __VA_ARGS__, 0) \
    SWIZZLE_4D_GEN2(name##y, __VA_ARGS__, 1) \
    SWIZZLE_4D_GEN2(name##z, __VA_ARGS__, 2) \
    SWIZZLE_4D_GEN2(name##w, __VA_ARGS__, 3)

    SWIZZLE_4D_GEN3(_0, get_zero)
    SWIZZLE_4D_GEN3(_1, get_one)
    SWIZZLE_4D_GEN3(x, 0)
    SWIZZLE_4D_GEN3(y, 1)
    SWIZZLE_4D_GEN3(z, 2)
    SWIZZLE_4D_GEN3(w, 3)

#define SWIZZLE_3D_GEN1(name, ...) \
    SWIZZLE(name##0, 3, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 3, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 3, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 3, __VA_ARGS__, 1) \
    SWIZZLE(name##z, 3, __VA_ARGS__, 2)

#define SWIZZLE_3D_GEN2(name, ...) \
    SWIZZLE_3D_GEN1(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_3D_GEN1(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_3D_GEN1(name##x, __VA_ARGS__, 0) \
    SWIZZLE_3D_GEN1(name##y, __VA_ARGS__, 1) \
    SWIZZLE_3D_GEN1(name##z, __VA_ARGS__, 2)

    SWIZZLE_3D_GEN2(_0, get_zero)
    SWIZZLE_3D_GEN2(_1, get_one)
    SWIZZLE_3D_GEN2(x, 0)
    SWIZZLE_3D_GEN2(y, 1)
    SWIZZLE_3D_GEN2(z, 2)

#define SWIZZLE_2D_GEN1(name, ...) \
    SWIZZLE(name##0, 2, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 2, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 2, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 2, __VA_ARGS__, 1)

    SWIZZLE_2D_GEN1(_0, get_zero)
    SWIZZLE_2D_GEN1(_1, get_one)
    SWIZZLE_2D_GEN1(x, 0)
    SWIZZLE_2D_GEN1(y, 1)

#undef SWIZZLE
#undef SWIZZLE_4D_GEN1
#undef SWIZZLE_4D_GEN2
#undef SWIZZLE_4D_GEN3
#undef SWIZZLE_3D_GEN1
#undef SWIZZLE_3D_GEN2
#undef SWIZZLE_2D_GEN1

private:
    container_type v;

    template<int I, bool FirstNegate, bool... RestNegate>
    [[nodiscard]] friend constexpr void neg_detail(numeric_array &rhs) noexcept
    {
        if constexpr (FirstNegate) {
            get<I>(rhs.v) = -get<I>(rhs.v);
        }

        if constexpr (sizeof...(RestNegate) != 0) {
            neg_detail<I + 1, RestNegate...>(rhs);
        }
    }

    template<int I, typename First, typename... Rest>
    [[nodiscard]] friend constexpr void
    transpose_detail(First const &first, Rest const &... rest, std::array<numeric_array, N> &r) noexcept
    {
        for (ssize_t j = 0; j != N; ++j) {
            r[j][I] = first[j];
        }

        if constexpr (sizeof...(Rest) != 0) {
            transpose_detail<I + 1, Rest...>(rest..., r);
        }
    }

    template<ssize_t I, ssize_t FirstElement, ssize_t... RestElements>
    [[nodiscard]] constexpr void swizzle_detail(numeric_array &r) const noexcept
    {
        static_assert(I < N);
        static_assert(FirstElement >= -2 && FirstElement < N, "Index out of bounds");

        get<I>(r) = get<FirstElement>(*this);
        if constexpr (sizeof...(RestElements) != 0) {
            swizzle_detail<I + 1, RestElements...>(r);
        }
    }

    constexpr static bool is_i8x1 = std::is_same_v<T, int8_t> && N == 1;
    constexpr static bool is_i8x2 = std::is_same_v<T, int8_t> && N == 2;
    constexpr static bool is_i8x4 = std::is_same_v<T, int8_t> && N == 4;
    constexpr static bool is_i8x8 = std::is_same_v<T, int8_t> && N == 8;
    constexpr static bool is_i8x16 = std::is_same_v<T, int8_t> && N == 16;
    constexpr static bool is_i8x32 = std::is_same_v<T, int8_t> && N == 32;
    constexpr static bool is_i8x64 = std::is_same_v<T, int8_t> && N == 64;
    constexpr static bool is_u8x1 = std::is_same_v<T, uint8_t> && N == 1;
    constexpr static bool is_u8x2 = std::is_same_v<T, uint8_t> && N == 2;
    constexpr static bool is_u8x4 = std::is_same_v<T, uint8_t> && N == 4;
    constexpr static bool is_u8x8 = std::is_same_v<T, uint8_t> && N == 8;
    constexpr static bool is_u8x16 = std::is_same_v<T, uint8_t> && N == 16;
    constexpr static bool is_u8x32 = std::is_same_v<T, uint8_t> && N == 32;
    constexpr static bool is_u8x64 = std::is_same_v<T, uint8_t> && N == 64;

    constexpr static bool is_i16x1 = std::is_same_v<T, int16_t> && N == 1;
    constexpr static bool is_i16x2 = std::is_same_v<T, int16_t> && N == 2;
    constexpr static bool is_i16x4 = std::is_same_v<T, int16_t> && N == 4;
    constexpr static bool is_i16x8 = std::is_same_v<T, int16_t> && N == 8;
    constexpr static bool is_i16x16 = std::is_same_v<T, int16_t> && N == 16;
    constexpr static bool is_i16x32 = std::is_same_v<T, int16_t> && N == 32;
    constexpr static bool is_u16x1 = std::is_same_v<T, uint16_t> && N == 1;
    constexpr static bool is_u16x2 = std::is_same_v<T, uint16_t> && N == 2;
    constexpr static bool is_u16x4 = std::is_same_v<T, uint16_t> && N == 4;
    constexpr static bool is_u16x8 = std::is_same_v<T, uint16_t> && N == 8;
    constexpr static bool is_u16x16 = std::is_same_v<T, uint16_t> && N == 16;
    constexpr static bool is_u16x32 = std::is_same_v<T, uint16_t> && N == 32;

    constexpr static bool is_i32x1 = std::is_same_v<T, int32_t> && N == 1;
    constexpr static bool is_i32x2 = std::is_same_v<T, int32_t> && N == 2;
    constexpr static bool is_i32x4 = std::is_same_v<T, int32_t> && N == 4;
    constexpr static bool is_i32x8 = std::is_same_v<T, int32_t> && N == 8;
    constexpr static bool is_i32x16 = std::is_same_v<T, int32_t> && N == 16;
    constexpr static bool is_u32x1 = std::is_same_v<T, uint32_t> && N == 1;
    constexpr static bool is_u32x2 = std::is_same_v<T, uint32_t> && N == 2;
    constexpr static bool is_u32x4 = std::is_same_v<T, uint32_t> && N == 4;
    constexpr static bool is_u32x8 = std::is_same_v<T, uint32_t> && N == 8;
    constexpr static bool is_u32x16 = std::is_same_v<T, uint32_t> && N == 16;
    constexpr static bool is_f32x1 = std::is_same_v<T, float> && N == 1;
    constexpr static bool is_f32x2 = std::is_same_v<T, float> && N == 2;
    constexpr static bool is_f32x4 = std::is_same_v<T, float> && N == 4;
    constexpr static bool is_f32x8 = std::is_same_v<T, float> && N == 8;
    constexpr static bool is_f32x16 = std::is_same_v<T, float> && N == 16;

    constexpr static bool is_i64x1 = std::is_same_v<T, int64_t> && N == 1;
    constexpr static bool is_i64x2 = std::is_same_v<T, int64_t> && N == 2;
    constexpr static bool is_i64x4 = std::is_same_v<T, int64_t> && N == 4;
    constexpr static bool is_i64x8 = std::is_same_v<T, int64_t> && N == 8;
    constexpr static bool is_u64x1 = std::is_same_v<T, uint64_t> && N == 1;
    constexpr static bool is_u64x2 = std::is_same_v<T, uint64_t> && N == 2;
    constexpr static bool is_u64x4 = std::is_same_v<T, uint64_t> && N == 4;
    constexpr static bool is_u64x8 = std::is_same_v<T, uint64_t> && N == 8;
    constexpr static bool is_f64x1 = std::is_same_v<T, double> && N == 1;
    constexpr static bool is_f64x2 = std::is_same_v<T, double> && N == 2;
    constexpr static bool is_f64x4 = std::is_same_v<T, double> && N == 4;
    constexpr static bool is_f64x8 = std::is_same_v<T, double> && N == 8;
};

using i8x1 = numeric_array<int8_t, 1>;
using i8x2 = numeric_array<int8_t, 2>;
using i8x4 = numeric_array<int8_t, 4>;
using i8x8 = numeric_array<int8_t, 8>;
using i8x16 = numeric_array<int8_t, 16>;
using i8x32 = numeric_array<int8_t, 32>;
using i8x64 = numeric_array<int8_t, 64>;

using u8x1 = numeric_array<uint8_t, 1>;
using u8x2 = numeric_array<uint8_t, 2>;
using u8x4 = numeric_array<uint8_t, 4>;
using u8x8 = numeric_array<uint8_t, 8>;
using u8x16 = numeric_array<uint8_t, 16>;
using u8x32 = numeric_array<uint8_t, 32>;
using u8x64 = numeric_array<uint8_t, 64>;

using i16x1 = numeric_array<int16_t, 1>;
using i16x2 = numeric_array<int16_t, 2>;
using i16x4 = numeric_array<int16_t, 4>;
using i16x8 = numeric_array<int16_t, 8>;
using i16x16 = numeric_array<int16_t, 16>;
using i16x32 = numeric_array<int16_t, 32>;

using u16x1 = numeric_array<uint16_t, 1>;
using u16x2 = numeric_array<uint16_t, 2>;
using u16x4 = numeric_array<uint16_t, 4>;
using u16x8 = numeric_array<uint16_t, 8>;
using u16x16 = numeric_array<uint16_t, 16>;
using u16x32 = numeric_array<uint16_t, 32>;

using i32x1 = numeric_array<int32_t, 1>;
using i32x2 = numeric_array<int32_t, 2>;
using i32x4 = numeric_array<int32_t, 4>;
using i32x8 = numeric_array<int32_t, 8>;
using i32x16 = numeric_array<int32_t, 16>;

using u32x1 = numeric_array<uint32_t, 1>;
using u32x2 = numeric_array<uint32_t, 2>;
using u32x4 = numeric_array<uint32_t, 4>;
using u32x8 = numeric_array<uint32_t, 8>;
using u32x16 = numeric_array<uint32_t, 16>;

using f32x1 = numeric_array<float, 1>;
using f32x2 = numeric_array<float, 2>;
using f32x4 = numeric_array<float, 4>;
using f32x8 = numeric_array<float, 8>;
using f32x16 = numeric_array<float, 16>;

using i64x1 = numeric_array<int64_t, 1>;
using i64x2 = numeric_array<int64_t, 2>;
using i64x4 = numeric_array<int64_t, 4>;
using i64x8 = numeric_array<int64_t, 8>;

using u64x1 = numeric_array<uint64_t, 1>;
using u64x2 = numeric_array<uint64_t, 2>;
using u64x4 = numeric_array<uint64_t, 4>;
using u64x8 = numeric_array<uint64_t, 8>;

using f64x1 = numeric_array<double, 1>;
using f64x2 = numeric_array<double, 2>;
using f64x4 = numeric_array<double, 4>;
using f64x8 = numeric_array<double, 8>;

} // namespace tt

namespace std {

template<class T, std::size_t N>
struct tuple_size<tt::numeric_array<T, N>> : std::integral_constant<std::size_t, N> {
};

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, tt::numeric_array<T, N>> {
    using type = T;
};

} // namespace std
