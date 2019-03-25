
#include "pch.h"

#include <TTauri/strings.hpp>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>

using namespace std;

BOOST_AUTO_TEST_SUITE(TTauriString);

BOOST_AUTO_TEST_CASE(UTF8toUTF32)
{
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"Hello World"s) == U"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\u0001"s) == U"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\u0001_"s) == U"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\u0011"s) == U"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\u0011_"s) == U"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\u0111"s) == U"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\u0111_"s) == U"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\u1111"s) == U"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\u1111_"s) == U"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\U011111"s) == U"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\U011111_"s) == U"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"\U111111"s) == U"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"_\U111111_"s) == U"_\U111111_"s);
}

BOOST_AUTO_TEST_CASE(UTF16toUTF32)
{
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"Hello World"s) == U"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\u0001"s) == U"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\u0001_"s) == U"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\u0011"s) == U"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\u0011_"s) == U"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\u0111"s) == U"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\u0111_"s) == U"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\u1111"s) == U"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\u1111_"s) == U"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\U011111"s) == U"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\U011111_"s) == U"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"\U111111"s) == U"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"_\U111111_"s) == U"_\U111111_"s);
}

BOOST_AUTO_TEST_CASE(UTF8toUTF16)
{
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"Hello World"s) == u"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\u0001"s) == u"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\u0001_"s) == u"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\u0011"s) == u"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\u0011_"s) == u"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\u0111"s) == u"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\u0111_"s) == u"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\u1111"s) == u"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\u1111_"s) == u"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\U011111"s) == u"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\U011111_"s) == u"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"\U111111"s) == u"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"_\U111111_"s) == u"_\U111111_"s);
}

BOOST_AUTO_TEST_CASE(UTF32toUTF8)
{
    BOOST_CHECK(TTauri::translateString<std::string>(U"Hello World"s) == u8"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::string>(U"\u0001"s) == u8"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\u0001_"s) == u8"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"\u0011"s) == u8"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\u0011_"s) == u8"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"\u0111"s) == u8"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\u0111_"s) == u8"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"\u1111"s) == u8"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\u1111_"s) == u8"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"\U011111"s) == u8"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\U011111_"s) == u8"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"\U111111"s) == u8"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(U"_\U111111_"s) == u8"_\U111111_"s);
}

BOOST_AUTO_TEST_CASE(UTF32toUTF16)
{
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"Hello World"s) == u"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\u0001"s) == u"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\u0001_"s) == u"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\u0011"s) == u"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\u0011_"s) == u"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\u0111"s) == u"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\u0111_"s) == u"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\u1111"s) == u"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\u1111_"s) == u"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\U011111"s) == u"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\U011111_"s) == u"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"\U111111"s) == u"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"_\U111111_"s) == u"_\U111111_"s);
}

BOOST_AUTO_TEST_CASE(UTF16toUTF8)
{
    BOOST_CHECK(TTauri::translateString<std::string>(u"Hello World"s) == u8"Hello World"s);

    BOOST_CHECK(TTauri::translateString<std::string>(u"\u0001"s) == u8"\u0001"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\u0001_"s) == u8"_\u0001_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"\u0011"s) == u8"\u0011"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\u0011_"s) == u8"_\u0011_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"\u0111"s) == u8"\u0111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\u0111_"s) == u8"_\u0111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"\u1111"s) == u8"\u1111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\u1111_"s) == u8"_\u1111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"\U011111"s) == u8"\U011111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\U011111_"s) == u8"_\U011111_"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"\U111111"s) == u8"\U111111"s);
    BOOST_CHECK(TTauri::translateString<std::string>(u"_\U111111_"s) == u8"_\U111111_"s);
}

BOOST_AUTO_TEST_SUITE_END();
