// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "formula.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace hi;

TEST(Formula, Literals)
{
    std::unique_ptr<formula_node> e;

    ASSERT_NO_THROW(e = parse_formula("42"));
    ASSERT_EQ(e->string(), "42");

    ASSERT_NO_THROW(e = parse_formula("42.0"));
    ASSERT_EQ(e->string(), "42.0");

    ASSERT_NO_THROW(e = parse_formula("\"hello\""));
    ASSERT_EQ(e->string(), "\"hello\"");

    ASSERT_NO_THROW(e = parse_formula("true"));
    ASSERT_EQ(e->string(), "true");

    ASSERT_NO_THROW(e = parse_formula("false"));
    ASSERT_EQ(e->string(), "false");

    ASSERT_NO_THROW(e = parse_formula("null"));
    ASSERT_EQ(e->string(), "null");

    ASSERT_NO_THROW(e = parse_formula("foo"));
    ASSERT_EQ(e->string(), "foo");
}

TEST(Formula, BinaryOperatorsLeftToRightAssociativity)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("4 - 2 - 1"));
    ASSERT_EQ(e->string(), "((4 - 2) - 1)");
    ASSERT_EQ(e->evaluate(context), 1);

    ASSERT_NO_THROW(e = parse_formula("depth - data.level - 1"));
    ASSERT_EQ(e->string(), "((depth - (data . level)) - 1)");
}

TEST(Formula, BinaryOperatorsRightToLeftAssociativity)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("4 -= 2 -= 1"));
    ASSERT_EQ(e->string(), "(4 -= (2 -= 1))");
}

