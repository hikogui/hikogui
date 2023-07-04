// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <array>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
#include <cpuid.h>
#else
#error "Unsuported compiler for x64 cpu_id"
#endif

namespace hi {
inline namespace v1 {

class cpu_id {
public:
    constexpr static uint32_t processor_type_OEM = 0;
    constexpr static uint32_t processor_type_Intel_overdrive = 1;
    constexpr static uint32_t processor_type_dual_processor = 2;

    std::string vendor_id = {};
    std::string brand_name = {};

    uint32_t stepping_id:4 = 0;
    uint32_t model_id:8 = 0;
    uint32_t family_id:9 = 0;
    uint32_t processor_type:2 = 0;

    uint64_t features = 0;


    size_t cache_flush_size = 0;

    /** Local processor id.
     */
    uint8_t APIC_id = 0;

    cpu_id(cpu_id const &) noexcept = default;
    cpu_id(cpu_id &&) noexcept = default;
    cpu_id &operator=(cpu_id const &) noexcept = default;
    cpu_id &operator=(cpu_id &&) noexcept = default;

    cpu_id() noexcept
    {
        hilet leaf0 = get_leaf(0);
        hilet max_leaf = leaf0.a;

        // vendor_id are 12 characters from ebx, edx, ecx in that order.
        vendor_id.resize(12);
        std::memcpy(vendor_id.data() + 0, leaf0.b, 4);
        std::memcpy(vendor_id.data() + 4, leaf0.d, 4);
        std::memcpy(vendor_id.data() + 8, leaf0.c, 4);

        size_t brand_index = 0;
        if (max_leaf >= 1) {
            hilet leaf1 = get_leaf(1);

            stepping_id = leaf1.a & 0xf;
            model_id = (leaf1.a >> 4) & 0xf;
            family_id = (leaf1.a >> 8) & 0xf;
            processor_type = (leaf1.a >> 12) & 0x3;

            if (family_id == 0x6 or family_id == 0xf) {
                // Extended model is concatenated.
                model_id |= ((leaf1.a >> 16) & 0xf) << 4;
            }
            if (family_id == 0xf) {
                // Extended family is simply added.
                family_id += (leaf1.a >> 20) & 0xff;
            }

            brand_index = leaf1.b & 0xff;
            cache_flush_size = ((leaf1.b >> 8) & 0xff) * 8;
            APIC_id = (leaf1.b >> 24) & 0xff;

            
            
        }
    }


    [[nodiscard]] bool has_aesni() const noexcept
    {
        return to_bool(instruction_set & instruction_set_aesni);
    }

    [[nodiscard]] bool has_avx() const noexcept
    {
        return to_bool(instruction_set & instruction_set_avx);
    }

    [[nodiscard]] bool has_cmpxchg16b() const noexcept
    {
        return to_bool(instruction_set & instruction_set_cmpxchg16b);
    }

    [[nodiscard]] bool has_clfsh() const noexcept
    {
        return to_bool(instruction_set & instruction_set_clfsh);
    }

    [[nodiscard]] bool has_cmov() const noexcept
    {
        return to_bool(instruction_set & instruction_set_cmov);
    }

    [[nodiscard]] bool has_cx8() const noexcept
    {
        return to_bool(instruction_set & instruction_set_cx8);
    }

    [[nodiscard]] bool has_fma() const noexcept
    {
        return to_bool(instruction_set & instruction_set_fma);
    }

    [[nodiscard]] bool has_f16c() const noexcept
    {
        return to_bool(instruction_set & instruction_set_f16c);
    }

    [[nodiscard]] bool has_fxsr() const noexcept
    {
        return to_bool(instruction_set & instruction_set_fxsr);
    }

    [[nodiscard]] bool has_sse() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sse);
    }

