// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/expression.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace tt;

TEST(Expression, Literals) {
    std::unique_ptr<expression_node> e;

    ASSERT_NO_THROW(e = parse_expression("42"));
    ASSERT_EQ(e->string(), "42");

    ASSERT_NO_THROW(e = parse_expression("42.0"));
    ASSERT_EQ(e->string(), "42.0");

    ASSERT_NO_THROW(e = parse_expression("\"hello\""));
    ASSERT_EQ(e->string(), "\"hello\"");

    ASSERT_NO_THROW(e = parse_expression("true"));
    ASSERT_EQ(e->string(), "true");

    ASSERT_NO_THROW(e = parse_expression("false"));
    ASSERT_EQ(e->string(), "false");

    ASSERT_NO_THROW(e = parse_expression("null"));
    ASSERT_EQ(e->string(), "null");

    ASSERT_NO_THROW(e = parse_expression("foo"));
    ASSERT_EQ(e->string(), "foo");
}

TEST(Expression, BinaryOperatorsLeftToRightAssociativity) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("4 - 2 - 1"));
    ASSERT_EQ(e->string(), "((4 - 2) - 1)");
    ASSERT_EQ(e->evaluate(context), 1);

    ASSERT_NO_THROW(e = parse_expression("depth - data.level - 1"));
    ASSERT_EQ(e->string(), "((depth - (data . level)) - 1)");


    
}

TEST(Expression, BinaryOperatorsRightToLeftAssociativity) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("4 -= 2 -= 1"));
    ASSERT_EQ(e->string(), "(4 -= (2 -= 1))");
}


