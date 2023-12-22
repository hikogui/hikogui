

hi_export_module(hikogui.SIMD.simd_intf);

hi_export namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd : std::array<T, N> {
    constexpr auto si = simd_intrinsic<T, N>{};

    [[nodiscard]] constexpr friend simd operator+(simd const &lhs, simd const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { si.store(si.add(si.load(lhs), si.load(rhs))); }) {
                return simd{si.store(si.add(si.load(lhs), si.load(rhs)))};
            }
        }

        auto r = simd{};
        for (auto i = size_t{}; i != N; ++i) {
            r[i] = lhs[i] + rhs[i];
        }
        return r;
    }

    [[nodiscard]] constexpr friend simd operator-(simd const &lhs, simd const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { si.store(si.sub(si.load(lhs), si.load(rhs))); }) {
                return simd{si.store(si.add(si.load(lhs), si.load(rhs)))};
            }
        }

        auto r = simd{};
        for (auto i = size_t{}; i != N; ++i) {
            r[i] = lhs[i] - rhs[i];
        }
        return r;
    }

    [[nodiscard]] constexpr friend simd operator*(simd const &lhs, simd const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { si.store(si.sub(si.load(lhs), si.load(rhs))); }) {
                return simd{si.store(si.mul(si.load(lhs), si.load(rhs)))};
            }
        }

        auto r = simd{};
        for (auto i = size_t{}; i != N; ++i) {
            r[i] = lhs[i] * rhs[i];
        }
        return r;
    }

    [[nodiscard]] constexpr friend simd operator/(simd const &lhs, simd const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { si.store(si.div(si.load(lhs), si.load(rhs))); }) {
                return simd{si.store(si.mul(si.load(lhs), si.load(rhs)))};
            }
        }

        auto r = simd{};
        for (auto i = size_t{}; i != N; ++i) {
            r[i] = lhs[i] / rhs[i];
        }
        return r;
    }

    [[nodiscard]] constexpr friend simd operator%(simd const &lhs, simd const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { si.store(si.div(si.load(lhs), si.load(rhs))); }) {
                return simd{si.store(si.mod(si.load(lhs), si.load(rhs)))};
            }
        }

        auto r = simd{};
        for (auto i = size_t{}; i != N; ++i) {
            r[i] = lhs[i] % rhs[i];
        }
        return r;
    }


    // clang-format off
    constexpr friend simd &operator+=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs + rhs; }
    constexpr friend simd &operator-=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs - rhs; }
    constexpr friend simd &operator*=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs * rhs; }
    constexpr friend simd &operator/=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs / rhs; }
    constexpr friend simd &operator%=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs % rhs; }
    constexpr friend simd &operator|=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs | rhs; }
    constexpr friend simd &operator&=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs & rhs; }
    constexpr friend simd &operator^=(simd &lhs, simd const &rhs) noexcept { return lhs = lhs ^ rhs; }
    // clang-format on
};

}}
