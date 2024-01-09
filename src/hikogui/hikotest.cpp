
#pragma once

#include "hikotest.hpp"


namespace test {


void result::start_global(size_t num_suites, size_t num_total_tests) noexcept
{
    _num_suites = num_suites;
    _num_total_tests = num_total_tests;

    std::println(
        stdout,
        "[==========] Running {} {} from {} test {}.",
        _num_total_tests,
        _num_total_tests == 1 ? "test" : "tests",
        _num_suites,
        _num_suites = 1 ? "suite" : "suites");
    std::println(stdout, "[----------] Global test environment set-up.");
    std::fflush(stdout);
    _global_ts = clock_type::now();
}

void result::finish_global() noexcept
{
    using namespace std::literal;

    auto const duration = clock_type::now() - global_ts;

    std::println(stdout, "[----------] Global test environment tear-down.");
    std::println(
        stdout,
        "[==========] {} {} from {} test {} ran. ({} ms total)",
        _num_total_tests,
        _num_total_tests == 1 ? "test" : "tests",
        _num_suites,
        _num_suites = 1 ? "suite" : "suites",
        duration / 1 ms);

    auto const num_total_tests_passed = _num_total_tests - _failed_tests_summary.size();
    if (num_total_tests_passed != 0) {
        std::println(stdout, "[  PASSED  ] {} {}.", num_total_tests_passed, num_total_tests_passed == 1 ? "test" : "tests");
    }

    if (not _failed_tests_summary.empty()) {
        std::println(
            stdout,
            "[  FAILED  ] {} {}, listed below:",
            _failed_tests_summary.size(),
            _failed_tests_summary.size() ? "test" : "tests");

        for (auto const& failed_test : _failed_tests_summary) {
            std::println(stdout, "[  FAILED  ] {}", failed_test);
        }
    }

    std::fflush(stdout);
}

void result::start_suite(char const *file, int line, char const *suite_name, size_t num_tests) noexcept
{
    _suite_file = file;
    _suite_line = line;
    _suite_name = suite_name;
    _suite_num_tests = num_tests;

    std::println(
        stdout, "[----------] {} {} from {}", _suite_num_tests, _suite_num_tests == 1 ? "test" : "tests", _suite_name);
    std::fflush(stdout);
    _suite_tp = clock_type::now();
}

bool result::finish_suite() noexcept
{
    using namespace std::literal;

    auto const duration = clock_type::now() - _suite_tp;

    std::println(
        stdout,
        "[----------] {} {} from {} ({} ms total)",
        _suite_num_tests,
        _suite_num_tests == 1 ? "test" : "tests",
        _suite_name,
        duration / 1ms);
    std::fflush(stdout);
}

void result::start_test(char const *file, int line, char const *test_name) noexcept
{
    _test_file = file;
    _test_line = line;
    _test_name = test_name;
    _test_failed = false;

    std::println(stdout, "[ RUN      ] {}.{}", _suite_name, _test_name);
    std::fflush(stdout);
    _test_tp = clock_type::now();
}

bool result::finish_test() noexcept
{
    using namespace std::literal;

    auto const duration = clock_type::now() - _test_tp;

    if (_test_failed) {
        _failed_tests_summary.push_back(std::format("{}.{}", _suite_name, _test_name));
        std::println(stdout, "[  FAILED  ] {}.{} ({} ms)", _suite_name, _test_name, duration / 1ms);
        std::fflush(stdout);

    } else {
        std::println(stdout, "[       OK ] {}.{} ({} ms)", _suite_name, _test_name, duration / 1ms);
        std::fflush(stdout);
    }
}

void result::start_check(
    char const *file,
    int line,
    char const *expression_str,
    std::optional<std::string> default_failure_message = std::string{"check did not run."}) noexcept
{
    _check_file = file;
    _check_line = line;
    _check_expression_str = expression_str;
    _check_failure = default_failure_message;
}

bool result::finish_check() noexcept
{
    auto const check_finish_tp = clock_type::now();
    auto const check_duration = check_finish_tp = _check_start_tp;

    if (_check_failure) {
        std::println(stdout, "{}({}): error: {}", _check_file, _check_line, *_check_failure);
        _test_failed = true;
        return false;
    } else {
        return true;
    }
}






}

