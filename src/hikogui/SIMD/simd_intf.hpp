// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "native_f32x4_sse.hpp"
#include "native_f64x4_avx.hpp"
#include "native_i32x4_sse2.hpp"
#include "native_i64x4_avx2.hpp"
#include "native_u32x4_sse2.hpp"
#include "native_simd_conversions_x86.hpp"

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <ostream>
#include <string>
#include <array>
#include <type_traits>
#include <concepts>
#include <bit>
#include <climits>
#include <utility>
#include <compare>

hi_export_module(hikogui.SIMD : intf);

hi_warning_push();
// C4702 unreachable code: Suppressed due intrinsics and std::is_constant_evaluated()
hi_warning_ignore_msvc(4702);
// C26467: Converting from floating point to unsigned integral types results in non-portable code if...
// We are trying to get SIMD intrinsics to work the same as scalar functions. 
hi_warning_ignore_msvc(26467);
// C26472: Don't use static_cast for arithmetic conversions.
// We are trying to get SIMD intrinsics to work the same as scalar functions.
hi_warning_ignore_msvc(26472)

#define HI_X_runtime_evaluate_if_valid(...) \
    do { \
        if (not std::is_constant_evaluated()) { \
            if constexpr (requires { __VA_ARGS__; }) { \
                return __VA_ARGS__; \
            } \
        } \
    } while (false)

#define HI_X_accessor(index, name) \
    [[nodiscard]] constexpr T name() const noexcept \
        requires(N > index) \
    { \
        HI_X_runtime_evaluate_if_valid(get<index>(reg())); \
        return std::get<index>(v); \
    } \
\
    [[nodiscard]] constexpr T& name() noexcept \
        requires(N > index) \
    { \
        return std::get<index>(v); \
    }

#define HI_X_binary_math_op(op) \
    [[nodiscard]] friend constexpr simd operator op(simd lhs, simd rhs) noexcept \
        requires(requires(value_type a, value_type b) { a op b; }) \
    { \
        HI_X_runtime_evaluate_if_valid(simd{lhs.reg() op rhs.reg()}); \
\
        auto r = simd{}; \
        for (std::size_t i = 0; i != N; ++i) { \
            r[i] = lhs[i] op rhs[i]; \
        } \
        return r; \
    }

#define HI_X_unary_bit_op(op) \
    [[nodiscard]] friend constexpr simd operator op(simd rhs) noexcept \
    { \
        HI_X_runtime_evaluate_if_valid(simd{op rhs.reg()}); \
\
        auto r = simd{}; \
        for (std::size_t i = 0; i != N; ++i) { \
            hilet rhs_vi = std::bit_cast<unsigned_type>(rhs[i]); \
            hilet r_vi = static_cast<unsigned_type>(op rhs_vi); \
            r[i] = std::bit_cast<value_type>(r_vi); \
        } \
        return r; \
    }

#define HI_X_binary_bit_op(op) \
    [[nodiscard]] friend constexpr simd operator op(simd lhs, simd rhs) noexcept \
    { \
        HI_X_runtime_evaluate_if_valid(simd{lhs.reg() op rhs.reg()}); \
\
        auto r = simd{}; \
        for (std::size_t i = 0; i != N; ++i) { \
            hilet lhs_vi = std::bit_cast<unsigned_type>(lhs[i]); \
            hilet rhs_vi = std::bit_cast<unsigned_type>(rhs[i]); \
            hilet r_vi = static_cast<unsigned_type>(lhs_vi op rhs_vi); \
            r[i] = std::bit_cast<value_type>(r_vi); \
        } \
        return r; \
    }

#define HI_X_binary_shift_op(op) \
    [[nodiscard]] friend constexpr simd operator op(simd const& lhs, unsigned int rhs) noexcept \
    { \
        HI_X_runtime_evaluate_if_valid(simd{lhs.reg() op rhs}); \
\
        auto r = simd{}; \
        for (std::size_t i = 0; i != N; ++i) { \
            r.v[i] = lhs.v[i] op rhs; \
        } \
        return r; \
    }

#define HI_X_binary_cmp_op(op) \
    [[nodiscard]] friend constexpr simd operator op(simd lhs, simd rhs) noexcept \
        requires(requires(value_type a, value_type b) { a op b; }) \
    { \
        HI_X_runtime_evaluate_if_valid(simd{lhs.reg() op rhs.reg()}); \
\
        auto r = simd{}; \
        for (std::size_t i = 0; i != N; ++i) { \
            r[i] = lhs[i] op rhs[i] ? ones_value : zero_value; \
        } \
        return r; \
    }

