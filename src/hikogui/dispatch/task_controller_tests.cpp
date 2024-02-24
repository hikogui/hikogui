// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "dispatch.hpp"
#include <hikotest/hikotest.hpp>
#include <atomic>

TEST_SUITE(task_controller_suite)
{
    static hi::task<int> my_task_progress(hi::progress_token progress_token, int *x)
    {
        using namespace std::literals;

        progress_token = 0.0f;

        while (std::atomic_ref(*x) == 0) {
            co_await 16ms;
        }

        progress_token = 0.5f;

        while (std::atomic_ref(*x) == 1) {
            co_await 16ms;
        }

        progress_token = 1.0f;
        co_return 42;
    }

    TEST_CASE(progress_task_test)
    {
        using namespace std::literals;

        auto x = 0;
        std::atomic<float> y = 0.0f;
        auto x_ref = std::atomic_ref(x);

        auto tc = hi::task_controller<int>{};
        REQUIRE(not tc.runnable());
        REQUIRE(tc.features() == hi::cancel_features_type::none);

        auto progress_cb = tc.subscribe([&]() {
            y = tc.progress();
        });

        REQUIRE(tc.progress() == 0.0f);
        REQUIRE(y == 0.0f);

        tc.set_function(task_controller_suite::my_task_progress, &x);
        REQUIRE(tc.runnable());
        REQUIRE(tc.progress() == 0.0f);
        REQUIRE(y == 0.0f);
        REQUIRE(tc.features() == hi::cancel_features_type::progress);

        tc.run();
        x_ref = 1;
        while (y == 0.0f) {
            REQUIRE(not tc.done());
            hi::loop::local().resume_once();
        }
        REQUIRE(tc.progress() == 0.5f);
        REQUIRE(y == 0.5f);

        x_ref = 2;
        while (not tc.done()) {
            hi::loop::local().resume_once();
        }
        REQUIRE(tc.progress() == 1.0f);
        REQUIRE(y == 1.0f);
        REQUIRE(tc.value() == 42);
    }

    static int my_function_stop_token(std::stop_token stop_token, int *x)
    {
        using namespace std::literals;

        std::atomic_ref(*x).store(42);
        while (not stop_token.stop_requested()) {
            std::this_thread::sleep_for(16ms);
        }
        std::atomic_ref(*x).store(5);
        return *x + 1;
    }

    TEST_CASE(cancelable_function_test)
    {
        using namespace std::literals;

        auto x = 0;
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};

        auto tc = hi::task_controller<int>{};
        REQUIRE(not tc.runnable());
        REQUIRE(tc.features() == hi::cancel_features_type::none);

        tc.set_function(task_controller_suite::my_function_stop_token, &x);
        REQUIRE(tc.runnable());
        REQUIRE(tc.features() == hi::cancel_features_type::stop);

        tc.run();
        while (std::atomic_ref(x).load() == 0) {
            REQUIRE(not tc.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        auto const wait_until = std::chrono::utc_clock::now() + 100ms;
        while (std::chrono::utc_clock::now() < wait_until) {
            REQUIRE(not tc.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        tc.request_stop();

        while (not tc.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 5);
        REQUIRE(tc.value() == 6);
    }
};
