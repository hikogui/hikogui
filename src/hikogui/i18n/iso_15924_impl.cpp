// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_15924.hpp"
#include "../utility/module.hpp"
#include "../algorithm/module.hpp"
#include <array>

namespace hi::inline v1 {

struct iso_15924_info {
    fixed_string<4> code4;
    fixed_string<4> code4_open_type;
    uint16_t number;

    constexpr iso_15924_info(
        char const (&code4)[5],
        char const (&code4_open_type)[5],
        uint16_t number) noexcept :
        code4{to_title(fixed_string(code4))}, code4_open_type{code4_open_type}, number(number)
    {
    }

    constexpr iso_15924_info() noexcept : iso_15924_info("zzzz", 999) {}

    constexpr iso_15924_info(char const (&code4)[5], uint16_t number) noexcept :
        iso_15924_info(code4, code4, number)
    {
    }
};

constexpr std::array iso_15924_infos = {
#ifndef __INTELLISENSE__
    iso_15924_info{"adlm", 166},
    iso_15924_info{"afak", 439},
    iso_15924_info{"aghb", 239},
    iso_15924_info{"ahom", 338},
    iso_15924_info{"arab", 160},
    iso_15924_info{"aran", 161},
    iso_15924_info{"armi", 124},
    iso_15924_info{"armn", 230},
    iso_15924_info{"avst", 134},
    iso_15924_info{"bali", 360},
    iso_15924_info{"bamu", 435},
    iso_15924_info{"bass", 259},
    iso_15924_info{"batk", 365},
    iso_15924_info{"beng", "bng2", 325},
    iso_15924_info{"bhks", 334},
    iso_15924_info{"blis", 550},
    iso_15924_info{"bopo", 285},
    iso_15924_info{"brah", 300},
    iso_15924_info{"brai", 570},
    iso_15924_info{"bugi", 367},
    iso_15924_info{"buhd", 372},
    iso_15924_info{"cakm", 349},
    iso_15924_info{"cans", 440},
    iso_15924_info{"cari", 201},
    iso_15924_info{"cham", 358},
    iso_15924_info{"cher", 445},
    iso_15924_info{"chrs", 109},
    iso_15924_info{"cirt", 291},
    iso_15924_info{"copt", 204},
    iso_15924_info{"cpmn", 402},
    iso_15924_info{"cprt", 403},
    iso_15924_info{"cyrl", 220},
    iso_15924_info{"cyrs", 221},
    iso_15924_info{"deva", "dev2", 315},
    iso_15924_info{"diak", 342},
    iso_15924_info{"dogr", 328},
    iso_15924_info{"dsrt", 250},
    iso_15924_info{"dupl", 755},
    iso_15924_info{"egyd", 70},
    iso_15924_info{"egyh", 60},
    iso_15924_info{"egyp", 50},
    iso_15924_info{"elba", 226},
    iso_15924_info{"elym", 128},
    iso_15924_info{"ethi", 430},
    iso_15924_info{"geok", 241},
    iso_15924_info{"geor", 240},
    iso_15924_info{"glag", 225},
    iso_15924_info{"gong", 312},
    iso_15924_info{"gonm", 313},
    iso_15924_info{"goth", 206},
    iso_15924_info{"gran", 343},
    iso_15924_info{"grek", 200},
    iso_15924_info{"gujr", "gjr2", 320},
    iso_15924_info{"guru", "gur2", 310},
    iso_15924_info{"hanb", 503},
    iso_15924_info{"hang", 286},
    iso_15924_info{"hani", 500},
    iso_15924_info{"hano", 371},
    iso_15924_info{"hans", 501},
    iso_15924_info{"hant", 502},
    iso_15924_info{"hatr", 127},
    iso_15924_info{"hebr", 125},
    iso_15924_info{"hira", 410},
    iso_15924_info{"hluw", 80},
    iso_15924_info{"hmng", 450},
    iso_15924_info{"hmnp", 451},
    iso_15924_info{"hrkt", 412},
    iso_15924_info{"hung", 176},
    iso_15924_info{"inds", 610},
    iso_15924_info{"ital", 210},
    iso_15924_info{"jamo", 284},
    iso_15924_info{"java", 361},
    iso_15924_info{"jpan", 413},
    iso_15924_info{"jurc", 510},
    iso_15924_info{"kali", 357},
    iso_15924_info{"kana", 411},
    iso_15924_info{"khar", 305},
    iso_15924_info{"khmr", 355},
    iso_15924_info{"khoj", 322},
    iso_15924_info{"kitl", 505},
    iso_15924_info{"kits", 288},
    iso_15924_info{"knda", "knd2", 345},
    iso_15924_info{"kore", 287},
    iso_15924_info{"kpel", 436},
    iso_15924_info{"kthi", 317},
    iso_15924_info{"lana", 351},
    iso_15924_info{"laoo", "lao ", 356},
    iso_15924_info{"latf", 217},
    iso_15924_info{"latg", 216},
    iso_15924_info{"latn", 215},
    iso_15924_info{"leke", 364},
    iso_15924_info{"lepc", 335},
    iso_15924_info{"limb", 336},
    iso_15924_info{"lina", 400},
    iso_15924_info{"linb", 401},
    iso_15924_info{"lisu", 399},
    iso_15924_info{"loma", 437},
    iso_15924_info{"lyci", 202},
    iso_15924_info{"lydi", 116},
    iso_15924_info{"mahj", 314},
    iso_15924_info{"maka", 366},
    iso_15924_info{"mand", 140},
    iso_15924_info{"mani", 139},
    iso_15924_info{"marc", 332},
    iso_15924_info{"maya", 90},
    iso_15924_info{"medf", 265},
    iso_15924_info{"mend", 438},
    iso_15924_info{"merc", 101},
    iso_15924_info{"mero", 100},
    iso_15924_info{"mlym", "mlm2", 347},
    iso_15924_info{"modi", 324},
    iso_15924_info{"mong", 145},
    iso_15924_info{"moon", 218},
    iso_15924_info{"mroo", 264},
    iso_15924_info{"mtei", 337},
    iso_15924_info{"mult", 323},
    iso_15924_info{"mymr", "mym2", 350},
    iso_15924_info{"nand", 311},
    iso_15924_info{"narb", 106},
    iso_15924_info{"nbat", 159},
    iso_15924_info{"newa", 333},
    iso_15924_info{"nkdb", 85},
    iso_15924_info{"nkgb", 420},
    iso_15924_info{"nkoo", "nko ", 165},
    iso_15924_info{"nshu", 499},
    iso_15924_info{"ogam", 212},
    iso_15924_info{"olck", 261},
    iso_15924_info{"orkh", 175},
    iso_15924_info{"orya", "ory2", 327},
    iso_15924_info{"osge", 219},
    iso_15924_info{"osma", 260},
    iso_15924_info{"ougr", 143},
    iso_15924_info{"palm", 126},
    iso_15924_info{"pauc", 263},
    iso_15924_info{"pcun", 15},
    iso_15924_info{"pelm", 16},
    iso_15924_info{"perm", 227},
    iso_15924_info{"phag", 331},
    iso_15924_info{"phli", 131},
    iso_15924_info{"phlp", 132},
    iso_15924_info{"phlv", 133},
    iso_15924_info{"phnx", 115},
    iso_15924_info{"plrd", 282},
    iso_15924_info{"piqd", 293},
    iso_15924_info{"prti", 130},
    iso_15924_info{"psin", 103},
    iso_15924_info{"qaaa", 900},
    iso_15924_info{"qabv", "byzm", 947}, // Open-type
    iso_15924_info{"qabw", "musc", 948}, // Open-type
    iso_15924_info{"qabx", 949},
    iso_15924_info{"ranj", 303},
    iso_15924_info{"rjng", 363},
    iso_15924_info{"rohg", 167},
    iso_15924_info{"roro", 620},
    iso_15924_info{"runr", 211},
    iso_15924_info{"samr", 123},
    iso_15924_info{"sara", 292},
    iso_15924_info{"sarb", 105},
    iso_15924_info{"saur", 344},
    iso_15924_info{"sgnw", 95},
    iso_15924_info{"shaw", 281},
    iso_15924_info{"shrd", 319},
    iso_15924_info{"shui", 530},
    iso_15924_info{"sidd", 302},
    iso_15924_info{"sind", 318},
    iso_15924_info{"sinh", 348},
    iso_15924_info{"sogd", 141},
    iso_15924_info{"sogo", 142},
    iso_15924_info{"sora", 398},
    iso_15924_info{"soyo", 329},
    iso_15924_info{"sund", 362},
    iso_15924_info{"sylo", 316},
    iso_15924_info{"syrc", 135},
    iso_15924_info{"syre", 138},
    iso_15924_info{"syrj", 137},
    iso_15924_info{"syrn", 136},
    iso_15924_info{"tagb", 373},
    iso_15924_info{"takr", 321},
    iso_15924_info{"tale", 353},
    iso_15924_info{"talu", 354},
    iso_15924_info{"taml", "tml2", 346},
    iso_15924_info{"tang", 520},
    iso_15924_info{"tavt", 359},
    iso_15924_info{"telu", "tel2", 340},
    iso_15924_info{"teng", 290},
    iso_15924_info{"tfng", 120},
    iso_15924_info{"tglg", 370},
    iso_15924_info{"thaa", 170},
    iso_15924_info{"thai", 352},
    iso_15924_info{"tibt", 330},
    iso_15924_info{"tirh", 326},
    iso_15924_info{"tnsa", 275},
    iso_15924_info{"toto", 294},
    iso_15924_info{"ugar", 40},
    iso_15924_info{"vaii", "vai ", 470},
    iso_15924_info{"visp", 280},
    iso_15924_info{"vith", 228},
    iso_15924_info{"wara", 262},
    iso_15924_info{"wcho", 283},
    iso_15924_info{"wole", 480},
    iso_15924_info{"xpeo", 30},
    iso_15924_info{"xsux", 20},
    iso_15924_info{"yezi", 192},
    iso_15924_info{"yiii", "yi  ", 460},
    iso_15924_info{"zanb", 339},
    iso_15924_info{"zinh", 994},
    iso_15924_info{"zmth", "math", 995},
    iso_15924_info{"zsye", 993},
    iso_15924_info{"zsym", 996},
    iso_15924_info{"zxxx", 997},
    iso_15924_info{"zyyy", "DFLT", 998},
#endif
    iso_15924_info{"zzzz", 999}};

constexpr auto iso_15924_code4_by_number_init() noexcept
{
    auto r = std::array<fixed_string<4>, 1000>{};

    for (hilet &info : iso_15924_infos) {
        r[info.number] = info.code4;
    }

    return r;
}

constexpr auto iso_15924_code4_open_type_by_number_init() noexcept
{
    auto r = std::array<fixed_string<4>, 1000>{};

    for (hilet &info : iso_15924_infos) {
        r[info.number] = info.code4_open_type;
    }

    return r;
}

constexpr auto iso_15924_number_by_code4_init() noexcept
{
    constexpr size_t array_size = std::tuple_size_v<decltype(iso_15924_infos)>;
    using record_type = std::pair<fixed_string<4>, uint16_t>;

    auto r = std::array<record_type, array_size>{};
    for (auto i = 0_uz; i != iso_15924_infos.size(); ++i) {
        r[i] = {iso_15924_infos[i].code4, iso_15924_infos[i].number};
    }
    std::sort(r.begin(), r.end(), [](hilet &a, hilet &b) {
        return a.first < b.first;
    });

    return r;
}

constexpr auto iso_15924_code4_by_number = iso_15924_code4_by_number_init();
constexpr auto iso_15924_code4_open_type_by_number = iso_15924_code4_open_type_by_number_init();
constexpr auto iso_15924_number_by_code4 = iso_15924_number_by_code4_init();

iso_15924::iso_15924(std::string_view code4)
{
    if (code4.size() != 4) {
        throw parse_error(std::format("Invalid script '{}'", code4));
    }

    hilet code4_ = to_title(code4);

    hilet it = std::lower_bound(
        iso_15924_number_by_code4.begin(), iso_15924_number_by_code4.end(), code4_, [](hilet &item, hilet &value) {
            return item.first < value;
        });

    if (it == iso_15924_number_by_code4.end() or it->first != code4_) {
        throw parse_error(std::format("Unknown script '{}'", code4));
    }
    
    _v = it->second;
}

[[nodiscard]] std::string_view iso_15924::code4() const noexcept
{
    hi_assert(_v < 1000);
    return static_cast<std::string_view>(iso_15924_code4_by_number[_v]);
}

[[nodiscard]] std::string_view iso_15924::code4_open_type() const noexcept
{
    hi_assert(_v < 1000);
    return static_cast<std::string_view>(iso_15924_code4_open_type_by_number[_v]);
}

[[nodiscard]] bool iso_15924::left_to_right() const noexcept
{
    switch (_v) {
    case 50: // Hyro
    case 105: // Sarb
    case 106: // Narb
    case 115: // Phnx
    case 116: // Lydi
    case 123: // Samr
    case 124: // Armi
    case 125: // Hebr
    case 126: // Palm
    case 127: // Hatr
    case 130: // Prti
    case 131: // Phli
    case 132: // Phlp
    case 133: // Phlv
    case 134: // Evst
    case 135: // Syrc
    case 136: // Syrn
    case 137: // Syrj
    case 138: // Syre
    case 140: // Mand
    case 141: // Sogd
    case 142: // Sogo
    case 159: // Nbat
    case 160: // Arab
    case 161: // Aran
    case 165: // Nkoo
    case 166: // Adlm
    case 167: // Rohg
    case 170: // Thaa
    case 175: // Orkh
    case 176: // Hung
    case 192: // Yezi
    case 210: // Ital
    case 305: // Khar
    case 403: // Cprt
    case 438: // Mend
    case 495: // Ethi
    case 610: // Inds
        return false;
    default:
        return true;
    }
}

} // namespace hi::inline v1
