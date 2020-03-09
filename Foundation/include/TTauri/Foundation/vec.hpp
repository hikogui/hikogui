
#include "TTauri/Foundation/vec_utils.hpp"

namespace TTauri {

/** Swizzle returning a vec4.
 * @param name The name of the swizzle function.
 * @param a The first element: 0=x, 1=y, 2=z, 3=w
 * @param b The second element: 0=x, 1=y, 2=z, 3=w
 * @param c The third element: 0=x, 1=y, 2=z, 3=w
 * @param d The fourth element: 0=x, 1=y, 2=z, 3=w
 * @return A vec with swizzled elements.
 */
#define SWIZZLE4(name, a, b, c, d)\
    [[nodiscard]] name() const noexcept {\
        return {_mm_permute_ps(v, _MM_SHUFFLE(d,c,b,a))};\
    }

/** Swizzle returning a vec3.
 * @param name The name of the swizzle function.
 * @param a The first element: 0=x, 1=y, 2=z, 3=w
 * @param b The second element: 0=x, 1=y, 2=z, 3=w
 * @param c The third element: 0=x, 1=y, 2=z, 3=w
 * @return A vec with swizzled elements, with the last elements being repeated.
 */
#define SWIZZLE3(name, a, b, c)\
    [[nodiscard]] name() const noexcept {\
        return {_mm_permute_ps(v, _MM_SHUFFLE(c,c,b,a))};\
    }

/** Swizzle returning a vec.
 * @param name The name of the swizzle function.
 * @param a The first element: 0=x, 1=y, 2=z, 3=w
 * @param b The second element: 0=x, 1=y, 2=z, 3=w
 * @return A vec with swizzled elements, with the last elements being repeated.
 */
#define SWIZZLE2(name, a, b)\
    [[nodiscard]] name() const noexcept {\
        return {_mm_permute_ps(v, _MM_SHUFFLE(b,b,b,a))};\
    }

/** A 4D vector.
 *
 * If you need a 3D vector or point, you can use this vector class
 * as a homogenious coordinate.
 *
 * If you use this vector as a color, then x=Red, y=Green, z=Blue, w=Alpha.
 *  - Alpha is linear: 0.0 is transparent, 1.0 is opaque
 *  - Red/Green/Blue are based on the linear-scRGB floating point format:
 *    values between 0.0 and 1.0 is equivilant to linear-sRGB (no gamma curve).
 *    values are allowed to be outside of this range for high-dynamic-range
 *    and high-color-gamut. 1.0,1.0,1.0 equals 80 cd/m2 and should be the maximum
 *    value for user interfaces.
 */ 
struct vec {
    /* Intrinsic value of the vec.
     * The elements in __m128 are assigned as follows.
     *  - [127:96] w, alpha
     *  - [95:64] z, blue
     *  - [63:32] y, green
     *  - [31:0] x, red
     */
    __m128 v;

    /* Create a zeroed out vec.
     */
    vec() noexcept :
        v(_mm_setzero_ps()) {}

    vec(vec const &rhs) noexcept :
        v(rhs.v) {}

    vec &operator=(vec const &rhs) {
        v = rhs.v;
        return *this;
    }

    /** Create a vec out of 2 to 4 values.
     * This vector is used as a homogenious coordinate, meaning:
     *  - vectors have w=0.0 (A direction and distance)
     *  - points have w=1.0 (A position in space)
     *
     * When this vector is used for color then:
     *  - x=Red, y=Green, z=Blue, w=Alpha
     *
     */
    vec(float x, float y, float z=0.0f, float w=0.0f) noexcept :
        v(_mm_set_ps(w, z, y, x)) {}

    /** Create a vec out of a __m128
     */
    vec(__m128 rhs) noexcept :
        v(rhs) {}

    /** Create a vec out of a __m128
     */
    vec &operator=(__m128 rhs) noexcept {
        v = rhs;
        return *this;
    }

    /** Convert a vec to a __m128.
     */
    operator __m128 () const noexcept {
        return v;
    }

