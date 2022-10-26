// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include <array>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
#include <cpuid.h>
#else
#error "Unsuported compiler for x64 cpu_id"
#endif

namespace hi::inline v1 {

#if HI_COMPILER == HI_CC_MSVC
std::array<uint32_t, 4> cpu_id_x64_result cpu_id_x64(uint32_t cpu_id_leaf)
{
    std::array<int, 4> info;
    __cpuid(info.data(), static_cast<int>(cpu_id_leaf));

    std::array<uint32_t, 4> r;
    r[0] = static_cast<uint32_t> info[0];
    r[1] = static_cast<uint32_t> info[1];
    r[2] = static_cast<uint32_t> info[2];
    r[3] = static_cast<uint32_t> info[3];
    return r;
}

#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
std::array<uint32_t, 4> cpu_id_x64_result cpu_id_x64(uint32_t cpu_id_leaf)
{
    std::array<uint32_t, 4> r;
    __cpuid(cpu_id_leaf, r[0], r[1], r[2], r[3]);
    return r;
}

#else
#error "Unsuported compiler for x64 cpu_id"
#endif

inline std::array<uint32_t, 4> cpu_id_leaf1 = cpu_id_x64(1);
inline std::array<uint32_t, 4> cpu_id_leaf7 = cpu_id_x64(7);

template<int Bit>
bool cpu_id_leaf1_ecx()
{
    constexpr uint32_t mask = 1 << Bit;
    return (cpu_id_leaf1[2] & mask) != 0;
}

template<int Bit>
bool cpu_id_leaf1_edx()
{
    constexpr uint32_t mask = 1 << Bit;
    return (cpu_id_leaf1[3] & mask) != 0;
}

template<int Bit>
bool cpu_id_leaf7_ebx()
{
    constexpr uint32_t mask = 1 << Bit;
    return (cpu_id_leaf7[1] & mask) != 0;
}

template<int Bit>
bool cpu_id_leaf7_ecx()
{
    constexpr uint32_t mask = 1 << Bit;
    return (cpu_id_leaf7[2] & mask) != 0;
}

template<int Bit>
bool cpu_id_leaf7_edx()
{
    constexpr uint32_t mask = 1 << Bit;
    return (cpu_id_leaf7[3] & mask) != 0;
}

// LEAF1.0: EDX
bool cpu_has_fpu()
{
    return cpu_id_leaf1_edx<0>();
}
bool cpu_has_vme()
{
    return cpu_id_leaf1_edx<1>();
}
bool cpu_has_de()
{
    return cpu_id_leaf1_edx<2>();
}
bool cpu_has_pse()
{
    return cpu_id_leaf1_edx<3>();
}
bool cpu_has_tsc()
{
    return cpu_id_leaf1_edx<4>();
}
bool cpu_has_msr()
{
    return cpu_id_leaf1_edx<5>();
}
bool cpu_has_pae()
{
    return cpu_id_leaf1_edx<6>();
}
bool cpu_has_mce()
{
    return cpu_id_leaf1_edx<7>();
}
bool cpu_has_cx8()
{
    return cpu_id_leaf1_edx<8>();
}
bool cpu_has_apic()
{
    return cpu_id_leaf1_edx<9>();
}
// reserved
bool cpu_has_sep()
{
    return cpu_id_leaf1_edx<11>();
}
bool cpu_has_mtrr()
{
    return cpu_id_leaf1_edx<12>();
}
bool cpu_has_pge()
{
    return cpu_id_leaf1_edx<13>();
}
bool cpu_has_mca()
{
    return cpu_id_leaf1_edx<14>();
}
bool cpu_has_cmov()
{
    return cpu_id_leaf1_edx<15>();
}
bool cpu_has_pat()
{
    return cpu_id_leaf1_edx<16>();
}
bool cpu_has_pse_36()
{
    return cpu_id_leaf1_edx<17>();
}
bool cpu_has_psn()
{
    return cpu_id_leaf1_edx<18>();
}
bool cpu_has_clfsh()
{
    return cpu_id_leaf1_edx<19>();
}
// reserved
bool cpu_has_ds()
{
    return cpu_id_leaf1_edx<21>();
}
bool cpu_has_acpi()
{
    return cpu_id_leaf1_edx<22>();
}
bool cpu_has_mmx()
{
    return cpu_id_leaf1_edx<23>();
}
bool cpu_has_fxsr()
{
    return cpu_id_leaf1_edx<24>();
}
bool cpu_has_sse()
{
    return cpu_id_leaf1_edx<25>();
}
bool cpu_has_sse2()
{
    return cpu_id_leaf1_edx<26>();
}
bool cpu_has_ss()
{
    return cpu_id_leaf1_edx<27>();
}
bool cpu_has_htt()
{
    return cpu_id_leaf1_edx<28>();
}
bool cpu_has_tm()
{
    return cpu_id_leaf1_edx<29>();
}
bool cpu_has_ia64()
{
    return cpu_id_leaf1_edx<30>();
}
bool cpu_has_pbe()
{
    return cpu_id_leaf1_edx<31>();
}

// LEAF1.0: ECX
bool cpu_has_sse3()
{
    return cpu_id_leaf1_ecx<0>();
}
bool cpu_has_pclmulqdq()
{
    return cpu_id_leaf1_ecx<1>();
}
bool cpu_has_dtes64()
{
    return cpu_id_leaf1_ecx<2>();
}
bool cpu_has_monitor()
{
    return cpu_id_leaf1_ecx<3>();
}
bool cpu_has_ds_cpl()
{
    return cpu_id_leaf1_ecx<4>();
}
bool cpu_has_vmx()
{
    return cpu_id_leaf1_ecx<5>();
}
bool cpu_has_smx()
{
    return cpu_id_leaf1_ecx<6>();
}
bool cpu_has_est()
{
    return cpu_id_leaf1_ecx<7>();
}
bool cpu_has_tm2()
{
    return cpu_id_leaf1_ecx<8>();
}
bool cpu_has_ssse3()
{
    return cpu_id_leaf1_ecx<9>();
}
bool cpu_has_cnxt_id()
{
    return cpu_id_leaf1_ecx<10>();
}
bool cpu_has_sdbg()
{
    return cpu_id_leaf1_ecx<11>();
}
bool cpu_has_fma()
{
    return cpu_id_leaf1_ecx<12>();
}
bool cpu_has_cx16()
{
    return cpu_id_leaf1_ecx<13>();
}
bool cpu_has_xtpr()
{
    return cpu_id_leaf1_ecx<14>();
}
bool cpu_has_pdcm()
{
    return cpu_id_leaf1_ecx<15>();
}
// reserved
bool cpu_has_pcid()
{
    return cpu_id_leaf1_ecx<17>();
}
bool cpu_has_dca()
{
    return cpu_id_leaf1_ecx<18>();
}
bool cpu_has_sse4_1()
{
    return cpu_id_leaf1_ecx<19>();
}
bool cpu_has_sse4_2()
{
    return cpu_id_leaf1_ecx<20>();
}
bool cpu_has_x2apic()
{
    return cpu_id_leaf1_ecx<21>();
}
bool cpu_has_movbe()
{
    return cpu_id_leaf1_ecx<22>();
}
bool cpu_has_popcnt()
{
    return cpu_id_leaf1_ecx<23>();
}
bool cpu_has_tsc_deadline()
{
    return cpu_id_leaf1_ecx<24>();
}
bool cpu_has_aes()
{
    return cpu_id_leaf1_ecx<25>();
}
bool cpu_has_xsave()
{
    return cpu_id_leaf1_ecx<26>();
}
bool cpu_has_osxsave()
{
    return cpu_id_leaf1_ecx<27>();
}
bool cpu_has_avx()
{
    return cpu_id_leaf1_ecx<28>();
}
bool cpu_has_f16c()
{
    return cpu_id_leaf1_ecx<29>();
}
bool cpu_has_rdrnd()
{
    return cpu_id_leaf1_ecx<30>();
}
bool cpu_has_hypervisor()
{
    return cpu_id_leaf1_ecx<31>();
}

// LEAF1.0: EBX

// LEAF1.0: EAX
bool cpu_stepping()
{
    return cpu_id_leaf1[0] & 0xf;
}
bool cpu_model_id()
{
    uint32_t family_id = (cpu_id_leaf1[0] >> 8) & 0xf;
    uint32_t model_id = (cpu_id_leaf1[0] >> 4) & 0xf;
    if (family_id == 6 || family_id == 15) {
        uint32_t extended_model_id = (cpu_id_leaf1[0] >> 16) & 0xf;
        return (extended_model_id << 4) | model_id;
    } else {
        return model_id;
    }
}
bool cpu_family_id()
{
    uint32_t family_id = (cpu_id_leaf1[0] >> 8) & 0xf;
    if (family_id == 15) {
        uint32_t extended_family_id = (cpu_id_leaf1[0] >> 20) & 0xff;
        return family_id + extended_family_id;
    } else {
        return extended_family_id;
    }
}

// LEAF7.0: EBX
bool cpu_has_fsgsbase()
{
    return cpu_id_leaf7_ebx<0>();
}
bool cpu_has_tsc_adjust()
{
    return cpu_id_leaf7_ebx<1>();
}
bool cpu_has_sgx()
{
    return cpu_id_leaf7_ebx<2>();
}
bool cpu_has_bmi1()
{
    return cpu_id_leaf7_ebx<3>();
}
bool cpu_has_hle()
{
    return cpu_id_leaf7_ebx<4>();
}
bool cpu_has_avx2()
{
    return cpu_id_leaf7_ebx<5>();
}
// reserved
bool cpu_has_smep()
{
    return cpu_id_leaf7_ebx<7>();
}
bool cpu_has_bmi2()
{
    return cpu_id_leaf7_ebx<8>();
}
bool cpu_has_erms()
{
    return cpu_id_leaf7_ebx<9>();
}
bool cpu_has_invpcid()
{
    return cpu_id_leaf7_ebx<10>();
}
bool cpu_has_rtm()
{
    return cpu_id_leaf7_ebx<11>();
}
bool cpu_has_pqm()
{
    return cpu_id_leaf7_ebx<12>();
}
bool cpu_has_deprecated_fpu_cs_ds()
{
    return cpu_id_leaf7_ebx<13>();
}
bool cpu_has_mpx()
{
    return cpu_id_leaf7_ebx<14>();
}
bool cpu_has_pqe()
{
    return cpu_id_leaf7_ebx<15>();
}
bool cpu_has_avx512_f()
{
    return cpu_id_leaf7_ebx<16>();
}
bool cpu_has_avx512_dq()
{
    return cpu_id_leaf7_ebx<17>();
}
bool cpu_has_rdseed()
{
    return cpu_id_leaf7_ebx<18>();
}
bool cpu_has_adx()
{
    return cpu_id_leaf7_ebx<19>();
}
bool cpu_has_smap()
{
    return cpu_id_leaf7_ebx<20>();
}
bool cpu_has_avx512_ifma()
{
    return cpu_id_leaf7_ebx<21>();
}
bool cpu_has_pcommit()
{
    return cpu_id_leaf7_ebx<22>();
}
bool cpu_has_clflushopt()
{
    return cpu_id_leaf7_ebx<23>();
}
bool cpu_has_clwb()
{
    return cpu_id_leaf7_ebx<24>();
}
bool cpu_has_intelpt()
{
    return cpu_id_leaf7_ebx<25>();
}
bool cpu_has_avx512_pf()
{
    return cpu_id_leaf7_ebx<26>();
}
bool cpu_has_avx512_er()
{
    return cpu_id_leaf7_ebx<27>();
}
bool cpu_has_avx512_cd()
{
    return cpu_id_leaf7_ebx<28>();
}
bool cpu_has_sha()
{
    return cpu_id_leaf7_ebx<29>();
}
bool cpu_has_avx512_bw()
{
    return cpu_id_leaf7_ebx<30>();
}
bool cpu_has_avx512_vl()
{
    return cpu_id_leaf7_ebx<31>();
}

// LEAF7.0: ECX
bool cpu_has_prefetchwt1()
{
    return cpu_id_leaf7_ecx<0>();
}
bool cpu_has_avx512_vbmi()
{
    return cpu_id_leaf7_ecx<1>();
}
bool cpu_has_umip()
{
    return cpu_id_leaf7_ecx<2>();
}
bool cpu_has_pku()
{
    return cpu_id_leaf7_ecx<3>();
}
bool cpu_has_ospke()
{
    return cpu_id_leaf7_ecx<4>();
}
bool cpu_has_waitpkg()
{
    return cpu_id_leaf7_ecx<5>();
}
bool cpu_has_avx512_vmbi2()
{
    return cpu_id_leaf7_ecx<6>();
}
bool cpu_has_shstk()
{
    return cpu_id_leaf7_ecx<7>();
}
bool cpu_has_gfni()
{
    return cpu_id_leaf7_ecx<8>();
}
bool cpu_has_vaes()
{
    return cpu_id_leaf7_ecx<9>();
}
bool cpu_has_vpclmulqdq()
{
    return cpu_id_leaf7_ecx<10>();
}
bool cpu_has_avx512_vnni()
{
    return cpu_id_leaf7_ecx<11>();
}
bool cpu_has_avx512_bitalg()
{
    return cpu_id_leaf7_ecx<12>();
}
// reserved
bool cpu_has_avx512_vpopcntdq()
{
    return cpu_id_leaf7_ecx<14>();
}
// reserved
bool cpu_has_5level_paging()
{
    return cpu_id_leaf7_ecx<16>();
}
int cpu_has_mawau() {}
bool cpu_has_rdpid()
{
    return cpu_id_leaf7_ecx<22>();
}
// reserved
// reserved
bool cpu_has_cldemote()
{
    return cpu_id_leaf7_ecx<25>();
}
// reserved
bool cpu_has_movdir()
{
    return cpu_id_leaf7_ecx<27>();
}
bool cpu_has_movdir64b()
{
    return cpu_id_leaf7_ecx<28>();
}
// reserved
bool cpu_has_sgx_lc()
{
    return cpu_id_leaf7_ecx<30>();
}
// reserved

// LEAF7.0: EDX
// reserved
// reserved
bool cpu_has_avx512_4vnniw()
{
    return cpu_id_leaf7_edx<2>();
}
bool cpu_has_avx512_4fmaps()
{
    return cpu_id_leaf7_edx<3>();
}
bool cpu_has_fsrm()
{
    return cpu_id_leaf7_edx<4>();
}
bool cpu_has_pconfig()
{
    return cpu_id_leaf7_edx<18>();
}
// reserved
bool cpu_has_ibt()
{
    return cpu_id_leaf7_edx<20>();
}
// reserved 5
bool cpu_has_spec_ctrl()
{
    return cpu_id_leaf7_edx<26>();
}
bool cpu_has_stibp()
{
    return cpu_id_leaf7_edx<27>();
}
// reserved
bool cpu_has_capabilities()
{
    return cpu_id_leaf7_edx<29>();
}
// reserved
bool cpu_has_ssbd()
{
    return cpu_id_leaf7_edx<31>();
}
} // namespace hi::inline v1