TEST(Expression, BinaryOperators) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("1 + 2"));
    ASSERT_EQ(e->string(), "(1 + 2)");

    ASSERT_NO_THROW(e = parse_expression("1 + 2 * 3"));
    ASSERT_EQ(e->string(), "(1 + (2 * 3))");

    ASSERT_NO_THROW(e = parse_expression("1 * 2 + 3"));
    ASSERT_EQ(e->string(), "((1 * 2) + 3)");

    ASSERT_NO_THROW(e = parse_expression("(1 + 2) * 3"));
    ASSERT_EQ(e->string(), "((1 + 2) * 3)");

    ASSERT_NO_THROW(e = parse_expression("42 - 6"));
    ASSERT_EQ(e->string(), "(42 - 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 36);

    ASSERT_NO_THROW(e = parse_expression("42 + 6"));
    ASSERT_EQ(e->string(), "(42 + 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 48);

    ASSERT_NO_THROW(e = parse_expression("42 * 6"));
    ASSERT_EQ(e->string(), "(42 * 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 252);

    ASSERT_NO_THROW(e = parse_expression("42 ** 6"));
    ASSERT_EQ(e->string(), "(42 ** 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 5489031744);

    ASSERT_NO_THROW(e = parse_expression("42 / 6"));
    ASSERT_EQ(e->string(), "(42 / 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 7);

    ASSERT_NO_THROW(e = parse_expression("42 % 6"));
    ASSERT_EQ(e->string(), "(42 % 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_expression("42 & 6"));
    ASSERT_EQ(e->string(), "(42 & 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2);

    ASSERT_NO_THROW(e = parse_expression("42 | 6"));
    ASSERT_EQ(e->string(), "(42 | 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 46);

    ASSERT_NO_THROW(e = parse_expression("42 ^ 6"));
    ASSERT_EQ(e->string(), "(42 ^ 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 44);

    ASSERT_NO_THROW(e = parse_expression("42 << 6"));
    ASSERT_EQ(e->string(), "(42 << 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2688);

    ASSERT_NO_THROW(e = parse_expression("42 >> 6"));
    ASSERT_EQ(e->string(), "(42 >> 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_expression("42 == 6"));
    ASSERT_EQ(e->string(), "(42 == 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_expression("42 != 6"));
    ASSERT_EQ(e->string(), "(42 != 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_expression("42 < 6"));
    ASSERT_EQ(e->string(), "(42 < 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_expression("42 > 6"));
    ASSERT_EQ(e->string(), "(42 > 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_expression("42 <= 6"));
    ASSERT_EQ(e->string(), "(42 <= 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_expression("42 >= 6"));
    ASSERT_EQ(e->string(), "(42 >= 6)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, true);

    ASSERT_NO_THROW(e = parse_expression("42 && 0"));
    ASSERT_EQ(e->string(), "(42 && 0)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 0);

    ASSERT_NO_THROW(e = parse_expression("42 || 0"));
    ASSERT_EQ(e->string(), "(42 || 0)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    // Context does not have 'a' set yet.
    ASSERT_NO_THROW(e = parse_expression("a = 2"));
    ASSERT_EQ(e->string(), "(a = 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 2);
    ASSERT_EQ(context.get("a"), 2);

    // Context has 'a' set to 2.
    ASSERT_NO_THROW(e = parse_expression("a = 42"));
    ASSERT_EQ(e->string(), "(a = 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_expression("a += 2"));
    ASSERT_EQ(e->string(), "(a += 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 44);
    ASSERT_EQ(context.get("a"), 44);

    ASSERT_NO_THROW(e = parse_expression("a -= 2"));
    ASSERT_EQ(e->string(), "(a -= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_expression("a *= 2"));
    ASSERT_EQ(e->string(), "(a *= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 84);
    ASSERT_EQ(context.get("a"), 84);

    ASSERT_NO_THROW(e = parse_expression("a /= 2"));
    ASSERT_EQ(e->string(), "(a /= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("a"), 42);

    ASSERT_NO_THROW(e = parse_expression("a %= 15"));
    ASSERT_EQ(e->string(), "(a %= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 12);
    ASSERT_EQ(context.get("a"), 12);

    ASSERT_NO_THROW(e = parse_expression("a <<= 2"));
    ASSERT_EQ(e->string(), "(a <<= 2)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 48);
    ASSERT_EQ(context.get("a"), 48);

    ASSERT_NO_THROW(e = parse_expression("a >>= 1"));
    ASSERT_EQ(e->string(), "(a >>= 1)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 24);
    ASSERT_EQ(context.get("a"), 24);

    ASSERT_NO_THROW(e = parse_expression("a &= 15"));
    ASSERT_EQ(e->string(), "(a &= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 8);
    ASSERT_EQ(context.get("a"), 8);

    ASSERT_NO_THROW(e = parse_expression("a ^= 15"));
    ASSERT_EQ(e->string(), "(a ^= 15)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 7);
    ASSERT_EQ(context.get("a"), 7);

    ASSERT_NO_THROW(e = parse_expression("a |= 17"));
    ASSERT_EQ(e->string(), "(a |= 17)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 23);
    ASSERT_EQ(context.get("a"), 23);
}

TEST(Expression, UnaryOperators) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("~ 1"));
    ASSERT_EQ(e->string(), "(~ 1)");

    ASSERT_NO_THROW(e = parse_expression("~ 1 + 2"));
    ASSERT_EQ(e->string(), "((~ 1) + 2)");

    ASSERT_NO_THROW(e = parse_expression("~ (1 + 2)"));
    ASSERT_EQ(e->string(), "(~ (1 + 2))");

    ASSERT_NO_THROW(e = parse_expression("1 + ~2"));
    ASSERT_EQ(e->string(), "(1 + (~ 2))");

    ASSERT_NO_THROW(e = parse_expression("~ 42"));
    ASSERT_EQ(e->string(), "(~ 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, -43);

    ASSERT_NO_THROW(e = parse_expression("! 42"));
    ASSERT_EQ(e->string(), "(! 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, false);

    ASSERT_NO_THROW(e = parse_expression("- 42"));
    ASSERT_EQ(e->string(), "(- 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, -42);

    ASSERT_NO_THROW(e = parse_expression("+ 42"));
    ASSERT_EQ(e->string(), "(+ 42)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    ASSERT_NO_THROW(e = parse_expression("++ 1")); ASSERT_EQ(e->string(), "(++ 1)");
    ASSERT_NO_THROW(e = parse_expression("-- 1")); ASSERT_EQ(e->string(), "(-- 1)");
}

TEST(Expression, IndexOperator) {
    std::unique_ptr<expression_node> e;
    datum r;
    datum expected;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("foo[2]"));
    ASSERT_EQ(e->string(), "(foo[2])");

    ASSERT_NO_THROW(e = parse_expression("!foo[2]"));
    ASSERT_EQ(e->string(), "(! (foo[2]))");

    ASSERT_NO_THROW(e = parse_expression("(!foo)[2]"));
    ASSERT_EQ(e->string(), "((! foo)[2])");

    ASSERT_NO_THROW(e = parse_expression("foo = [1, 2, 42, 3]"));
    ASSERT_EQ(e->string(), "(foo = [1, 2, 42, 3])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::vector{1, 2, 42, 3};
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_expression("foo[2]"));
    ASSERT_EQ(e->string(), "(foo[2])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);

    ASSERT_NO_THROW(e = parse_expression("foo[1] = 33"));
    ASSERT_EQ(e->string(), "((foo[1]) = 33)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 33);
    expected = datum::vector{1, 33, 42, 3};
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_expression("foo[1] += 33"));
    ASSERT_EQ(e->string(), "((foo[1]) += 33)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 66);
    expected = datum::vector{1, 66, 42, 3};
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_expression("foo += 4"));
    ASSERT_EQ(e->string(), "(foo += 4)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::vector{1, 66, 42, 3, 4};
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);
}

TEST(Expression, Binding) {
    std::unique_ptr<expression_node> e;
    datum r;
    datum expected;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("foo = [33, 42]"));
    ASSERT_EQ(e->string(), "(foo = [33, 42])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::vector{33, 42};
    ASSERT_EQ(r, expected);
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_expression("[bar, baz] = foo"));
    ASSERT_EQ(e->string(), "([bar, baz] = foo)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    ASSERT_EQ(context.get("bar"), 33);
    ASSERT_EQ(context.get("baz"), 42);

    ASSERT_NO_THROW(e = parse_expression("[foo[1], foo[0]] = foo"));
    ASSERT_EQ(e->string(), "([(foo[1]), (foo[0])] = foo)");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 42);
    expected = datum::vector{42, 33};
    ASSERT_EQ(context.get("foo"), expected);
}

TEST(Expression, FunctionCall) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;

    ASSERT_NO_THROW(e = parse_expression("float()"));
    ASSERT_EQ(e->string(), "(float())");

    ASSERT_NO_THROW(e = parse_expression("float(2)"));
    ASSERT_EQ(e->string(), "(float(2))");

    ASSERT_NO_THROW(e = parse_expression("float(2, 3)"));
    ASSERT_EQ(e->string(), "(float(2, 3))");

    ASSERT_NO_THROW(e = parse_expression("!float(2)"));
    ASSERT_EQ(e->string(), "(! (float(2)))");

    ASSERT_NO_THROW(e = parse_expression("(!float)(2)"));
    ASSERT_EQ(e->string(), "((! float)(2))");

    ASSERT_NO_THROW(e = parse_expression("float(5)"));
    ASSERT_EQ(e->string(), "(float(5))");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(to_string(r), "5.0");

}

TEST(Expression, MethodCall) {
    std::unique_ptr<expression_node> e;
    datum r;
    expression_evaluation_context context;
    datum::vector expected;

    ASSERT_NO_THROW(e = parse_expression("foo = [1, 2, 3]"));
    ASSERT_EQ(e->string(), "(foo = [1, 2, 3])");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::vector{1, 2, 3};
    ASSERT_EQ(r, expected);

    ASSERT_NO_THROW(e = parse_expression("foo.append(4.2)"));
    ASSERT_EQ(e->string(), "((foo . append)(4.2))");
    ASSERT_NO_THROW(r = e->evaluate(context));
    expected = datum::vector{datum{1}, datum{2}, datum{3}, datum{4.2}};
    ASSERT_EQ(context.get("foo"), expected);

    ASSERT_NO_THROW(e = parse_expression("foo.pop()"));
    ASSERT_EQ(e->string(), "((foo . pop)())");
    ASSERT_NO_THROW(r = e->evaluate(context));
    ASSERT_EQ(r, 4.2);
    expected = datum::vector{1, 2, 3};
    ASSERT_EQ(context.get("foo"), expected);

}

TEST(Expression, Members) {
    std::unique_ptr<expression_node> e;

    ASSERT_NO_THROW(e = parse_expression("foo.bar"));
    ASSERT_EQ(e->string(), "(foo . bar)");

    ASSERT_NO_THROW(e = parse_expression("foo.append(2, 3)"));
    ASSERT_EQ(e->string(), "((foo . append)(2, 3))");
}

TEST(Expression, Vector) {
    std::unique_ptr<expression_node> e;

    ASSERT_NO_THROW(e = parse_expression("[]"));
    ASSERT_EQ(e->string(), "[]");

    ASSERT_NO_THROW(e = parse_expression("[1]"));
    ASSERT_EQ(e->string(), "[1]");

    ASSERT_NO_THROW(e = parse_expression("[1, 2, 3]"));
    ASSERT_EQ(e->string(), "[1, 2, 3]");

    ASSERT_NO_THROW(e = parse_expression("[1, 2, 3,]"));
    ASSERT_EQ(e->string(), "[1, 2, 3]");
}

TEST(Expression, Map) {
    std::unique_ptr<expression_node> e;

    ASSERT_NO_THROW(e = parse_expression("{}"));
    ASSERT_EQ(e->string(), "{}");

    ASSERT_NO_THROW(e = parse_expression("{1: 1.1}"));
    ASSERT_EQ(e->string(), "{1: 1.1}");

    ASSERT_NO_THROW(e = parse_expression("{1: 1.1, 2: 2.2}"));
    ASSERT_EQ(e->string(), "{1: 1.1, 2: 2.2}");

    ASSERT_NO_THROW(e = parse_expression("{1: 1.1, 2: 2.2, }"));
    ASSERT_EQ(e->string(), "{1: 1.1, 2: 2.2}");
}
