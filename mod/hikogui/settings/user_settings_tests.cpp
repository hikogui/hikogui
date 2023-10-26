// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "user_settings.hpp"
#include "../path/path.hpp"
#include "../concurrency/concurrency.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <format>

class user_settings_tests : public ::testing::Test {
protected:
    void SetUp() override
    {
        hi::set_application_name(std::format("hikogui_tests thread={}", hi::current_thread_id()));
        hi::set_application_vendor("HikoGUI");

        hi::delete_user_settings();
    }

    void TearDown() override
    {
        hi::delete_user_settings();
    }
};

TEST_F(user_settings_tests, is_null)
{
    auto result = hi::get_user_setting<int>("foo");
    ASSERT_FALSE(result);
    ASSERT_EQ(result.error(), std::errc::no_such_file_or_directory);
}

TEST_F(user_settings_tests, set_int_value)
{
    hi::set_user_setting("foo", 1);
    ASSERT_EQ(hi::get_user_setting<int>("foo"), 1);
}


TEST_F(user_settings_tests, overwrite_int_value)
{
    hi::set_user_setting("foo", 1);
    ASSERT_EQ(hi::get_user_setting<int>("foo"), 1);

    hi::set_user_setting("foo", 42);
    ASSERT_EQ(hi::get_user_setting<int>("foo"), 42);
}

TEST_F(user_settings_tests, delete_int_value)
{
    hi::set_user_setting("foo", 1);
    ASSERT_EQ(hi::get_user_setting<int>("foo"), 1);

    hi::delete_user_setting("foo");
    auto result = hi::get_user_setting<int>("foo");
    ASSERT_FALSE(result);
    ASSERT_EQ(result.error(), std::errc::no_such_file_or_directory);
}