    /** Initialize a vec with all elements set to a value.
     * Useful as a scalar converter, when combined with an
     * arithmatic operator.
     */
    vec(float rhs) noexcept:
        v(_mm_set_ps1(rhs)) {}

    /** Initialize a vec with all elements set to a value.
     * Useful as a scalar converter, when combined with an
     * arithmatic operator.
     */
    vec &operator=(__mm128 rhs) noexcept {
        v = _mm_set_ps1(rhs);
        return *this;
    }

    template<int I>
    vec &set(float rhs) noexcept {
        static_assert(I >= 0 && I <= 3);
        let tmp = _mm_set_ss(w);
        v = _mm_insert_ps(v, tmp, I << 4);
        return *this;
    }

    template<int I>
    float get(float rhs) const noexcept {
        static_assert(I >= 0 && I <= 3);
        let tmp = _mm_permute_ps(v, I);
        return _mm_cvtss_f32(tmp);
    }

    float operator[](int i) const noexcept {
        ttauri_assume(i >= 0 && i <= 3);
        let tmp = _mm_permute_ps(v, i);
        return _mm_cvtss_f32(tmp);
    }

    vec &x(float x) noexcept { return set<0>(x); }
    vec &y(float y) noexcept { return set<1>(x); }
    vec &z(float z) noexcept { return set<2>(x); }
    vec &w(float w) noexcept { return set<3>(x); }
    float x() const noexcept { return get<0>(); }
    float y() const noexcept { return get<1>(); }
    float z() const noexcept { return get<2>(); }
    float w() const noexcept { return get<3>(); }

    vec &operator+=(vec const &rhs) noexcept {
        v = _mm_add_ps(v, rhs.v);
        return *this;
    }

    vec &operator-=(vec const &rhs) noexcept {
        v = _mm_sub_ps(v, rhs.v);
        return *this;
    }

    vec &operator*=(vec const &rhs) noexcept {
        v = _mm_mul_ps(v, rhs.v);
        return *this;
    }

    vec &operator/=(vec const &rhs) noexcept {
        v = _mm_div_ps(v, rhs.v);
        return *this;
    }

    float length_squared() const noexcept {
        let tmp1 = _mm_mul_ps(v, v);
        let tmp2 = _mm_hadd_ps(tmp1, tmp1);
        let tmp3 = _mm_hadd_ps(tmp2, tmp2);
        return _mm_cvtss_f32(tmp3);
    }

    float length() const noexcept {
        let tmp1 = _mm_mul_ps(v, v);
        let tmp2 = _mm_hadd_ps(tmp1, tmp1);
        let tmp3 = _mm_hadd_ps(tmp2, tmp2);
        let tmp4 = _mm_sqrt_ps(tmp3);
        return _mm_cvtss_f32(tmp4);
    }

    [[nodiscard]] friend vec operator+(vec const &lhs, vec const &rhs) noexcept {
        return {_mm_add_ps(lhs.v, rhs.v}};
    }

    [[nodiscard]] friend vec operator-(vec const &lhs, vec const &rhs) noexcept {
        return {_mm_sub_ps(lhs.v, rhs.v}};
    }

    [[nodiscard]] friend vec operator*(vec const &lhs, vec const &rhs) noexcept {
        return {_mm_mul_ps(lhs.v, rhs.v}};
    }

    [[nodiscard]] friend vec operator/(vec const &lhs, vec const &rhs) noexcept {
        return {_mm_div_ps(lhs.v, rhs.v}};
    }

    [[nodiscard]] friend bool operator==(vec const &lhs, vec const &rhs) noexcept {
        let tmp1 = _mm_cmpeq_ps(lhs.v, rhs.v);
        let tmp2 = _mm_movemask_ps(tmp1);
        return tmp2 == 0b1111;
    }

