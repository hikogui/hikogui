

#include "hikotest.hpp"
#include <utility>
#include <print>
#include <cassert>
#include <optional>
#include <filesystem>
#include <expected>

namespace test {

[[nodiscard]] bool filter::match_test(std::string_view suite_name, std::string_view test_name) const noexcept
{
    return true;
}

[[nodiscard]] bool filter::match_suite(std::string_view suite_name) const noexcept
{
    return true;
}

[[nodiscard]] static std::expected<filter, std::string> parse_filter(std::string_view str) noexcept
{
    return std::unexpected{"Not implemented"};
}

}

static bool option_list_tests = false;
static ::test::filter option_filter = {};
static std::optional<std::filesystem::path> option_xml_output_path = std::nullopt;

[[noreturn]] static void print_help(int exit_code) noexcept
{
    std::println(stdout, "This program contains tests written using HikoTest.");
    std::println(stdout, "You can use the following command line flags to control its behaviour:");
    std::println(stdout, "");
    std::println(stdout, "Test Selection:");
    std::println(stdout, "  --gtest_list_tests");
    std::println(stdout, "      List the names of all tests instead of running them.");
    std::println(stdout, "  --gtest_filter=POSITIVE_PATTERNS[-NEGATIVE_PATTERNS]");
    std::println(stdout, "      Run only the tests whose name matches one of the patterns.");
    std::println(stdout, "");
    std::println(stdout, "Test Output:");
    std::println(stdout, "  --gtest_output=xml[:FILE_PATH]");
    std::println(stdout, "      Generate a XML report with the given file name.");
    std::exit(exit_code);
}

static void parse_arguments(int argc, char *argv[]) noexcept
{
    if (argc == 0) {
        std::println(stderr, "Empty argument list, expect at least the executable name in argv[0].");
        std::terminate();
    }

    for (auto i = 1; i != argc; ++i) {
        auto const arg = std::string_view{argv[i]};
        if (arg == "--help") {
            print_help(0);

        } else if (arg.starts_with("--gtest_color=")) {
            continue;

        } else if (arg == "--gtest_also_run_disabled_tests") {
            continue;

        } else if (arg == "--gtest_break_on_failure") {
            continue;

        } else if (arg == "--gtest_list_tests") {
            option_list_tests = true;

        } else if (arg.starts_with("--gtest_filter=")) {
            if (auto const filter = ::test::parse_filter(arg.substr(15))) {
                option_filter = *filter;

            } else {
                std::println(stderr, "error: {}.\n", filter.error());
                print_help(2);
            }
        } else if (arg.starts_with("--gtest_output=xml:")) {
            option_xml_output_path = std::filesystem::path{arg.substr(19)};

        } else {
            std::println(stderr, "Unknown command line argument {}.", arg);
            std::println(stderr, "These are the command line argument given:");
            for (auto j = 1; j != argc; ++j) {
                std::println(stderr, "  {}", argv[j]);
            }
            std::println(stderr, "");
            print_help(2);
        }
    }
}

struct collect_stats_result {
    size_t num_tests;
    size_t num_suites;
    size_t num_failures;
    std::vector<std::string> failed_tests;
};

[[nodiscard]] static collect_stats_result collect_stats(::test::filter const &filter) noexcept
{
    auto r = collect_stats_result{};

    for (auto &suite : ::test::suites) {
        if (filter.match_suite(suite.suite_name)) {
            auto const stats = suite.collect_stats(filter);

            ++r.num_suites;
            r.num_tests += stats.num_tests;
            r.num_failures += stats.num_failures;
            r.failed_tests.append_range(stats.failed_tests);
        }
    }

    return r;
}

[[nodiscard]] static ::test::hr_time_point_type run_tests_start(::test::filter const &filter) noexcept
{
    auto const stats = collect_stats(filter);

    std::println(
        stdout,
        "[==========] Running {} {} from {} test {}.",
        stats.num_tests,
        stats.num_tests == 1 ? "test" : "tests",
        stats.num_suites,
        stats.num_suites == 1 ? "suite" : "suites");
    std::println(stdout, "[----------] Global test environment set-up.");
    std::fflush(stdout);

    return ::test::hr_clock_type::now();
}

[[nodiscard]] static bool run_tests_finish(::test::filter const &filter, ::test::hr_time_point_type time_point) noexcept
{
    using namespace std::literals;

    auto const duration = ::test::hr_clock_type::now() - time_point;

    auto const stats = collect_stats(filter);

    std::println(stdout, "[----------] Global test environment tear-down.");
    std::println(
        stdout,
        "[==========] {} {} from {} test {} ran. ({} ms total)",
        stats.num_tests,
        stats.num_tests == 1 ? "test" : "tests",
        stats.num_suites,
        stats.num_suites == 1 ? "suite" : "suites",
        duration / 1ms);

    auto const num_passed = stats.num_tests - stats.num_failures;
    if (num_passed != 0) {
        std::println(stdout, "[  PASSED  ] {} {}.", num_passed, num_passed == 1 ? "test" : "tests");
    }

    if (stats.num_failures != 0) {
        std::println(
            stdout,
            "[  FAILED  ] {} {}, listed below:",
            stats.num_failures,
            stats.num_failures == 1 ? "test" : "tests");

        for (auto const& failed_test : stats.failed_tests) {
            std::println(stdout, "[  FAILED  ] {}", failed_test);
        }
    }

    return stats.num_failures == 0;
}

static bool run_tests(::test::filter const &filter)
{
    auto const time_point = run_tests_start(filter);

    for (auto &suite: ::test::suites) {
        if (filter.match_suite(suite.suite_name)) {
            suite.run_tests(filter);
        }
    }

    return run_tests_finish(filter, time_point);    
}

static void generate_junit_xml(::test::filter const &filter, FILE *out)
{
    auto const stats = collect_stats(filter);

    std::println(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    std::println(out, "<testsuites test=\"{}\" name=\"AllTests\">", stats.num_tests);

    for (auto &suite: ::test::suites) {
        if (filter.match_suite(suite.suite_name)) {
            suite.generate_junit_xml(filter, out);
        }
    }

    std::println(out, "</testsuites>");
}

static void list_tests(::test::filter const& filter) noexcept
{
    for (auto const& suite : ::test::suites) {
        if (filter.match_suite(suite.suite_name)) {
            std::println(stdout, "{}.", suite.suite_name);
            for (auto const &test : suite.tests) {
                if (filter.match_test(test.suite_name, test.test_name)) {
                    std::println(stdout, "  {}", test.test_name);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    std::println(stdout, "Running main() from {}", __FILE__);
    parse_arguments(argc, argv);

    auto success = true;
    if (option_list_tests) {
        list_tests(option_filter);
    } else {
        success = run_tests(option_filter);
    }

    FILE *xml_output = nullptr;
    if (option_xml_output_path) {
        xml_output = fopen(option_xml_output_path->string().c_str(), "w");
        if (xml_output == nullptr) {
            std::println(stdout, "Could not open xml-file {}", option_xml_output_path->string());
            print_help(2);
        }

        generate_junit_xml(option_filter, xml_output);

        if (fclose(xml_output) != 0) {
            std::println(stdout, "Could not close xml-file {}", option_xml_output_path->string());
            print_help(1);
        }

    }

    return success ? 0 : 1;
}