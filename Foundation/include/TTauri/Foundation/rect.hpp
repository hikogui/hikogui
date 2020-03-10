// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"

namespace TTauri {

class rect {
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - 31:0 - min-x
     *  - 63:32 - min-y
     *  - 95:64 - max-x
     *  - 127:96 - max-y
     */
    __m128 v;

public:
    force_inline rect() : noexcept : rect(_mm_setzero_ps()) {}
    force_inline rect(rect const &rhs) noexcept = default;
    force_inline rect &operator=(rect const &rhs) noexcept = default;
    force_inline rect(rect &&rhs) noexcept = default;
    force_inline rect &operator=(rect &&rhs) noexcept = default;

    rect(__m128 rhs) noexcept :
        v(rhs) {}

    rect &operator=(__m128 rhs) noexcept {
        v = rhs;
        return *this;
    }

    operator __m128 () const noexcept {
        return v;
    }

    force_inline rect(float x, float y, float width, float height) noexcept :
        rect(vec(x, y, x + width, y + height)) {}

    force_inline rect(vec const &offset, vec const &extent) noexcept :
        rect(offset.xyxy() + extent._00xy()) {}

    rect &operator|=(rect const &rhs) noexcept {
        return *this = *this | rhs;
    }

    rect &operator+=(vec const &rhs) noexcept {
        return *this = *this + rhs;
    }

    rect &operator-=(vec const &rhs) noexcept {
        return *this = *this - rhs;
    }

    [[nodiscard]] force_inline float x() const noexcept {
        return v.x();
    }

    [[nodiscard]] force_inline float y() const noexcept {
        return v.y();
    }

    /** Get coordinate of a corner.
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    */
    template<size_t I>
    [[nodiscard]] force_inline vec corner() const noexcept {
        static_assert(I <= 3);
        if constexpr (I == 0) {
            return v.xy01();
        } else if constexpr (I == 1) {
            return v.zy01();
        } else if constexpr (I == 2) {
            return v.xw01();
        } else {
            return v.zw01();
        }
    }

    /** Get coordinate of a corner.
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    */
    template<size_t I>
    [[nodiscard]] force_inline vec corner(float z) const noexcept {
        return corner<I>().z(z);
    }

    [[nodiscard]] force_inline vec offset() const noexcept { return corner<0>(); }
    [[nodiscard]] force_inline vec offset(float z) const noexcept { return corner<0>(z); }

    [[nodiscard]] vec extent() const noexcept {
        return corner<3>() - corner<0>();
    }

    [[nodiscard]] bool contains(vec const &rhs) const noexcept {
        return contains(*this, rhs);
    }

    rect &expand(float rhs) noexcept {
        return *this = expand(*this, rhs);
    }

    [[nodiscard]] friend bool operator==(rect const &lhs, rect const &rhs) noexcept {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(rect const &lhs, rect const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend rect operator|(rect const &lhs, rect const &rhs) noexcept {
        return {_mm_blend_ps(min(lhs, rhs), max(lhs, rhs), 0x1100)};
    }

    [[nodiscard]] friend rect operator+(rect const &lhs, vec const &rhs) noexcept {
        return lhs + rhs.xyxy();
    }

    [[nodiscard]] friend rect operator-(rect const &lhs, vec const &rhs) noexcept {
        return lhs - rhs.xyxy();
    }

    [[nodiscard]] friend rect expand(rect const &lhs, float rhs) noexcept {
        let _000r = _mm_set_ss(rhs);
        let _00rr = _mm_permute_ps(_000r, _MM_SHUFFLE(1,1,0,0));
        let _rr00 = _mm_permute_ps(_000r, _MM_SHUFFLE(0,0,1,1));
        return (lhs - _00rr) + _rr00;
    }

    [[nodiscard]] friend bool contains(rect const &lhs, vec const &rhs) const noexcept {
        return (lhs >= rhs.xyxy()) == 0b0011;
    }

};

}

