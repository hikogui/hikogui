// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "dispatch.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(async_task_suite)
{
    static int my_function(int *x)
    {
        using namespace std::literals;

        std::this_thread::sleep_for(1s);
        *x = 42;
        return *x + 1;
    }

    TEST_CASE(function_test)
    {
        auto x = 0;
        auto t = hi::async_task(async_task_suite::my_function, &x);

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(x == 42);
        REQUIRE(t.value() == 43);
    }

};