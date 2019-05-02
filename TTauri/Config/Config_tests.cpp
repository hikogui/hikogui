// Copyright 2019 Pokitec
// All rights reserved.

#include <gtest/gtest.h>
#include <Windows.h>
#include "Config.hpp"

using namespace std;
using namespace TTauri::Config;

/*
a = 1;

[foo.bar]
b = 2;
c = [1,2,3];
d = [
{name:"foo", value:3},
{name:"bar", value:5.4},
{name:"baz", value:"nein"}
];
*/
TEST(TTauriConfigConfig, ConfigTest) {
    try {
        auto config = Config("Config/TestFiles/config_test.txt");
        ASSERT_TRUE(config.success());

        // Accessing
        ASSERT_EQ(config.value<int64_t>("a"), 1);
        ASSERT_EQ(config.value<int64_t>("foo.bar.b"), 2);
        ASSERT_EQ(config.value<int64_t>("foo.bar.c.2"), 3);
        ASSERT_EQ(config.value<int64_t>("foo.bar.d.0.value"), 3);

        // Promoting
        ASSERT_EQ(config.value<double>("a"), 1.0);
        ASSERT_EQ(config.value<std::filesystem::path>("foo.bar.d.2.value"), std::filesystem::path("nein"));

    } catch (boost::exception &e) {
        std::cerr << boost::diagnostic_information(e);
        throw;
    }
}

