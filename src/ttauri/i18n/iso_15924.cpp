// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_15924.hpp"
#include "../unicode/unicode_script.hpp"
#include <array>

namespace tt::inline v1 {

struct iso_15924_info {
    char const *code;
    char const *open_type;
    tt::unicode_script unicode_script;
    uint16_t nr;

    constexpr iso_15924_info() noexcept : code("Zzzz"), open_type("zzzz"), unicode_script(unicode_script::Unknown), nr(999) {}

    constexpr iso_15924_info(char const *code, char const *open_type, tt::unicode_script unicode_script, uint16_t nr) noexcept :
        code(code), open_type(open_type), unicode_script(unicode_script), nr(nr)
    {
    }

    constexpr iso_15924_info(char const *code, tt::unicode_script unicode_script, uint16_t nr) noexcept :
        iso_15924_info(code, code, unicode_script, nr)
    {
    }
};

using enum unicode_script;

// Byzantine Music 'byzm'
// Mathematical Alphanumeric Symbols 'math'
// Musical Symbols 'musc'
constexpr std::array iso_15924_by_code = {
    iso_15924_info{"Adlm", Adlam, 166},
    iso_15924_info{"Afak", Afaka, 439},
    iso_15924_info{"Aghb", Caucasian_Albanian, 239},
    iso_15924_info{"Ahom", Ahom, 338},
    iso_15924_info{"Arab", Arabic, 160},
    iso_15924_info{"Aran", Arabic, 161},
    iso_15924_info{"Armi", Imperial_Aramaic, 124},
    iso_15924_info{"Armn", Armenian, 230},
    iso_15924_info{"Avst", Avestan, 134},
    iso_15924_info{"Bali", Balinese, 360},
    iso_15924_info{"Bamu", Bamum, 435},
    iso_15924_info{"Bass", Bassa_Vah, 259},
    iso_15924_info{"Batk", Batak, 365},
    iso_15924_info{"Beng", "bng2", Bengali, 325},
    iso_15924_info{"Bhks", Bhaiksuki, 334},
    iso_15924_info{"Blis", Blissymbols, 550},
    iso_15924_info{"Bopo", Bopomofo, 285},
    iso_15924_info{"Brah", Brahmi, 300},
    iso_15924_info{"Brai", Braille, 570},
    iso_15924_info{"Bugi", Buginese, 367},
    iso_15924_info{"Buhd", Buhid, 372},
    iso_15924_info{"Cakm", Chakma, 349},
    iso_15924_info{"Cans", Canadian_Aboriginal, 440},
    iso_15924_info{"Cari", Carian, 201},
    iso_15924_info{"Cham", Cham, 358},
    iso_15924_info{"Cher", Cherokee, 445},
    iso_15924_info{"Chrs", Chorasmian, 109},
    iso_15924_info{"Cirt", Cirth, 291},
    iso_15924_info{"Copt", Coptic, 204},
    iso_15924_info{"Cpmn", Cypro_Minoan, 402},
    iso_15924_info{"Cprt", Cypriot, 403},
    iso_15924_info{"Cyrl", Cyrillic, 220},
    iso_15924_info{"Cyrs", Cyrillic, 221},
    iso_15924_info{"Deva", "dev2", Devanagari, 315},
    iso_15924_info{"Diak", Dives_Akuru, 342},
    iso_15924_info{"Dogr", Dogra, 328},
    iso_15924_info{"Dsrt", Deseret, 250},
    iso_15924_info{"Dupl", Duployan, 755},
    iso_15924_info{"Egyd", Egyptian_Demotic, 70},
    iso_15924_info{"Egyh", Egyptian_Hieratic, 60},
    iso_15924_info{"Egyp", Egyptian_Hieroglyphs, 50},
    iso_15924_info{"Elba", Elbasan, 226},
    iso_15924_info{"Elym", Elymaic, 128},
    iso_15924_info{"Ethi", Ethiopic, 430},
    iso_15924_info{"Geok", Khutsuri, 241},
    iso_15924_info{"Geor", Georgian, 240},
    iso_15924_info{"Glag", Glagolitic, 225},
    iso_15924_info{"Gong", Gunjala_Gondi, 312},
    iso_15924_info{"Gonm", Masaram_Gondi, 313},
    iso_15924_info{"Goth", Gothic, 206},
    iso_15924_info{"Gran", Grantha, 343},
    iso_15924_info{"Grek", Greek, 200},
    iso_15924_info{"Gujr", "gjr2", Gujarati, 320},
    iso_15924_info{"Guru", "gur2", Gurmukhi, 310},
    iso_15924_info{"Hanb", Han, 503},
    iso_15924_info{"Hang", Hangul, 286},
    iso_15924_info{"Hani", Han, 500},
    iso_15924_info{"Hano", Hanunoo, 371},
    iso_15924_info{"Hans", Han, 501},
    iso_15924_info{"Hant", Han, 502},
    iso_15924_info{"Hatr", Hatran, 127},
    iso_15924_info{"Hebr", Hebrew, 125},
    iso_15924_info{"Hira", Hiragana, 410},
    iso_15924_info{"Hluw", Anatolian_Hieroglyphs, 80},
    iso_15924_info{"Hmng", Pahawh_Hmong, 450},
    iso_15924_info{"Hmnp", Nyiakeng_Puachue_Hmong, 451},
    iso_15924_info{"Hrkt", Hiragana, 412},
    iso_15924_info{"Hung", Old_Hungarian, 176},
    iso_15924_info{"Inds", Indus, 610},
    iso_15924_info{"Ital", Old_Italic, 210},
    iso_15924_info{"Jamo", Hangul, 284},
    iso_15924_info{"Java", Javanese, 361},
    iso_15924_info{"Jpan", Han, 413},
    iso_15924_info{"Jurc", Jurchen, 510},
    iso_15924_info{"Kali", Kayah_Li, 357},
    iso_15924_info{"Kana", Katakana, 411},
    iso_15924_info{"Khar", Kharoshthi, 305},
    iso_15924_info{"Khmr", Khmer, 355},
    iso_15924_info{"Khoj", Khojki, 322},
    iso_15924_info{"Kitl", Khitan_Large_Script, 505},
    iso_15924_info{"Kits", Khitan_Small_Script, 288},
    iso_15924_info{"Knda", "knd2", Kannada, 345},
    iso_15924_info{"Kore", Hangul, 287},
    iso_15924_info{"Kpel", Kpelle, 436},
    iso_15924_info{"Kthi", Kaithi, 317},
    iso_15924_info{"Lana", Tai_Tham, 351},
    iso_15924_info{"Laoo", "lao ", Lao, 356},
    iso_15924_info{"Latf", Latin, 217},
    iso_15924_info{"Latg", Latin, 216},
    iso_15924_info{"Latn", Latin, 215},
    iso_15924_info{"Leke", Leke, 364},
    iso_15924_info{"Lepc", Lepcha, 335},
    iso_15924_info{"Limb", Limbu, 336},
    iso_15924_info{"Lina", Linear_A, 400},
    iso_15924_info{"Linb", Linear_B, 401},
    iso_15924_info{"Lisu", Lisu, 399},
    iso_15924_info{"Loma", Loma, 437},
    iso_15924_info{"Lyci", Lycian, 202},
    iso_15924_info{"Lydi", Lydian, 116},
    iso_15924_info{"Mahj", Mahajani, 314},
    iso_15924_info{"Maka", Makasar, 366},
    iso_15924_info{"Mand", Mandaic, 140},
    iso_15924_info{"Mani", Manichaean, 139},
    iso_15924_info{"Marc", Marchen, 332},
    iso_15924_info{"Maya", Mayan_Hieroglyphs, 90},
    iso_15924_info{"Medf", Medefaidrin, 265},
    iso_15924_info{"Mend", Mende_Kikakui, 438},
    iso_15924_info{"Merc", Meroitic_Cursive, 101},
    iso_15924_info{"Mero", Meroitic_Hieroglyphs, 100},
    iso_15924_info{"Mlym", "mlm2", Malayalam, 347},
    iso_15924_info{"Modi", Modi, 324},
    iso_15924_info{"Mong", Mongolian, 145},
    iso_15924_info{"Moon", Moon, 218},
    iso_15924_info{"Mroo", Mro, 264},
    iso_15924_info{"Mtei", Meetei_Mayek, 337},
    iso_15924_info{"Mult", Multani, 323},
    iso_15924_info{"Mymr", "mym2", Myanmar, 350},
    iso_15924_info{"Nand", Nandinagari, 311},
    iso_15924_info{"Narb", Old_North_Arabian, 106},
    iso_15924_info{"Nbat", Nabataean, 159},
    iso_15924_info{"Newa", Newa, 333},
    iso_15924_info{"Nkdb", Naxi_Dongba, 85},
    iso_15924_info{"Nkgb", Nakhi_Geba, 420},
    iso_15924_info{"Nkoo", "nko ", Nko, 165},
    iso_15924_info{"Nshu", Nushu, 499},
    iso_15924_info{"Ogam", Ogham, 212},
    iso_15924_info{"Olck", Ol_Chiki, 261},
    iso_15924_info{"Orkh", Old_Turkic, 175},
    iso_15924_info{"Orya", "ory2", Oriya, 327},
    iso_15924_info{"Osge", Osage, 219},
    iso_15924_info{"Osma", Osmanya, 260},
    iso_15924_info{"Ougr", Old_Uyghur, 143},
    iso_15924_info{"Palm", Palmyrene, 126},
    iso_15924_info{"Pauc", Pau_Cin_Hau, 263},
    iso_15924_info{"Pcun", Proto_Cuneiform, 15},
    iso_15924_info{"Pelm", Proto_Elamite, 16},
    iso_15924_info{"Perm", Old_Permic, 227},
    iso_15924_info{"Phag", Phags_Pa, 331},
    iso_15924_info{"Phli", Inscriptional_Pahlavi, 131},
    iso_15924_info{"Phlp", Psalter_Pahlavi, 132},
    iso_15924_info{"Phlv", Book_Pahlavi, 133},
    iso_15924_info{"Phnx", Phoenician, 115},
    iso_15924_info{"Plrd", Miao, 282},
    iso_15924_info{"Piqd", Kligon, 293},
    iso_15924_info{"Prti", Inscriptional_Parthian, 130},
    iso_15924_info{"Psin", Proto_Sinaitic, 103},
    iso_15924_info{"Qaaa", Private_Use, 900},
    iso_15924_info{"Qabx", Private_Use, 949},
    iso_15924_info{"Ranj", Ranjana, 303},
    iso_15924_info{"Rjng", Rejang, 363},
    iso_15924_info{"Rohg", Hanifi_Rohingya, 167},
    iso_15924_info{"Roro", Rongorongo, 620},
    iso_15924_info{"Runr", Runic, 211},
    iso_15924_info{"Samr", Samaritan, 123},
    iso_15924_info{"Sara", Sarati, 292},
    iso_15924_info{"Sarb", Old_South_Arabian, 105},
    iso_15924_info{"Saur", Saurashtra, 344},
    iso_15924_info{"Sgnw", SignWriting, 95},
    iso_15924_info{"Shaw", Shavian, 281},
    iso_15924_info{"Shrd", Sharada, 319},
    iso_15924_info{"Shui", Shuishu, 530},
    iso_15924_info{"Sidd", Siddham, 302},
    iso_15924_info{"Sind", Khudawadi, 318},
    iso_15924_info{"Sinh", Sinhala, 348},
    iso_15924_info{"Sogd", Sogdian, 141},
    iso_15924_info{"Sogo", Old_Sogdian, 142},
    iso_15924_info{"Sora", Sora_Sompeng, 398},
    iso_15924_info{"Soyo", Soyombo, 329},
    iso_15924_info{"Sund", Sundanese, 362},
    iso_15924_info{"Sylo", Syloti_Nagri, 316},
    iso_15924_info{"Syrc", Syriac, 135},
    iso_15924_info{"Syre", Syriac, 138},
    iso_15924_info{"Syrj", Syriac, 137},
    iso_15924_info{"Syrn", Syriac, 136},
    iso_15924_info{"Tagb", Tagbanwa, 373},
    iso_15924_info{"Takr", Takri, 321},
    iso_15924_info{"Tale", Tai_Le, 353},
    iso_15924_info{"Talu", New_Tai_Lue, 354},
    iso_15924_info{"Taml", "tml2", Tamil, 346},
    iso_15924_info{"Tang", Tangsa, 520},
    iso_15924_info{"Tavt", Tai_Viet, 359},
    iso_15924_info{"Telu", "tel2", Telugu, 340},
    iso_15924_info{"Teng", Tengwar, 290},
    iso_15924_info{"Tfng", Tifinagh, 120},
    iso_15924_info{"Tglg", Tagalog, 370},
    iso_15924_info{"Thaa", Thaana, 170},
    iso_15924_info{"Thai", Thai, 352},
    iso_15924_info{"Tibt", Tibetan, 330},
    iso_15924_info{"Tirh", Tirhuta, 326},
    iso_15924_info{"Tnsa", Tangsa, 275},
    iso_15924_info{"Toto", Toto, 294},
    iso_15924_info{"Ugar", Ugaritic, 40},
    iso_15924_info{"Vaii", "vai ", Vai, 470},
    iso_15924_info{"Visp", Visible_Speech, 280},
    iso_15924_info{"Vith", Vithkuqi, 228},
    iso_15924_info{"Wara", Warang_Citi, 262},
    iso_15924_info{"Wcho", Wancho, 283},
    iso_15924_info{"Wole", Woleai, 480},
    iso_15924_info{"Xpeo", Old_Persian, 30},
    iso_15924_info{"Xsux", Cuneiform, 20},
    iso_15924_info{"Yezi", Yezidi, 192},
    iso_15924_info{"Yiii", "yi  ", Yi, 460},
    iso_15924_info{"Zanb", Zanabazar_Square, 339},
    iso_15924_info{"Zinh", Inherited, 994},
    iso_15924_info{"Zmth", "math", Common, 995},
    iso_15924_info{"Zsye", Common, 993},
    iso_15924_info{"Zsym", Common, 996},
    iso_15924_info{"Zxxx", Unknown, 997},
    iso_15924_info{"Zyyy", "DFLT", Common, 998},
    iso_15924_info{"Zzzz", Unknown, 999}};

constexpr auto iso_15924_table_by_nr_init() noexcept
{
    auto r = std::array<iso_15924_info, 1000>{};

    for (ttlet &info : iso_15924_by_code) {
        r[info.nr] = info;
    }

    return r;
}

constexpr auto iso_15924_table_by_nr = iso_15924_table_by_nr_init();

// iso_15924(unicode_script const &script) noexcept {}

[[nodiscard]] char const *iso_15924::code() const noexcept
{
    tt_axiom(_v < 1000);
    return iso_15924_table_by_nr[_v].code;
}

[[nodiscard]] char const *iso_15924::open_type() const noexcept
{
    tt_axiom(_v < 1000);
    return iso_15924_table_by_nr[_v].open_type;
}

} // namespace tt::inline v1
