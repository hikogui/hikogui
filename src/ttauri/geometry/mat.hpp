
#pragma once

#include "mat.hpp"
#include "vec3.hpp"
#include "point3.hpp"
#include "quaternion3.hpp"

namespace tt {

// clang-format off
template<int D> requires (D == 2 || D == 3)
class mat {
public:
    // Identity matrix.
    class I {
    public:
        constexpr I(I const &) noexcept = default;    
        constexpr I(I &&) noexcept = default;    
        constexpr I &operator=(I const &) noexcept = default;    
        constexpr I &operator=(I &&) noexcept = default;    

        template<int E>
        [[nodiscard]] constexpr friend vec<E> operator(vec<E> const &lhs, mat::I const &) noexcept { return lhs; }
        template<int E>
        [[nodiscard]] constexpr friend point<E> operator(point<E> const &lhs, mat::I const &) noexcept { return lhs; }
        template<int E>
        [[nodiscard]] constexpr friend mat<E> operator(mat<E> const &lhs, mat::I const &) noexcept { return lhs; }
        template<int E>
        [[nodiscard]] constexpr friend mat<E>::I operator(mat<E>::I const &, mat::I const &) noexcept { return {}; }
    };

    // Translation matrix.
    class T {
    public:
        constexpr T(T const &) noexcept = default;    
        constexpr T(T &&) noexcept = default;    
        constexpr T &operator=(T const &) noexcept = default;    
        constexpr T &operator=(T &&) noexcept = default;    

        [[nodiscard]] constexpr explicit operator f32x4() const noexcept { return v; }
        [[nodiscard]] constexpr explicit T3(f32x4 const &other) noexcept : v(other)
        {
            if constexpr (D == 2) {
                tt_axiom(v.z() == 0.0);
            }
            tt_axiom(v.w() == 0.0);
        }

        [[nodiscard]] constexpr operator mat() const noexcept {
            ttlet ones = f32x4::broadcast(1.0};
            return mat{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + v};
        }

        [[nodiscard]] constexpr T() noexcept : v() {}
        [[nodiscard]] constexpr T(mat::I const &) noexcept : v() {}
        template<int E> requires (E < D)
        [[nodiscard]] constexpr T(T<E> const &other) noexcept : v(other.v) {}
        template<int E> requires (E <= D)
        [[nodiscard]] constexpr T(vec<E> const &other) noexcept : v(other.v) {}
        [[nodiscard]] constexpr T(float x, float y) noexcept requires (D == 2): v(x, y, 0.0, 0.0) {}
        [[nodiscard]] constexpr T(float x, float y, float z=0.0) noexcept requires (D == 3): v(x, y, z, 0.0) {}

        template<int E>
        [[nodiscard]] constexpr friend vec<E> operator*(vec<E> const &lhs, mat::T const &) noexcept
        {
            // Vectors are not translated.
            return lhs;
        }

        template<int E>
        [[nodiscard]] constexpr friend point<std::max(E,D)> operator*(point<E> const &lhs, mat::T const &rhs) noexcept
        {
            return point<std:max(E,D)>{lhs._v + rhs._v};
        }

        template<int E>
        [[nodiscard]] constexpr friend mat::T operator*(mat<E>::I const &, mat::T const &rhs) noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr friend mat<std::max(E,D)>::T operator*(mat<E>::T const &lhs, mat::T const &rhs) noexcept
        {
            return mat<std::max(E,D)>::T{lhs.v + rhs.v};
        }

    private:
        f32x4 _v;
    };

    // Scale matrix
    class S {
    public:
        constexpr S(S const &) noexcept = default;    
        constexpr S(S &&) noexcept = default;    
        constexpr S &operator=(S const &) noexcept = default;    
        constexpr S &operator=(S &&) noexcept = default;    

        [[nodiscard]] constexpr explicit operator f32x4() const noexcept { return _v; }
        [[nodiscard]] constexpr explicit S(f32x4 const &v) noexcept : _v(v)
        {
            if constexpr (D == 2) {
                tt_axiom(_v.z() == 1.0);
            }
            tt_axiom(_v.w() == 1.0);
        }

