// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "datum.hpp"
#include "JSON.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(datum_suite) {

TEST_CASE(IntOperations)
{
    using namespace std::literals;

    auto const v = hi::datum{42};

    REQUIRE(static_cast<int>(v) == 42);
    REQUIRE(static_cast<float>(v) == 42.0);
    REQUIRE(static_cast<std::string>(v) == "42"s);
    REQUIRE(to_bool(v) == true);

    REQUIRE(holds_alternative<long long>(v) == true);
    REQUIRE(holds_alternative<double>(v) == false);
    REQUIRE(holds_alternative<std::string>(v) == false);

    REQUIRE((v == 42));
    REQUIRE(not (v < 42));
    REQUIRE(not (v < 41));
    REQUIRE((v < 43));
    REQUIRE(v - 5 == 37);

    REQUIRE((v == 42.0));
    REQUIRE(not (v < 42.0));
    REQUIRE(not (v < 41.0));
    REQUIRE((v < 43.0));

    auto const a = v + 3;
    REQUIRE(holds_alternative<long long>(a));
    REQUIRE((a == 45));

    auto const b = v + 3.0;
    REQUIRE(holds_alternative<double>(b));
    REQUIRE((b == 45.0));

    REQUIRE_THROWS((void)(hi::datum(-42) >> -1), std::domain_error);
    REQUIRE_THROWS((void)(hi::datum(42) >> -1), std::domain_error);

    REQUIRE(hi::datum(42) << 0 == 42);
    REQUIRE(hi::datum(42) >> 0 == 42);
    REQUIRE(hi::datum(42) << 1 == 84);
    REQUIRE(hi::datum(-42) >> 1 == -21);
    REQUIRE(hi::datum(-42) << 1 == -84);

    REQUIRE(hi::datum(42) << 63 == 0);
    REQUIRE(hi::datum(42) >> 63 == 0);
    REQUIRE(hi::datum(-42) >> 63 == -1);
    REQUIRE_THROWS((void)(hi::datum(42) << 64), std::domain_error);
    REQUIRE_THROWS((void)(hi::datum(42) >> 64), std::domain_error);
    REQUIRE_THROWS((void)(hi::datum(-42) >> 64), std::domain_error);
}

TEST_CASE(NegativeIntOperations)
{
    using namespace std::literals;

    auto const v = hi::datum{-1};

    REQUIRE(static_cast<int>(v) == -1);
    REQUIRE(static_cast<std::string>(v) == "-1"s);
}

TEST_CASE(FloatOperations)
{
    using namespace std::literals;

    auto const v = hi::datum{42.0};

    REQUIRE(static_cast<int>(v) == 42);
    REQUIRE(static_cast<float>(v) == 42.0);
    REQUIRE(static_cast<std::string>(v) == "42"s);
    REQUIRE(to_string(v) == "42"s);
    REQUIRE(std::format("{}", v) == "42"s);
    REQUIRE(repr(v) == "42.0"s);
    REQUIRE(to_bool(v));

    REQUIRE((v == 42.0));
    REQUIRE(not (v < 42.0));
    REQUIRE(not (v < 41.0));
    REQUIRE((v < 43.0));

    REQUIRE((v == 42));
    REQUIRE(not (v < 42));
    REQUIRE(not (v < 41));
    REQUIRE((v < 43));

    auto const a = v + 3;
    REQUIRE(holds_alternative<double>(a));
    REQUIRE((a == 45.0));

    auto const b = v + 3.0;
    REQUIRE(holds_alternative<double>(b));
    REQUIRE((b == 45.0));
}

TEST_CASE(StringOperations)
{
    using namespace std::literals;

    auto const v = hi::datum{"Hello World"};

    REQUIRE(static_cast<std::string>(v) == "Hello World"s);
}

TEST_CASE(ArrayOperations)
{
    auto const v = hi::datum::make_vector(11, 12, 13, 14, 15);

    REQUIRE(v[0] == 11);
    REQUIRE(v[1] == 12);
    REQUIRE(v[2] == 13);
    REQUIRE(v[3] == 14);
    REQUIRE(v[4] == 15);
    REQUIRE_THROWS((void)(v[5]), std::overflow_error);

    REQUIRE_THROWS((void)(v[-6]), std::overflow_error);
    REQUIRE(v[-5] == 11);
    REQUIRE(v[-4] == 12);
    REQUIRE(v[-3] == 13);
    REQUIRE(v[-2] == 14);
    REQUIRE(v[-1] == 15);
}