    [[nodiscard]] bool has_sse2() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sse2);
    }

    [[nodiscard]] bool has_sse3() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sse3);
    }

    [[nodiscard]] bool has_ssse3() const noexcept
    {
        return to_bool(instruction_set & instruction_set_ssse3);
    }

    [[nodiscard]] bool has_sse4_1() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sse4_1);
    }

    [[nodiscard]] bool has_sse4_2() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sse4_2);
    }

    [[nodiscard]] bool has_movbe() const noexcept
    {
        return to_bool(instruction_set & instruction_set_movbe);
    }

    [[nodiscard]] bool has_mmx() const noexcept
    {
        return to_bool(instruction_set & instruction_set_mmx);
    }

    [[nodiscard]] bool has_msr() const noexcept
    {
        return to_bool(instruction_set & instruction_set_msr);
    }

    [[nodiscard]] bool has_osxsave() const noexcept
    {
        return to_bool(instruction_set & instruction_set_osxsave);
    }

    [[nodiscard]] bool has_pclmulqdq() const noexcept
    {
        return to_bool(instruction_set & instruction_set_pclmulqdq);
    }

    [[nodiscard]] bool has_popcnt() const noexcept
    {
        return to_bool(instruction_set & instruction_set_popcnt);
    }

    [[nodiscard]] bool has_rdrand() const noexcept
    {
        return to_bool(instruction_set & instruction_set_rdrand);
    }

    [[nodiscard]] bool has_sep() const noexcept
    {
        return to_bool(instruction_set & instruction_set_sep);
    }

    [[nodiscard]] bool has_tsc() const noexcept
    {
        return to_bool(instruction_set & instruction_set_tsc);
    }

    [[nodiscard]] bool has_xsave() const noexcept
    {
        return to_bool(instruction_set & instruction_set_xsave);
    }

    [[nodiscard]] bool has_acpi() const noexcept
    {
        return to_bool(features & features_acpi);
    }

    [[nodiscard]] bool has_apic() const noexcept
    {
        return to_bool(features & features_apic);
    }

    [[nodiscard]] bool has_cnxt_id() const noexcept
    {
        return to_bool(features & features_cnxt_id);
    }

    [[nodiscard]] bool has_dca() const noexcept
    {
        return to_bool(features & features_dca);
    }

    [[nodiscard]] bool has_de() const noexcept
    {
        return to_bool(features & features_de);
    }

    [[nodiscard]] bool has_ds() const noexcept
    {
        return to_bool(features & features_ds);
    }

    [[nodiscard]] bool has_ds_cpl() const noexcept
    {
        return to_bool(features & features_ds_cpl);
    }

    [[nodiscard]] bool has_dtes64() const noexcept
    {
        return to_bool(features & features_dtes64);
    }

    [[nodiscard]] bool has_eist() const noexcept
    {
        return to_bool(features & features_eist);
    }

    [[nodiscard]] bool has_fpu() const noexcept
    {
        return to_bool(features & features_fpu);
    }

    [[nodiscard]] bool has_htt() const noexcept
    {
        return to_bool(features & features_htt);
    }

    [[nodiscard]] bool has_mca() const noexcept
    {
        return to_bool(features & features_mca);
    }

    [[nodiscard]] bool has_mce() const noexcept
    {
        return to_bool(features & features_mce);
    }

    [[nodiscard]] bool has_monitor() const noexcept
    {
        return to_bool(features & features_monitor);
    }

    [[nodiscard]] bool has_mttr() const noexcept
    {
        return to_bool(features & features_mttr);
    }

    [[nodiscard]] bool has_pae() const noexcept
    {
        return to_bool(features & features_pae);
    }

    [[nodiscard]] bool has_pat() const noexcept
    {
        return to_bool(features & features_pat);
    }

    [[nodiscard]] bool has_pbe() const noexcept
    {
        return to_bool(features & features_pbe);
    }

    [[nodiscard]] bool has_pcid() const noexcept
    {
        return to_bool(features & features_pcid);
    }

    [[nodiscard]] bool has_pdcm() const noexcept
    {
        return to_bool(features & features_pdcm);
    }

    [[nodiscard]] bool has_pge() const noexcept
    {
        return to_bool(features & features_pge);
    }

    [[nodiscard]] bool has_pse() const noexcept
    {
        return to_bool(features & features_pse);
    }

    [[nodiscard]] bool has_pse_36() const noexcept
    {
        return to_bool(features & features_pse_36);
    }

    [[nodiscard]] bool has_psn() const noexcept
    {
        return to_bool(features & features_psm);
    }

    [[nodiscard]] bool has_sdbg() const noexcept
    {
        return to_bool(features & features_sdbg);
    }

    [[nodiscard]] bool has_smx() const noexcept
    {
        return to_bool(features & features_smx);
    }

    [[nodiscard]] bool has_ss() const noexcept
    {
        return to_bool(features & features_ss);
    }

    [[nodiscard]] bool has_tm() const noexcept
    {
        return to_bool(features & features_tm);
    }

    [[nodiscard]] bool has_tm2() const noexcept
    {
        return to_bool(features & features_tm2);
    }

    [[nodiscard]] bool has_tsc_deadline() const noexcept
    {
        return to_bool(features & features_tsc_deadline);
    }

    [[nodiscard]] bool has_vme() const noexcept
    {
        return to_bool(features & features_vme);
    }

    [[nodiscard]] bool has_vmx() const noexcept
    {
        return to_bool(features & features_vmx);
    }

    [[nodiscard]] bool has_x2apic() const noexcept
    {
        return to_bool(features & features_x2apic);
    }

    [[nodiscard]] bool has_xtpr() const noexcept
    {
        return to_bool(features & features_xtpr);
    }

