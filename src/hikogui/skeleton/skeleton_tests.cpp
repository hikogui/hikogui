// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/skeleton/skeleton.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(skeleton, Text)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(t = parse_skeleton(std::filesystem::path{}, ""));
    ASSERT_EQ(to_string(*t), "<top >");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(result, "");

    ASSERT_NO_THROW(t = parse_skeleton(std::filesystem::path{}, "foo"));
    ASSERT_EQ(to_string(*t), "<top <text foo>>");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(result, "foo");
}

TEST(skeleton, Placeholder)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(t = parse_skeleton(std::filesystem::path{}, "foo${42}bar"));
    ASSERT_EQ(to_string(*t), "<top <text foo><placeholder 42><text bar>>");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(result, "foo42bar");
}

TEST(skeleton, If)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#a = 42\n"
            "#if a == 42\n"
            "forty two\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 42)>"
        "<if (a == 42)"
        "<text forty two\n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "forty two\n"
        "bar\n");

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "# a = 43\n"
            "#if a == 42\n"
            "forty two\n"
            "#elif a == 43\n"
            "forty three\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 43)>"
        "<if (a == 42)"
        "<text forty two\n>"
        "elif (a == 43)"
        "<text forty three\n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "forty three\n"
        "bar\n");

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "# a = 2\n"
            "#if a == 42\n"
            "forty two\n"
            "#elif a == 43\n"
            "forty three\n"
            "#else\n"
            "something else\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 2)>"
        "<if (a == 42)"
        "<text forty two\n>"
        "elif (a == 43)"
        "<text forty three\n>"
        "else "
        "<text something else\n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "something else\n"
        "bar\n");
}

TEST(skeleton, For)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#for a: [42, 43]\n"
            "value is ${a}\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<for a: [42, 43]"
        "<text value is ><placeholder a><text \n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "value is 42\n"
        "value is 43\n"
        "bar\n");

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#for a: [42, 43]\n"
            "value is ${a}\n"
            "#else\n"
            "No values\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<for a: [42, 43]"
        "<text value is ><placeholder a><text \n>"
        "else "
        "<text No values\n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "value is 42\n"
        "value is 43\n"
        "bar\n");
}

TEST(skeleton, While)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "# a = 40\n"
            "#while a < 42\n"
            "    value is ${a}\n"
            "    # ++a\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 40)>"
        "<while (a < 42)"
        "<text     value is ><placeholder a><text \n>"
        "<expression (++ a)>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "    value is 40\n"
        "    value is 41\n"
        "bar\n");

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "# a = 38\n"
            "#while a < 42\n"
            "    #if a == 40\n"
            "        #break\n"
            "    #end\n"
            "    value is ${a}\n"
            "    # ++a\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 38)>"
        "<while (a < 42)"
        "<text ><if (a == 40)"
        "<text ><break>"
        "<text >>"
        "<text     value is ><placeholder a><text \n>"
        "<expression (++ a)>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "    value is 38\n"
        "    value is 39\n"
        "bar\n");

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "# a = 38\n"
            "#while a < 42\n"
            "    # ++a\n"
            "    #if a == 40\n"
            "        #continue\n"
            "    #end\n"
            "    value is ${a}\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<expression (a = 38)>"
        "<while (a < 42)"
        "<text ><expression (++ a)>"
        "<text ><if (a == 40)"
        "<text ><continue>"
        "<text >>"
        "<text     value is ><placeholder a><text \n>"
        ">"
        "<text bar\n>"
        ">");
    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "    value is 39\n"
        "    value is 41\n"
        "    value is 42\n"
        "bar\n");
}

TEST(skeleton, DoWhile)
{
    std::unique_ptr<skeleton_node> t;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#do\n"
            "value is ${a}\n"
            "#while a < 42\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<do "
        "<text value is ><placeholder a><text \n>"
        "(a < 42)>"
        "<text bar\n>"
        ">");
}

TEST(skeleton, Function)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#function foo(bar, baz)\n"
            "value is ${bar + baz}\n"
            "#end\n"
            "${foo(1, 2)}\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<function foo(bar,baz)"
        "<text value is ><placeholder (bar + baz)><text \n>"
        ">"
        "<placeholder (foo(1, 2))><text \n>"
        ">");

    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "value is 3\n\n");
}

TEST(skeleton, FunctionReplaceAndSuper)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#function foo(bar, baz)\n"
            "value is ${bar + baz}\n"
            "#end\n"
            "bar\n"
            "#function foo(bar, baz)\n"
            "value is ${bar * baz}\n"
            "Previous ${super(bar,baz)}\n"
            "#end\n"
            "baz\n"
            "${foo(12, 3)}\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<function foo(bar,baz)"
        "<text value is ><placeholder (bar + baz)><text \n>"
        ">"
        "<text bar\n>"
        "<function foo(bar,baz)"
        "<text value is ><placeholder (bar * baz)>"
        "<text \nPrevious ><placeholder (super(bar, baz))><text \n>"
        ">"
        "<text baz\n>"
        "<placeholder (foo(12, 3))><text \n>"
        ">");

    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "bar\n"
        "baz\n"
        "value is 36\n"
        "Previous value is 15\n\n\n");
}

TEST(skeleton, FunctionReturn)
{
    std::unique_ptr<skeleton_node> t;
    std::string result;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#function foo(bar, baz)\n"
            "    This text is ignored\n"
            "    #return bar + baz\n"
            "#end\n"
            "bar\n"
            "${foo(12, 3)}\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<function foo(bar,baz)"
        "<text     This text is ignored\n>"
        "<return (bar + baz)>"
        ">"
        "<text bar\n>"
        "<placeholder (foo(12, 3))><text \n>"
        ">");

    ASSERT_NO_THROW(result = t->evaluate_output());
    ASSERT_EQ(
        result,
        "foo\n"
        "bar\n"
        "15\n");
}

TEST(skeleton, Block)
{
    std::unique_ptr<skeleton_node> t;

    ASSERT_NO_THROW(
        t = parse_skeleton(
            std::filesystem::path{},
            "foo\n"
            "#block foo\n"
            "value is ${1 + 2}\n"
            "#end\n"
            "bar\n"));
    ASSERT_EQ(
        to_string(*t),
        "<top "
        "<text foo\n>"
        "<block foo"
        "<text value is ><placeholder (1 + 2)><text \n>"
        ">"
        "<text bar\n>"
        ">");
}

TEST(skeleton, Include)
{
    std::unique_ptr<skeleton_node> t;

    ASSERT_NO_THROW(t = parse_skeleton(std::filesystem::path{"includer.ttt"}));
    ASSERT_EQ(
        normalize_lf(to_string(*t)),
        "<top "
        "<text foo\n>"
        "<top "
        "<text baz\n>"
        ">"
        "<text bar\n>"
        ">");
}