    [[nodiscard]] friend bool operator!=(vec const &lhs, vec const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend float dot(vec const &lhs, vec const &rhs) noexcept {
        let tmp1 = _mm_mul_ps(lhs.v, rhs.v);
        let tmp2 = _mm_hadd_ps(tmp1, tmp1);
        let tmp3 = _mm_hadd_ps(tmp2, tmp2);
        return _mm_cvtss_f32(tmp3);
    }

    [[nodiscard]] friend float viktor_cross(vec const &lhs, vec const &rhs) noexcept {
        // a.x * b.y - a.y * b.x
        let tmp1 = _mm_permute_ps(rhs.v, _MM_SHUFFLE(2,3,0,1));
        let tmp2 = _mm_mul_ps(lhs.v, tmp1);
        let tmp3 = _mm_hsub_ps(tmp2, tmp2);
        return _mm_cvtss_f32(tmp3);
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    [[nodiscord]] friend vec cross(vec const &lhs, vec const &rhs) noexcept {
        let a_left = _mm_permute_ps(lhs.v, _MM_SHUFFLE(3,0,2,1));
        let b_left = _mm_permute_ps(rhs.v, _MM_SHUFFLE(3,1,0,2));
        let left = _mm_mul_ps(a_left, b_left);

        let a_right = _mm_permute_ps(lhs.v, _MM_SHUFFLE(3,1,0,2));
        let b_right = _mm_permute_ps(rhs.v, _MM_SHUFFLE(3,0,2,1));
        let right = _mm_mul_ps(a_right, b_right);
        return {_mm_sub_ps(left, right)};
    }

    [[nodiscard]] friend std::string to_string(vec const &rhs) noexcept {
        return fmt::format("({}, {}, {}, {})", rhs.x(), rhs.y(), rhs.z(), rhs.w());
    }

    std::ostream friend &operator<<(std::ostream &lhs, vec const &rhs) noexcept {
        return lhs << to_string(rhs);
    }

    template<std::size_t I>
    [[nodiscard]] friend float get(vec const &rhs) noexcept {
        return rhs.get<I>();
    }

    [[nodiscard]] friend float at(vec const &rhs, size_t i) noexcept {
        if (i > 3) {
            throw out_of_range("range check");
        }
        return rhs[i];
    }

    SWIZZLE4(xxxx,0,0,0,0) SWIZZLE4(xxxy,0,0,0,1) SWIZZLE4(xxxz,0,0,0,2) SWIZZLE4(xxxw,0,0,0,3)
    SWIZZLE4(xxyx,0,0,1,0) SWIZZLE4(xxyy,0,0,1,1) SWIZZLE4(xxyz,0,0,1,2) SWIZZLE4(xxyw,0,0,1,3)
    SWIZZLE4(xxzx,0,0,2,0) SWIZZLE4(xxzy,0,0,2,1) SWIZZLE4(xxzz,0,0,2,2) SWIZZLE4(xxzw,0,0,2,3)
    SWIZZLE4(xxwx,0,0,3,0) SWIZZLE4(xxwy,0,0,3,1) SWIZZLE4(xxwz,0,0,3,2) SWIZZLE4(xxww,0,0,3,3)
    SWIZZLE4(xyxx,0,1,0,0) SWIZZLE4(xyxy,0,1,0,1) SWIZZLE4(xyxz,0,1,0,2) SWIZZLE4(xyxw,0,1,0,3)
    SWIZZLE4(xyyx,0,1,1,0) SWIZZLE4(xyyy,0,1,1,1) SWIZZLE4(xyyz,0,1,1,2) SWIZZLE4(xyyw,0,1,1,3)
    SWIZZLE4(xyzx,0,1,2,0) SWIZZLE4(xyzy,0,1,2,1) SWIZZLE4(xyzz,0,1,2,2) SWIZZLE4(xyzw,0,1,2,3)
    SWIZZLE4(xywx,0,1,3,0) SWIZZLE4(xywy,0,1,3,1) SWIZZLE4(xywz,0,1,3,2) SWIZZLE4(xyww,0,1,3,3)
    SWIZZLE4(xzxx,0,2,0,0) SWIZZLE4(xzxy,0,2,0,1) SWIZZLE4(xzxz,0,2,0,2) SWIZZLE4(xzxw,0,2,0,3)
    SWIZZLE4(xzyx,0,2,1,0) SWIZZLE4(xzyy,0,2,1,1) SWIZZLE4(xzyz,0,2,1,2) SWIZZLE4(xzyw,0,2,1,3)
    SWIZZLE4(xzzx,0,2,2,0) SWIZZLE4(xzzy,0,2,2,1) SWIZZLE4(xzzz,0,2,2,2) SWIZZLE4(xzzw,0,2,2,3)
    SWIZZLE4(xzwx,0,2,3,0) SWIZZLE4(xzwy,0,2,3,1) SWIZZLE4(xzwz,0,2,3,2) SWIZZLE4(xzww,0,2,3,3)
    SWIZZLE4(xwxx,0,3,0,0) SWIZZLE4(xwxy,0,3,0,1) SWIZZLE4(xwxz,0,3,0,2) SWIZZLE4(xwxw,0,3,0,3)
    SWIZZLE4(xwyx,0,3,1,0) SWIZZLE4(xwyy,0,3,1,1) SWIZZLE4(xwyz,0,3,1,2) SWIZZLE4(xwyw,0,3,1,3)
    SWIZZLE4(xwzx,0,3,2,0) SWIZZLE4(xwzy,0,3,2,1) SWIZZLE4(xwzz,0,3,2,2) SWIZZLE4(xwzw,0,3,2,3)
    SWIZZLE4(xwwx,0,3,3,0) SWIZZLE4(xwwy,0,3,3,1) SWIZZLE4(xwwz,0,3,3,2) SWIZZLE4(xwww,0,3,3,3)
    SWIZZLE4(yxxx,1,0,0,0) SWIZZLE4(yxxy,1,0,0,1) SWIZZLE4(yxxz,1,0,0,2) SWIZZLE4(yxxw,1,0,0,3)
    SWIZZLE4(yxyx,1,0,1,0) SWIZZLE4(yxyy,1,0,1,1) SWIZZLE4(yxyz,1,0,1,2) SWIZZLE4(yxyw,1,0,1,3)
    SWIZZLE4(yxzx,1,0,2,0) SWIZZLE4(yxzy,1,0,2,1) SWIZZLE4(yxzz,1,0,2,2) SWIZZLE4(yxzw,1,0,2,3)
    SWIZZLE4(yxwx,1,0,3,0) SWIZZLE4(yxwy,1,0,3,1) SWIZZLE4(yxwz,1,0,3,2) SWIZZLE4(yxww,1,0,3,3)
    SWIZZLE4(yyxx,1,1,0,0) SWIZZLE4(yyxy,1,1,0,1) SWIZZLE4(yyxz,1,1,0,2) SWIZZLE4(yyxw,1,1,0,3)
    SWIZZLE4(yyyx,1,1,1,0) SWIZZLE4(yyyy,1,1,1,1) SWIZZLE4(yyyz,1,1,1,2) SWIZZLE4(yyyw,1,1,1,3)
    SWIZZLE4(yyzx,1,1,2,0) SWIZZLE4(yyzy,1,1,2,1) SWIZZLE4(yyzz,1,1,2,2) SWIZZLE4(yyzw,1,1,2,3)
    SWIZZLE4(yywx,1,1,3,0) SWIZZLE4(yywy,1,1,3,1) SWIZZLE4(yywz,1,1,3,2) SWIZZLE4(yyww,1,1,3,3)
    SWIZZLE4(yzxx,1,2,0,0) SWIZZLE4(yzxy,1,2,0,1) SWIZZLE4(yzxz,1,2,0,2) SWIZZLE4(yzxw,1,2,0,3)
    SWIZZLE4(yzyx,1,2,1,0) SWIZZLE4(yzyy,1,2,1,1) SWIZZLE4(yzyz,1,2,1,2) SWIZZLE4(yzyw,1,2,1,3)
    SWIZZLE4(yzzx,1,2,2,0) SWIZZLE4(yzzy,1,2,2,1) SWIZZLE4(yzzz,1,2,2,2) SWIZZLE4(yzzw,1,2,2,3)
    SWIZZLE4(yzwx,1,2,3,0) SWIZZLE4(yzwy,1,2,3,1) SWIZZLE4(yzwz,1,2,3,2) SWIZZLE4(yzww,1,2,3,3)
    SWIZZLE4(ywxx,1,3,0,0) SWIZZLE4(ywxy,1,3,0,1) SWIZZLE4(ywxz,1,3,0,2) SWIZZLE4(ywxw,1,3,0,3)
    SWIZZLE4(ywyx,1,3,1,0) SWIZZLE4(ywyy,1,3,1,1) SWIZZLE4(ywyz,1,3,1,2) SWIZZLE4(ywyw,1,3,1,3)
    SWIZZLE4(ywzx,1,3,2,0) SWIZZLE4(ywzy,1,3,2,1) SWIZZLE4(ywzz,1,3,2,2) SWIZZLE4(ywzw,1,3,2,3)
    SWIZZLE4(ywwx,1,3,3,0) SWIZZLE4(ywwy,1,3,3,1) SWIZZLE4(ywwz,1,3,3,2) SWIZZLE4(ywww,1,3,3,3)
    SWIZZLE4(zxxx,2,0,0,0) SWIZZLE4(zxxy,2,0,0,1) SWIZZLE4(zxxz,2,0,0,2) SWIZZLE4(zxxw,2,0,0,3)
    SWIZZLE4(zxyx,2,0,1,0) SWIZZLE4(zxyy,2,0,1,1) SWIZZLE4(zxyz,2,0,1,2) SWIZZLE4(zxyw,2,0,1,3)
    SWIZZLE4(zxzx,2,0,2,0) SWIZZLE4(zxzy,2,0,2,1) SWIZZLE4(zxzz,2,0,2,2) SWIZZLE4(zxzw,2,0,2,3)
    SWIZZLE4(zxwx,2,0,3,0) SWIZZLE4(zxwy,2,0,3,1) SWIZZLE4(zxwz,2,0,3,2) SWIZZLE4(zxww,2,0,3,3)
    SWIZZLE4(zyxx,2,1,0,0) SWIZZLE4(zyxy,2,1,0,1) SWIZZLE4(zyxz,2,1,0,2) SWIZZLE4(zyxw,2,1,0,3)
    SWIZZLE4(zyyx,2,1,1,0) SWIZZLE4(zyyy,2,1,1,1) SWIZZLE4(zyyz,2,1,1,2) SWIZZLE4(zyyw,2,1,1,3)
    SWIZZLE4(zyzx,2,1,2,0) SWIZZLE4(zyzy,2,1,2,1) SWIZZLE4(zyzz,2,1,2,2) SWIZZLE4(zyzw,2,1,2,3)
    SWIZZLE4(zywx,2,1,3,0) SWIZZLE4(zywy,2,1,3,1) SWIZZLE4(zywz,2,1,3,2) SWIZZLE4(zyww,2,1,3,3)
    SWIZZLE4(zzxx,2,2,0,0) SWIZZLE4(zzxy,2,2,0,1) SWIZZLE4(zzxz,2,2,0,2) SWIZZLE4(zzxw,2,2,0,3)
    SWIZZLE4(zzyx,2,2,1,0) SWIZZLE4(zzyy,2,2,1,1) SWIZZLE4(zzyz,2,2,1,2) SWIZZLE4(zzyw,2,2,1,3)
    SWIZZLE4(zzzx,2,2,2,0) SWIZZLE4(zzzy,2,2,2,1) SWIZZLE4(zzzz,2,2,2,2) SWIZZLE4(zzzw,2,2,2,3)
    SWIZZLE4(zzwx,2,2,3,0) SWIZZLE4(zzwy,2,2,3,1) SWIZZLE4(zzwz,2,2,3,2) SWIZZLE4(zzww,2,2,3,3)
    SWIZZLE4(zwxx,2,3,0,0) SWIZZLE4(zwxy,2,3,0,1) SWIZZLE4(zwxz,2,3,0,2) SWIZZLE4(zwxw,2,3,0,3)
    SWIZZLE4(zwyx,2,3,1,0) SWIZZLE4(zwyy,2,3,1,1) SWIZZLE4(zwyz,2,3,1,2) SWIZZLE4(zwyw,2,3,1,3)
    SWIZZLE4(zwzx,2,3,2,0) SWIZZLE4(zwzy,2,3,2,1) SWIZZLE4(zwzz,2,3,2,2) SWIZZLE4(zwzw,2,3,2,3)
    SWIZZLE4(zwwx,2,3,3,0) SWIZZLE4(zwwy,2,3,3,1) SWIZZLE4(zwwz,2,3,3,2) SWIZZLE4(zwww,2,3,3,3)
    SWIZZLE4(wxxx,3,0,0,0) SWIZZLE4(wxxy,3,0,0,1) SWIZZLE4(wxxz,3,0,0,2) SWIZZLE4(wxxw,3,0,0,3)
    SWIZZLE4(wxyx,3,0,1,0) SWIZZLE4(wxyy,3,0,1,1) SWIZZLE4(wxyz,3,0,1,2) SWIZZLE4(wxyw,3,0,1,3)
    SWIZZLE4(wxzx,3,0,2,0) SWIZZLE4(wxzy,3,0,2,1) SWIZZLE4(wxzz,3,0,2,2) SWIZZLE4(wxzw,3,0,2,3)
    SWIZZLE4(wxwx,3,0,3,0) SWIZZLE4(wxwy,3,0,3,1) SWIZZLE4(wxwz,3,0,3,2) SWIZZLE4(wxww,3,0,3,3)
    SWIZZLE4(wyxx,3,1,0,0) SWIZZLE4(wyxy,3,1,0,1) SWIZZLE4(wyxz,3,1,0,2) SWIZZLE4(wyxw,3,1,0,3)
    SWIZZLE4(wyyx,3,1,1,0) SWIZZLE4(wyyy,3,1,1,1) SWIZZLE4(wyyz,3,1,1,2) SWIZZLE4(wyyw,3,1,1,3)
    SWIZZLE4(wyzx,3,1,2,0) SWIZZLE4(wyzy,3,1,2,1) SWIZZLE4(wyzz,3,1,2,2) SWIZZLE4(wyzw,3,1,2,3)
    SWIZZLE4(wywx,3,1,3,0) SWIZZLE4(wywy,3,1,3,1) SWIZZLE4(wywz,3,1,3,2) SWIZZLE4(wyww,3,1,3,3)
    SWIZZLE4(wzxx,3,2,0,0) SWIZZLE4(wzxy,3,2,0,1) SWIZZLE4(wzxz,3,2,0,2) SWIZZLE4(wzxw,3,2,0,3)
    SWIZZLE4(wzyx,3,2,1,0) SWIZZLE4(wzyy,3,2,1,1) SWIZZLE4(wzyz,3,2,1,2) SWIZZLE4(wzyw,3,2,1,3)
    SWIZZLE4(wzzx,3,2,2,0) SWIZZLE4(wzzy,3,2,2,1) SWIZZLE4(wzzz,3,2,2,2) SWIZZLE4(wzzw,3,2,2,3)
    SWIZZLE4(wzwx,3,2,3,0) SWIZZLE4(wzwy,3,2,3,1) SWIZZLE4(wzwz,3,2,3,2) SWIZZLE4(wzww,3,2,3,3)
    SWIZZLE4(wwxx,3,3,0,0) SWIZZLE4(wwxy,3,3,0,1) SWIZZLE4(wwxz,3,3,0,2) SWIZZLE4(wwxw,3,3,0,3)
    SWIZZLE4(wwyx,3,3,1,0) SWIZZLE4(wwyy,3,3,1,1) SWIZZLE4(wwyz,3,3,1,2) SWIZZLE4(wwyw,3,3,1,3)
    SWIZZLE4(wwzx,3,3,2,0) SWIZZLE4(wwzy,3,3,2,1) SWIZZLE4(wwzz,3,3,2,2) SWIZZLE4(wwzw,3,3,2,3)
    SWIZZLE4(wwwx,3,3,3,0) SWIZZLE4(wwwy,3,3,3,1) SWIZZLE4(wwwz,3,3,3,2) SWIZZLE4(wwww,3,3,3,3)

    SWIZZLE3(xxx,0,0,0) SWIZZLE3(xxy,0,0,1) SWIZZLE3(xxz,0,0,2) SWIZZLE3(xxw,0,0,3)
    SWIZZLE3(xyx,0,1,0) SWIZZLE3(xyy,0,1,1) SWIZZLE3(xyz,0,1,2) SWIZZLE3(xyw,0,1,3)
    SWIZZLE3(xzx,0,2,0) SWIZZLE3(xzy,0,2,1) SWIZZLE3(xzz,0,2,2) SWIZZLE3(xzw,0,2,3)
    SWIZZLE3(xwx,0,3,0) SWIZZLE3(xwy,0,3,1) SWIZZLE3(xwz,0,3,2) SWIZZLE3(xww,0,3,3)
    SWIZZLE3(yxx,1,0,0) SWIZZLE3(yxy,1,0,1) SWIZZLE3(yxz,1,0,2) SWIZZLE3(yxw,1,0,3)
    SWIZZLE3(yyx,1,1,0) SWIZZLE3(yyy,1,1,1) SWIZZLE3(yyz,1,1,2) SWIZZLE3(yyw,1,1,3)
    SWIZZLE3(yzx,1,2,0) SWIZZLE3(yzy,1,2,1) SWIZZLE3(yzz,1,2,2) SWIZZLE3(yzw,1,2,3)
    SWIZZLE3(ywx,1,3,0) SWIZZLE3(ywy,1,3,1) SWIZZLE3(ywz,1,3,2) SWIZZLE3(yww,1,3,3)
    SWIZZLE3(zxx,2,0,0) SWIZZLE3(zxy,2,0,1) SWIZZLE3(zxz,2,0,2) SWIZZLE3(zxw,2,0,3)
    SWIZZLE3(zyx,2,1,0) SWIZZLE3(zyy,2,1,1) SWIZZLE3(zyz,2,1,2) SWIZZLE3(zyw,2,1,3)
    SWIZZLE3(zzx,2,2,0) SWIZZLE3(zzy,2,2,1) SWIZZLE3(zzz,2,2,2) SWIZZLE3(zzw,2,2,3)
    SWIZZLE3(zwx,2,3,0) SWIZZLE3(zwy,2,3,1) SWIZZLE3(zwz,2,3,2) SWIZZLE3(zww,2,3,3)
    SWIZZLE3(wxx,3,0,0) SWIZZLE3(wxy,3,0,1) SWIZZLE3(wxz,3,0,2) SWIZZLE3(wxw,3,0,3)
    SWIZZLE3(wyx,3,1,0) SWIZZLE3(wyy,3,1,1) SWIZZLE3(wyz,3,1,2) SWIZZLE3(wyw,3,1,3)
    SWIZZLE3(wzx,3,2,0) SWIZZLE3(wzy,3,2,1) SWIZZLE3(wzz,3,2,2) SWIZZLE3(wzw,3,2,3)
    SWIZZLE3(wwx,3,3,0) SWIZZLE3(wwy,3,3,1) SWIZZLE3(wwz,3,3,2) SWIZZLE3(www,3,3,3)

    SWIZZLE2(xx,0,0) SWIZZLE2(xy,0,1) SWIZZLE2(xz,0,2) SWIZZLE2(xw,0,3)
    SWIZZLE2(yx,1,0) SWIZZLE2(yy,1,1) SWIZZLE2(yz,1,2) SWIZZLE2(yw,1,3)
    SWIZZLE2(zx,2,0) SWIZZLE2(zy,2,1) SWIZZLE2(zz,2,2) SWIZZLE2(zw,2,3)
    SWIZZLE2(wx,3,0) SWIZZLE2(wy,3,1) SWIZZLE2(wz,3,2) SWIZZLE2(ww,3,3)

    SWIZZLE1(x,0) SWIZZLE2(y,1) SWIZZLE2(z,2) SWIZZLE2(w,3)
};

}

// Remove the vec_utils.hpp
#undef SWIZZLE4
#undef SWIZZLE3
#undef SWIZZLE2
