// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include "terminate.hpp"
#include "cast.hpp"
#include "initialize.hpp"
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

/** CPU-ID.
 *
 * This module together with the <hikogui/macros.hpp> header is used to
 * handle CPU specific implementation.
 *
 * There are three mechanics that work together:
 *  - `HI_HAS_*` CPU feature that will always be available.
 *  - `hi::has_*()` CPU feature that is avaiable at runtime.
 *  - `hi_target()` turn on a CPU feature for this function.
 *
 * HikoGUI determines the `HI_HAS_*` macros based on the CPU architecture
 * command line arguments of the compiler.
 *  - MSVC: `/arch:`
 *  - clang and gcc: `-march=`, `-mcpu=` and other `-m*` arguments.
 *
 * The `hi::has_*()` functions are constexpr true if the corrosponding
 * `HI_HAS_*` macro is set. The other `hi::has_*()` functions determine
 * the existance of that CPU feature based on the cached result of the cpu-id
 * instruction.
 *
 * Clang and gcc require that `-march=` and `-m*` command line arguments
 * are set to be able to use a corrosponding compiler intrinsic. For
 * example: if you want to use the `_mm_cvtph_ps()` intrinsic than the
 * `-mf16c` must be passed as the compile time argument.
 *
 * Clang and gcc also allows you to decorate a function with the `hi_target()`
 * attribute (which is ignored on MSVC) to enable code-generation and
 * the use of specific intrinsics for a single function.
 *
 * @module hikogui.utility.cpu_id
 */
hi_export_module(hikogui.utility.cpu_id);

