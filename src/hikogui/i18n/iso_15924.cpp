// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_15924.hpp"
#include "../unicode/unicode_script.hpp"
#include "../cast.hpp"
#include "../fixed_string.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include <array>

namespace hi::inline v1 {
using enum unicode_script;

struct iso_15924_info {
    fixed_string<4> code4;
    fixed_string<4> code4_open_type;
    hi::unicode_script unicode_script;
    uint16_t number;

    constexpr iso_15924_info(
        char const (&code4)[5],
        char const (&code4_open_type)[5],
        hi::unicode_script unicode_script,
        uint16_t number) noexcept :
        code4{to_title(fixed_string(code4))}, code4_open_type{code4_open_type}, unicode_script(unicode_script), number(number)
    {
    }

    constexpr iso_15924_info() noexcept : iso_15924_info("zzzz", Zzzz, 999) {}

    constexpr iso_15924_info(char const (&code4)[5], hi::unicode_script unicode_script, uint16_t number) noexcept :
        iso_15924_info(code4, code4, unicode_script, number)
    {
    }
};

constexpr std::array iso_15924_infos = {
#ifndef __INTELLISENSE__
    iso_15924_info{"adlm", Adlam, 166},
    iso_15924_info{"afak", Afaka, 439},
    iso_15924_info{"aghb", Caucasian_Albanian, 239},
    iso_15924_info{"ahom", Ahom, 338},
    iso_15924_info{"arab", Arabic, 160},
    iso_15924_info{"aran", Arabic_Nastaliq, 161},
    iso_15924_info{"armi", Imperial_Aramaic, 124},
    iso_15924_info{"armn", Armenian, 230},
    iso_15924_info{"avst", Avestan, 134},
    iso_15924_info{"bali", Balinese, 360},
    iso_15924_info{"bamu", Bamum, 435},
    iso_15924_info{"bass", Bassa_Vah, 259},
    iso_15924_info{"batk", Batak, 365},
    iso_15924_info{"beng", "bng2", Bengali, 325},
    iso_15924_info{"bhks", Bhaiksuki, 334},
    iso_15924_info{"blis", Blissymbols, 550},
    iso_15924_info{"bopo", Bopomofo, 285},
    iso_15924_info{"brah", Brahmi, 300},
    iso_15924_info{"brai", Braille, 570},
    iso_15924_info{"bugi", Buginese, 367},
    iso_15924_info{"buhd", Buhid, 372},
    iso_15924_info{"cakm", Chakma, 349},
    iso_15924_info{"cans", Canadian_Aboriginal, 440},
    iso_15924_info{"cari", Carian, 201},
    iso_15924_info{"cham", Cham, 358},
    iso_15924_info{"cher", Cherokee, 445},
    iso_15924_info{"chrs", Chorasmian, 109},
    iso_15924_info{"cirt", Cirth, 291},
    iso_15924_info{"copt", Coptic, 204},
    iso_15924_info{"cpmn", Cypro_Minoan, 402},
    iso_15924_info{"cprt", Cypriot, 403},
    iso_15924_info{"cyrl", Cyrillic, 220},
    iso_15924_info{"cyrs", Cyrillic_Old_Church_Slavonic, 221},
    iso_15924_info{"deva", "dev2", Devanagari, 315},
    iso_15924_info{"diak", Dives_Akuru, 342},
    iso_15924_info{"dogr", Dogra, 328},
    iso_15924_info{"dsrt", Deseret, 250},
    iso_15924_info{"dupl", Duployan, 755},
    iso_15924_info{"egyd", Egyptian_Demotic, 70},
    iso_15924_info{"egyh", Egyptian_Hieratic, 60},
    iso_15924_info{"egyp", Egyptian_Hieroglyphs, 50},
    iso_15924_info{"elba", Elbasan, 226},
    iso_15924_info{"elym", Elymaic, 128},
    iso_15924_info{"ethi", Ethiopic, 430},
    iso_15924_info{"geok", Khutsuri, 241},
    iso_15924_info{"geor", Georgian, 240},
    iso_15924_info{"glag", Glagolitic, 225},
    iso_15924_info{"gong", Gunjala_Gondi, 312},
    iso_15924_info{"gonm", Masaram_Gondi, 313},
    iso_15924_info{"goth", Gothic, 206},
    iso_15924_info{"gran", Grantha, 343},
    iso_15924_info{"grek", Greek, 200},
    iso_15924_info{"gujr", "gjr2", Gujarati, 320},
    iso_15924_info{"guru", "gur2", Gurmukhi, 310},
    iso_15924_info{"hanb", Han_Bopomofo, 503},
    iso_15924_info{"hang", Hangul, 286},
    iso_15924_info{"hani", Han, 500},
    iso_15924_info{"hano", Hanunoo, 371},
    iso_15924_info{"hans", Han_Simplified, 501},
    iso_15924_info{"hant", Han_Traditional, 502},
    iso_15924_info{"hatr", Hatran, 127},
    iso_15924_info{"hebr", Hebrew, 125},
    iso_15924_info{"hira", Hiragana, 410},
    iso_15924_info{"hluw", Anatolian_Hieroglyphs, 80},
    iso_15924_info{"hmng", Pahawh_Hmong, 450},
    iso_15924_info{"hmnp", Nyiakeng_Puachue_Hmong, 451},
    iso_15924_info{"hrkt", Japanese_Syllabaries, 412},
    iso_15924_info{"hung", Old_Hungarian, 176},
    iso_15924_info{"inds", Indus, 610},
    iso_15924_info{"ital", Old_Italic, 210},
    iso_15924_info{"jamo", Jamo, 284},
    iso_15924_info{"java", Javanese, 361},
    iso_15924_info{"jpan", Japanese, 413},
    iso_15924_info{"jurc", Jurchen, 510},
    iso_15924_info{"kali", Kayah_Li, 357},
    iso_15924_info{"kana", Katakana, 411},
    iso_15924_info{"khar", Kharoshthi, 305},
    iso_15924_info{"khmr", Khmer, 355},
    iso_15924_info{"khoj", Khojki, 322},
    iso_15924_info{"kitl", Khitan_Large_Script, 505},
    iso_15924_info{"kits", Khitan_Small_Script, 288},
    iso_15924_info{"knda", "knd2", Kannada, 345},
    iso_15924_info{"kore", Korean, 287},
    iso_15924_info{"kpel", Kpelle, 436},
    iso_15924_info{"kthi", Kaithi, 317},
    iso_15924_info{"lana", Tai_Tham, 351},
    iso_15924_info{"laoo", "lao ", Lao, 356},
    iso_15924_info{"latf", Latin_Fraktur, 217},
    iso_15924_info{"latg", Latin_Gaelic, 216},
    iso_15924_info{"latn", Latin, 215},
    iso_15924_info{"leke", Leke, 364},
    iso_15924_info{"lepc", Lepcha, 335},
    iso_15924_info{"limb", Limbu, 336},
    iso_15924_info{"lina", Linear_A, 400},
    iso_15924_info{"linb", Linear_B, 401},
    iso_15924_info{"lisu", Lisu, 399},
    iso_15924_info{"loma", Loma, 437},
    iso_15924_info{"lyci", Lycian, 202},
    iso_15924_info{"lydi", Lydian, 116},
    iso_15924_info{"mahj", Mahajani, 314},
    iso_15924_info{"maka", Makasar, 366},
    iso_15924_info{"mand", Mandaic, 140},
    iso_15924_info{"mani", Manichaean, 139},
    iso_15924_info{"marc", Marchen, 332},
    iso_15924_info{"maya", Mayan_Hieroglyphs, 90},
    iso_15924_info{"medf", Medefaidrin, 265},
    iso_15924_info{"mend", Mende_Kikakui, 438},
    iso_15924_info{"merc", Meroitic_Cursive, 101},
    iso_15924_info{"mero", Meroitic_Hieroglyphs, 100},
    iso_15924_info{"mlym", "mlm2", Malayalam, 347},
    iso_15924_info{"modi", Modi, 324},
    iso_15924_info{"mong", Mongolian, 145},
    iso_15924_info{"moon", Moon, 218},
    iso_15924_info{"mroo", Mro, 264},
    iso_15924_info{"mtei", Meetei_Mayek, 337},
    iso_15924_info{"mult", Multani, 323},
    iso_15924_info{"mymr", "mym2", Myanmar, 350},
    iso_15924_info{"nand", Nandinagari, 311},
    iso_15924_info{"narb", Old_North_Arabian, 106},
    iso_15924_info{"nbat", Nabataean, 159},
    iso_15924_info{"newa", Newa, 333},
    iso_15924_info{"nkdb", Naxi_Dongba, 85},
    iso_15924_info{"nkgb", Nakhi_Geba, 420},
    iso_15924_info{"nkoo", "nko ", Nko, 165},
    iso_15924_info{"nshu", Nushu, 499},
    iso_15924_info{"ogam", Ogham, 212},
    iso_15924_info{"olck", Ol_Chiki, 261},
    iso_15924_info{"orkh", Old_Turkic, 175},
    iso_15924_info{"orya", "ory2", Oriya, 327},
    iso_15924_info{"osge", Osage, 219},
    iso_15924_info{"osma", Osmanya, 260},
    iso_15924_info{"ougr", Old_Uyghur, 143},
    iso_15924_info{"palm", Palmyrene, 126},
    iso_15924_info{"pauc", Pau_Cin_Hau, 263},
    iso_15924_info{"pcun", Proto_Cuneiform, 15},
    iso_15924_info{"pelm", Proto_Elamite, 16},
    iso_15924_info{"perm", Old_Permic, 227},
    iso_15924_info{"phag", Phags_Pa, 331},
    iso_15924_info{"phli", Inscriptional_Pahlavi, 131},
    iso_15924_info{"phlp", Psalter_Pahlavi, 132},
    iso_15924_info{"phlv", Book_Pahlavi, 133},
    iso_15924_info{"phnx", Phoenician, 115},
    iso_15924_info{"plrd", Miao, 282},
    iso_15924_info{"piqd", Kligon, 293},
    iso_15924_info{"prti", Inscriptional_Parthian, 130},
    iso_15924_info{"psin", Proto_Sinaitic, 103},
    iso_15924_info{"qaaa", Private_Use_aa, 900},
    iso_15924_info{"qabv", "byzm", Byzantine_Music, 947}, // Open-type
    iso_15924_info{"qabw", "musc", Music, 948}, // Open-type
    iso_15924_info{"qabx", Private_Use_bx, 949},
    iso_15924_info{"ranj", Ranjana, 303},
    iso_15924_info{"rjng", Rejang, 363},
    iso_15924_info{"rohg", Hanifi_Rohingya, 167},
    iso_15924_info{"roro", Rongorongo, 620},
    iso_15924_info{"runr", Runic, 211},
    iso_15924_info{"samr", Samaritan, 123},
    iso_15924_info{"sara", Sarati, 292},
    iso_15924_info{"sarb", Old_South_Arabian, 105},
    iso_15924_info{"saur", Saurashtra, 344},
    iso_15924_info{"sgnw", SignWriting, 95},
    iso_15924_info{"shaw", Shavian, 281},
    iso_15924_info{"shrd", Sharada, 319},
    iso_15924_info{"shui", Shuishu, 530},
    iso_15924_info{"sidd", Siddham, 302},
    iso_15924_info{"sind", Khudawadi, 318},
    iso_15924_info{"sinh", Sinhala, 348},
    iso_15924_info{"sogd", Sogdian, 141},
    iso_15924_info{"sogo", Old_Sogdian, 142},
    iso_15924_info{"sora", Sora_Sompeng, 398},
    iso_15924_info{"soyo", Soyombo, 329},
    iso_15924_info{"sund", Sundanese, 362},
    iso_15924_info{"sylo", Syloti_Nagri, 316},
    iso_15924_info{"syrc", Syriac, 135},
    iso_15924_info{"syre", Syriac_Estrangelo, 138},
    iso_15924_info{"syrj", Syriac_Western, 137},
    iso_15924_info{"syrn", Syriac_Eastern, 136},
    iso_15924_info{"tagb", Tagbanwa, 373},
    iso_15924_info{"takr", Takri, 321},
    iso_15924_info{"tale", Tai_Le, 353},
    iso_15924_info{"talu", New_Tai_Lue, 354},
    iso_15924_info{"taml", "tml2", Tamil, 346},
    iso_15924_info{"tang", Tangsa, 520},
    iso_15924_info{"tavt", Tai_Viet, 359},
    iso_15924_info{"telu", "tel2", Telugu, 340},
    iso_15924_info{"teng", Tengwar, 290},
    iso_15924_info{"tfng", Tifinagh, 120},
    iso_15924_info{"tglg", Tagalog, 370},
    iso_15924_info{"thaa", Thaana, 170},
    iso_15924_info{"thai", Thai, 352},
    iso_15924_info{"tibt", Tibetan, 330},
    iso_15924_info{"tirh", Tirhuta, 326},
    iso_15924_info{"tnsa", Tangsa, 275},
    iso_15924_info{"toto", Toto, 294},
    iso_15924_info{"ugar", Ugaritic, 40},
    iso_15924_info{"vaii", "vai ", Vai, 470},
    iso_15924_info{"visp", Visible_Speech, 280},
    iso_15924_info{"vith", Vithkuqi, 228},
    iso_15924_info{"wara", Warang_Citi, 262},
    iso_15924_info{"wcho", Wancho, 283},
    iso_15924_info{"wole", Woleai, 480},
    iso_15924_info{"xpeo", Old_Persian, 30},
    iso_15924_info{"xsux", Cuneiform, 20},
    iso_15924_info{"yezi", Yezidi, 192},
    iso_15924_info{"yiii", "yi  ", Yi, 460},
    iso_15924_info{"zanb", Zanabazar_Square, 339},
    iso_15924_info{"zinh", Inherited, 994},
    iso_15924_info{"zmth", "math", Mathematical_Notation, 995},
    iso_15924_info{"zsye", Symbols_Emoji, 993},
    iso_15924_info{"zsym", Symbols, 996},
    iso_15924_info{"zxxx", Unwritten_Documents, 997},
    iso_15924_info{"zyyy", "DFLT", Common, 998},
#endif
    iso_15924_info{"zzzz", Zzzz, 999}};

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

constexpr auto iso_15924_number_by_unicode_script_init() noexcept
{
    auto r = std::array<uint16_t, 256>{};

    for (auto &item : r) {
        item = 0;
    }

    for (hilet &info : iso_15924_infos) {
        r[to_underlying(info.unicode_script)] = info.number;
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
constexpr auto iso_15924_number_by_unicode_script = iso_15924_number_by_unicode_script_init();
constexpr auto iso_15924_number_by_code4 = iso_15924_number_by_code4_init();

iso_15924::iso_15924(hi::unicode_script const &script) noexcept : _v(iso_15924_number_by_unicode_script[to_underlying(script)]) {}

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

[[nodiscard]] unicode_bidi_class iso_15924::writing_direction() const noexcept
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
        return unicode_bidi_class::R;
    default:
        return unicode_bidi_class::L;
    }
}

} // namespace hi::inline v1
