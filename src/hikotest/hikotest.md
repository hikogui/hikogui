HikoTest Unit Testing Framework
===============================

I created my own unit testing framework since I had several issues
with other unit-testing frame works with testing the HikoGUI library.

Design decissions:
 * Do not catch exceptions or other traps. To make it easy to
   debug crashing tests.
 * Limit the amount of stack space required. To not cause the
   address sanitizer to trigger on stack overflow with a bunch of simple tests.
 * Comparisons for: values, ranges, absolute difference and ranges
   of absolute difference.
 * Print operands of comparisons using `std::format()`, `std::ostream()`,
   `to_string()`, `.string()` or `.str()`.
 * Command line and output compatible with google-test so that
   is can be used with IDEs.
 * Very low number of test macros: `REQUIRE()` and `REQUIRE_THROW()`.
 * A test-suite is a class, a test is a member-function.
  

Example test suite:
```cpp
#include <hikotest/hikotest.hpp>

class my_suite : public ::test::suite<my_suite> {
public:
    TEST_CASE(first_test)
    {
        auto foo = 42;
        auto bar = 42.2f;

        ASSERT(foo == 42);
        ASSERT(0.5, bar == 42);
    }

    TEST_CASE(second_test)
    {
        auto foo = std::array{1, 42};
        auto bar = std::array{0.9f, 42.2f};

        ASSERT(foo == std::array{1, 42});
        ASSERT(0.5, bar == std::array{1, 42});
    }

    TEST_CASE(third_test)
    {
        ASSERT_THROW(std::runtime_error, throw std::runtime_error{"oops"});
    }
};
```

Asserts
-------

### ASSERT(expression, [error])

Expression may be:
 * A comparison expression using one of the following operators: `==`, `!=`, `<`, `>`, `<=`, `>=`.
 * A boolean expression; resulting in a type that can be explicitly converted to `bool`.

You may escape a comparison expression by surrounding it with parentheses so that
it will be interpreted as a boolean expression.

Internally `ASSERT()` will use the spaceship operator `<=>` on the left side of the expression.
This is done to separate the left operand from the comparison, so that the comparison can
be replaced by the unit testing framework.

By `error` argument may be one of the following:
 * If no `error` argument is given then an exact match is expected.
 * If a floating point number if used as the `error` argument then
   the match is done by subtracted the operands and comparing it with
   the `error` value.
 * If a `::test::error<>` class is used then comparison is done using
   the error value.

### ASSERT\_THROW(expression)

Command Line Arguments
----------------------

 :--------------------------- |:---------------------------------
  `--help`                    | List all command line arguments.
  `--gtest_list_tests`        | List all the tests in the executable.
  `--gtest_filter=<filter>`   | Filter the tests to be run.
  `--gtest_output=xml:<path>` | Write a JUnit XML file to `<path>`.

### filter

A filter contains two parts:
 * On the left side a colon ':' separated list of inclusion glob patterns
 * On the right side a minus '-' sign followed by a colon ':' separated list of exclusion glob pattern.

Both the inclusion list and exclusion lists are optional.

The glob pattern may contain:
 * A character that is part of a C++ identifier
 * A single '.' to separate the 'suite' and 'test'.
 * A '?' to replace a single character
 * A '*' to replace zero or more characters.