        [[nodiscard]] constexpr operator mat() const noexcept {
            return mat{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
        }

        [[nodiscard]] constexpr S() noexcept : _v(1.0, 1.0, 1.0, 1.0) {}
        [[nodiscard]] constexpr S(mat::I const &) noexcept : _v(1.0, 1.0, 1.0, 1.0) {}
        [[nodiscard]] constexpr S(float value) noexcept requires (D == 2) : _v(value, value, 1.0, 1.0) {}
        [[nodiscard]] constexpr S(float value) noexcept requires (D == 3) : _v(value, value, value, 1.0) {}
        [[nodiscard]] constexpr S(float x, float y) noexcept requires (D == 2) : _v(x, y, 1.0, 1.0) {}
        [[nodiscard]] constexpr S(float x, float y, float z = 1.0) noexcept requires (D == 3): _v(x, y, z, 1.0) {}

        template<int E>
        [[nodiscard]] constexpr friend vec<E> operator(vec<E> const &lhs, mat::S const &rhs) noexcept
        {
            return vec<E>{lhs._v * rhs._v};
        }

        template<int E>
        [[nodiscard]] constexpr friend point<E> operator(point<E> const &lhs, mat::S const &rhs) noexcept
        {
            return point<E>{lhs._v * rhs._v};
        }

        [[nodiscard]] constexpr friend mat::S operator(mat::I const &, mat::S const &rhs) noexcept
        {
            return rhs;
        }

        template<int E>
        [[nodiscard]] constexpr friend mat<std::max(D,E)>::S operator(mat<E>::S const &lhs, mat::S const &rhs) noexcept
        {
            return mat<std::max(D,E)>::S{lhs._v * rhs._v};
        }

    private:
        f32x4 _v;
    };

    class R {
    public:
        R(R const &) noexcept = default;    
        R(R &&) noexcept = default;    
        R &operator=(R const &) noexcept = default;    
        R &operator=(R &&) noexcept = default;

        [[nodiscard]] R(float angle, vec3 axis) noexcept requires (D == 3) : _v()
        {
            ttlet half_angle = angle * 0.5f
            ttlet C = std::cos(half_angle);
            ttlet S = std::sin(half_angle);

            _v = axis.normalize().v * S;
            _v.w() = C;
        }

        /** Convert quaternion to matrix.
         *
         */
        [[nodiscard]] constexpr operator mat() const noexcept requires (D == 3)
        {
            // Original from https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
            //   1 - 2(yy + zz) |     2(xy - zw) |     2(xz + yw)
            //       2(xy + zw) | 1 - 2(xx + zz) |     2(yz - xw)
            //       2(xz - yw) |     2(yz + xw) | 1 - 2(xx + yy)

            // Flipping adds and multiplies:
            //   1 - 2(zz + yy) |     2(xy - zw) |     2(yw + xz)
            //       2(zw + yx) | 1 - 2(xx + zz) |     2(yz - xw)
            //       2(zx - yw) |     2(xw + zy) | 1 - 2(yy + xx)

            // All multiplies.
            ttlet x_mul = _v.xxxx() * _v;
            ttlet y_mul = _v.yyyy() * _v;
            ttlet z_mul = _v.zzzz() * _v;

            auto twos = f32x4(-2.0f, 2.0, 2.0, 0.0);
            auto one = f32x4(1.0f);
            ttlet col0 = one + addsub<0b0011>(z_mul.zwxy(), y_mul.yxwz()) * twos;
            one = one.yxzw();
            twos = twos.yxzw();
            ttlet col1 = one + addsub<0b0110>(x_mul.yxwz(), z_mul.wzyx()) * twos;
            one = one.xzyw();
            twos = twos.xzyw();
            ttlet col2 = one + addsub<0b0101>(y_mul.wzyx(), x_mul.zwxy()) * twos;
            one = one.xywz();
            return mat{col0, col1, col2, one};
        }

        std::pair<float,vec3> angle_and_axis() const noexcept requires (D == 3)
        {
            ttlet rcp_length = rcp_hypot<3>(v);
            ttlet length = 1.0f / inv_length;

            return {
                2.0f * std::atan2(length),
                vec3{v.xyz0 * rcp_length}
            };
        }

    private:
        /** Rotation is stored as a quaternion
         * w + x*i + y*j + z*k
         */
        f32x4 _v;        
    };

    mat(mat const &) noexcept = default;
    mat(mat &&) noexcept = default;
    mat &operator=(mat const &) noexcept = default;
    mat &operator=(mat &&) noexcept = default;

    mat() noexcept;
    mat(I const &other) noexcept;
    mat(T const &other) noexcept;
    mat(S const &other) noexcept;
    mat(R const &other) noexcept;

private:
    f32x4 col1;
    f32x4 col2;
    f32x4 col3;
    f32x4 col4;
};

}