#define HI_X_binary_op_broadcast(op) \
    [[nodiscard]] friend constexpr auto operator op(value_type lhs, simd rhs) noexcept \
        requires(requires(value_type a, value_type b) { a op b; }) \
    { \
        return broadcast(lhs) op rhs; \
    } \
\
    [[nodiscard]] friend constexpr auto operator op(simd lhs, value_type rhs) noexcept \
        requires(requires(value_type a, value_type b) { a op b; }) \
    { \
        return lhs op broadcast(rhs); \
    }

#define HI_X_inplace_op(long_op, short_op) \
    template<typename Rhs> \
    constexpr simd& operator long_op(Rhs rhs) noexcept \
        requires(requires { *this short_op rhs; }) \
    { \
        return *this = *this short_op rhs; \
    }

hi_export namespace hi::inline v1 {

template<typename T>
concept simd_value_type = arithmetic<T> or std::same_as<T, half>;

template<simd_value_type T, std::size_t N>
struct simd {
    using value_type = T;
    constexpr static size_t size = N;

    using unsigned_type = make_uintxx_t<sizeof(value_type) * CHAR_BIT>;

    constexpr static bool has_native_type = requires { typename native_simd<value_type, size>::value_type; };
    using native_type = native_simd<value_type, size>;

    using array_type = std::array<value_type, size>;
    using size_type = typename array_type::size_type;
    using difference_type = typename array_type::difference_type;
    using reference = typename array_type::reference;
    using const_reference = typename array_type::const_reference;
    using pointer = typename array_type::pointer;
    using const_pointer = typename array_type::const_pointer;
    using iterator = typename array_type::iterator;
    using const_iterator = typename array_type::const_iterator;

    constexpr static value_type zero_value = value_type{};
    constexpr static value_type ones_value = std::bit_cast<value_type>(std::numeric_limits<unsigned_type>::max());

    array_type v;

    constexpr simd() noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { *this = simd{native_type{}}; }) {
                *this = simd{native_type{}};
            }
        }
        v = array_type{};
    }

    constexpr simd(simd const& rhs) noexcept = default;
    constexpr simd(simd&& rhs) noexcept = default;
    constexpr simd& operator=(simd const& rhs) noexcept = default;
    constexpr simd& operator=(simd&& rhs) noexcept = default;

    template<simd_value_type U>
    [[nodiscard]] constexpr explicit simd(simd<U, N> const& other) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { *this = simd{native_type{other.reg()}}; }) {
                *this = simd{native_type{other.reg()}};
                return;
            }
        }

        for (std::size_t i = 0; i != N; ++i) {
            if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                // SSE conversion round floats before converting to integer.
                v[i] = static_cast<value_type>(std::round(other[i]));
            } else {
                v[i] = static_cast<value_type>(other[i]);
            }
        }
    }

    template<simd_value_type U>
    [[nodiscard]] constexpr explicit simd(simd<U, size / 2> const& a, simd<U, size / 2> const& b) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd{native_type{a.reg(), b.reg()}}; }) {
                *this = simd{native_type{a.reg(), b.reg()}};
                return;
            }
        }

        for (std::size_t i = 0; i != size; ++i) {
            hilet tmp = i < (size / 2) ? a[i] : b[i];
            if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                // SSE conversion round floats before converting to integer.
                v[i] = static_cast<value_type>(std::round(tmp));
            } else {
                v[i] = static_cast<value_type>(tmp);
            }
        }
    }

    template<std::convertible_to<value_type>... Args>
    [[nodiscard]] constexpr explicit simd(value_type first, Args... args) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd{native_type{first, static_cast<value_type>(args)...}}; }) {
                *this = simd{native_type{first, static_cast<value_type>(args)...}};
                return;
            }
        }

        v = array_type{first, static_cast<value_type>(args)...};
    }

    [[nodiscard]] constexpr static simd broadcast(T rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{native_type::broadcast(rhs)});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = rhs;
        }
        return r;
    }

    [[nodiscard]] constexpr static simd epsilon() noexcept
    {
        if constexpr (std::is_floating_point_v<T>) {
            return broadcast(std::numeric_limits<T>::epsilon());
        } else {
            return broadcast(T{0});
        }
    }

    [[nodiscard]] simd(std::array<T, N> const& rhs) noexcept : v(rhs) {}

    simd& operator=(std::array<T, N> const& rhs) noexcept
    {
        v = rhs;
        return *this;
    }

    [[nodiscard]] operator std::array<T, N>() const noexcept
    {
        return v;
    }

    [[nodiscard]] explicit simd(native_type rhs) noexcept
        requires(requires { typename native_type::value_type; })
        : v(static_cast<array_type>(rhs))
    {
    }

    [[nodiscard]] auto reg() const noexcept
        requires(requires { typename native_type::value_type; })
    {
        return native_type{v};
    }

    template<simd_value_type O, size_t M>
    [[nodiscard]] constexpr static simd cast_from(simd<O, M> const& rhs) noexcept
        requires(sizeof(simd<O, M>) == sizeof(simd))
    {
        HI_X_runtime_evaluate_if_valid(simd{native_type::cast_from(rhs.reg())});

        return std::bit_cast<simd>(rhs);
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    template<std::size_t S>
    [[nodiscard]] constexpr static simd load(std::byte const *ptr) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{native_type{ptr}});

        auto r = simd{};
        std::memcpy(&r, ptr, S);
        return r;
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    [[nodiscard]] constexpr static simd load(std::byte const *ptr) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{native_type{ptr}});

        auto r = simd{};
        std::memcpy(&r, ptr, sizeof(r));
        return r;
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    [[nodiscard]] constexpr static simd load(T const *ptr) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{native_type{ptr}});

        auto r = simd{};
        std::memcpy(&r, ptr, sizeof(r));
        return r;
    }

    template<std::size_t S>
    constexpr void store(std::byte *ptr) const noexcept
    {
        HI_X_runtime_evaluate_if_valid(reg().store(ptr));
        std::memcpy(ptr, this, S);
    }

    /** Store a numeric array into memory.
     * @param[out] ptr A pointer to where the numeric array should be stored into memory.
     */
    constexpr void store(std::byte *ptr) const noexcept
    {
        HI_X_runtime_evaluate_if_valid(reg().store(ptr));
        store<sizeof(*this)>(ptr);
    }

    [[nodiscard]] constexpr size_t mask() const noexcept
    {
        HI_X_runtime_evaluate_if_valid(reg().mask());

        auto r = 0_uz;
        for (auto i = N; i != 0; --i) {
            r <<= 1;
            r |= std::bit_cast<unsigned_type>(v[i - 1]) >> (sizeof(unsigned_type) * CHAR_BIT - 1);
        }
        return r;
    }

    [[nodiscard]] constexpr T const& operator[](std::size_t i) const noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        hi_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T& operator[](std::size_t i) noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        hi_axiom(i < N);
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

    HI_X_accessor(0, x);
    HI_X_accessor(1, y);
    HI_X_accessor(2, z);
    HI_X_accessor(3, w);
    HI_X_accessor(0, r);
    HI_X_accessor(1, g);
    HI_X_accessor(2, b);
    HI_X_accessor(3, a);
    HI_X_accessor(0, width);
    HI_X_accessor(1, height);
    HI_X_accessor(2, depth);

    HI_X_binary_math_op(+);
    HI_X_binary_math_op(-);
    HI_X_binary_math_op(*);
    HI_X_binary_math_op(/);
    HI_X_binary_math_op(%);

    HI_X_binary_cmp_op(==);
    HI_X_binary_cmp_op(!=);
    HI_X_binary_cmp_op(<);
    HI_X_binary_cmp_op(>);
    HI_X_binary_cmp_op(<=);
    HI_X_binary_cmp_op(>=);

    HI_X_unary_bit_op(~);
    HI_X_binary_bit_op(^);
    HI_X_binary_bit_op(&);
    HI_X_binary_bit_op(|);
    HI_X_binary_shift_op(<<);
    HI_X_binary_shift_op(>>);

    [[nodiscard]] friend constexpr bool equal(simd lhs, simd rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(equal(lhs.reg(), rhs.reg()));

        for (auto i = 0_uz; i != N; ++i) {
            if (lhs.v[i] != rhs.v[i]) {
                return false;
            }
        }
        return true;
    }

    HI_X_binary_op_broadcast(==);
    HI_X_binary_op_broadcast(!=);
    HI_X_binary_op_broadcast(<);
    HI_X_binary_op_broadcast(>);
    HI_X_binary_op_broadcast(<=);
    HI_X_binary_op_broadcast(>=);
    HI_X_binary_op_broadcast(+);
    HI_X_binary_op_broadcast(-);
    HI_X_binary_op_broadcast(*);
    HI_X_binary_op_broadcast(/);
    HI_X_binary_op_broadcast(%);
    HI_X_binary_op_broadcast(&);
    HI_X_binary_op_broadcast(|);
    HI_X_binary_op_broadcast(^);

    HI_X_inplace_op(+=, +);
    HI_X_inplace_op(-=, -);
    HI_X_inplace_op(*=, *);
    HI_X_inplace_op(/=, /);
    HI_X_inplace_op(%=, %);
    HI_X_inplace_op(|=, |);
    HI_X_inplace_op(&=, &);
    HI_X_inplace_op(^=, ^);
    HI_X_inplace_op(<<=, <<);
    HI_X_inplace_op(>>=, >>);

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<std::size_t I>
    [[nodiscard]] friend constexpr T& get(simd& rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return std::get<I>(rhs.v);
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<std::size_t I>
    [[nodiscard]] friend constexpr T get(simd const& rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        HI_X_runtime_evaluate_if_valid(get<I>(rhs.reg()));
        return std::get<I>(rhs.v);
    }

    /** Insert a value in the array.
     *
     * @tparam I The index into the array.
     * @param lhs The vector to insert the value into.
     * @param rhs The value to insert.
     * @return The vector with the inserted value.
     */
    template<std::size_t I>
    [[nodiscard]] constexpr friend simd insert(simd const& lhs, value_type rhs) noexcept
    {
        static_assert(I < size);
        HI_X_runtime_evaluate_if_valid(simd{insert<I>(lhs.reg(), rhs)});

        auto r = lhs;
        std::get<I>(r.v) = rhs;
        return r;
    }

    /** Set individual elements to zero.
     *
     * @tparam Mask bit mask where '1' means to zero, '0' to keep original.
     */
    template<std::size_t Mask = ~std::size_t{0}>
    [[nodiscard]] friend constexpr simd set_zero(simd rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{set_zero<Mask>(rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            if (to_bool((Mask >> i) & 1)) {
                r.v[i] = T{0};
            } else {
                r.v[i] = rhs.v[i];
            }
        }
        return r;
    }

    /** Blend two numeric arrays.
     *
     * @tparam Mask One bit for each element selects; 0: lhs, 1: rhs.
     * @param lhs The left hand side
     * @param rhs The right hand side
     * @return The blended array.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr simd blend(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{blend<Mask>(lhs.reg(), rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = to_bool((Mask >> i) & 1) ? rhs[i] : lhs[i];
        }
        return r;
    }

    /** Blend the values using a dynamic mask.
     */
    [[nodiscard]] friend constexpr simd blend(simd const& a, simd const& b, simd const& mask)
    {
        HI_X_runtime_evaluate_if_valid(simd{blend(a.reg(), b.reg(), mask.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = mask[i] != T{0} ? b[i] : a[i];
        }
        return r;
    }

    /** Negate individual elements.
     *
     * @tparam Mask bit mask where '1' means to negate, '0' to keep original.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr simd neg(simd rhs) noexcept
    {
        return blend<Mask>(rhs, -rhs);
    }

    [[nodiscard]] friend constexpr simd operator-(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{-rhs.reg()});
        return T{0} - rhs;
    }

    [[nodiscard]] friend constexpr simd abs(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{abs(rhs.reg())});
        return max(rhs, -rhs);
    }

    [[nodiscard]] friend constexpr simd rcp(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{rcp(rhs.reg())});
        return T{1} / rhs;
    }

    [[nodiscard]] friend constexpr simd sqrt(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{sqrt(rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::sqrt(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd rcp_sqrt(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{rcp_sqrt(rhs.reg())});
        return rcp(sqrt(rhs));
    }

    [[nodiscard]] friend constexpr simd floor(simd const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(simd{floor(rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::floor(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd ceil(simd const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(simd{ceil(rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::ceil(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd round(simd const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(simd{round(rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::round(rhs.v[i]);
        }
        return r;
    }

    /** Take a dot product.
     *
     * @tparam Mask A mask for which elements participate in the dot product.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the dot product.
     */
    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline friend constexpr T dot(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(dot<Mask>(lhs.reg(), rhs.reg())));

        auto r = T{};
        for (std::size_t i = 0; i != N; ++i) {
            if (to_bool(Mask & (1_uz << i))) {
                r += lhs.v[i] * rhs.v[i];
            }
        }
        return r;
    }

    /** Take the length of the vector
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend T hypot(simd const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(get<0>(sqrt(dot<Mask>(rhs.reg(), rhs.reg()))));
        return std::sqrt(dot<Mask>(rhs, rhs));
    }

    /** Take the squared length of the vector.
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline friend constexpr T squared_hypot(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(dot<Mask>(rhs.reg(), rhs.reg())));
        return dot<Mask>(rhs, rhs);
    }

    /** Take a reciprocal of the length.
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T rcp_hypot(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(rcp_sqrt(dot<Mask>(rhs.reg(), rhs.reg()))));
        return 1.0f / hypot<Mask>(rhs);
    }

    /** Normalize a vector.
     * All elements that do not participate in the normalization will be set to zero.
     *
     * @tparam Mask A mask for which elements participate in the normalization calculation.
     * @param rhs The right hand side.
     * @return The normalized vector.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr simd normalize(simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{rhs * rcp_sqrt(dot<Mask>(rhs.reg(), rhs.reg()))});

        hilet rcp_hypot_ = rcp_hypot<Mask>(rhs);

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            if (to_bool(Mask & (1_uz << i))) {
                r.v[i] = rhs.v[i] * rcp_hypot_;
            }
        }
        return r;
    }

    /** Rotate left.
     *
     * @note It is undefined behavior if: rhs <= 0 or rhs >= sizeof(value_type) * CHAR_BIT.
     */
    [[nodiscard]] friend constexpr simd rotl(simd const& lhs, unsigned int rhs) noexcept
    {
        hi_axiom(rhs > 0 and rhs < sizeof(value_type) * CHAR_BIT);

        hilet remainder = narrow_cast<unsigned int>(sizeof(value_type) * CHAR_BIT - rhs);

        return (lhs << rhs) | (lhs >> remainder);
    }

    /** Rotate right.
     *
     * @note It is undefined behavior if: rhs <= 0 or rhs >= sizeof(value_type) * CHAR_BIT.
     */
    [[nodiscard]] friend constexpr simd rotr(simd const& lhs, unsigned int rhs) noexcept
    {
        hi_axiom(rhs > 0 and rhs < sizeof(value_type) * CHAR_BIT);

        hilet remainder = narrow_cast<unsigned int>(sizeof(value_type) * CHAR_BIT - rhs);

        return (lhs >> rhs) | (lhs << remainder);
    }

    [[nodiscard]] friend constexpr simd min(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{min(lhs.reg(), rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::min(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd max(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{max(lhs.reg(), rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::max(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd clamp(simd const& lhs, simd const& low, simd const& high) noexcept
    {
        return min(max(lhs, low), high);
    }

    [[nodiscard]] friend constexpr simd hadd(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{horizontal_add(lhs.reg(), rhs.reg())});

        hi_axiom(N % 2 == 0);

        auto r = simd{};

        std::size_t src_i = 0;
        std::size_t dst_i = 0;
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

    [[nodiscard]] friend constexpr simd hsub(simd const& lhs, simd const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(simd{horizontal_sub(lhs.reg(), rhs.reg())});

        hi_axiom(N % 2 == 0);

        auto r = simd{};

        std::size_t src_i = 0;
        std::size_t dst_i = 0;
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

    /** Add or subtract individual elements.
     *
     * @tparam Mask bit mask where '1' means to add, '0' means to subtract.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr simd addsub(simd const& lhs, simd const& rhs) noexcept
    {
        constexpr std::size_t not_mask = (1 << N) - 1;
        return lhs + neg<Mask ^ not_mask>(rhs);
    }

    /** Calculate the 2D normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr simd cross_2D(simd const& rhs) noexcept
        requires(N >= 2)
    {
        return simd{-rhs.y(), rhs.x()};
    }

    /** Calculate the 2D unit-normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr simd normal_2D(simd const& rhs) noexcept
        requires(N >= 2)
    {
        return normalize<0b0011>(cross_2D(rhs));
    }

    /** Calculate the cross-product between two 2D vectors.
     * a.x * b.y - a.y * b.x
     */
    [[nodiscard]] friend constexpr float cross_2D(simd const& lhs, simd const& rhs) noexcept
        requires(N >= 2)
    {
        hilet tmp1 = rhs.yxwz();
        hilet tmp2 = lhs * tmp1;
        hilet tmp3 = hsub(tmp2, tmp2);
        return get<0>(tmp3);
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    [[nodiscard]] constexpr friend simd cross_3D(simd const& lhs, simd const& rhs) noexcept
        requires(N == 4)
    {
        hilet a_left = lhs.yzxw();
        hilet b_left = rhs.zxyw();
        hilet left = a_left * b_left;

        hilet a_right = lhs.zxyw();
        hilet b_right = rhs.yzxw();
        hilet right = a_right * b_right;
        return left - right;
    }

    [[nodiscard]] constexpr static simd byte_srl_shuffle_indices(unsigned int rhs)
        requires(std::is_same_v<value_type, int8_t> and size == 16)
    {
        static_assert(std::endian::native == std::endian::little);

        auto r = simd{};
        for (auto i = 0; i != 16; ++i) {
            if ((i + rhs) < 16) {
                r[i] = narrow_cast<int8_t>(i + rhs);
            } else {
                // Indices set to -1 result in a zero after a byte shuffle.
                r[i] = -1;
            }
        }
        return r;
    }

    [[nodiscard]] constexpr static simd byte_sll_shuffle_indices(unsigned int rhs)
        requires(std::is_same_v<value_type, int8_t> and size == 16)
    {
        static_assert(std::endian::native == std::endian::little);

        auto r = simd{};
        for (auto i = 0; i != 16; ++i) {
            if ((i - rhs) >= 0) {
                r[i] = narrow_cast<int8_t>(i - rhs);
            } else {
                // Indices set to -1 result in a zero after a byte shuffle.
                r[i] = -1;
            }
        }
        return r;
    }

    /** Shuffle a 16x byte array, using the indices from the right-hand-side.
     */
    [[nodiscard]] friend constexpr simd permute(simd const& lhs, simd const& rhs) noexcept
        requires(std::is_integral_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(simd{permute(lhs.reg(), rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            if (rhs[i] >= 0) {
                r[i] = lhs[rhs[i] & 0xf];
            } else {
                r[i] = 0;
            }
        }

        return r;
    }

    /** Find a point at the midpoint between two points.
     */
    [[nodiscard]] friend constexpr simd midpoint(simd const& p1, simd const& p2) noexcept
    {
        return (p1 + p2) * 0.5f;
    }

    /** Find the point on the other side and at the same distance of an anchor-point.
     */
    [[nodiscard]] friend constexpr simd reflect_point(simd const& p, simd const anchor) noexcept
    {
        return anchor - (p - anchor);
    }

    hi_warning_push();
    // C26494 Variable '...' is uninitialized. Always initialize an object (type.5).
    // Internal to _MM_TRANSPOSE4_PS
    hi_warning_ignore_msvc(26494);
    template<typename... Columns>
    [[nodiscard]] friend constexpr std::array<simd, size> transpose(Columns const&...columns) noexcept
    {
        static_assert(sizeof...(Columns) == size, "Can only transpose square matrices");

        if (not std::is_constant_evaluated()) {
            if constexpr (requires { transpose(columns.reg()...); }) {
                hilet tmp = transpose(columns.reg()...);
                auto r = std::array<simd, size>{};
                for (auto i = 0_uz; i != size; ++i) {
                    r[i] = simd{tmp[i]};
                }
                return r;
            }
        }

        auto r = std::array<simd, N>{};
        auto f = [&r, &columns... ]<std::size_t... Ints>(std::index_sequence<Ints...>)
        {
            auto tf = [&r](auto i, auto v) {
                for (std::size_t j = 0; j != N; ++j) {
                    r[j][i] = v[j];
                }
                return 0;
            };
            static_cast<void>((tf(Ints, columns) + ...));
        };
        f(std::make_index_sequence<sizeof...(columns)>{});
        return r;
    }
    hi_warning_pop();

    [[nodiscard]] constexpr friend simd composit(simd const& under, simd const& over) noexcept
        requires(N == 4 && std::is_floating_point_v<T>)
    {
        if (get<3>(over) <= value_type{0}) {
            // fully transparent.
            return under;
        }
        if (get<3>(over) >= value_type{1}) {
            // fully opaque;
            return over;
        }

        hilet over_alpha = over.wwww();
        hilet under_alpha = under.wwww();

        hilet over_color = over.xyz1();
        hilet under_color = under.xyz1();

        hilet output_color = over_color * over_alpha + under_color * under_alpha * (T{1} - over_alpha);

        return output_color / output_color.www1();
    }

    [[nodiscard]] constexpr friend simd composit(simd const& under, simd const& over) noexcept
        requires(std::is_same_v<value_type, half> and size == 4)
    {
        return simd{composit(static_cast<simd<float, 4>>(under), static_cast<simd<float, 4>>(over))};
    }

    [[nodiscard]] friend std::string to_string(simd const& rhs) noexcept
    {
        auto r = std::string{};

        r += '(';
        for (std::size_t i = 0; i != N; ++i) {
            if (i != 0) {
                r += "; ";
            }
            r += std::format("{}", rhs[i]);
        }
        r += ')';
        return r;
    }

    friend std::ostream& operator<<(std::ostream& lhs, simd const& rhs)
    {
        return lhs << to_string(rhs);
    }

    /** Insert an element from rhs into the result.
     * This function copies the lhs, then inserts one element from rhs into the result.
     * It also can clear any of the elements to zero.
     */
    template<std::size_t FromElement, std::size_t ToElement>
    [[nodiscard]] constexpr friend simd insert(simd const& lhs, simd const& rhs)
    {
        HI_X_runtime_evaluate_if_valid(simd{insert<FromElement, ToElement>(lhs.reg(), rhs.reg())});

        auto r = simd{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = (i == ToElement) ? rhs[FromElement] : lhs[i];
        }

        return r;
    }

    /** swizzle around the elements of the numeric array.
     *
     * @tparam Order a list of elements encoded as characters, 'a' - 'z' for indices to elements.
     *         '0' for a literal zero and '1' for a literal one.
     * @return A new array with the elements ordered based on the @a Order.
     *         The elements at the end of the array are set to zero.
     */
    template<fixed_string Order>
    [[nodiscard]] constexpr simd swizzle() const
    {
        static_assert(Order.size() <= N);

        HI_X_runtime_evaluate_if_valid(simd{reg().template swizzle<Order>()});

        auto r = simd{};
        swizzle_detail<0, Order>(r);
        return r;
    }

#define SWIZZLE(name, str) \
    [[nodiscard]] constexpr simd name() const noexcept \
        requires(sizeof(str) - 1 <= N) \
    { \
        return swizzle<str>(); \
    }

#define SWIZZLE_4D(name, str) \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

#define SWIZZLE_3D(name, str) \
    SWIZZLE_4D(name##0, str "0") \
    SWIZZLE_4D(name##1, str "1") \
    SWIZZLE_4D(name##x, str "a") \
    SWIZZLE_4D(name##y, str "b") \
    SWIZZLE_4D(name##z, str "c") \
    SWIZZLE_4D(name##w, str "d") \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

#define SWIZZLE_2D(name, str) \
    SWIZZLE_3D(name##0, str "0") \
    SWIZZLE_3D(name##1, str "1") \
    SWIZZLE_3D(name##x, str "a") \
    SWIZZLE_3D(name##y, str "b") \
    SWIZZLE_3D(name##z, str "c") \
    SWIZZLE_3D(name##w, str "d") \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

    SWIZZLE_2D(_0, "0")
    SWIZZLE_2D(_1, "1")
    SWIZZLE_2D(x, "a")
    SWIZZLE_2D(y, "b")
    SWIZZLE_2D(z, "c")
    SWIZZLE_2D(w, "d")

#undef SWIZZLE
#undef SWIZZLE_2D
#undef SWIZZLE_3D
#undef SWIZZLE_4D

    template<size_t I, fixed_string Order>
    constexpr void swizzle_detail(simd& r) const noexcept
    {
        static_assert(I < size);

        // Get the source element, or '0'.
        constexpr char c = I < Order.size() ? get<I>(Order) : '0';

        if constexpr (c == '1') {
            r = insert<I>(r, value_type{1});

        } else if constexpr (c == '0') {
            r = insert<I>(r, value_type{0});

        } else if constexpr (c >= 'a' and c <= 'v') {
            constexpr size_t src_index = c - 'a';
            static_assert(src_index < size);

            r = insert<I>(r, get<src_index>(*this));

        } else if constexpr (c >= 'w' and c <= 'z') {
            constexpr size_t src_index = c == 'x' ? 0 : c == 'y' ? 1 : c == 'z' ? 2 : 3;
            static_assert(src_index < size);

            r = insert<I>(r, get<src_index>(*this));

        } else {
            hi_static_no_default();
        }

        if constexpr (I + 1 < size) {
            swizzle_detail<I + 1, Order>(r);
        }
    }
};

using i8x1 = simd<int8_t, 1>;
using i8x2 = simd<int8_t, 2>;
using i8x4 = simd<int8_t, 4>;
using i8x8 = simd<int8_t, 8>;
using i8x16 = simd<int8_t, 16>;
using i8x32 = simd<int8_t, 32>;
using i8x64 = simd<int8_t, 64>;

using u8x1 = simd<uint8_t, 1>;
using u8x2 = simd<uint8_t, 2>;
using u8x4 = simd<uint8_t, 4>;
using u8x8 = simd<uint8_t, 8>;
using u8x16 = simd<uint8_t, 16>;
using u8x32 = simd<uint8_t, 32>;
using u8x64 = simd<uint8_t, 64>;

using i16x1 = simd<int16_t, 1>;
using i16x2 = simd<int16_t, 2>;
using i16x4 = simd<int16_t, 4>;
using i16x8 = simd<int16_t, 8>;
using i16x16 = simd<int16_t, 16>;
using i16x32 = simd<int16_t, 32>;

using u16x1 = simd<uint16_t, 1>;
using u16x2 = simd<uint16_t, 2>;
using u16x4 = simd<uint16_t, 4>;
using u16x8 = simd<uint16_t, 8>;
using u16x16 = simd<uint16_t, 16>;
using u16x32 = simd<uint16_t, 32>;

using f16x4 = simd<half, 4>;

using i32x1 = simd<int32_t, 1>;
using i32x2 = simd<int32_t, 2>;
using i32x4 = simd<int32_t, 4>;
using i32x8 = simd<int32_t, 8>;
using i32x16 = simd<int32_t, 16>;

using u32x1 = simd<uint32_t, 1>;
using u32x2 = simd<uint32_t, 2>;
using u32x4 = simd<uint32_t, 4>;
using u32x8 = simd<uint32_t, 8>;
using u32x16 = simd<uint32_t, 16>;

using f32x1 = simd<float, 1>;
using f32x2 = simd<float, 2>;
using f32x4 = simd<float, 4>;
using f32x8 = simd<float, 8>;
using f32x16 = simd<float, 16>;

using i64x1 = simd<int64_t, 1>;
using i64x2 = simd<int64_t, 2>;
using i64x4 = simd<int64_t, 4>;
using i64x8 = simd<int64_t, 8>;

using u64x1 = simd<uint64_t, 1>;
using u64x2 = simd<uint64_t, 2>;
using u64x4 = simd<uint64_t, 4>;
using u64x8 = simd<uint64_t, 8>;

using f64x1 = simd<double, 1>;
using f64x2 = simd<double, 2>;
using f64x4 = simd<double, 4>;
using f64x8 = simd<double, 8>;

} // namespace hi::inline v1

template<class T, std::size_t N>
struct std::tuple_size<hi::simd<T, N>> : std::integral_constant<std::size_t, N> {};

template<std::size_t I, class T, std::size_t N>
struct std::tuple_element<I, hi::simd<T, N>> {
    using type = T;
};

template<typename T, size_t N>
struct std::equal_to<::hi::simd<T, N>> {
    constexpr bool operator()(::hi::simd<T, N> const& lhs, ::hi::simd<T, N> const& rhs) const noexcept
    {
        return equal(lhs, rhs);
    }
};

// Add equality operator to Google-test internal namespace so that ASSERT_EQ() work.
template<typename T, size_t N>
hi_inline bool operator==(::hi::simd<T, N> lhs, ::hi::simd<T, N> rhs) noexcept
{
    return std::equal_to{}(lhs, rhs);
}

// Add equality operator to Google-test internal namespace so that ASSERT_NE() work.
template<typename T, size_t N>
hi_inline bool operator!=(::hi::simd<T, N> lhs, ::hi::simd<T, N> rhs) noexcept
{
    return not std::equal_to<::hi::simd<T, N>>{}(lhs, rhs);
}

#undef HI_X_accessor
#undef HI_X_binary_cmp_op
#undef HI_X_binary_math_op
#undef HI_X_unary_bit_op
#undef HI_X_binary_bit_op
#undef HI_X_binary_shift_op
#undef HI_X_binary_op_broadcast
#undef HI_X_inplace_op
#undef HI_X_runtime_evaluate_if_valid

hi_warning_pop();
