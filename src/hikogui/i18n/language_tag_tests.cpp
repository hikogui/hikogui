// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language_tag.hpp"
#include "../macros.hpp"
#include <format>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

using tag = hi::language_tag;
using namespace std::literals;

hi_warning_push() hi_warning_ignore_msvc(4834)

TEST(language_tag, parse)
{
    ASSERT_EQ(to_string(tag::parse("")), "");

    ASSERT_EQ(to_string(tag::parse("nl")), "nl");
    ASSERT_EQ(to_string(tag::parse("nl-NL")), "nl-NL");
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-NL")), "nl-Cyrl-NL");
    ASSERT_EQ(to_string(tag::parse("nl-BE")), "nl-BE");
    ASSERT_EQ(to_string(tag::parse("NL-be")), "nl-BE");
    ASSERT_EQ(to_string(tag::parse("nl-56")), "nl-BE");
    ASSERT_EQ(to_string(tag::parse("nl-056")), "nl-BE");
    // "foo" language extension is ignored.
    ASSERT_EQ(to_string(tag::parse("nl-foo-056")), "nl-BE");
    // "foo" language is accepted even though it doesn't exist and has no default script.
    ASSERT_EQ(to_string(tag::parse("foo-056")), "foo-BE");
    // "abcde" variant is ignored.
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-NL-abcde")), "nl-Cyrl-NL");
    // "a-bcde" extension is ignored.
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-NL-a-bcde")), "nl-Cyrl-NL");
    // "x-bcde" user-defined extension is ignored.
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-NL-x-bcde")), "nl-Cyrl-NL");

    tag tmp;
    ASSERT_THROW(tmp = tag::parse("x-NL"), hi::parse_error);
    ASSERT_THROW(tmp = tag::parse("xxxx-NL"), hi::parse_error);
    // "Food" script does not exist.
    ASSERT_THROW(tmp = tag::parse("nl-Food-NL"), hi::parse_error);
    // Region "AA" does not exist.
    ASSERT_THROW(tmp = tag::parse("nl-Latn-AA"), hi::parse_error);
}
TEST(language_tag, construct)
{
    ASSERT_EQ(to_string(tag{""}), "");

    ASSERT_EQ(to_string(tag{"nl"}), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag{"nl-NL"}), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag{"nl-Cyrl-NL"}), "nl-Cyrl-NL");
    ASSERT_EQ(to_string(tag{"nl-BE"}), "nl-Latn-BE");
    ASSERT_EQ(to_string(tag{"NL-be"}), "nl-Latn-BE");
    ASSERT_EQ(to_string(tag{"nl-56"}), "nl-Latn-BE");
    ASSERT_EQ(to_string(tag{"nl-056"}), "nl-Latn-BE");
    // "foo" language extension is ignored.
    ASSERT_EQ(to_string(tag{"nl-foo-056"}), "nl-Latn-BE");
    // "foo" language is accepted even though it doesn't exist and has no default script.
    ASSERT_EQ(to_string(tag{"foo-056"}), "foo-BE");
    // "abcde" variant is ignored.
    ASSERT_EQ(to_string(tag{"nl-Cyrl-NL-abcde"}), "nl-Cyrl-NL");
    // "a-bcde" extension is ignored.
    ASSERT_EQ(to_string(tag{"nl-Cyrl-NL-a-bcde"}), "nl-Cyrl-NL");
    // "x-bcde" user-defined extension is ignored.
    ASSERT_EQ(to_string(tag{"nl-Cyrl-NL-x-bcde"}), "nl-Cyrl-NL");

    ASSERT_THROW(tag{"x-NL"}, hi::parse_error);
    ASSERT_THROW(tag{"xxxx-NL"}, hi::parse_error);
    // "Food" script does not exist.
    ASSERT_THROW(tag{"nl-Food-NL"}, hi::parse_error);
    // Region "AA" does not exist.
    ASSERT_THROW(tag{"nl-Latn-AA"}, hi::parse_error);
}

TEST(language_tag, shrink)
{
    ASSERT_EQ(to_string(tag{"nl"}.shrink()), "nl");
    ASSERT_EQ(to_string(tag{"nl-NL"}.shrink()), "nl");
    ASSERT_EQ(to_string(tag{"nl-Latn"}.shrink()), "nl");
    ASSERT_EQ(to_string(tag{"nl-Latn-NL"}.shrink()), "nl");
    ASSERT_EQ(to_string(tag{"nl-BE"}.shrink()), "nl-BE");
    ASSERT_EQ(to_string(tag{"nl-Latn-BE"}.shrink()), "nl-BE");

    ASSERT_EQ(to_string(tag{"nl-Cyrl"}.shrink()), "nl-Cyrl");
    ASSERT_EQ(to_string(tag{"nl-Cyrl-NL"}.shrink()), "nl-Cyrl");
    ASSERT_EQ(to_string(tag{"nl-Cyrl-BE"}.shrink()), "nl-Cyrl-BE");
}

