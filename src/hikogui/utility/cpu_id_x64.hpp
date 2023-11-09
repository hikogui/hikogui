// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

/** CPU ID
 *
 *
 * The MSVC `/arch:` argument determines which instructions are used by
 * the compiler for code generation. This argument also sets the __AVX__ macros.
 * However you can use any intrinsic that is supported by the compiler.
 *
 * In other words if you have a x64-MSVC compiler and you set the /arch:AVX
 * then the compiler will only directly use instructions upto AVX. The
 * __AVX__ macro is set, but not __AVX2__. However you are still allowed to
 * use the _mm256_* instructions.
 * 
 */

#include "debugger.hpp"
#include "cast.hpp"
#include "../macros.hpp"
#include <array>
#include <utility>
#include <cstdint>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
#include <cpuid.h>
#else
#error "Unsupported compiler for x64 cpu_id"
#endif

hi_export_module(hikogui.utility.cpu_id);

namespace hi {
inline namespace v1 {

enum class cpu_feature {
    // x86-64-v1
    cmov,
    cx8,
    fpu,
    fxsr,
    mmx,
    osfxsr,
    sce,
    sse,
    sse2,
    // x86-64-v2
    cx16,
    lahf,
    popcnt,
    sse3,
    sse4_1,
    sse4_2,
    ssse3,
    // x86-64-v3
    avx,
    avx2,
    bmi1,
    bmi2,
    f16c,
    fma,
    lzcnt,
    movbe,
    osxsave,
    // x86-64-v4
    avx512f,
    avx512bw,
    avx512cd,
    avx512dq,
    avx512vl,
    // others
    avx512pf,
    avx512er,
};

template<std::integral Lhs>
[[nodiscard]] constexpr unsigned long long operator<<(Lhs const &lhs, cpu_feature const &rhs) noexcept
{
    hi_assert(std::cmp_equal(lhs, 1));
    hi_assert(std::cmp_less(std::to_underlying(rhs), 64));

    return static_cast<unsigned long long>(lhs) << std::to_underlying(rhs);
}

enum class cpu_feature_mask : uint64_t {
    cmov       = 1 << cpu_feature::cmov,
    cx8        = 1 << cpu_feature::cx8,
    fpu        = 1 << cpu_feature::fpu,
    fxsr       = 1 << cpu_feature::fxsr,
    mmx        = 1 << cpu_feature::mmx,
    osfxsr     = 1 << cpu_feature::osfxsr,
    sce        = 1 << cpu_feature::sce,
    sse        = 1 << cpu_feature::sse,
    sse2       = 1 << cpu_feature::sse2,
    x86_64_v1  = cmov | cx8 | fpu | fxsr | mmx | osfxsr | sce | sse | sse2,

    cx16       = 1 << cpu_feature::cx16,
    lahf       = 1 << cpu_feature::lahf,
    popcnt     = 1 << cpu_feature::popcnt,
    sse3       = 1 << cpu_feature::sse3,
    sse4_1     = 1 << cpu_feature::sse4_1,
    sse4_2     = 1 << cpu_feature::sse4_2,
    ssse3      = 1 << cpu_feature::ssse3,
    x86_64_v2  = x86_64_v1 | cx16 | lahf | popcnt | sse3 | sse4_1 | sse4_2 | ssse3,

    avx        = 1 << cpu_feature::avx,
    avx2       = 1 << cpu_feature::avx2,
    bmi1       = 1 << cpu_feature::bmi1,
    bmi2       = 1 << cpu_feature::bmi2,
    f16c       = 1 << cpu_feature::f16c,
    fma        = 1 << cpu_feature::fma,
    lzcnt      = 1 << cpu_feature::lzcnt,
    movbe      = 1 << cpu_feature::movbe,
    osxsave    = 1 << cpu_feature::osxsave,
    x86_64_v3  = x86_64_v2 | avx | avx2 | bmi1 | bmi2 | f16c | fma | lzcnt | movbe | osxsave,

    avx512f    = 1 << cpu_feature::avx512f,
    avx512bw   = 1 << cpu_feature::avx512bw,
    avx512cd   = 1 << cpu_feature::avx512cd,
    avx512dq   = 1 << cpu_feature::avx512dq,
    avx512vl   = 1 << cpu_feature::avx512vl,
    x86_64_v4  = x86_64_v3 | avx512f | avx512bw | avx512cd | avx512dq | avx512vl,

    avx512pf   = 1 << cpu_feature::avx512pf,
    avx512er   = 1 << cpu_feature::avx512er,
};

[[nodiscard]] constexpr cpu_feature_mask operator|(cpu_feature_mask const &lhs, cpu_feature_mask const &rhs) noexcept
{
    return static_cast<cpu_feature_mask>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr cpu_feature_mask operator&(cpu_feature_mask const &lhs, cpu_feature_mask const &rhs) noexcept
{
    return static_cast<cpu_feature_mask>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(cpu_feature_mask const &rhs) noexcept
{
    return std::to_underlying(rhs) != 0;
}

[[nodiscard]] constexpr cpu_feature_mask operator|(cpu_feature_mask const &lhs, cpu_feature const &rhs) noexcept
{
    hilet rhs_ = static_cast<cpu_feature_mask>(1 << rhs);
    return lhs | rhs_;
}

[[nodiscard]] constexpr cpu_feature_mask operator&(cpu_feature_mask const &lhs, cpu_feature const &rhs) noexcept
{
    hilet rhs_ = static_cast<cpu_feature_mask>(1 << rhs);
    return lhs & rhs_;
}

constexpr cpu_feature_mask &operator|=(cpu_feature_mask &lhs, cpu_feature const &rhs) noexcept
{
    return lhs = lhs | rhs;
}

struct cpu_id_result {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    [[nodiscard]] bool eax_bit(int bit_nr) const noexcept
    {
        return to_bool(eax & (1U << bit_nr));
    }

    [[nodiscard]] bool ebx_bit(int bit_nr) const noexcept
    {
        return to_bool(ebx & (1U << bit_nr));
    }

    [[nodiscard]] bool ecx_bit(int bit_nr) const noexcept
    {
        return to_bool(ecx & (1U << bit_nr));
    }

    [[nodiscard]] bool edx_bit(int bit_nr) const noexcept
    {
        return to_bool(edx & (1U << bit_nr));
    }
};

/** A generic x86 cpu-id instruction.
 *
 * @param leaf_id The leaf of the cpu-id to query
 * @param index The index inside the leaf
 * @return aex, ebx, ecx, edx
 */
[[nodiscard]] hi_inline cpu_id_result cpu_id(uint32_t leaf_id, uint32_t index = 0) noexcept
{
    auto r = cpu_id_result{};

#if HI_COMPILER == HI_CC_MSVC
    auto tmp = std::array<int, 4>{};
    __cpuidex(tmp.data(), static_cast<int>(leaf_id), static_cast<int>(index));
    std::memcpy(&r, tmp.data(), sizeof(cpu_id_result));

#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
    __cpuid_count(leaf_id, index, r.eax, r.ebx, r.ecx, r.edx);

#else
#error "cpu_id() not implemented"
#endif

    return r;
}



[[nodiscard]] hi_inline cpu_feature_mask cpu_features_init() noexcept
{
    auto r = cpu_feature_mask{};

    hilet leaf0 = cpu_id(0);
    hilet max_leaf = leaf0.eax;

    // clang-format off
    if (max_leaf >= 1) {
        hilet leaf1 = cpu_id(1);

        if (leaf1.ecx_bit( 0)) { r |= cpu_feature::sse3; }
        if (leaf1.ecx_bit( 9)) { r |= cpu_feature::ssse3; }
        if (leaf1.ecx_bit(19)) { r |= cpu_feature::sse4_1; }
        if (leaf1.ecx_bit(20)) { r |= cpu_feature::sse4_2; }
        if (leaf1.ecx_bit(28)) { r |= cpu_feature::avx; }
        if (leaf1.ecx_bit(29)) { r |= cpu_feature::f16c; }

        if (leaf1.edx_bit(25)) { r |= cpu_feature::sse; }
        if (leaf1.edx_bit(26)) { r |= cpu_feature::sse2; }
    }

    if (max_leaf >= 7) {
        hilet leaf7 = cpu_id(7);

        if (leaf7.ebx_bit( 3)) { r |= cpu_feature::bmi1; }
        if (leaf7.ebx_bit( 5)) { r |= cpu_feature::avx2; }
        if (leaf7.ebx_bit( 8)) { r |= cpu_feature::bmi2; }
        if (leaf7.ebx_bit(16)) { r |= cpu_feature::avx512f; }
        if (leaf7.ebx_bit(17)) { r |= cpu_feature::avx512dq; }
        if (leaf7.ebx_bit(26)) { r |= cpu_feature::avx512pf; }
        if (leaf7.ebx_bit(27)) { r |= cpu_feature::avx512er; }
        if (leaf7.ebx_bit(28)) { r |= cpu_feature::avx512cd; }
        if (leaf7.ebx_bit(30)) { r |= cpu_feature::avx512bw; }
        if (leaf7.ebx_bit(31)) { r |= cpu_feature::avx512vl; }

    }
    // clang-format on


    return r;
}

inline cpu_feature_mask const cpu_features = cpu_features_init();

// clang-format off
#if HI_HAS_SSE
[[nodiscard]] constexpr bool has_sse() noexcept { return true; }
#else
[[nodiscard]] bool has_sse() noexcept { return to_bool(cpu_features & cpu_feature::sse); }
#endif

#if HI_HAS_SSE2
[[nodiscard]] constexpr bool has_sse2() noexcept { return true; }
#else
[[nodiscard]] bool has_sse2() noexcept { return to_bool(cpu_features & cpu_feature::sse2); }
#endif

#if HI_HAS_SSE3
[[nodiscard]] constexpr bool has_sse3() noexcept { return true; }
#else
[[nodiscard]] bool has_sse3() noexcept { return to_bool(cpu_features & cpu_feature::sse3); }
#endif

#if HI_HAS_SSSE3
[[nodiscard]] constexpr bool has_ssse3() noexcept { return true; }
#else
[[nodiscard]] bool has_ssse3() noexcept { return to_bool(cpu_features & cpu_feature::ssse3); }
#endif

#if HI_HAS_SSE4_1
[[nodiscard]] constexpr bool has_sse4_1() noexcept { return true; }
#else
[[nodiscard]] bool has_sse4_1() noexcept { return to_bool(cpu_features & cpu_feature::sse4_1); }
#endif

#if HI_HAS_SSE4_2
[[nodiscard]] constexpr bool has_sse4_2() noexcept { return true; }
#else
[[nodiscard]] bool has_sse4_2() noexcept { return to_bool(cpu_features & cpu_feature::sse4_2); }
#endif

#if HI_HAS_AVX
[[nodiscard]] constexpr bool has_avx() noexcept { return true; }
#else
[[nodiscard]] bool has_avx() noexcept { return to_bool(cpu_features & cpu_feature::avx); }
#endif

#if HI_HAS_AVX2
[[nodiscard]] constexpr bool has_avx2() noexcept { return true; }
#else
[[nodiscard]] bool has_avx2() noexcept { return to_bool(cpu_features & cpu_feature::avx2); }
#endif

#if HI_HAS_F16C
[[nodiscard]] constexpr bool has_f16c() noexcept { return true; }
#else
[[nodiscard]] bool has_f16c() noexcept { return to_bool(cpu_features & cpu_feature::f16c); }
#endif

// clang-format on

}}

