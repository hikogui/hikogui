// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_3166.hpp"
#include "../cast.hpp"
#include "../fixed_string.hpp"
#include "../exception.hpp"
#include "../charconv.hpp"
#include "../check.hpp"
#include <array>

namespace hi::inline v1 {

struct iso_3166_info {
    fixed_string<2> code2;
    fixed_string<3> code3;
    uint16_t number;
};

// clang-format off
constexpr auto iso_3166_infos = std::array{
#ifndef __INTELLISENSE__
    iso_3166_info{"AF", "AFG", 4},
    iso_3166_info{"AL", "ALB", 8},
    iso_3166_info{"AQ", "ATA", 10},
    iso_3166_info{"DZ", "DZA", 12},
    iso_3166_info{"AS", "ASM", 16},
    iso_3166_info{"AD", "AND", 20},
    iso_3166_info{"AO", "AGO", 24},
    iso_3166_info{"AG", "ATG", 28},
    iso_3166_info{"AZ", "AZE", 31},
    iso_3166_info{"AR", "ARG", 32},
    iso_3166_info{"AU", "AUS", 36},
    iso_3166_info{"AT", "AUT", 40},
    iso_3166_info{"BS", "BHS", 44},
    iso_3166_info{"BH", "BHR", 48},
    iso_3166_info{"BD", "BGD", 50},
    iso_3166_info{"AM", "ARM", 51},
    iso_3166_info{"BB", "BRB", 52},
    iso_3166_info{"BE", "BEL", 56},
    iso_3166_info{"BM", "BMU", 60},
    iso_3166_info{"BT", "BTN", 64},
    iso_3166_info{"BO", "BOL", 68},
    iso_3166_info{"BA", "BIH", 70},
    iso_3166_info{"BW", "BWA", 72},
    iso_3166_info{"BV", "BVT", 74},
    iso_3166_info{"BR", "BRA", 76},
    iso_3166_info{"BZ", "BLZ", 84},
    iso_3166_info{"IO", "IOT", 86},
    iso_3166_info{"SB", "SLB", 90},
    iso_3166_info{"VG", "VGB", 92},
    iso_3166_info{"BN", "BRN", 96},
    iso_3166_info{"BG", "BGR", 100},
    iso_3166_info{"MM", "MMR", 104},
    iso_3166_info{"BI", "BDI", 108},
    iso_3166_info{"BY", "BLR", 112},
    iso_3166_info{"KH", "KHM", 116},
    iso_3166_info{"CM", "CMR", 120},
    iso_3166_info{"CA", "CAN", 124},
    iso_3166_info{"CV", "CPV", 132},
    iso_3166_info{"KY", "CYM", 136},
    iso_3166_info{"CF", "CAF", 140},
    iso_3166_info{"LK", "LKA", 144},
    iso_3166_info{"TD", "TCD", 148},
    iso_3166_info{"CL", "CHL", 152},
    iso_3166_info{"CN", "CHN", 156},
    iso_3166_info{"TW", "TWN", 158},
    iso_3166_info{"CX", "CXR", 162},
    iso_3166_info{"CC", "CCK", 166},
    iso_3166_info{"CO", "COL", 170},
    iso_3166_info{"KM", "COM", 174},
    iso_3166_info{"YT", "MYT", 175},
    iso_3166_info{"CG", "COG", 178},
    iso_3166_info{"CD", "COD", 180},
    iso_3166_info{"CK", "COK", 184},
    iso_3166_info{"CR", "CRI", 188},
    iso_3166_info{"HR", "HRV", 191},
    iso_3166_info{"CU", "CUB", 192},
    iso_3166_info{"CY", "CYP", 196},
    iso_3166_info{"CZ", "CZE", 203},
    iso_3166_info{"BJ", "BEN", 204},
    iso_3166_info{"DK", "DNK", 208},
    iso_3166_info{"DM", "DMA", 212},
    iso_3166_info{"DO", "DOM", 214},
    iso_3166_info{"EC", "ECU", 218},
    iso_3166_info{"SV", "SLV", 222},
    iso_3166_info{"GQ", "GNQ", 226},
    iso_3166_info{"ET", "ETH", 231},
    iso_3166_info{"ER", "ERI", 232},
    iso_3166_info{"EE", "EST", 233},
    iso_3166_info{"FO", "FRO", 234},
    iso_3166_info{"FK", "FLK", 238},
    iso_3166_info{"GS", "SGS", 239},
    iso_3166_info{"FJ", "FJI", 242},
    iso_3166_info{"FI", "FIN", 246},
    iso_3166_info{"AX", "ALA", 248},
    iso_3166_info{"FR", "FRA", 250},
    iso_3166_info{"GF", "GUF", 254},
    iso_3166_info{"PF", "PYF", 258},
    iso_3166_info{"TF", "ATF", 260},
    iso_3166_info{"DJ", "DJI", 262},
    iso_3166_info{"GA", "GAB", 266},
    iso_3166_info{"GE", "GEO", 268},
    iso_3166_info{"GM", "GMB", 270},
    iso_3166_info{"PS", "PSE", 275},
    iso_3166_info{"DE", "DEU", 276},
    iso_3166_info{"GH", "GHA", 288},
    iso_3166_info{"GI", "GIB", 292},
    iso_3166_info{"KI", "KIR", 296},
    iso_3166_info{"GR", "GRC", 300},
    iso_3166_info{"GL", "GRL", 304},
    iso_3166_info{"GD", "GRD", 308},
    iso_3166_info{"GP", "GLP", 312},
    iso_3166_info{"GU", "GUM", 316},
    iso_3166_info{"GT", "GTM", 320},
    iso_3166_info{"GN", "GIN", 324},
    iso_3166_info{"GY", "GUY", 328},
    iso_3166_info{"HT", "HTI", 332},
    iso_3166_info{"HM", "HMD", 334},
    iso_3166_info{"VA", "VAT", 336},
    iso_3166_info{"HN", "HND", 340},
    iso_3166_info{"HK", "HKG", 344},
    iso_3166_info{"HU", "HUN", 348},
    iso_3166_info{"IS", "ISL", 352},
    iso_3166_info{"IN", "IND", 356},
    iso_3166_info{"ID", "IDN", 360},
    iso_3166_info{"IR", "IRN", 364},
    iso_3166_info{"IQ", "IRQ", 368},
    iso_3166_info{"IE", "IRL", 372},
    iso_3166_info{"IL", "ISR", 376},
    iso_3166_info{"IT", "ITA", 380},
    iso_3166_info{"CI", "CIV", 384},
    iso_3166_info{"JM", "JAM", 388},
    iso_3166_info{"JP", "JPN", 392},
    iso_3166_info{"KZ", "KAZ", 398},
    iso_3166_info{"JO", "JOR", 400},
    iso_3166_info{"KE", "KEN", 404},
    iso_3166_info{"KP", "PRK", 408},
    iso_3166_info{"KR", "KOR", 410},
    iso_3166_info{"KW", "KWT", 414},
    iso_3166_info{"KG", "KGZ", 417},
    iso_3166_info{"LA", "LAO", 418},
    iso_3166_info{"LB", "LBN", 422},
    iso_3166_info{"LS", "LSO", 426},
    iso_3166_info{"LV", "LVA", 428},
    iso_3166_info{"LR", "LBR", 430},
    iso_3166_info{"LY", "LBY", 434},
    iso_3166_info{"LI", "LIE", 438},
    iso_3166_info{"LT", "LTU", 440},
    iso_3166_info{"LU", "LUX", 442},
    iso_3166_info{"MO", "MAC", 446},
    iso_3166_info{"MG", "MDG", 450},
    iso_3166_info{"MW", "MWI", 454},
    iso_3166_info{"MY", "MYS", 458},
    iso_3166_info{"MV", "MDV", 462},
    iso_3166_info{"ML", "MLI", 466},
    iso_3166_info{"MT", "MLT", 470},
    iso_3166_info{"MQ", "MTQ", 474},
    iso_3166_info{"MR", "MRT", 478},
    iso_3166_info{"MU", "MUS", 480},
    iso_3166_info{"MX", "MEX", 484},
    iso_3166_info{"MC", "MCO", 492},
    iso_3166_info{"MN", "MNG", 496},
    iso_3166_info{"MD", "MDA", 498},
    iso_3166_info{"ME", "MNE", 499},
    iso_3166_info{"MS", "MSR", 500},
    iso_3166_info{"MA", "MAR", 504},
    iso_3166_info{"MZ", "MOZ", 508},
    iso_3166_info{"OM", "OMN", 512},
    iso_3166_info{"NA", "NAM", 516},
    iso_3166_info{"NR", "NRU", 520},
    iso_3166_info{"NP", "NPL", 524},
    iso_3166_info{"NL", "NLD", 528},
    iso_3166_info{"CW", "CUW", 531},
    iso_3166_info{"AW", "ABW", 533},
    iso_3166_info{"SX", "SXM", 534},
    iso_3166_info{"BQ", "BES", 535},
    iso_3166_info{"NC", "NCL", 540},
    iso_3166_info{"VU", "VUT", 548},
    iso_3166_info{"NZ", "NZL", 554},
    iso_3166_info{"NI", "NIC", 558},
    iso_3166_info{"NE", "NER", 562},
    iso_3166_info{"NG", "NGA", 566},
    iso_3166_info{"NU", "NIU", 570},
    iso_3166_info{"NF", "NFK", 574},
    iso_3166_info{"NO", "NOR", 578},
    iso_3166_info{"MP", "MNP", 580},
    iso_3166_info{"UM", "UMI", 581},
    iso_3166_info{"FM", "FSM", 583},
    iso_3166_info{"MH", "MHL", 584},
    iso_3166_info{"PW", "PLW", 585},
    iso_3166_info{"PK", "PAK", 586},
    iso_3166_info{"PA", "PAN", 591},
    iso_3166_info{"PG", "PNG", 598},
    iso_3166_info{"PY", "PRY", 600},
    iso_3166_info{"PE", "PER", 604},
    iso_3166_info{"PH", "PHL", 608},
    iso_3166_info{"PN", "PCN", 612},
    iso_3166_info{"PL", "POL", 616},
    iso_3166_info{"PT", "PRT", 620},
    iso_3166_info{"GW", "GNB", 624},
    iso_3166_info{"TL", "TLS", 626},
    iso_3166_info{"PR", "PRI", 630},
    iso_3166_info{"QA", "QAT", 634},
    iso_3166_info{"RE", "REU", 638},
    iso_3166_info{"RO", "ROU", 642},
    iso_3166_info{"RU", "RUS", 643},
    iso_3166_info{"RW", "RWA", 646},
    iso_3166_info{"BL", "BLM", 652},
    iso_3166_info{"SH", "SHN", 654},
    iso_3166_info{"KN", "KNA", 659},
    iso_3166_info{"AI", "AIA", 660},
    iso_3166_info{"LC", "LCA", 662},
    iso_3166_info{"MF", "MAF", 663},
    iso_3166_info{"PM", "SPM", 666},
    iso_3166_info{"VC", "VCT", 670},
    iso_3166_info{"SM", "SMR", 674},
    iso_3166_info{"ST", "STP", 678},
    iso_3166_info{"SA", "SAU", 682},
    iso_3166_info{"SN", "SEN", 686},
    iso_3166_info{"RS", "SRB", 688},
    iso_3166_info{"SC", "SYC", 690},
    iso_3166_info{"SL", "SLE", 694},
    iso_3166_info{"SG", "SGP", 702},
    iso_3166_info{"SK", "SVK", 703},
    iso_3166_info{"VN", "VNM", 704},
    iso_3166_info{"SI", "SVN", 705},
    iso_3166_info{"SO", "SOM", 706},
    iso_3166_info{"ZA", "ZAF", 710},
    iso_3166_info{"ZW", "ZWE", 716},
    iso_3166_info{"ES", "ESP", 724},
    iso_3166_info{"SS", "SSD", 728},
    iso_3166_info{"SD", "SDN", 729},
    iso_3166_info{"EH", "ESH", 732},
    iso_3166_info{"SR", "SUR", 740},
    iso_3166_info{"SJ", "SJM", 744},
    iso_3166_info{"SZ", "SWZ", 748},
    iso_3166_info{"SE", "SWE", 752},
    iso_3166_info{"CH", "CHE", 756},
    iso_3166_info{"SY", "SYR", 760},
    iso_3166_info{"TJ", "TJK", 762},
    iso_3166_info{"TH", "THA", 764},
    iso_3166_info{"TG", "TGO", 768},
    iso_3166_info{"TK", "TKL", 772},
    iso_3166_info{"TO", "TON", 776},
    iso_3166_info{"TT", "TTO", 780},
    iso_3166_info{"AE", "ARE", 784},
    iso_3166_info{"TN", "TUN", 788},
    iso_3166_info{"TR", "TUR", 792},
    iso_3166_info{"TM", "TKM", 795},
    iso_3166_info{"TC", "TCA", 796},
    iso_3166_info{"TV", "TUV", 798},
    iso_3166_info{"UG", "UGA", 800},
    iso_3166_info{"UA", "UKR", 804},
    iso_3166_info{"MK", "MKD", 807},
    iso_3166_info{"EG", "EGY", 818},
    iso_3166_info{"GB", "GBR", 826},
    iso_3166_info{"GG", "GGY", 831},
    iso_3166_info{"JE", "JEY", 832},
    iso_3166_info{"IM", "IMN", 833},
    iso_3166_info{"TZ", "TZA", 834},
    iso_3166_info{"US", "USA", 840},
    iso_3166_info{"VI", "VIR", 850},
    iso_3166_info{"BF", "BFA", 854},
    iso_3166_info{"UY", "URY", 858},
    iso_3166_info{"UZ", "UZB", 860},
    iso_3166_info{"VE", "VEN", 862},
    iso_3166_info{"WF", "WLF", 876},
    iso_3166_info{"WS", "WSM", 882},
    iso_3166_info{"YE", "YEM", 887},
#endif
    iso_3166_info{"ZM", "ZMB", 894}};

// clang-format on

constexpr auto iso_3166_code2_by_number_init() noexcept
{
    auto r = std::array<fixed_string<2>, 1000>{};
    for (auto i = 0_uz; i != r.size(); ++i) {
        r[i] = "ZZ";
    }

    for (hilet& info : iso_3166_infos) {
        r[info.number] = info.code2;
    }
    return r;
}

constexpr auto iso_3166_code3_by_number_init() noexcept
{
    auto r = std::array<fixed_string<3>, 1000>{};
    for (auto i = 0_uz; i != r.size(); ++i) {
        r[i] = "ZZZ";
    }

    for (hilet& info : iso_3166_infos) {
        r[info.number] = info.code3;
    }
    return r;
}

constexpr auto iso_3166_number_by_code2_init() noexcept
{
    constexpr auto size = std::tuple_size_v<decltype(iso_3166_infos)>;
    using type = std::pair<fixed_string<2>, uint16_t>;

    auto r = std::array<type, size>{};
    for (auto i = 0_uz; i != iso_3166_infos.size(); ++i) {
        hilet& info = iso_3166_infos[i];
        r[i] = {info.code2, info.number};
    }

    std::sort(r.begin(), r.end(), [](hilet& a, hilet& b) {
        return a.first < b.first;
    });
    return r;
}

constexpr auto iso_3166_number_by_code3_init() noexcept
{
    constexpr auto size = std::tuple_size_v<decltype(iso_3166_infos)>;
    using type = std::pair<fixed_string<3>, uint16_t>;

    auto r = std::array<type, size>{};
    for (auto i = 0_uz; i != iso_3166_infos.size(); ++i) {
        hilet& info = iso_3166_infos[i];
        r[i] = {info.code3, info.number};
    }

    std::sort(r.begin(), r.end(), [](hilet& a, hilet& b) {
        return a.first < b.first;
    });
    return r;
}

constexpr auto iso_3166_code2_by_number = iso_3166_code2_by_number_init();
constexpr auto iso_3166_code3_by_number = iso_3166_code3_by_number_init();
constexpr auto iso_3166_number_by_code2 = iso_3166_number_by_code2_init();
constexpr auto iso_3166_number_by_code3 = iso_3166_number_by_code3_init();

iso_3166::iso_3166(std::string_view str)
{
    if (is_digit(str)) {
        _v = from_string<uint16_t>(str);
        hi_parse_check(_v < 1000, "ISO-3166 number must be between 000 and 999, got '{}'", _v);

    } else if (str.size() == 2) {
        auto str_up = to_upper(str);

        hilet it = std::lower_bound(
            iso_3166_number_by_code2.begin(),
            iso_3166_number_by_code2.end(),
            str_up,
            [](hilet& item, hilet& value) {
                return item.first < value;
            });

        hi_parse_check(
            it != iso_3166_number_by_code2.end() and it->first == str_up,
            "Could not find ISO-3166 2 letter language code '{}'",
            str);

        _v = it->second;

    } else if (str.size() == 3) {
        auto str_up = to_upper(str);

        hilet it = std::lower_bound(
            iso_3166_number_by_code3.begin(),
            iso_3166_number_by_code3.end(),
            str_up,
            [](hilet& item, hilet& value) {
                return item.first < value;
            });

        hi_parse_check(
            it != iso_3166_number_by_code3.end() and it->first == str_up,
            "Could not find ISO-3166 3 letter language code '{}'",
            str);

        _v = it->second;

    } else {
        throw parse_error(std::format("Could not parse ISO-3166 code '{}'", str));
    }
}

[[nodiscard]] std::string_view iso_3166::code2() const noexcept
{
    hi_assert(_v < 1000);
    return iso_3166_code2_by_number[_v];
}

[[nodiscard]] std::string_view iso_3166::code3() const noexcept
{
    hi_assert(_v < 1000);
    return iso_3166_code3_by_number[_v];
}

} // namespace hi::inline v1
