
#pragma once

#include "array_generic.hpp"
#include "half.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <utility>
#include <tuple>
#include <concepts>

hi_export_module(hikogui.SIMD.simd_intf);

hi_export namespace hi {
inline namespace v1 {

template<typename T, std::size_t N>
struct simd : std::array<T, N> {
    using array_type = std::array<T, N>;
    using value_type = T;
    using generic_type = array_generic<T, N>;

    constexpr simd() noexcept = default;
    constexpr simd(simd const&) noexcept = default;
    constexpr simd(simd&&) noexcept = default;
    constexpr simd& operator=(simd const&) noexcept = default;
    constexpr simd& operator=(simd&&) noexcept = default;

    template<std::same_as<value_type>... Args>
    constexpr simd(Args... args) noexcept
        requires(sizeof...(Args) == N)
        : array_type(generic_type::set(args...))
    {
    }

    constexpr explicit simd(value_type a) noexcept : array_type(generic_type::set(a)) {}

    template<typename O>
    constexpr simd(std::array<O, N> a) noexcept : array_type(generic_type::convert(a))
    {
    }

    [[nodiscard]] constexpr static simd make_undefined() noexcept
    {
        return simd{generic_type::undefined()};
    }

    [[nodiscard]] constexpr static simd make_zero() noexcept
    {
        return simd{generic_type::set_zero()};
    }

    [[nodiscard]] constexpr static simd make_one() noexcept
    {
        return simd{generic_type::set_one()};
    }

    [[nodiscard]] constexpr static simd make_all_ones() noexcept
    {
        return simd{generic_type::set_all_ones()};
    }

    [[nodiscard]] constexpr static simd broadcast(value_type a) noexcept
    {
        return simd{generic_type::broadcast(a)};
    }

    [[nodiscard]] constexpr static simd broadcast(array_type a) noexcept
    {
        return simd{generic_type::broadcast(a)};
    }

    [[nodiscard]] constexpr static simd make_mask(std::size_t mask) noexcept
    {
        return simd{generic_type::set_mask(mask)};
    }

    [[nodiscard]] constexpr std::size_t mask() noexcept
    {
        return generic_type::get_mask(*this);
    }

    [[nodiscard]] constexpr value_type x() const noexcept
    {
        return generic_type::template get<0>(*this);
    }

    [[nodiscard]] constexpr value_type y() const noexcept
    {
        return generic_type::template get<1>(*this);
    }

    [[nodiscard]] constexpr value_type z() const noexcept
    {
        return generic_type::template get<2>(*this);
    }

    [[nodiscard]] constexpr value_type w() const noexcept
    {
        return generic_type::template get<3>(*this);
    }

    [[nodiscard]] constexpr value_type& x() noexcept
    {
        return std::get<0>(*this);
    }

    [[nodiscard]] constexpr value_type& y() noexcept
    {
        return std::get<1>(*this);
    }

    [[nodiscard]] constexpr value_type& z() noexcept
    {
        return std::get<2>(*this);
    }

    [[nodiscard]] constexpr value_type& w() noexcept
    {
        return std::get<3>(*this);
    }

    [[nodiscard]] constexpr value_type r() const noexcept
    {
        return generic_type::template get<0>(*this);
    }

    [[nodiscard]] constexpr value_type g() const noexcept
    {
        return generic_type::template get<1>(*this);
    }

    [[nodiscard]] constexpr value_type b() const noexcept
    {
        return generic_type::template get<2>(*this);
    }

    [[nodiscard]] constexpr value_type a() const noexcept
    {
        return generic_type::template get<3>(*this);
    }

    [[nodiscard]] constexpr value_type& r() noexcept
    {
        return std::get<0>(*this);
    }

    [[nodiscard]] constexpr value_type& g() noexcept
    {
        return std::get<1>(*this);
    }

    [[nodiscard]] constexpr value_type& b() noexcept
    {
        return std::get<2>(*this);
    }

    [[nodiscard]] constexpr value_type& a() noexcept
    {
        return std::get<3>(*this);
    }

    [[nodiscard]] constexpr value_type width() const noexcept
    {
        return generic_type::template get<0>(*this);
    }

    [[nodiscard]] constexpr value_type height() const noexcept
    {
        return generic_type::template get<1>(*this);
    }

    [[nodiscard]] constexpr value_type depth() const noexcept
    {
        return generic_type::template get<2>(*this);
    }

    [[nodiscard]] constexpr value_type& width() noexcept
    {
        return std::get<0>(*this);
    }

    [[nodiscard]] constexpr value_type& height() noexcept
    {
        return std::get<1>(*this);
    }

    [[nodiscard]] constexpr value_type& depth() noexcept
    {
        return std::get<2>(*this);
    }

    [[nodiscard]] constexpr friend simd operator-(array_type a) noexcept
    {
        return simd{generic_type::neg(a)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd neg_mask(array_type a) noexcept
    {
        return simd{generic_type::template neg_mask<Mask>(a)};
    }

    [[nodiscard]] constexpr friend simd operator~(array_type a) noexcept
    {
        return simd{generic_type::inv(a)};
    }

    [[nodiscard]] constexpr friend simd rcp(array_type a) noexcept
    {
        return simd{generic_type::rcp(a)};
    }

    [[nodiscard]] constexpr friend simd sqrt(array_type a) noexcept
    {
        return simd{generic_type::sqrt(a)};
    }

    [[nodiscard]] constexpr friend simd rsqrt(array_type a) noexcept
    {
        return simd{generic_type::rsqrt(a)};
    }

    [[nodiscard]] constexpr friend simd abs(array_type a) noexcept
    {
        return simd{generic_type::abs(a)};
    }

    [[nodiscard]] constexpr friend simd round(array_type a) noexcept
    {
        return simd{generic_type::round(a)};
    }

    [[nodiscard]] constexpr friend simd floor(array_type a) noexcept
    {
        return simd{generic_type::floor(a)};
    }

    [[nodiscard]] constexpr friend simd ceil(array_type a) noexcept
    {
        return simd{generic_type::ceil(a)};
    }

    [[nodiscard]] constexpr friend simd operator+(array_type a, array_type b) noexcept
    {
        return simd{generic_type::add(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator-(array_type a, array_type b) noexcept
    {
        return simd{generic_type::sub(a, b)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd addsub_mask(array_type a, array_type b) noexcept
    {
        return simd{generic_type::template addsub_mask<Mask>(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator*(array_type a, array_type b) noexcept
    {
        return simd{generic_type::mul(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator/(array_type a, array_type b) noexcept
    {
        return simd{generic_type::div(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator%(array_type a, array_type b) noexcept
    {
        return simd{generic_type::mod(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator==(array_type a, array_type b) noexcept
    {
        return simd{generic_type::eq(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator!=(array_type a, array_type b) noexcept
    {
        return simd{generic_type::ne(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator<(array_type a, array_type b) noexcept
    {
        return simd{generic_type::lt(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator>(array_type a, array_type b) noexcept
    {
        return simd{generic_type::gt(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator<=(array_type a, array_type b) noexcept
    {
        return simd{generic_type::le(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator>=(array_type a, array_type b) noexcept
    {
        return simd{generic_type::ge(a, b)};
    }

    [[nodiscard]] constexpr friend bool test(array_type a, array_type b) noexcept
    {
        return generic_type::test(a, b);
    }

    [[nodiscard]] constexpr friend bool equal(array_type a, array_type b) noexcept
    {
        return generic_type::all_equal(a, b);
    }

    [[nodiscard]] constexpr friend simd max(simd a, simd b) noexcept
    {
        return simd{generic_type::max(a, b)};
    }

    [[nodiscard]] constexpr friend simd min(simd a, simd b) noexcept
    {
        return simd{generic_type::min(a, b)};
    }

    [[nodiscard]] constexpr friend simd clamp(simd v, simd lo, simd hi) noexcept
    {
        return simd{generic_type::clamp(v, lo, hi)};
    }

    [[nodiscard]] constexpr friend simd operator|(array_type a, array_type b) noexcept
    {
        return simd{generic_type::_or(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator&(array_type a, array_type b) noexcept
    {
        return simd{generic_type::_and(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator^(array_type a, array_type b) noexcept
    {
        return simd{generic_type::_xor(a, b)};
    }

    [[nodiscard]] constexpr friend simd sll(array_type a, unsigned int b) noexcept
    {
        return simd{generic_type::sll(a, b)};
    }

    [[nodiscard]] constexpr friend simd sra(array_type a, unsigned int b) noexcept
    {
        return simd{generic_type::sra(a, b)};
    }

    [[nodiscard]] constexpr friend simd srl(array_type a, unsigned int b) noexcept
    {
        return simd{generic_type::srl(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator<<(array_type a, unsigned int b) noexcept
    {
        return simd{generic_type::sll(a, b)};
    }

    [[nodiscard]] constexpr friend simd operator>>(array_type a, unsigned int b) noexcept
    {
        if constexpr (std::signed_integral<value_type>) {
            return simd{generic_type::sra(a, b)};
        } else {
            return simd{generic_type::srl(a, b)};
        }
    }

    [[nodiscard]] constexpr friend simd andnot(array_type a, array_type b) noexcept
    {
        return simd{generic_type::andnot(a, b)};
    }

    [[nodiscard]] constexpr friend simd hadd(array_type a, array_type b) noexcept
    {
        return simd{generic_type::hadd(a, b)};
    }

    [[nodiscard]] constexpr friend simd hsub(array_type a, array_type b) noexcept
    {
        return simd{generic_type::hsub(a, b)};
    }

    template<int... Indices>
    [[nodiscard]] constexpr friend simd shuffle(array_type a) noexcept
    {
        return simd{generic_type::template shuffle<Indices...>(a)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd blend(array_type a, array_type b) noexcept
    {
        return simd{generic_type::template blend<Mask>(a, b)};
    }

    template<int... Indices>
    [[nodiscard]] constexpr friend simd swizzle(array_type a) noexcept
    {
        return simd{generic_type::template swizzle<Indices...>(a)};
    }

    [[nodiscard]] constexpr friend simd sum(array_type a) noexcept
    {
        return simd{generic_type::sum(a)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd dot(array_type a, array_type b) noexcept
    {
        return simd{generic_type::template dot<Mask>(a, b)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd hypot(array_type a) noexcept
    {
        return simd{generic_type::template hypot<Mask>(a)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd rhypot(array_type a) noexcept
    {
        return simd{generic_type::template rhypot<Mask>(a)};
    }

    template<std::size_t Mask>
    [[nodiscard]] constexpr friend simd normalize(array_type a) noexcept
    {
        return simd{generic_type::template normalize<Mask>(a)};
    }

    template<std::derived_from<array_type>... Columns>
    [[nodiscard]] constexpr friend std::array<simd, N> transpose(Columns... columns) noexcept
    {
        auto const tmp = generic_type::template transpose<Columns...>(columns...);
        auto r = std::array<simd, N>{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = tmp[i];
        }
        return r;
    }

    constexpr simd& operator+=(array_type a) noexcept
    {
        return *this = *this + a;
    }

    constexpr simd& operator-=(array_type a) noexcept
    {
        return *this = *this - a;
    }

    constexpr simd& operator*=(array_type a) noexcept
    {
        return *this = *this * a;
    }

    constexpr simd& operator/=(array_type a) noexcept
    {
        return *this = *this / a;
    }

    constexpr simd& operator%=(array_type a) noexcept
    {
        return *this = *this % a;
    }

    constexpr simd& operator|=(array_type a) noexcept
    {
        return *this = *this | a;
    }

    constexpr simd& operator&=(array_type a) noexcept
    {
        return *this = *this & a;
    }

    constexpr simd& operator^=(array_type a) noexcept
    {
        return *this = *this ^ a;
    }

#define X_SWIZZLE_2D(NAME, X, Y) \
    [[nodiscard]] constexpr simd NAME() const noexcept \
        requires(N == 2) \
    { \
        return swizzle<X, Y>(*this); \
    }

#define X_SWIZZLE_2D_Y(NAME, X) \
    X_SWIZZLE_2D(NAME##1, X, -2) \
    X_SWIZZLE_2D(NAME##0, X, -1) \
    X_SWIZZLE_2D(NAME##x, X, 0) \
    X_SWIZZLE_2D(NAME##y, X, 1)

    X_SWIZZLE_2D_Y(_1, -2)
    X_SWIZZLE_2D_Y(_0, -1)
    X_SWIZZLE_2D_Y(x, 0)
    X_SWIZZLE_2D_Y(y, 1)

#define X_SWIZZLE_4D(NAME, X, Y, Z, W) \
    [[nodiscard]] constexpr simd NAME() const noexcept \
        requires(N == 4) \
    { \
        return swizzle<X, Y, Z, W>(*this); \
    }

#define X_SWIZZLE_4D_W(NAME, X, Y, Z) \
    X_SWIZZLE_4D(NAME##1, X, Y, Z, -2) \
    X_SWIZZLE_4D(NAME##0, X, Y, Z, -1) \
    X_SWIZZLE_4D(NAME##x, X, Y, Z, 0) \
    X_SWIZZLE_4D(NAME##y, X, Y, Z, 1) \
    X_SWIZZLE_4D(NAME##z, X, Y, Z, 2) \
    X_SWIZZLE_4D(NAME##w, X, Y, Z, 3)

#define X_SWIZZLE_4D_Z(NAME, X, Y) \
    X_SWIZZLE_4D_W(NAME##1, X, Y, -2) \
    X_SWIZZLE_4D_W(NAME##0, X, Y, -1) \
    X_SWIZZLE_4D_W(NAME##x, X, Y, 0) \
    X_SWIZZLE_4D_W(NAME##y, X, Y, 1) \
    X_SWIZZLE_4D_W(NAME##z, X, Y, 2) \
    X_SWIZZLE_4D_W(NAME##w, X, Y, 3)

#define X_SWIZZLE_4D_Y(NAME, X) \
    X_SWIZZLE_4D_Z(NAME##1, X, -2) \
    X_SWIZZLE_4D_Z(NAME##0, X, -1) \
    X_SWIZZLE_4D_Z(NAME##x, X, 0) \
    X_SWIZZLE_4D_Z(NAME##y, X, 1) \
    X_SWIZZLE_4D_Z(NAME##z, X, 2) \
    X_SWIZZLE_4D_Z(NAME##w, X, 3)

    X_SWIZZLE_4D_Y(_1, -2)
    X_SWIZZLE_4D_Y(_0, -1)
    X_SWIZZLE_4D_Y(x, 0)
    X_SWIZZLE_4D_Y(y, 1)
    X_SWIZZLE_4D_Y(z, 2)
    X_SWIZZLE_4D_Y(w, 3)
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

static_assert(equal(f32x2{2.0f, 3.0f}.xx(), f32x2{2.0f, 2.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.xy(), f32x2{2.0f, 3.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.x0(), f32x2{2.0f, 0.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.x1(), f32x2{2.0f, 1.0f}));

static_assert(equal(f32x2{2.0f, 3.0f}.yx(), f32x2{3.0f, 2.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.yy(), f32x2{3.0f, 3.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.y0(), f32x2{3.0f, 0.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}.y1(), f32x2{3.0f, 1.0f}));

static_assert(equal(f32x2{2.0f, 3.0f}._0x(), f32x2{0.0f, 2.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._0y(), f32x2{0.0f, 3.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._00(), f32x2{0.0f, 0.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._01(), f32x2{0.0f, 1.0f}));

static_assert(equal(f32x2{2.0f, 3.0f}._1x(), f32x2{1.0f, 2.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._1y(), f32x2{1.0f, 3.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._10(), f32x2{1.0f, 0.0f}));
static_assert(equal(f32x2{2.0f, 3.0f}._11(), f32x2{1.0f, 1.0f}));


} // namespace v1
}


template<class T, size_t N>
struct std::tuple_size<::hi::simd<T, N>> : std::integral_constant<size_t, N> {};

template<size_t I, class T, size_t N>
struct std::tuple_element<I, ::hi::simd<T, N>> {
    using type = T;
};

template<typename T, size_t N>
struct std::equal_to<::hi::simd<T, N>> {
    constexpr bool operator()(::hi::simd<T, N> const& lhs, ::hi::simd<T, N> const& rhs) const noexcept
    {
        return equal(lhs, rhs);
    }
};

hi_export template<typename T, size_t N>
struct std::formatter<::hi::simd<T, N>, char> : std::formatter<std::string, char> {
    auto format(::hi::simd<T, N> const& t, auto& fc) const
    {
        auto str = std::string{"("};

        for (auto i = 0; i != N; ++i) {
            if (i != 0) {
                str += ", ";
            }
            str += std::format("{}", t[i]);
        }
        str += ')';

        return std::formatter<std::string, char>::format(str, fc);
    }
};