inline static hi::datum bookstore = hi::parse_JSON(
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

TEST_CASE(find)
{
    auto authors1 = bookstore.find(hi::jsonpath("$.store.book[*].author"));
    REQUIRE(authors1.size() == 4);
    //REQUIRE(*(authors1[0]) == "Nigel Rees");
    //REQUIRE(*(authors1[1]) == "Evelyn Waugh");
    //REQUIRE(*(authors1[2]) == "Herman Melville");
    //REQUIRE(*(authors1[3]) == "J. R. R. Tolkien");

    auto authors2 = bookstore.find(hi::jsonpath("$..author"));
    REQUIRE(authors2.size() == 4);
    REQUIRE(*(authors2[0]) == "Nigel Rees");
    REQUIRE(*(authors2[1]) == "Evelyn Waugh");
    REQUIRE(*(authors2[2]) == "Herman Melville");
    REQUIRE(*(authors2[3]) == "J. R. R. Tolkien");

    auto things = bookstore.find(hi::jsonpath("$.store.*"));
    REQUIRE(things.size() == 2);
    REQUIRE(things[0]->size() == 2); // attributes of bicycle
    REQUIRE(things[1]->size() == 4); // list of books

    auto prices = bookstore.find(hi::jsonpath("$.store..price"));
    REQUIRE(prices.size() == 5);
    REQUIRE(*(prices[0]) == 19.95); // bicycle first
    REQUIRE(*(prices[1]) == 8.95);
    REQUIRE(*(prices[2]) == 12.99);
    REQUIRE(*(prices[3]) == 8.99);
    REQUIRE(*(prices[4]) == 22.99);

    auto book3 = bookstore.find(hi::jsonpath("$..book[2]"));
    REQUIRE(book3.size() == 1);
    REQUIRE((*(book3[0]))["title"] == "Moby Dick");

    auto last_book = bookstore.find(hi::jsonpath("$..book[-1:]"));
    REQUIRE(last_book.size() == 1);
    REQUIRE((*(last_book[0]))["title"] == "The Lord of the Rings");

    auto first_two_books = bookstore.find(hi::jsonpath("$..book[:2]"));
    REQUIRE(first_two_books.size() == 2);
    REQUIRE((*(first_two_books[0]))["title"] == "Sayings of the Century");
    REQUIRE((*(first_two_books[1]))["title"] == "Sword of Honour");

    auto everything_flat = bookstore.find(hi::jsonpath("$..*"));
    REQUIRE(everything_flat.size() == 27);
}

TEST_CASE(find_one_or_create1)
{
    auto bookstore_copy = bookstore;

    auto *new_book = bookstore_copy.find_one_or_create(hi::jsonpath("$.store.book[4]"));
    hi_assert_not_null(new_book);

    *new_book = hi::datum::make_map("title", "Hitchhikers Guide To The Galaxy", "price", 42.0);

    REQUIRE(bookstore_copy["store"]["book"][4]["title"] == "Hitchhikers Guide To The Galaxy");
    REQUIRE(bookstore_copy["store"]["book"][4]["price"] == 42.0);
}

TEST_CASE(find_one_or_create2)
{
    auto bookstore_copy = bookstore;

    auto *new_book_title = bookstore_copy.find_one_or_create(hi::jsonpath("$.store.book[4].title"));
    hi_assert_not_null(new_book_title);

    *new_book_title = "Hitchhikers Guide To The Galaxy";

    REQUIRE(bookstore_copy["store"]["book"][4]["title"] == "Hitchhikers Guide To The Galaxy");
}

TEST_CASE(remove1)
{
    auto bookstore_copy = bookstore;

    auto removed1 = bookstore_copy.remove(hi::jsonpath("$.store.book[-1]"));
    REQUIRE(removed1);
    REQUIRE(bookstore_copy["store"]["book"].size() == 3);

    auto removed2 = bookstore_copy.remove(hi::jsonpath("$.store.book[0]"));
    REQUIRE(removed2);
    REQUIRE(bookstore_copy["store"]["book"].size() == 2);
    REQUIRE(bookstore_copy["store"]["book"][0]["title"] == "Sword of Honour");
}

TEST_CASE(remove2)
{
    auto bookstore_copy = bookstore;

    auto removed1 = bookstore_copy.remove(hi::jsonpath("$..price"));
    REQUIRE(removed1);
    REQUIRE(not bookstore_copy["store"]["book"][0].contains("price"));
    REQUIRE(bookstore_copy["store"]["book"][0].contains("title"));
    REQUIRE(not bookstore_copy["store"]["bicycle"].contains("price"));
    REQUIRE(bookstore_copy["store"]["bicycle"].contains("color"));

    auto removed2 = bookstore_copy.remove(hi::jsonpath("$..color"));
    REQUIRE(removed2);
    REQUIRE(not bookstore_copy["store"]["book"][0].contains("price"));
    REQUIRE(bookstore_copy["store"]["book"][0].contains("title"));
    REQUIRE(not bookstore_copy["store"].contains("bicycle"));

    auto removed3 = bookstore_copy.remove(hi::jsonpath("$..book[5].title"));
    REQUIRE(not removed3);

    auto removed4 = bookstore_copy.remove(hi::jsonpath("$..book[1].title"));
    REQUIRE(removed4);
    REQUIRE(bookstore_copy["store"]["book"].size() == 4);
    REQUIRE(not bookstore_copy["store"]["book"][1].contains("title"));

    auto removed5 = bookstore_copy.remove(hi::jsonpath("$..book[1].author"));
    REQUIRE(removed5);
    REQUIRE(bookstore_copy["store"]["book"].size() == 4);
    REQUIRE(not bookstore_copy["store"]["book"][1].contains("author"));

    auto removed6 = bookstore_copy.remove(hi::jsonpath("$..book[1].category"));
    REQUIRE(removed6);
    REQUIRE(bookstore_copy["store"]["book"].size() == 3);
    REQUIRE(bookstore_copy["store"]["book"][1]["title"] == "Moby Dick");
}

};