private:
    // clang-format off
    constexpr static uint64_t instruction_set_aesni        = 0x0000'0000'0000'0001;
    constexpr static uint64_t instruction_set_avx          = 0x0000'0000'0000'0002;
    constexpr static uint64_t instruction_set_cmpxchg16b   = 0x0000'0000'0000'0004;
    constexpr static uint64_t instruction_set_clfsh        = 0x0000'0000'0000'0008;
    constexpr static uint64_t instruction_set_cmov         = 0x0000'0000'0000'0010;
    constexpr static uint64_t instruction_set_cx8          = 0x0000'0000'0000'0020;
    constexpr static uint64_t instruction_set_fma          = 0x0000'0000'0000'0040;
    constexpr static uint64_t instruction_set_f16c         = 0x0000'0000'0000'0080;
    constexpr static uint64_t instruction_set_fxsr         = 0x0000'0000'0000'0100;
    constexpr static uint64_t instruction_set_sse          = 0x0000'0000'0000'0200;
    constexpr static uint64_t instruction_set_sse2         = 0x0000'0000'0000'0300;
    constexpr static uint64_t instruction_set_sse3         = 0x0000'0000'0000'0800;
    constexpr static uint64_t instruction_set_ssse3        = 0x0000'0000'0000'1000;
    constexpr static uint64_t instruction_set_sse4_1       = 0x0000'0000'0000'2000;
    constexpr static uint64_t instruction_set_sse4_2       = 0x0000'0000'0000'4000;
    constexpr static uint64_t instruction_set_movbe        = 0x0000'0000'0000'8000;
    constexpr static uint64_t instruction_set_mmx          = 0x0000'0000'0001'0000;
    constexpr static uint64_t instruction_set_msr          = 0x0000'0000'0002'0000;
    constexpr static uint64_t instruction_set_osxsave      = 0x0000'0000'0004'0000;
    constexpr static uint64_t instruction_set_pclmulqdq    = 0x0000'0000'0008'0000;
    constexpr static uint64_t instruction_set_popcnt       = 0x0000'0000'0010'0000;
    constexpr static uint64_t instruction_set_rdrand       = 0x0000'0000'0020'0000;
    constexpr static uint64_t instruction_set_sep          = 0x0000'0000'0040'0000;
    constexpr static uint64_t instruction_set_tsc          = 0x0000'0000'0080'0000;
    constexpr static uint64_t instruction_set_xsave        = 0x0000'0000'0100'0000;

    constexpr static uint64_t features_acpi                = 0x0000'0000'0000'0001;
    constexpr static uint64_t features_apic                = 0x0000'0000'0000'0002;
    constexpr static uint64_t features_cnxt_id             = 0x0000'0000'0000'0004;
    constexpr static uint64_t features_dca                 = 0x0000'0000'0000'0008;
    constexpr static uint64_t features_de                  = 0x0000'0000'0000'0010;
    constexpr static uint64_t features_ds                  = 0x0000'0000'0000'0020;
    constexpr static uint64_t features_ds_cpl              = 0x0000'0000'0000'0040;
    constexpr static uint64_t features_dtes64              = 0x0000'0000'0000'0080;
    constexpr static uint64_t features_eist                = 0x0000'0000'0000'0100;
    constexpr static uint64_t features_fpu                 = 0x0000'0000'0000'0200;
    constexpr static uint64_t features_htt                 = 0x0000'0000'0000'0400;
    constexpr static uint64_t features_mca                 = 0x0000'0000'0000'0800;
    constexpr static uint64_t features_mce                 = 0x0000'0000'0000'1000;
    constexpr static uint64_t features_monitor             = 0x0000'0000'0000'2000;
    constexpr static uint64_t features_mttr                = 0x0000'0000'0000'4000;
    constexpr static uint64_t features_pae                 = 0x0000'0000'0000'8000;
    constexpr static uint64_t features_pat                 = 0x0000'0000'0001'0000;
    constexpr static uint64_t features_pbe                 = 0x0000'0000'0002'0000;
    constexpr static uint64_t features_pcid                = 0x0000'0000'0004'0000;
    constexpr static uint64_t features_pdcm                = 0x0000'0000'0008'0000;
    constexpr static uint64_t features_pge                 = 0x0000'0000'0010'0000;
    constexpr static uint64_t features_pse                 = 0x0000'0000'0020'0000;
    constexpr static uint64_t features_pse_36              = 0x0000'0000'0040'0000;
    constexpr static uint64_t features_psn                 = 0x0000'0000'0080'0000;
    constexpr static uint64_t features_sdbg                = 0x0000'0000'0100'0000;
    constexpr static uint64_t features_smx                 = 0x0000'0000'0200'0000;
    constexpr static uint64_t features_ss                  = 0x0000'0000'0400'0000;
    constexpr static uint64_t features_tm                  = 0x0000'0000'0800'0000;
    constexpr static uint64_t features_tm2                 = 0x0000'0000'1000'0000;
    constexpr static uint64_t features_tsc_deadline        = 0x0000'0000'2000'0000;
    constexpr static uint64_t features_vme                 = 0x0000'0000'4000'0000;
    constexpr static uint64_t features_vmx                 = 0x0000'0000'8000'0000;
    constexpr static uint64_t features_x2apic              = 0x0000'0001'0000'0000;
    constexpr static uint64_t features_xtpr                = 0x0000'0002'0000'0000;
    // clang-format off

    uint32_t instruction_set = 0;
    uint64_t features = 0;

    struct leaf_type {
        uint32_t a; // EAX
        uint32_t b; // EBX
        uint32_t c; // ECX
        uint32_t d; // EDX
    };

#if HI_COMPILER == HI_CC_MSVC

    [[nodiscard]] static leaf_type get_leaf(uint32_t leaf_id, uint32_t index = 0) noexcept
    {
        query_result_type r;
        int tmp[4];

        __cpuindex(tmp, char_cast<int>(cpu_id_leaf), char_cast<int>(index));

        std::memcpy(&r, tmp, sizeof(result_type));
        return r;
    }

#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG

    [[nodiscard]] static leaf_type get_leaf(uint32_t leaf_id, uint32_t index = 0) noexcept
    {
        query_result_type r;
        __cpuid_count(leaf_id, index, r.a, r.b, r.c, r.d);
        return r;
    }

#else
#error "Unsuported compiler for x64 cpu_id"
#endif
};

}}

