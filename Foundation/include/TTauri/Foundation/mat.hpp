
/** A 4x4 matrix.
 * You can use this to transform vec (which has 4 elements)
 */
struct mat {
    __m128 col0;
    __m128 col1;
    __m128 col2;
    __m128 col3;

    /** Create an identity matrix.
     */
    mat() noexcept {
        __m128 tmp = _mm_set_ss(1.0);
        col0 = tmp;
        col1 = _mm_permute_ps(tmp, _MM_SHUFFLE(3,3,0,3));
        col2 = _mm_permute_ps(tmp, _MM_SHUFFLE(3,0,3,3));
        col3 = _mm_permute_ps(tmp, _MM_SHUFFLE(0,3,3,3));
    }

    mat(mat const &rhs) noexcept :
        col0(rhs.col0), col1(rhs.col1), col2(rhs.col2), col3(rhs.col3) {}

    mat &operator=(mat const &rhs) noexcept {
        col0 = rhs.col0;
        col1 = rhs.col1;
        col2 = rhs.col2;
        col3 = rhs.col3;
        return *this;
    }

    /** Create a matrix for 4 columns
     */
    mat(__m128 col0, __m128 col1, __m128 col2, __m128 col3) noexcept :
        col0(col0), col1(col1), col2(col2), col3(col3) {}

    /** Transpose this matrix.
     */
    mat &transpose() noexcept {
        _MM_TRANSPOSE4_PS(col0, col1, col2, col3);
        return *this;
    }

    /** Matrix/Vector multiplication.
     * Used for transforming vectors.
     */
    [[nodiscard]] friend vec<N> operator*(mat const &lhs, vec<N> const &rhs) noexcept {
        let xxxx = _mm_permute_ps(rhs, _MM_SHUFFLE(0,0,0,0));
        let col0_xxxx = _mm_mul_ps(lhs.col0, xxxx);
        let yyyy = _mm_permute_ps(rhs, _MM_SHUFFLE(1,1,1,1));
        let col1_yyyy = _mm_mul_ps(lhs.col1, yyyy);
        let zzzz = _mm_permute_ps(rhs, _MM_SHUFFLE(2,2,2,2));
        let col2_zzzz = _mm_mul_ps(lhs.col2, zzzz);
        let wwww = _mm_permute_ps(rhs, _MM_SHUFFLE(3,3,3,3));
        let col3_wwww = _mm_mul_ps(lhs.col3, wwww);
        let col01 = _mm_add_ps(col0_xxxx, col1_yyyy);
        let col012 = _mm_add_ps(col01, col2_zzzz);
        let col0123 = _mm_add_ps(col012, col3_wwww);
        return {col0123};
    }

    /** Matrix/Matrix multiplication.
     */
    [[nodiscard]] friend mat operator*(mat const &lhs, mat const &rhs) noexcept {
        return {
            lhs * vec{rhs.col0},
            lhs * vec{rhs.col1},
            lhs * vec{rhs.col2},
            lhs * vec{rhs.col3}
        };
    }

    /** Matrix transpose.
     */
    [[nodiscard]] friend mat transpose(mat const &rhs) noexcept {
        auto col0 = rhs.col0;
        auto col1 = rhs.col1;
        auto col2 = rhs.col2;
        auto col3 = rhs.col3;
        _MM_TRANSPOSE4_PS(col0, col1, col2, col3);
        return {col0, col1, col2, col3};
    }

    /** Create a translation matrix.
     */
    [[nodiscard]] static mat translate(vec rhs) noexcept {
        let col0 = _mm_set_ss(1.0f);
        let col1 = _mm_permute_ps(col0, _MM_SHUFFLE(1,1,0,1));
        let col2 = _mm_permute_ps(col0, _MM_SHUFFLE(1,0,1,1));
        let col3 = _mm_insert_ps(rhs.v, col0, 0b00'11'0000);
        return {col0, col1, col2, col3};
    }

    /** Create a scaling matrix.
     */
    [[nodiscard]] static mat scale(vec rhs) noexcept {
        let tmp = _mm_set_ps1(1.0f);
        let col0 = _mm_insert_ps(tmp, rhs.v, 0b00'00'1110);
        let col1 = _mm_insert_ps(tmp, rhs.v, 0b01'01'1101);
        let col2 = _mm_insert_ps(tmp, rhs.v, 0b10'10'1011);
        let col3 = _mm_insert_ps(tmp, tmp, 0b11'11'0111);
        return {col0, col1, col2, col3};
    }

    /** Create a scaling matrix.
     */
    [[nodiscard]] static mat scale(float rhs) noexcept {
        let tmp = _mm_set_ps(0.0f, rhs, 1.0f, 0.0f);
        let col0 = _mm_permute_ps(tmp, _MM_SHUFFLE(0,0,0,2);
        let col1 = _mm_permute_ps(tmp, _MM_SHUFFLE(0,0,2,0);
        let col2 = _mm_permute_ps(tmp, _MM_SHUFFLE(0,2,0,0);
        let col3 = _mm_permute_ps(tmp, _MM_SHUFFLE(1,0,0,0);
        return {col0, col1, col2, col3};
    }

    /** Create a rotation matrix.
     * @param N 0 = rotate around x-axis, 1=rotate around y-axis, 2=rotate around z-axis
     * @param rhs Angle in radials counter-clockwise.
     */
    template<int N=2>
    [[nodiscard]] static mat rotate(float rhs) noexcept {
        let s = sin(rhs);
        let c = cos(rhs);
        let tmp1 = _mm_set_ps(c, s, 1.0f, 0.0f);
        let tmp2 = _mm_insert_ps(tmp1, _mm_set_ss(-s), 0b00'10'0000);

        if constexpr (N == 0) {
            let col0 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,0,1));
            let col1 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,2,3,0));
            let col2 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,3,2,0)); // -sin
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        } else if constexpr (N == 1) {
            let col0 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,2,0,3)); // -sin
            let col1 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,1,0));
            let col2 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,3,0,2));
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        } else {
            let col0 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,2,3));
            let col1 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,0,3,2)); // -sin
            let col2 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,1,0,0));
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        }
    }

};
