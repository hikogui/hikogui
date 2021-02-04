
#pragma once

#include "vec.hpp"
#include "point.hpp"

namespace tt {

class matrix {
public:
    class I;
    template<int D>
    class T;
    template<int D>
    class S;
    template<int D>
    class R;

    // Identity matrix.
    class I {
    public:
        constexpr I(I const &) noexcept = default;
        constexpr I(I &&) noexcept = default;
        constexpr I &operator=(I const &) noexcept = default;
        constexpr I &operator=(I &&) noexcept = default;

        constexpr I() noexcept = default;

        template<int E>
        [[nodiscard]] constexpr vec<E> operator*(vec<E> const &rhs) const noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr point<E> operator*(point<E> const &rhs) const noexcept
        {
            return rhs;
        }

        [[nodiscard]] constexpr matrix operator*(matrix const &rhs) const noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr I operator*(I const &) const noexcept
        {
            return {};
        }

        template<int E>
        [[nodiscard]] constexpr matrix::T<E> operator*(matrix::T<E> const &rhs) const noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr matrix::S<E> operator*(matrix::S<E> const &rhs) const noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr matrix::R<E> operator*(matrix::R<E> const &rhs) const noexcept
        {
            return rhs;
        }

        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return true;
        }
    };

    // Translation matrix.
    template<int D>
    class T {
    public:
        static_assert(D == 2 || D == 3, "Only 2D or 3D translation-matrices are supported");

        constexpr T(T const &) noexcept = default;
        constexpr T(T &&) noexcept = default;
        constexpr T &operator=(T const &) noexcept = default;
        constexpr T &operator=(T &&) noexcept = default;

        [[nodiscard]] constexpr operator matrix() const noexcept
        {
            tt_axiom(is_valid());
            ttlet ones = f32x4::broadcast(1.0);
            return matrix{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
        }

        [[nodiscard]] constexpr T() noexcept : _v() {}

        [[nodiscard]] constexpr T(matrix::I const &) noexcept : matrix::T() {}

        [[nodiscard]] constexpr explicit operator f32x4() const noexcept
        {
            tt_axiom(is_valid());
            return _v;
        }

        [[nodiscard]] constexpr explicit T(f32x4 const &other) noexcept : _v(other)
        {
            tt_axiom(is_valid());
        }

        template<int E>
        requires(E < D) [[nodiscard]] constexpr T(matrix::T<E> const &other) noexcept : _v(static_cast<f32x4>(other))
        {
            tt_axiom(is_valid());
        }

        template<int E>
        requires(E <= D) [[nodiscard]] constexpr explicit T(vec<E> const &other) noexcept : _v(static_cast<f32x4>(other))
        {
            tt_axiom(is_valid());
        }

        [[nodiscard]] constexpr T(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0, 0.0) {}

        [[nodiscard]] constexpr T(float x, float y, float z = 0.0) noexcept requires(D == 3) : _v(x, y, z, 0.0) {}

        template<int E>
        [[nodiscard]] constexpr vec<E> operator*(vec<E> const &rhs) const noexcept
        {
            // Vectors are not translated.
            tt_axiom(is_valid() && rhs.is_valid());
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr point<std::max(D, E)> operator*(point<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return point<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
        }

        [[nodiscard]] constexpr T operator*(matrix::I const &) const noexcept
        {
            tt_axiom(is_valid());
            return *this;
        }

        template<int E>
        [[nodiscard]] constexpr auto operator*(matrix::T<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return matrix::T<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
        }

        template<int E>
        [[nodiscard]] constexpr auto operator*(matrix::S<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return matrix{
                static_cast<f32x4>(rhs).x000(), static_cast<f32x4>(rhs)._0y00(), static_cast<f32x4>(rhs)._00z0(), _v.xyz1()};
        }

        template<int E>
        [[nodiscard]] constexpr bool operator==(matrix::T<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return {_v == static_cast<f32x4>(rhs)};
        }

        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
        }

    private:
        f32x4 _v;
    };

    /** Scale matrix.
     */
    template<int D>
    class S {
    public:
        static_assert(D == 2 || D == 3, "Only 2D or 3D scale-matrices are supported");

        constexpr S(S const &) noexcept = default;
        constexpr S(S &&) noexcept = default;
        constexpr S &operator=(S const &) noexcept = default;
        constexpr S &operator=(S &&) noexcept = default;

        [[nodiscard]] constexpr explicit operator f32x4() const noexcept
        {
            tt_axiom(is_valid());
            return _v;
        }

        [[nodiscard]] constexpr explicit S(f32x4 const &v) noexcept : _v(v)
        {
            tt_axiom(is_valid());
        }

        [[nodiscard]] constexpr operator matrix() const noexcept
        {
            tt_axiom(is_valid());
            return matrix{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
        }

        [[nodiscard]] constexpr S() noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

        [[nodiscard]] constexpr S(matrix::I const &) noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

        [[nodiscard]] constexpr S(float value) noexcept requires(D == 2) : _v(value, value, 1.0, 1.0) {}

        [[nodiscard]] constexpr S(float value) noexcept requires(D == 3) : _v(value, value, value, 1.0) {}

        [[nodiscard]] constexpr S(float x, float y) noexcept requires(D == 2) : _v(x, y, 1.0, 1.0) {}

        [[nodiscard]] constexpr S(float x, float y, float z = 1.0) noexcept requires(D == 3) : _v(x, y, z, 1.0) {}

        template<int E>
        [[nodiscard]] constexpr vec<E> operator*(vec<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return vec<E>{_v * static_cast<f32x4>(rhs)};
        }

        template<int E>
        [[nodiscard]] constexpr point<E> operator*(point<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return point<E>{_v * static_cast<f32x4>(rhs)};
        }

        [[nodiscard]] constexpr S operator*(matrix::I const &) const noexcept
        {
            tt_axiom(is_valid());
            return *this;
        }

        template<int E>
        [[nodiscard]] constexpr auto operator*(matrix::T<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return matrix{_v.x000(), _v._0y00(), _v._00z0(), _v * static_cast<f32x4>(rhs).xyz1()};
        }

        template<int E>
        [[nodiscard]] constexpr auto operator*(matrix::S<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return matrix::S<std::max(D, E)>{_v * static_cast<f32x4>(rhs)};
        }

        template<int E>
        [[nodiscard]] constexpr bool operator==(matrix::S<E> const &rhs) const noexcept
        {
            tt_axiom(is_valid() && rhs.is_valid());
            return {_v == static_cast<f32x4>(rhs)};
        }

        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return _v.w() == 1.0f && (D == 3 || _v.z() == 1.0f);
        }

