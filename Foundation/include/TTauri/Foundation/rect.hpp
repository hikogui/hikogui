

#pragma once

namespace TTauri {

struct rect {
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - 31:0 - x-offset
     *  - 63:32 - y-offset
     *  - 95:64 - width
     *  - 127:96 - height
     */
    __m128 v;

    rect() : noexcept :
        v(_mm_setzero_ps()) {}

    rect(rect const &rhs) noexcept :
        v(rhs.v) {}

    rect &operator=(rect const &rhs) noexcept {
        v = rhs.v;
        return *this;
    }

    rect(__m128 rhs) noexcept :
        v(rhs) {}

    rect &operator=(__m128 rhs) noexcept {
        v = rhs;
        return *this;
    }

    operator __m128 () const noexcept {
        return v;
    }

    rect(float x, float y, float width, float height) noexcept :
        v(_mm_set_ps(height, width, y, x) {}

    rect(vec const &offset, vec const &extent) noexcept :
        v(_mm_shuffle_ps(offset.v, extent.v, _MM_SHUFFLE(1,0,1,0))) {}

    [[nodiscard]] float x() const noexcept { return get<0>(); }
    [[nodiscard]] float y() const noexcept { return get<1>(); }
    [[nodiscard]] float width() const noexcept { return get<2>(); }
    [[nodiscard]] float height() const noexcept { return get<3>(); }
    rect &x(float rhs) noexcept { return set<0>(rhs); }
    rect &y(float rhs) noexcept { return set<1>(rhs); }
    rect &width(float rhs) noexcept { return set<2>(rhs); }
    rect &height(float rhs) noexcept { return set<3>(rhs); }

    [[nodiscard]] vec offset() const noexcept { return vec{v}.xy01(); }
    [[nodiscard]] vec extent() const noexcept { return vec{v}.zw00(); }

    rect &offset(vec const &rhs) noexcept {
        v = _mm_shuffle_ps(rhs.v, v, _MM_SHUFFLE(1,0,1,0));
        return *this;
    }

    rect &extent(vec const &rhs) noexcept {
        v = _mm_shuffle_ps(v, rhs.v, _MM_SHUFFLE(1,0,1,0));
        return *this;
    }

    /** Get coordinate of a corner.
     * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
     */
    template<size_t I>
    [[nodiscard]] vec corner() const noexcept {
        static_assert(I <= 3);
        if constexpr (I == 0) {
            return offset();
        } else if constexpr (I == 1) {
            return offset() + vec{v}.z000;
        } else if constexpr (I == 2) {
            return offset() + vec{v}.0w00;
        } else {
            return offset() + extent();
        }
    }

    [[nodiscard]] std::array<vec,4> corners() const noexcept {
        std::array<vec,4> r;
        let off = offset();
        let ext = extent();
        r[0] = off;
        r[1] = _mm_add_ss(off, ext);
        r[2] = _mm_add_ps(off, _mm_permute_ps(ext, _MM_SHUFFLE(3,2,1,3)));
        r[3] = off + ext;

        return r;
    }


    rect &expand(float rhs) noexcept {
        let _000r = _mm_set_ss(rhs);
        let _00rr = _mm_permute_ps(_000r, _MM_SHUFFLE(1,1,0,0));
        let tmp1 = _mm_sub_ps(v, _00rr);
        let _rr00 = _mm_permute_ps(_000r, _MM_SHUFFLE(0,0,1,1));
        let tmp2 = _mm_add_ps(tmp1, _rr00); 
        v = _mm_add_ps(tmp2, _rr00); 
        return *this;
    }

    [[nodiscard]] friend rect expand(rect const &lhs, float rhs) {
        let _000r = _mm_set_ss(rhs);
        let _00rr = _mm_permute_ps(_000r, _MM_SHUFFLE(1,1,0,0));
        let tmp1 = _mm_sub_ps(lhs.v, _00rr);
        let _rr00 = _mm_permute_ps(_000r, _MM_SHUFFLE(0,0,1,1));
        let tmp2 = _mm_add_ps(tmp1, _rr00); 
        return {_mm_add_ps(tmp2, _rr00)};
    }

};

}

