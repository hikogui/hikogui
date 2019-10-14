// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/strings.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(TTauriString, Split) {
    let result = split(std::string{"path1/path2"}, '/');
    let check_value = std::vector<std::string>{"path1", "path2"};
    ASSERT_EQ(result, check_value);
}

TEST(TTauriString, UTF8toUTF32) {
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"Hello World"s), U"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\u0001"s), U"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\u0001_"s), U"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\u0011"s), U"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\u0011_"s), U"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\u0111"s), U"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\u0111_"s), U"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\u1111"s), U"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\u1111_"s), U"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\U00011111"s), U"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\U00011111_"s), U"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"\U00101111"s), U"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u8"_\U00101111_"s), U"_\U00101111_"s);
}

TEST(TTauriString, UTF16toUTF32) {
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"Hello World"s), U"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\u0001"s), U"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\u0001_"s), U"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\u0011"s), U"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\u0011_"s), U"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\u0111"s), U"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\u0111_"s), U"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\u1111"s), U"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\u1111_"s), U"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\U00011111"s), U"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\U00011111_"s), U"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"\U00101111"s), U"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::u32string>(u"_\U00101111_"s), U"_\U00101111_"s);
}

TEST(TTauriString, UTF8toUTF16) {
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"Hello World"s), u"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\u0001"s), u"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\u0001_"s), u"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\u0011"s), u"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\u0011_"s), u"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\u0111"s), u"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\u0111_"s), u"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\u1111"s), u"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\u1111_"s), u"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\U00011111"s), u"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\U00011111_"s), u"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"\U00101111"s), u"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(u8"_\U00101111_"s), u"_\U00101111_"s);
}

TEST(TTauriString, UTF32toUTF8) {
    ASSERT_EQ(TTauri::translateString<std::string>(U"Hello World"s), u8"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::string>(U"\u0001"s), u8"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\u0001_"s), u8"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"\u0011"s), u8"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\u0011_"s), u8"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"\u0111"s), u8"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\u0111_"s), u8"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"\u1111"s), u8"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\u1111_"s), u8"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"\U00011111"s), u8"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\U00011111_"s), u8"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"\U00101111"s), u8"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(U"_\U00101111_"s), u8"_\U00101111_"s);
}

TEST(TTauriString, UTF32toUTF16) {
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"Hello World"s), u"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\u0001"s), u"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\u0001_"s), u"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\u0011"s), u"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\u0011_"s), u"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\u0111"s), u"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\u0111_"s), u"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\u1111"s), u"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\u1111_"s), u"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\U00011111"s), u"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\U00011111_"s), u"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"\U00101111"s), u"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::u16string>(U"_\U00101111_"s), u"_\U00101111_"s);
}

TEST(TTauriString, UTF16toUTF8) {
    ASSERT_EQ(TTauri::translateString<std::string>(u"Hello World"s), u8"Hello World"s);

    ASSERT_EQ(TTauri::translateString<std::string>(u"\u0001"s), u8"\u0001"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\u0001_"s), u8"_\u0001_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"\u0011"s), u8"\u0011"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\u0011_"s), u8"_\u0011_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"\u0111"s), u8"\u0111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\u0111_"s), u8"_\u0111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"\u1111"s), u8"\u1111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\u1111_"s), u8"_\u1111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"\U00011111"s), u8"\U00011111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\U00011111_"s), u8"_\U00011111_"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"\U00101111"s), u8"\U00101111"s);
    ASSERT_EQ(TTauri::translateString<std::string>(u"_\U00101111_"s), u8"_\U00101111_"s);
}
