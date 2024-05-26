// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "expected_optional.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(expected_optional) {

    [[nodiscard]] static hi::expected_optional<int, std::string> get_value() noexcept
    {
        return 42;
    }

    TEST_CASE(value_test) {
        auto tmp = get_value();

        REQUIRE(static_cast<bool>(tmp));
        REQUIRE(tmp.has_value());
        REQUIRE(not tmp.has_error());
        REQUIRE(*tmp == 42);
    }

    [[nodiscard]] static hi::expected_optional<int, std::string> get_nullopt() noexcept
    {
        return std::nullopt;
    }

    TEST_CASE(nullopt_test) {
        auto tmp = get_nullopt();

        REQUIRE(not tmp);
        REQUIRE(not tmp.has_value());
        REQUIRE(not tmp.has_error());
    }

    [[nodiscard]] static hi::expected_optional<int, std::string> get_error() noexcept
    {
        return std::unexpected{"foo"};
    }

    TEST_CASE(error_test) {
        auto tmp = get_error();

        REQUIRE(not tmp);
        REQUIRE(not tmp.has_value());
        REQUIRE(tmp.has_error());
        REQUIRE(tmp.error() == "foo");
    }
};
