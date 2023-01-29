// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "datum.hpp"
#include "utility/module.hpp"
#include "codec/JSON.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace hi;

TEST(datum, IntOperations)
{
    hilet v = datum{42};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42"s);
    ASSERT_EQ(to_bool(v), true);

    ASSERT_EQ(holds_alternative<long long>(v), true);
    ASSERT_EQ(holds_alternative<double>(v), false);
    ASSERT_EQ(holds_alternative<decimal>(v), false);
    ASSERT_EQ(holds_alternative<std::string>(v), false);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);
    ASSERT_EQ(v - 5, 37);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);

    hilet a = v + 3;
    ASSERT_EQ(holds_alternative<long long>(a), true);
    ASSERT_EQ(a == 45, true);

    hilet b = v + 3.0;
    ASSERT_EQ(holds_alternative<double>(b), true);
    ASSERT_EQ(b == 45.0, true);

    ASSERT_THROW((void)(datum(-42) >> -1), std::domain_error);
    ASSERT_THROW((void)(datum(42) >> -1), std::domain_error);

    ASSERT_EQ(datum(42) << 0, 42);
    ASSERT_EQ(datum(42) >> 0, 42);
    ASSERT_EQ(datum(42) << 1, 84);
    ASSERT_EQ(datum(-42) >> 1, -21);
    ASSERT_EQ(datum(-42) << 1, -84);

    ASSERT_EQ(datum(42) << 63, 0);
    ASSERT_EQ(datum(42) >> 63, 0);
    ASSERT_EQ(datum(-42) >> 63, -1);
    ASSERT_THROW((void)(datum(42) << 64), std::domain_error);
    ASSERT_THROW((void)(datum(42) >> 64), std::domain_error);
    ASSERT_THROW((void)(datum(-42) >> 64), std::domain_error);
}

TEST(datum, DecimalOperations)
{
    hilet v = decimal(-25);
    ASSERT_EQ(static_cast<decimal>(datum{v}), v);
}

TEST(datum, NegativeIntOperations)
{
    hilet v = datum{-1};

    ASSERT_EQ(static_cast<int>(v), -1);
    ASSERT_EQ(static_cast<std::string>(v), "-1"s);
}

TEST(datum, FloatOperations)
{
    hilet v = datum{42.0};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42"s);
    ASSERT_EQ(to_string(v), "42"s);
    ASSERT_EQ(std::format("{}", v), "42"s);
    ASSERT_EQ(repr(v), "42.0"s);
    ASSERT_EQ(to_bool(v), true);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);

    hilet a = v + 3;
    ASSERT_EQ(holds_alternative<double>(a), true);
    ASSERT_EQ(a == 45.0, true);

    hilet b = v + 3.0;
    ASSERT_EQ(holds_alternative<double>(b), true);
    ASSERT_EQ(b == 45.0, true);
}

TEST(datum, StringOperations)
{
    hilet v = datum{"Hello World"};

    ASSERT_EQ(static_cast<std::string>(v), "Hello World"s);
}

TEST(datum, ArrayOperations)
{
    hilet v = datum::make_vector(11, 12, 13, 14, 15);

    ASSERT_EQ(v[0], 11);
    ASSERT_EQ(v[1], 12);
    ASSERT_EQ(v[2], 13);
    ASSERT_EQ(v[3], 14);
    ASSERT_EQ(v[4], 15);
    ASSERT_THROW((void)(v[5]), std::overflow_error);

    ASSERT_THROW((void)(v[-6]), std::overflow_error);
    ASSERT_EQ(v[-5], 11);
    ASSERT_EQ(v[-4], 12);
    ASSERT_EQ(v[-3], 13);
    ASSERT_EQ(v[-2], 14);
    ASSERT_EQ(v[-1], 15);
}

static datum bookstore = parse_JSON(
    "{\n"
    "    \"store\" : {\n"
    "        \"book\" : [\n"
    "            {\n"
    "                \"category\" : \"reference\",\n"
    "                 \"author\" : \"Nigel Rees\",\n"
    "                 \"title\" : \"Sayings of the Century\",\n"
    "                 \"price\" : 8.95\n"
    "            }, {\n"
    "                 \"category\" : \"fiction\",\n"
    "                 \"author\" : \"Evelyn Waugh\",\n"
    "                 \"title\" : \"Sword of Honour\",\n"
    "                 \"price\" : 12.99\n"
    "            }, {\n"
    "                \"category\" : \"fiction\",\n"
    "                \"author\" : \"Herman Melville\",\n"
    "                \"title\" : \"Moby Dick\",\n"
    "                \"isbn\" : \"0-553-21311-3\",\n"
    "                \"price\" : 8.99\n"
    "            }, {\n"
    "                \"category\" : \"fiction\",\n"
    "                \"author\" : \"J. R. R. Tolkien\",\n"
    "                \"title\" : \"The Lord of the Rings\",\n"
    "                \"isbn\" : \"0-395-19395-8\",\n"
    "                \"price\" : 22.99\n"
    "            }\n"
    "        ],\n"
    "        \"bicycle\" : {\n"
    "            \"color\" : \"red\",\n"
    "            \"price\" : 19.95\n"
    "        }\n"
    "    }\n"
    "}\n");