TEST(Formula, BinaryOperators)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("1 + 2"));
    ASSERT_EQ(e->string(), "(1 + 2)");

    ASSERT_NO_THROW(e = parse_formula("1 + 2 * 3"));
    ASSERT_EQ(e->string(), "(1 + (2 * 3))");

    ASSERT_NO_THROW(e = parse_formula("1 * 2 + 3"));
    ASSERT_EQ(e->string(), "((1 * 2) + 3)");

    ASSERT_NO_THROW(e = parse_formula("(1 + 2) * 3"));
    ASSERT_EQ(e->string(), "((1 + 2) * 3)");

    ASSERT_NO_THROW(e = parse_formula("42 - 6"));
    ASSERT_EQ(e->string(), "(42 - 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 36);

    ASSERT_NO_THROW(e = parse_formula("42 + 6"));
    ASSERT_EQ(e->string(), "(42 + 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 48);

    ASSERT_NO_THROW(e = parse_formula("42 * 6"));
    ASSERT_EQ(e->string(), "(42 * 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 252);

    ASSERT_NO_THROW(e = parse_formula("42 ** 6"));
    ASSERT_EQ(e->string(), "(42 ** 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 5489031744);

    ASSERT_NO_THROW(e = parse_formula("42 / 6"));
    ASSERT_EQ(e->string(), "(42 / 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 7);

    ASSERT_NO_THROW(e = parse_formula("42 % 6"));
    ASSERT_EQ(e->string(), "(42 % 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_formula("42 & 6"));
    ASSERT_EQ(e->string(), "(42 & 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2);

    ASSERT_NO_THROW(e = parse_formula("42 | 6"));
    ASSERT_EQ(e->string(), "(42 | 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 46);

    ASSERT_NO_THROW(e = parse_formula("42 ^ 6"));
    ASSERT_EQ(e->string(), "(42 ^ 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 44);

    ASSERT_NO_THROW(e = parse_formula("42 << 6"));
    ASSERT_EQ(e->string(), "(42 << 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2688);

    ASSERT_NO_THROW(e = parse_formula("42 >> 6"));
    ASSERT_EQ(e->string(), "(42 >> 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_formula("42 == 6"));
    ASSERT_EQ(e->string(), "(42 == 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_formula("42 != 6"));
    ASSERT_EQ(e->string(), "(42 != 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_formula("42 < 6"));
    ASSERT_EQ(e->string(), "(42 < 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_formula("42 > 6"));
    ASSERT_EQ(e->string(), "(42 > 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_formula("42 <= 6"));
    ASSERT_EQ(e->string(), "(42 <= 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_formula("42 >= 6"));
    ASSERT_EQ(e->string(), "(42 >= 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_formula("42 && 0"));
    ASSERT_EQ(e->string(), "(42 && 0)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_formula("42 || 0"));
    ASSERT_EQ(e->string(), "(42 || 0)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    // Context does not have 'a' set yet.
    ASSERT_NO_THROW(e = parse_formula("a = 2"));
    ASSERT_EQ(e->string(), "(a = 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2);
    ASSERT_EQ(context.get("a"), 2);

    // Context has 'a' set to 2.
    ASSERT_NO_THROW(e = parse_formula("a = 42"));
    ASSERT_EQ(e->string(), "(a = 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_formula("a += 2"));
    ASSERT_EQ(e->string(), "(a += 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 44);
    ASSERT_EQ(context.get("a"), 44);

    ASSERT_NO_THROW(e = parse_formula("a -= 2"));
    ASSERT_EQ(e->string(), "(a -= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_formula("a *= 2"));
    ASSERT_EQ(e->string(), "(a *= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 84);
    ASSERT_EQ(context.get("a"), 84);

    ASSERT_NO_THROW(e = parse_formula("a /= 2"));
    ASSERT_EQ(e->string(), "(a /= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_formula("a %= 15"));
    ASSERT_EQ(e->string(), "(a %= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 12);
    ASSERT_EQ(context.get("a"), 12);

    ASSERT_NO_THROW(e = parse_formula("a <<= 2"));
    ASSERT_EQ(e->string(), "(a <<= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 48);
    ASSERT_EQ(context.get("a"), 48);

    ASSERT_NO_THROW(e = parse_formula("a >>= 1"));
    ASSERT_EQ(e->string(), "(a >>= 1)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 24);
    ASSERT_EQ(context.get("a"), 24);

    ASSERT_NO_THROW(e = parse_formula("a &= 15"));
    ASSERT_EQ(e->string(), "(a &= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 8);
    ASSERT_EQ(context.get("a"), 8);

    ASSERT_NO_THROW(e = parse_formula("a ^= 15"));
    ASSERT_EQ(e->string(), "(a ^= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 7);
    ASSERT_EQ(context.get("a"), 7);

    ASSERT_NO_THROW(e = parse_formula("a |= 17"));
    ASSERT_EQ(e->string(), "(a |= 17)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 23);
    ASSERT_EQ(context.get("a"), 23);
}

TEST(Formula, UnaryOperators)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("~ 1"));
    ASSERT_EQ(e->string(), "(~ 1)");

    ASSERT_NO_THROW(e = parse_formula("~ 1 + 2"));
    ASSERT_EQ(e->string(), "((~ 1) + 2)");

    ASSERT_NO_THROW(e = parse_formula("~ (1 + 2)"));
    ASSERT_EQ(e->string(), "(~ (1 + 2))");

    ASSERT_NO_THROW(e = parse_formula("1 + ~2"));
    ASSERT_EQ(e->string(), "(1 + (~ 2))");

    ASSERT_NO_THROW(e = parse_formula("~ 42"));
    ASSERT_EQ(e->string(), "(~ 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, -43);

    ASSERT_NO_THROW(e = parse_formula("! 42"));
    ASSERT_EQ(e->string(), "(! 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_formula("- 42"));
    ASSERT_EQ(e->string(), "(- 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, -42);

    ASSERT_NO_THROW(e = parse_formula("+ 42"));
    ASSERT_EQ(e->string(), "(+ 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    ASSERT_NO_THROW(e = parse_formula("++ 1"));
    ASSERT_EQ(e->string(), "(++ 1)");
    ASSERT_NO_THROW(e = parse_formula("-- 1"));
    ASSERT_EQ(e->string(), "(-- 1)");
}

TEST(Formula, IndexOperator)
{
    std::unique_ptr<formula_node> e;
    datum r;
    datum expected;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("foo[2]"));
    ASSERT_EQ(e->string(), "(foo[2])");

    ASSERT_NO_THROW(e = parse_formula("!foo[2]"));
    ASSERT_EQ(e->string(), "(! (foo[2]))");

    ASSERT_NO_THROW(e = parse_formula("(!foo)[2]"));
    ASSERT_EQ(e->string(), "((! foo)[2])");

    ASSERT_NO_THROW(e = parse_formula("foo = [1, 2, 42, 3]"));
    ASSERT_EQ(e->string(), "(foo = [1, 2, 42, 3])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::make_vector(1, 2, 42, 3);
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_formula("foo[2]"));
    ASSERT_EQ(e->string(), "(foo[2])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    ASSERT_NO_THROW(e = parse_formula("foo[1] = 33"));
    ASSERT_EQ(e->string(), "((foo[1]) = 33)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 33);
    expected = datum::make_vector(1, 33, 42, 3);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_formula("foo[1] += 33"));
    ASSERT_EQ(e->string(), "((foo[1]) += 33)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 66);
    expected = datum::make_vector(1, 66, 42, 3);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_formula("foo += 4"));
    ASSERT_EQ(e->string(), "(foo += 4)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::make_vector(1, 66, 42, 3, 4);
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);
}

TEST(Formula, Binding)
{
    std::unique_ptr<formula_node> e;
    datum r;
    datum expected;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("foo = [33, 42]"));
    ASSERT_EQ(e->string(), "(foo = [33, 42])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::make_vector(33, 42);
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_formula("[bar, baz] = foo"));
    ASSERT_EQ(e->string(), "([bar, baz] = foo)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("bar"), 33);
    ASSERT_EQ(context.get("baz"), 42);

    ASSERT_NO_THROW(e = parse_formula("[foo[1], foo[0]] = foo"));
    ASSERT_EQ(e->string(), "([(foo[1]), (foo[0])] = foo)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    expected = datum::make_vector(42, 33);
    ASSERT_EQ(context.get("foo"), expected);
}

TEST(Formula, FunctionCall)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;

    ASSERT_NO_THROW(e = parse_formula("float()"));
    ASSERT_EQ(e->string(), "(float())");

    ASSERT_NO_THROW(e = parse_formula("float(2)"));
    ASSERT_EQ(e->string(), "(float(2))");

    ASSERT_NO_THROW(e = parse_formula("float(2, 3)"));
    ASSERT_EQ(e->string(), "(float(2, 3))");

    ASSERT_NO_THROW(e = parse_formula("!float(2)"));
    ASSERT_EQ(e->string(), "(! (float(2)))");

    ASSERT_NO_THROW(e = parse_formula("(!float)(2)"));
    ASSERT_EQ(e->string(), "((! float)(2))");

    ASSERT_NO_THROW(e = parse_formula("float(5)"));
    ASSERT_EQ(e->string(), "(float(5))");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(repr(r), "5.0");
}

TEST(Formula, MethodCall)
{
    std::unique_ptr<formula_node> e;
    datum r;
    formula_evaluation_context context;
    datum expected;

    ASSERT_NO_THROW(e = parse_formula("foo = [1, 2, 3]"));
    ASSERT_EQ(e->string(), "(foo = [1, 2, 3])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::make_vector(1, 2, 3);
    ASSERT_EQ(r, expected);

    ASSERT_NO_THROW(e = parse_formula("foo.append(4.2)"));
    ASSERT_EQ(e->string(), "((foo . append)(4.2))");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::make_vector(datum{1}, datum{2}, datum{3}, datum{4.2});
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_formula("foo.pop()"));
    ASSERT_EQ(e->string(), "((foo . pop)())");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 4.2);
    expected = datum::make_vector(1, 2, 3);
    ASSERT_EQ(context.get("foo"), expected);
}

TEST(Formula, Members)
{
    std::unique_ptr<formula_node> e;

    ASSERT_NO_THROW(e = parse_formula("foo.bar"));
    ASSERT_EQ(e->string(), "(foo . bar)");

    ASSERT_NO_THROW(e = parse_formula("foo.append(2, 3)"));
    ASSERT_EQ(e->string(), "((foo . append)(2, 3))");
}

TEST(Formula, Vector)
{
    std::unique_ptr<formula_node> e;

    ASSERT_NO_THROW(e = parse_formula("[]"));
    ASSERT_EQ(e->string(), "[]");

    ASSERT_NO_THROW(e = parse_formula("[1]"));
    ASSERT_EQ(e->string(), "[1]");

    ASSERT_NO_THROW(e = parse_formula("[1, 2, 3]"));
    ASSERT_EQ(e->string(), "[1, 2, 3]");

    ASSERT_NO_THROW(e = parse_formula("[1, 2, 3,]"));
    ASSERT_EQ(e->string(), "[1, 2, 3]");
}

TEST(Formula, Map)
{
    std::unique_ptr<formula_node> e;

    ASSERT_NO_THROW(e = parse_formula("{}"));
    ASSERT_EQ(e->string(), "{}");

    ASSERT_NO_THROW(e = parse_formula("{1: 1.1}"));
    ASSERT_EQ(e->string(), "{1: 1.1}");

    ASSERT_NO_THROW(e = parse_formula("{1: 1.1, 2: 2.2}"));
    ASSERT_EQ(e->string(), "{1: 1.1, 2: 2.2}");

    ASSERT_NO_THROW(e = parse_formula("{1: 1.1, 2: 2.2, }"));
    ASSERT_EQ(e->string(), "{1: 1.1, 2: 2.2}");
}