TEST(language_tag, expand)
{
    ASSERT_EQ(to_string(tag::parse("nl").expand()), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag::parse("nl-NL").expand()), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag::parse("nl-Latn").expand()), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag::parse("nl-Latn-NL").expand()), "nl-Latn-NL");
    ASSERT_EQ(to_string(tag::parse("nl-BE").expand()), "nl-Latn-BE");
    ASSERT_EQ(to_string(tag::parse("nl-Latn-BE").expand()), "nl-Latn-BE");

    ASSERT_EQ(to_string(tag::parse("nl-Cyrl").expand()), "nl-Cyrl-NL");
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-NL").expand()), "nl-Cyrl-NL");
    ASSERT_EQ(to_string(tag::parse("nl-Cyrl-BE").expand()), "nl-Cyrl-BE");

    ASSERT_EQ(to_string(tag::parse("en").expand()), "en-Latn-US");
    ASSERT_EQ(to_string(tag::parse("en-US").expand()), "en-Latn-US");
    ASSERT_EQ(to_string(tag::parse("en-Latn").expand()), "en-Latn-US");
    ASSERT_EQ(to_string(tag::parse("en-Latn-US").expand()), "en-Latn-US");
    ASSERT_EQ(to_string(tag::parse("en-GB").expand()), "en-Latn-GB");
    ASSERT_EQ(to_string(tag::parse("en-Latn-GB").expand()), "en-Latn-GB");

    ASSERT_EQ(to_string(tag::parse("en-Cyrl").expand()), "en-Cyrl-US");
    ASSERT_EQ(to_string(tag::parse("en-Cyrl-GB").expand()), "en-Cyrl-GB");
}

TEST(language_tag, variants)
{
    auto nl_Latn_NL_expected =
        std::vector<tag>{tag::parse("nl-Latn-NL"), tag::parse("nl-NL"), tag::parse("nl-Latn"), tag::parse("nl")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl-Latn-NL").variants()), nl_Latn_NL_expected);

    auto nl_NL_expected = std::vector<tag>{tag::parse("nl-NL"), tag::parse("nl")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl-NL").variants()), nl_NL_expected);

    auto nl_Latn_expected = std::vector<tag>{tag::parse("nl-Latn"), tag::parse("nl")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl-Latn").variants()), nl_Latn_expected);

    auto nl_expected = std::vector<tag>{tag::parse("nl")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl").variants()), nl_expected);
}

TEST(language_tag, canonical_variants)
{
    auto nl_Latn_NL_expected =
        std::vector<tag>{tag::parse("nl-Latn-NL"), tag::parse("nl-NL"), tag::parse("nl-Latn"), tag::parse("nl")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl-Latn-NL").canonical_variants()), nl_Latn_NL_expected);

    auto nl_Latn_BE_expected = std::vector<tag>{tag::parse("nl-Latn-BE"), tag::parse("nl-BE")};
    ASSERT_EQ(hi::make_vector(tag::parse("nl-Latn-BE").canonical_variants()), nl_Latn_BE_expected);
}

TEST(language_tag, all_variants)
{
    auto nl_Latn_NL_expected =
        std::vector<tag>{tag::parse("nl-Latn-NL"), tag::parse("nl-NL"), tag::parse("nl-Latn"), tag::parse("nl")};
    ASSERT_EQ(tag::parse("nl-Latn-NL").all_variants(), nl_Latn_NL_expected);

    auto nl_Latn_BE_expected = std::vector<tag>{
        tag::parse("nl-Latn-BE"),
        tag::parse("nl-BE"),
        tag::parse("nl-Latn"),
        tag::parse("nl"),
        tag::parse("nl-Latn-NL"),
        tag::parse("nl-NL")};
    ASSERT_EQ(tag::parse("nl-Latn-BE").all_variants(), nl_Latn_BE_expected);
}

TEST(language_tag, variant_vector)
{
    // Language order often used in the Netherlands:
    // - English with the Netherlands' local
    // - Fallback 1: English as spoken in Great-Britain.
    // - Fallback 2: Dutch as spoken in the Netherlands.
    auto test = std::vector<tag>{tag::parse("en-Latn-NL"), tag::parse("en-Latn-GB"), tag::parse("nl-Latn-NL")};

    // In the result we also automatically add a fallback to English as spoken in the United States, after
    // all the originally specified English tags.
    auto expected = std::vector<tag>{
        tag::parse("en-Latn-NL"),
        tag::parse("en-NL"),
        tag::parse("en-Latn-GB"),
        tag::parse("en-GB"),
        tag::parse("en-Latn"),
        tag::parse("en"),
        tag::parse("en-Latn-US"),
        tag::parse("en-US"),
        tag::parse("nl-Latn-NL"),
        tag::parse("nl-NL"),
        tag::parse("nl-Latn"),
        tag::parse("nl")};
    ASSERT_EQ(variants(test), expected);
}

hi_warning_pop()