TEST(datum, find)
{
    auto authors1 = bookstore.find(jsonpath("$.store.book[*].author"));
    ASSERT_EQ(size(authors1), 4);
    ASSERT_EQ(*(authors1[0]), "Nigel Rees");
    ASSERT_EQ(*(authors1[1]), "Evelyn Waugh");
    ASSERT_EQ(*(authors1[2]), "Herman Melville");
    ASSERT_EQ(*(authors1[3]), "J. R. R. Tolkien");

    auto authors2 = bookstore.find(jsonpath("$..author"));
    ASSERT_EQ(size(authors2), 4);
    ASSERT_EQ(*(authors2[0]), "Nigel Rees");
    ASSERT_EQ(*(authors2[1]), "Evelyn Waugh");
    ASSERT_EQ(*(authors2[2]), "Herman Melville");
    ASSERT_EQ(*(authors2[3]), "J. R. R. Tolkien");

    auto things = bookstore.find(jsonpath("$.store.*"));
    ASSERT_EQ(size(things), 2);
    ASSERT_EQ(size(*(things[0])), 2); // attributes of bicycle
    ASSERT_EQ(size(*(things[1])), 4); // list of books

    auto prices = bookstore.find(jsonpath("$.store..price"));
    ASSERT_EQ(size(prices), 5);
    ASSERT_EQ(*(prices[0]), 19.95); // bicycle first
    ASSERT_EQ(*(prices[1]), 8.95);
    ASSERT_EQ(*(prices[2]), 12.99);
    ASSERT_EQ(*(prices[3]), 8.99);
    ASSERT_EQ(*(prices[4]), 22.99);

    auto book3 = bookstore.find(jsonpath("$..book[2]"));
    ASSERT_EQ(size(book3), 1);
    ASSERT_EQ((*(book3[0]))["title"], "Moby Dick");

    auto last_book = bookstore.find(jsonpath("$..book[-1:]"));
    ASSERT_EQ(size(last_book), 1);
    ASSERT_EQ((*(last_book[0]))["title"], "The Lord of the Rings");

    auto first_two_books = bookstore.find(jsonpath("$..book[:2]"));
    ASSERT_EQ(size(first_two_books), 2);
    ASSERT_EQ((*(first_two_books[0]))["title"], "Sayings of the Century");
    ASSERT_EQ((*(first_two_books[1]))["title"], "Sword of Honour");

    auto everything_flat = bookstore.find(jsonpath("$..*"));
    ASSERT_EQ(size(everything_flat), 27);
}

TEST(datum, find_one_or_create1)
{
    auto bookstore_copy = bookstore;

    auto *new_book = bookstore_copy.find_one_or_create(jsonpath("$.store.book[4]"));
    hi_assert_not_null(new_book);

    *new_book = datum::make_map("title", "Hitchhikers Guide To The Galaxy", "price", 42.0);

    ASSERT_EQ(bookstore_copy["store"]["book"][4]["title"], "Hitchhikers Guide To The Galaxy");
    ASSERT_EQ(bookstore_copy["store"]["book"][4]["price"], 42.0);
}

TEST(datum, find_one_or_create2)
{
    auto bookstore_copy = bookstore;

    auto *new_book_title = bookstore_copy.find_one_or_create(jsonpath("$.store.book[4].title"));
    hi_assert_not_null(new_book_title);

    *new_book_title = "Hitchhikers Guide To The Galaxy";

    ASSERT_EQ(bookstore_copy["store"]["book"][4]["title"], "Hitchhikers Guide To The Galaxy");
}

TEST(datum, remove1)
{
    auto bookstore_copy = bookstore;

    auto removed1 = bookstore_copy.remove(jsonpath("$.store.book[-1]"));
    ASSERT_TRUE(removed1);
    ASSERT_EQ(size(bookstore_copy["store"]["book"]), 3);

    auto removed2 = bookstore_copy.remove(jsonpath("$.store.book[0]"));
    ASSERT_TRUE(removed2);
    ASSERT_EQ(size(bookstore_copy["store"]["book"]), 2);
    ASSERT_EQ(bookstore_copy["store"]["book"][0]["title"], "Sword of Honour");
}

TEST(datum, remove2)
{
    auto bookstore_copy = bookstore;

    auto removed1 = bookstore_copy.remove(jsonpath("$..price"));
    ASSERT_TRUE(removed1);
    ASSERT_FALSE(bookstore_copy["store"]["book"][0].contains("price"));
    ASSERT_TRUE(bookstore_copy["store"]["book"][0].contains("title"));
    ASSERT_FALSE(bookstore_copy["store"]["bicycle"].contains("price"));
    ASSERT_TRUE(bookstore_copy["store"]["bicycle"].contains("color"));

    auto removed2 = bookstore_copy.remove(jsonpath("$..color"));
    ASSERT_TRUE(removed2);
    ASSERT_FALSE(bookstore_copy["store"]["book"][0].contains("price"));
    ASSERT_TRUE(bookstore_copy["store"]["book"][0].contains("title"));
    ASSERT_FALSE(bookstore_copy["store"].contains("bicycle"));

    auto removed3 = bookstore_copy.remove(jsonpath("$..book[5].title"));
    ASSERT_FALSE(removed3);

    auto removed4 = bookstore_copy.remove(jsonpath("$..book[1].title"));
    ASSERT_TRUE(removed4);
    ASSERT_EQ(size(bookstore_copy["store"]["book"]), 4);
    ASSERT_FALSE(bookstore_copy["store"]["book"][1].contains("title"));

    auto removed5 = bookstore_copy.remove(jsonpath("$..book[1].author"));
    ASSERT_TRUE(removed5);
    ASSERT_EQ(size(bookstore_copy["store"]["book"]), 4);
    ASSERT_FALSE(bookstore_copy["store"]["book"][1].contains("author"));

    auto removed6 = bookstore_copy.remove(jsonpath("$..book[1].category"));
    ASSERT_TRUE(removed6);
    ASSERT_EQ(size(bookstore_copy["store"]["book"]), 3);
    ASSERT_EQ(bookstore_copy["store"]["book"][1]["title"], "Moby Dick");
}
