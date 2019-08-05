// Copyright 2019 Pokitec
// All rights reserved.

#include <gtest/gtest.h>
#include <Windows.h>
#include "Config.hpp"

using namespace std;
using namespace TTauri::Config;

TEST(TTauriConfigConfig, ConfigTest) {
    try {
        auto config = Config("Config/TestFiles/config_test.txt");
        //ASSERT_TRUE(config.success());

        // Accessing
        //ASSERT_EQ(config.value<int64_t>("a"), 1);
        //ASSERT_EQ(config.value<int64_t>("foo.bar.b"), 2);
        //ASSERT_EQ(config.value<int64_t>("foo.bar.c.2"), 3);
        //ASSERT_EQ(config.value<int64_t>("foo.bar.d.0.value"), 3);

        // Promoting
        //ASSERT_EQ(config.value<double>("a"), 1.0);
        //ASSERT_EQ(config.value<boost::filesystem::path>("foo.bar.d.2.value"), boost::filesystem::path("nein"));

        // Modifying
        //config["foo.bar.d.0.value"] = "hello"s;
        //ASSERT_EQ(config.value<std::string>("foo.bar.d.0.value"), "hello"s);

    } catch (boost::exception &e) {
        std::cerr << boost::diagnostic_information(e);
        throw;
    }
}

TEST(TTauriConfigConfig, SyntaxError) {
    try {
        {
            auto config = Config("Config/TestFiles/syntax_error.txt");
            ASSERT_TRUE(!config.success());
            ASSERT_EQ(config.error(), "Config/TestFiles/syntax_error.txt:4:1: syntax error, unexpected T_IDENTIFIER.");
        }

        {
            auto config = Config("Config/TestFiles/include_syntax_error.txt");
            ASSERT_TRUE(!config.success());
            ASSERT_EQ(config.error(),
                "Config/TestFiles/syntax_error.txt:4:1: syntax error, unexpected T_IDENTIFIER.\n"
                "Config/TestFiles/include_syntax_error.txt:2:1: Could not include file 'Config/TestFiles/syntax_error.txt'.");
        }

    } catch (boost::exception &e) {
        std::cerr << boost::diagnostic_information(e);
        throw;
    }
}