    private:
        f32x4 _v;
    };
    //
    //  template<int D>
    //  class R {
    //  public:
    //      static_assert(D == 2 || D == 3, "Only 2D or 3D rotation-matrices are supported");
    //
    //      matrix::R(matrix::R const &) noexcept = default;
    //      matrix::R(matrix::R &&) noexcept = default;
    //      matrix::R &operator=(matrix::R const &) noexcept = default;
    //      matrix::R &operator=(matrix::R &&) noexcept = default;
    //
    //      [[nodiscard]] R(float angle, vec<3> axis) noexcept requires(D == 3) : _v()
    //      {
    //          tt_axiom(axis.is_valid());
    //          tt_axiom(std::abs(hypot(axis) - 1.0f) < 0.0001f);
    //
    //          ttlet half_angle = angle * 0.5f;
    //          ttlet C = std::cos(half_angle);
    //          ttlet S = std::sin(half_angle);
    //
    //          _v = static_cast<f32x4>(axis) * S;
    //          _v.w() = C;
    //      }
    //
    //      /** Convert quaternion to matrix.
    //       *
    //       */
    //      [[nodiscard]] constexpr operator matrix() const noexcept
    //      {
    //          // Original from https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
    //          //   1 - 2(yy + zz) |     2(xy - zw) |     2(xz + yw)
    //          //       2(xy + zw) | 1 - 2(xx + zz) |     2(yz - xw)
    //          //       2(xz - yw) |     2(yz + xw) | 1 - 2(xx + yy)
    //
    //          // Flipping adds and multiplies:
    //          //   1 - 2(zz + yy) |     2(xy - zw) |     2(yw + xz)
    //          //       2(zw + yx) | 1 - 2(xx + zz) |     2(yz - xw)
    //          //       2(zx - yw) |     2(xw + zy) | 1 - 2(yy + xx)
    //
    //          // All multiplies.
    //          ttlet x_mul = _v.xxxx() * _v;
    //          ttlet y_mul = _v.yyyy() * _v;
    //          ttlet z_mul = _v.zzzz() * _v;
    //
    //          auto twos = f32x4(-2.0f, 2.0f, 2.0f, 0.0f);
    //          auto one = f32x4(1.0f, 0.0f, 0.0f, 0.0f);
    //          ttlet col0 = one + addsub<0b0011>(z_mul.zwxy(), y_mul.yxwz()) * twos;
    //          one = one.yxzw();
    //          twos = twos.yxzw();
    //          ttlet col1 = one + addsub<0b0110>(x_mul.yxwz(), z_mul.wzyx()) * twos;
    //          one = one.xzyw();
    //          twos = twos.xzyw();
    //          ttlet col2 = one + addsub<0b0101>(y_mul.wzyx(), x_mul.zwxy()) * twos;
    //          one = one.xywz();
    //          return matrix{col0, col1, col2, one};
    //      }
    //
    //      std::pair<float, vec<3>> angle_and_axis() const noexcept requires(D == 3)
    //      {
    //          ttlet rcp_length = rcp_hypot<0b0111>(_v);
    //          ttlet length = 1.0f / rcp_length;
    //
    //          return {2.0f * std::atan2(length), vec<3>{_v.xyz0() * rcp_length}};
    //      }
    //
    //  private:
    //      /** Rotation is stored as a quaternion
    //       * w + x*i + y*j + z*k
    //       */
    //      f32x4 _v;
    //  };

    using T2 = T<2>;
    using T3 = T<3>;
    using S2 = S<2>;
    using S3 = S<3>;

    constexpr matrix(matrix const &) noexcept = default;
    constexpr matrix(matrix &&) noexcept = default;
    constexpr matrix &operator=(matrix const &) noexcept = default;
    constexpr matrix &operator=(matrix &&) noexcept = default;

    constexpr matrix() noexcept :
        _col0(1.0f, 0.0f, 0.0f, 0.0f),
        _col1(0.0f, 1.0f, 0.0f, 0.0f),
        _col2(0.0f, 0.0f, 1.0f, 0.0f),
        _col3(0.0f, 0.0f, 0.0f, 1.0f){};

    constexpr matrix(f32x4 col0, f32x4 col1, f32x4 col2, f32x4 col3 = f32x4{0.0f, 0.0f, 0.0f, 0.1f}) noexcept :
        _col0(col0), _col1(col1), _col2(col2), _col3(col3)
    {
    }

    template<int E>
    [[nodiscard]] constexpr vec<3> operator*(vec<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return vec<3>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3 * static_cast<f32x4>(rhs).wwww()};
    }

    template<int E>
    [[nodiscard]] constexpr point<3> operator*(point<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return point<3>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3 * static_cast<f32x4>(rhs).wwww()};
    }

private:
    f32x4 _col0;
    f32x4 _col1;
    f32x4 _col2;
    f32x4 _col3;
};

} // namespace tt
