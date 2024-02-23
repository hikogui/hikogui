// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "dispatch.hpp"
#include <hikotest/hikotest.hpp>
#include <atomic>

TEST_SUITE(async_task_suite)
{
    static int my_function(int *x)
    {
        using namespace std::literals;

        std::this_thread::sleep_for(100ms);
        std::atomic_ref(*x).store(42);
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

    TEST_CASE(function2_test)
    {
        auto x = 0;
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};
        auto t = hi::cancelable_async_task(async_task_suite::my_function, stop_source.get_token(), progress_sink.get_token(), &x);

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(x == 42);
        REQUIRE(t.value() == 43);
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
        auto t = hi::cancelable_async_task(async_task_suite::my_function_stop_token, stop_source.get_token(), progress_sink.get_token(), &x);

        while (std::atomic_ref(x).load() == 0) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        auto const wait_until = std::chrono::utc_clock::now() + 100ms;
        while (std::chrono::utc_clock::now() < wait_until) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        stop_source.request_stop();

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 5);
        REQUIRE(t.value() == 6);
    }

    static int my_function_progress(hi::progress_token progress_token, int *x)
    {
        using namespace std::literals;

        progress_token = 0.0f;

        while (std::atomic_ref(*x) == 0) {
            std::this_thread::sleep_for(16ms);
        }

        progress_token = 0.5f;

        while (std::atomic_ref(*x) == 1) {
            std::this_thread::sleep_for(16ms);
        }

        progress_token = 1.0f;
        return 42;
    }

    TEST_CASE(progress_function_test)
    {
        using namespace std::literals;

        auto x = 0;
        std::atomic<float> y = 0.0f;
        auto x_ref = std::atomic_ref(x);
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};

        auto progress_cb = progress_sink.subscribe([&]() {
            y = progress_sink.value();
        });

        REQUIRE(progress_sink.value() == 0.0f);
        REQUIRE(y == 0.0f);

        auto t = hi::cancelable_async_task(async_task_suite::my_function_progress, stop_source.get_token(), progress_sink.get_token(), &x);

        x_ref = 1;

        while (y == 0.0f) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(progress_sink.value() == 0.5f);
        REQUIRE(y == 0.5f);

        x_ref = 2;

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(progress_sink.value() == 1.0f);
        REQUIRE(y == 1.0f);
        REQUIRE(t.value() == 42);
    }


    static hi::task<int> my_task(int *x)
    {
        using namespace std::literals;

        co_await 100ms;
        *x = 42;
        co_return *x + 1;
    }

    TEST_CASE(task_test)
    {
        auto x = 0;
        auto t = hi::async_task(async_task_suite::my_function, &x);

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(x == 42);
        REQUIRE(t.value() == 43);
    }

    TEST_CASE(task2_test)
    {
        auto x = 0;
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};
        auto t = hi::cancelable_async_task(async_task_suite::my_task, stop_source.get_token(), progress_sink.get_token(), &x);

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(x == 42);
        REQUIRE(t.value() == 43);
    }

    static hi::task<int> my_task_stop_token(std::stop_token stop_token, int *x)
    {
        *x = 42;
        co_await stop_token;
        *x = 5;
        co_return *x + 1;
    }

    TEST_CASE(cancelable_task_test)
    {
        using namespace std::literals;

        auto x = 0;
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};
        auto t = hi::cancelable_async_task(async_task_suite::my_task_stop_token, stop_source.get_token(), progress_sink.get_token(), &x);

        while (std::atomic_ref(x).load() == 0) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        auto const wait_until = std::chrono::utc_clock::now() + 100ms;
        while (std::chrono::utc_clock::now() < wait_until) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 42);

        stop_source.request_stop();

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(std::atomic_ref(x).load() == 5);
        REQUIRE(t.value() == 6);
    }

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
        auto stop_source = std::stop_source{};
        auto progress_sink = hi::progress_sink{};

        auto progress_cb = progress_sink.subscribe([&]() {
            y = progress_sink.value();
        });

        REQUIRE(progress_sink.value() == 0.0f);
        REQUIRE(y == 0.0f);

        auto t = hi::cancelable_async_task(async_task_suite::my_task_progress, stop_source.get_token(), progress_sink.get_token(), &x);

        x_ref = 1;

        while (y == 0.0f) {
            REQUIRE(not t.done());
            hi::loop::local().resume_once();
        }

        REQUIRE(progress_sink.value() == 0.5f);
        REQUIRE(y == 0.5f);

        x_ref = 2;

        while (not t.done()) {
            hi::loop::local().resume_once();
        }

        REQUIRE(progress_sink.value() == 1.0f);
        REQUIRE(y == 1.0f);
        REQUIRE(t.value() == 42);
    }
};