namespace hi {
inline namespace v1 {

/** Possible features of x86 CPUs.
 *
 * The features listed here are the ones which are required for
 * official microarchitecture levels:
 *  - x86-64-v1
 *  - x86-64-v2
 *  - x86-64-v3
 *  - x86-64-v4
 *
 * Plus some optional features that are used by HikoGUI.
 *
 */
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
    sha,
    aes,
    pclmul,
    rdrnd,
    rdseed,
};

template<std::integral Lhs>
[[nodiscard]] constexpr unsigned long long operator<<(Lhs const &lhs, cpu_feature const &rhs) noexcept
{
    hi_assert(std::cmp_equal(lhs, 1));
    hi_assert(std::cmp_less(std::to_underlying(rhs), 64));

    return static_cast<unsigned long long>(lhs) << std::to_underlying(rhs);
}

/** A mask of features.
 *
 * Currently this implementation can handle up to 64 features.
 */
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
    sha        = 1 << cpu_feature::sha,
    aes        = 1 << cpu_feature::aes,
    pclmul     = 1 << cpu_feature::pclmul,
    rdrnd      = 1 << cpu_feature::rdrnd,
    rdseed     = 1 << cpu_feature::rdseed,
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

namespace detail {

[[nodiscard]] hi_inline cpu_feature_mask cpu_features_init() noexcept
{
    initialize();

    // clang-format off
    auto r = cpu_feature_mask{};

    hilet leaf0 = cpu_id(0);
    hilet max_leaf = leaf0.eax;

    if (max_leaf >= 1) {
        hilet leaf1 = cpu_id(1);

        if (leaf1.ecx_bit( 0)) { r |= cpu_feature::sse3; }
        if (leaf1.ecx_bit( 1)) { r |= cpu_feature::pclmul; }
        if (leaf1.ecx_bit( 9)) { r |= cpu_feature::ssse3; }
        if (leaf1.ecx_bit(12)) { r |= cpu_feature::fma; }
        if (leaf1.ecx_bit(13)) { r |= cpu_feature::cx16; }
        if (leaf1.ecx_bit(19)) { r |= cpu_feature::sse4_1; }
        if (leaf1.ecx_bit(20)) { r |= cpu_feature::sse4_2; }
        if (leaf1.ecx_bit(22)) { r |= cpu_feature::movbe; }
        if (leaf1.ecx_bit(23)) { r |= cpu_feature::popcnt; }
        if (leaf1.ecx_bit(25)) { r |= cpu_feature::aes; }
        if (leaf1.ecx_bit(27)) { r |= cpu_feature::osxsave; }
        if (leaf1.ecx_bit(28)) { r |= cpu_feature::avx; }
        if (leaf1.ecx_bit(29)) { r |= cpu_feature::f16c; }
        if (leaf1.ecx_bit(30)) { r |= cpu_feature::rdrnd; }

        if (leaf1.edx_bit( 0)) { r |= cpu_feature::fpu; }
        if (leaf1.edx_bit( 8)) { r |= cpu_feature::cx8; }
        if (leaf1.edx_bit(15)) { r |= cpu_feature::cmov; }
        if (leaf1.edx_bit(23)) { r |= cpu_feature::mmx; }
        if (leaf1.edx_bit(24)) {
            r |= cpu_feature::fxsr;
            // Technically we need to read CR4, but this may be privileged.
            // Moden operating system do support it though.
            r |= cpu_feature::osfxsr;
        }
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
        if (leaf7.ebx_bit(18)) { r |= cpu_feature::rdseed; }
        if (leaf7.ebx_bit(26)) { r |= cpu_feature::avx512pf; }
        if (leaf7.ebx_bit(27)) { r |= cpu_feature::avx512er; }
        if (leaf7.ebx_bit(28)) { r |= cpu_feature::avx512cd; }
        if (leaf7.ebx_bit(29)) { r |= cpu_feature::sha; }
        if (leaf7.ebx_bit(30)) { r |= cpu_feature::avx512bw; }
        if (leaf7.ebx_bit(31)) { r |= cpu_feature::avx512vl; }
    }

    hilet leaf80 = cpu_id(0x8000'0000);
    hilet max_leaf8 = leaf80.eax;

    if (max_leaf8 >= 1) {
        hilet leaf81 = cpu_id(0x8000'0001);

        if (leaf81.ecx_bit( 0)) { r |= cpu_feature::lahf; }
        if (leaf81.ecx_bit( 5)) { r |= cpu_feature::lzcnt; }

        // edx[10] sce (only on AuthenticAMD Family 5 Model 7 CPUs)
        if (leaf81.edx_bit(11)) { r |= cpu_feature::sce; }
    }

#if HI_HAS_X86_64_V4
    if ((r & cpu_feature_mask::x86_64_v4) != cpu_feature_mask::x86_64_v4) { hi_assert_abort("Missing CPU feature: x86-64-v4"); }
#endif
#if HI_HAS_X86_64_V3
    if ((r & cpu_feature_mask::x86_64_v3) != cpu_feature_mask::x86_64_v3) { hi_assert_abort("Missing CPU feature: x86-64-v3"); }
#endif
#if HI_HAS_X86_64_V2
    if ((r & cpu_feature_mask::x86_64_v2) != cpu_feature_mask::x86_64_v2) { hi_assert_abort("Missing CPU feature: x86-64-v2"); }
#endif
#if HI_HAS_X86_64_V1
    if ((r & cpu_feature_mask::x86_64_v1) != cpu_feature_mask::x86_64_v1) { hi_assert_abort("Missing CPU feature: x86-64-v1"); }
#endif

#if HI_HAS_CMOV
    if (not to_bool(r & cpu_feature::cmov)) { hi_assert_abort("Missing CPU feature: CMOV"); }
#endif
#if HI_HAS_CX8
    if (not to_bool(r & cpu_feature::cx8)) { hi_assert_abort("Missing CPU feature: CX8"); }
#endif
#if HI_HAS_FPU
    if (not to_bool(r & cpu_feature::fpu)) { hi_assert_abort("Missing CPU feature: FPU"); }
#endif
#if HI_HAS_FXSR
    if (not to_bool(r & cpu_feature::fxsr)) { hi_assert_abort("Missing CPU feature: FXSR"); }
#endif
#if HI_HAS_OSFXSR
    if (not to_bool(r & cpu_feature::osfxsr)) { hi_assert_abort("Missing CPU feature: OSFXSR"); }
#endif
#if HI_HAS_SCE
    if (not to_bool(r & cpu_feature::sce)) { hi_assert_abort("Missing CPU feature: SCE"); }
#endif
#if HI_HAS_MMX
    if (not to_bool(r & cpu_feature::mmx)) { hi_assert_abort("Missing CPU feature: MMX"); }
#endif
#if HI_HAS_SSE
    if (not to_bool(r & cpu_feature::sse)) { hi_assert_abort("Missing CPU feature: SSE"); }
#endif
#if HI_HAS_SSE2
    if (not to_bool(r & cpu_feature::sse2)) { hi_assert_abort("Missing CPU feature: SSE2"); }
#endif
#if HI_HAS_CX16
    if (not to_bool(r & cpu_feature::cx16)) { hi_assert_abort("Missing CPU feature: CX16"); }
#endif
#if HI_HAS_LAHF
    if (not to_bool(r & cpu_feature::lahf)) { hi_assert_abort("Missing CPU feature: LAHF"); }
#endif
#if HI_HAS_POPCNT
    if (not to_bool(r & cpu_feature::popcnt)) { hi_assert_abort("Missing CPU feature: POPCNT"); }
#endif
#if HI_HAS_SSE3
    if (not to_bool(r & cpu_feature::sse3)) { hi_assert_abort("Missing CPU feature: SSE3"); }
#endif
#if HI_HAS_SSE4_1
    if (not to_bool(r & cpu_feature::sse4_1)) { hi_assert_abort("Missing CPU feature: SSE4_1"); }
#endif
#if HI_HAS_SSE4_2
    if (not to_bool(r & cpu_feature::sse4_2)) { hi_assert_abort("Missing CPU feature: SSE4_2"); }
#endif
#if HI_HAS_SSSE3
    if (not to_bool(r & cpu_feature::ssse3)) { hi_assert_abort("Missing CPU feature: SSSE3"); }
#endif
#if HI_HAS_LZCNT
    if (not to_bool(r & cpu_feature::lzcnt)) { hi_assert_abort("Missing CPU feature: LZCNT"); }
#endif
#if HI_HAS_MOVBE
    if (not to_bool(r & cpu_feature::movbe)) { hi_assert_abort("Missing CPU feature: MOVBE"); }
#endif
#if HI_HAS_OSXSAVE
    if (not to_bool(r & cpu_feature::osxsave)) { hi_assert_abort("Missing CPU feature: OSXSAVE"); }
#endif
#if HI_HAS_F16C
    if (not to_bool(r & cpu_feature::f16c)) { hi_assert_abort("Missing CPU feature: F16C"); }
#endif
#if HI_HAS_FMA
    if (not to_bool(r & cpu_feature::fma)) { hi_assert_abort("Missing CPU feature: FMA"); }
#endif
#if HI_HAS_BMI1
    if (not to_bool(r & cpu_feature::bmi1)) { hi_assert_abort("Missing CPU feature: BMI1"); }
#endif
#if HI_HAS_BMI2
    if (not to_bool(r & cpu_feature::bmi2)) { hi_assert_abort("Missing CPU feature: BMI2"); }
#endif
#if HI_HAS_AVX
    if (not to_bool(r & cpu_feature::avx)) { hi_assert_abort("Missing CPU feature: AVX"); }
#endif
#if HI_HAS_AVX2
    if (not to_bool(r & cpu_feature::avx2)) { hi_assert_abort("Missing CPU feature: AVX2"); }
#endif
#if HI_HAS_AVX512F
    if (not to_bool(r & cpu_feature::avx512f)) { hi_assert_abort("Missing CPU feature: AVX512F"); }
#endif
#if HI_HAS_AVX512BW
    if (not to_bool(r & cpu_feature::avx512bw)) { hi_assert_abort("Missing CPU feature: AVX512BW"); }
#endif
#if HI_HAS_AVX512CD
    if (not to_bool(r & cpu_feature::avx512cd)) { hi_assert_abort("Missing CPU feature: AVX512CD"); }
#endif
#if HI_HAS_AVX512DQ
    if (not to_bool(r & cpu_feature::avx512dq)) { hi_assert_abort("Missing CPU feature: AVX512DQ"); }
#endif
#if HI_HAS_AVX512VL
    if (not to_bool(r & cpu_feature::avx512vl)) { hi_assert_abort("Missing CPU feature: AVX512VL"); }
#endif
#if HI_HAS_AVX512PF
    if (not to_bool(r & cpu_feature::avx512pf)) { hi_assert_abort("Missing CPU feature: AVX512PF"); }
#endif
#if HI_HAS_AVX512ER
    if (not to_bool(r & cpu_feature::avx512er)) { hi_assert_abort("Missing CPU feature: AVX512ER"); }
#endif
#if HI_HAS_SHA
    if (not to_bool(r & cpu_feature::sha)) { hi_assert_abort("Missing CPU feature: SHA"); }
#endif
#if HI_HAS_AES
    if (not to_bool(r & cpu_feature::aes)) { hi_assert_abort("Missing CPU feature: AES"); }
#endif
#if HI_HAS_PCLMUL
    if (not to_bool(r & cpu_feature::pclmul)) { hi_assert_abort("Missing CPU feature: PCLMUL"); }
#endif
#if HI_HAS_RDRND
    if (not to_bool(r & cpu_feature::rdrnd)) { hi_assert_abort("Missing CPU feature: RDRND"); }
#endif
#if HI_HAS_RDSEED
    if (not to_bool(r & cpu_feature::rdseed)) { hi_assert_abort("Missing CPU feature: RDSEED"); }
#endif

    return r;
    // clang-format on
}

}

/** A set of features that are supported on this CPU.
 */
inline cpu_feature_mask const cpu_features = detail::cpu_features_init();

// clang-format off

/** This CPU has the CMOV (Conditional Move) instruction.
 */
#if HI_HAS_CMOV
[[nodiscard]] constexpr bool has_cmov() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_cmov() noexcept { return to_bool(cpu_features & cpu_feature::cmov); }
#endif

/** This CPU has the CMPXCG8 (Compare and exchange 8 bytes) instruction.
 */
#if HI_HAS_CX8
[[nodiscard]] constexpr bool has_cx8() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_cx8() noexcept { return to_bool(cpu_features & cpu_feature::cx8); }
#endif

/** This CPU has a floating-point co-processor.
 */
#if HI_HAS_FPU
[[nodiscard]] constexpr bool has_fpu() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_fpu() noexcept { return to_bool(cpu_features & cpu_feature::fpu); }
#endif

/** This CPU has the fxsave instruction.
 */
#if HI_HAS_FXSR
[[nodiscard]] constexpr bool has_fxsr() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_fxsr() noexcept { return to_bool(cpu_features & cpu_feature::fxsr); }
#endif

/** This operating system uses the FXSAVE instruction.
 */
#if HI_HAS_OSFXSR
[[nodiscard]] constexpr bool has_osfxsr() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_osfxsr() noexcept { return to_bool(cpu_features & cpu_feature::osfxsr); }
#endif

/** This operating system uses the SYSCALL instruction.
 */
#if HI_HAS_SCE
[[nodiscard]] constexpr bool has_sce() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sce() noexcept { return to_bool(cpu_features & cpu_feature::sce); }
#endif

/** This CPU has the MMX instruction set.
 */
#if HI_HAS_MMX
[[nodiscard]] constexpr bool has_mmx() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_mmx() noexcept { return to_bool(cpu_features & cpu_feature::mmx); }
#endif

/** This CPU has the SSE instruction set.
 */
#if HI_HAS_SSE
[[nodiscard]] constexpr bool has_sse() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sse() noexcept { return to_bool(cpu_features & cpu_feature::sse); }
#endif

/** This CPU has the SSE2 instruction set.
 */
#if HI_HAS_SSE2
[[nodiscard]] constexpr bool has_sse2() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sse2() noexcept { return to_bool(cpu_features & cpu_feature::sse2); }
#endif

/** This CPU has all the features for x86-64-v1 microarchitecture level.
 */
#if HI_HAS_X86_64_V1
[[nodiscard]] constexpr bool has_x86_64_v1() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_x86_64_v1() noexcept { return (cpu_features & cpu_feature_mask::x86_64_v1) == cpu_feature_mask::x86_64_v1; }
#endif

/** This CPU has the CMPXCG16 (Compare and exchange 16 bytes) instruction.
 */
#if HI_HAS_CX16
[[nodiscard]] constexpr bool has_cx16() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_cx16() noexcept { return to_bool(cpu_features & cpu_feature::cx16); }
#endif

/** This CPU has the LAHF and SAHF instructions.
 */
#if HI_HAS_LAHF
[[nodiscard]] constexpr bool has_lahf() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_lahf() noexcept { return to_bool(cpu_features & cpu_feature::lahf); }
#endif

/** This CPU has the POPCNT instructions.
 */
#if HI_HAS_POPCNT
[[nodiscard]] constexpr bool has_popcnt() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_popcnt() noexcept { return to_bool(cpu_features & cpu_feature::popcnt); }
#endif

/** This CPU has the SSE3 instruction set.
 */
#if HI_HAS_SSE3
[[nodiscard]] constexpr bool has_sse3() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sse3() noexcept { return to_bool(cpu_features & cpu_feature::sse3); }
#endif

/** This CPU has the SSSE3 instruction set.
 */
#if HI_HAS_SSSE3
[[nodiscard]] constexpr bool has_ssse3() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_ssse3() noexcept { return to_bool(cpu_features & cpu_feature::ssse3); }
#endif

/** This CPU has the SSE4.1 instruction set.
 */
#if HI_HAS_SSE4_1
[[nodiscard]] constexpr bool has_sse4_1() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sse4_1() noexcept { return to_bool(cpu_features & cpu_feature::sse4_1); }
#endif

/** This CPU has the SSE4.2 instruction set.
 */
#if HI_HAS_SSE4_2
[[nodiscard]] constexpr bool has_sse4_2() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sse4_2() noexcept { return to_bool(cpu_features & cpu_feature::sse4_2); }
#endif

/** This CPU has all the features for x86-64-v2 microarchitecture level.
 */
#if HI_HAS_X86_64_V2
[[nodiscard]] constexpr bool has_x86_64_v2() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_x86_64_v2() noexcept { return (cpu_features & cpu_feature_mask::x86_64_v2) == cpu_feature_mask::x86_64_v2; }
#endif

/** This CPU has float-16 conversion instructions.
 */
#if HI_HAS_F16C
[[nodiscard]] constexpr bool has_f16c() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_f16c() noexcept { return to_bool(cpu_features & cpu_feature::f16c); }
#endif

/** This CPU has fused-multiply-accumulate instructions.
 */
#if HI_HAS_FMA
[[nodiscard]] constexpr bool has_fma() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_fma() noexcept { return to_bool(cpu_features & cpu_feature::fma); }
#endif

/** This CPU has the BMI1 instruction set.
 */
#if HI_HAS_BMI1
[[nodiscard]] constexpr bool has_bmi1() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_bmi1() noexcept { return to_bool(cpu_features & cpu_feature::bmi1); }
#endif

/** This CPU has the BMI2 instruction set.
 */
#if HI_HAS_BMI2
[[nodiscard]] constexpr bool has_bmi2() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_bmi2() noexcept { return to_bool(cpu_features & cpu_feature::bmi2); }
#endif

/** This CPU has the LZCNT instruction.
 */
#if HI_HAS_LZCNT
[[nodiscard]] constexpr bool has_lzcnt() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_lzcnt() noexcept { return to_bool(cpu_features & cpu_feature::lzcnt); }
#endif

/** This CPU has the MOVBE (Move Big Endian) instruction.
 */
#if HI_HAS_MOVBE
[[nodiscard]] constexpr bool has_movbe() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_movbe() noexcept { return to_bool(cpu_features & cpu_feature::movbe); }
#endif

/** This operating system uses the SXSAVE instruction.
 */
#if HI_HAS_OSXSAVE
[[nodiscard]] constexpr bool has_osxsave() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_osxsave() noexcept { return to_bool(cpu_features & cpu_feature::osxsave); }
#endif

/** This CPU has the AVX instruction set.
 */
#if HI_HAS_AVX
[[nodiscard]] constexpr bool has_avx() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx() noexcept { return to_bool(cpu_features & cpu_feature::avx); }
#endif

/** This CPU has the AVX2 instruction set.
 */
#if HI_HAS_AVX2
[[nodiscard]] constexpr bool has_avx2() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx2() noexcept { return to_bool(cpu_features & cpu_feature::avx2); }
#endif

/** This CPU has all the features for x86-64-v3 microarchitecture level.
 */
#if HI_HAS_X86_64_V3
[[nodiscard]] constexpr bool has_x86_64_v3() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_x86_64_v3() noexcept { return (cpu_features & cpu_feature_mask::x86_64_v3) == cpu_feature_mask::x86_64_v3; }
#endif

/** This CPU has the AVX512F instruction set.
 */
#if HI_HAS_AVX512F
[[nodiscard]] constexpr bool has_avx512f() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512f() noexcept { return to_bool(cpu_features & cpu_feature::avx512f); }
#endif

/** This CPU has the AVX512BW instruction set.
 */
#if HI_HAS_AVX512BW
[[nodiscard]] constexpr bool has_avx512bw() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512bw() noexcept { return to_bool(cpu_features & cpu_feature::avx512bw); }
#endif

/** This CPU has the AVX512CD instruction set.
 */
#if HI_HAS_AVX512CD
[[nodiscard]] constexpr bool has_avx512cd() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512cd() noexcept { return to_bool(cpu_features & cpu_feature::avx512cd); }
#endif

/** This CPU has the AVX512DQ instruction set.
 */
#if HI_HAS_AVX512DQ
[[nodiscard]] constexpr bool has_avx512dq() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512dq() noexcept { return to_bool(cpu_features & cpu_feature::avx512dq); }
#endif

/** This CPU has the AVX512VL instruction set.
 */
#if HI_HAS_AVX512VL
[[nodiscard]] constexpr bool has_avx512vl() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512vl() noexcept { return to_bool(cpu_features & cpu_feature::avx512vl); }
#endif

/** This CPU has all the features for x86-64-v4 microarchitecture level.
 */
#if HI_HAS_X86_64_V4
[[nodiscard]] constexpr bool has_x86_64_v4() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_x86_64_v4() noexcept { return (cpu_features & cpu_feature_mask::x86_64_v4) == cpu_feature_mask::x86_64_v4; }
#endif

/** This CPU has the AVX512PF instruction set.
 */
#if HI_HAS_AVX512PF
[[nodiscard]] constexpr bool has_avx512pf() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512pf() noexcept { return to_bool(cpu_features & cpu_feature::avx512pf); }
#endif

/** This CPU has the AVX512ER instruction set.
 */
#if HI_HAS_AVX512ER
[[nodiscard]] constexpr bool has_avx512er() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_avx512er() noexcept { return to_bool(cpu_features & cpu_feature::avx512er); }
#endif

/** This CPU has the SHA cryptographical secure hash instruction set.
 */
#if HI_HAS_SHA
[[nodiscard]] constexpr bool has_sha() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_sha() noexcept { return to_bool(cpu_features & cpu_feature::sha); }
#endif

/** This CPU has the AES-NI block cypher instruction set.
 */
#if HI_HAS_AES
[[nodiscard]] constexpr bool has_aes() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_aes() noexcept { return to_bool(cpu_features & cpu_feature::aes); }
#endif

/** This CPU has the PCLMUL carry-less multiply instruction.
 */
#if HI_HAS_AES
[[nodiscard]] constexpr bool has_pclmul() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_pclmul() noexcept { return to_bool(cpu_features & cpu_feature::pclmul); }
#endif

/** This CPU has the RDRAND on-chip random number generator instruction.
 */
#if HI_HAS_RDRND
[[nodiscard]] constexpr bool has_rdrnd() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_rdrnd() noexcept { return to_bool(cpu_features & cpu_feature::rdrnd); }
#endif

/** This CPU has the RDSEED access to the conditioned on-chip entropy.
 */
#if HI_HAS_RDSEED
[[nodiscard]] constexpr bool has_rdseed() noexcept { return true; }
#else
[[nodiscard]] hi_inline bool has_rdseed() noexcept { return to_bool(cpu_features & cpu_feature::rdseed); }
#endif

// clang-format on

}}

