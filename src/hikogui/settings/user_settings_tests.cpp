// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "user_settings.hpp"
#include "../path/path.hpp"
#include "../concurrency/concurrency.hpp"
#include <hikotest/hikotest.hpp>
#include <format>

TEST_SUITE(user_settings) {
user_settings()
{
    hi::set_application_name(std::format("hikogui_tests thread={}", hi::current_thread_id()));
    hi::set_application_vendor("HikoGUI");

    hi::delete_user_settings();
}

~user_settings()
{
    hi::delete_user_settings();
}

TEST_CASE(is_null)
{
    auto result = hi::get_user_setting<int>("foo");
    REQUIRE(not result);
    REQUIRE(result.error() == std::errc::no_such_file_or_directory);
}

TEST_CASE(set_int_value)
{
    hi::set_user_setting("foo", 1);
    REQUIRE((hi::get_user_setting<int>("foo") == 1));
}

TEST_CASE(overwrite_int_value)
{
    hi::set_user_setting("foo", 1);
    REQUIRE((hi::get_user_setting<int>("foo") == 1));

    hi::set_user_setting("foo", 42);
    REQUIRE((hi::get_user_setting<int>("foo") == 42));
}

TEST_CASE(delete_int_value)
{
    hi::set_user_setting("foo", 1);
    REQUIRE((hi::get_user_setting<int>("foo") == 1));

    hi::delete_user_setting("foo");
    auto result = hi::get_user_setting<int>("foo");
    REQUIRE(not result);
    REQUIRE(result.error() == std::errc::no_such_file_or_directory);
}

}; // TEST_SUITE(user_settings)
