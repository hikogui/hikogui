
#include "pch.h"

#include <TTauri/strings.hpp>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>

using namespace std;

BOOST_AUTO_TEST_SUITE(TTauriString);

BOOST_AUTO_TEST_CASE(HelloWorldUTF8toUTF32)
{
    BOOST_CHECK(TTauri::translateString<std::u32string>(u8"Hello World"s) == U"Hello World"s);
}

BOOST_AUTO_TEST_CASE(HelloWorldUTF16toUTF32)
{
    BOOST_CHECK(TTauri::translateString<std::u32string>(u"Hello World"s) == U"Hello World"s);
}

BOOST_AUTO_TEST_CASE(HelloWorldUTF8toUTF16)
{
    BOOST_CHECK(TTauri::translateString<std::u16string>(u8"Hello World"s) == u"Hello World"s);
}

BOOST_AUTO_TEST_CASE(HelloWorldUTF32toUTF8)
{
    BOOST_CHECK(TTauri::translateString<std::string>(U"Hello World"s) == u8"Hello World"s);
}

BOOST_AUTO_TEST_CASE(HelloWorldUTF32toUTF16)
{
    BOOST_CHECK(TTauri::translateString<std::u16string>(U"Hello World"s) == u"Hello World"s);
}

BOOST_AUTO_TEST_CASE(HelloWorldUTF16toUTF8)
{
    BOOST_CHECK(TTauri::translateString<std::string>(u"Hello World"s) == u8"Hello World"s);
}

BOOST_AUTO_TEST_SUITE_END();
