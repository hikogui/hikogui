

#include "hikotest.hpp"
#include <utility>
#include <print>
#include <cassert>
#include <optional>
#include <filesystem>
#include <expected>

namespace test {

/** Get the number of suites and total number of tests.
 *
 * @return num_suites, total_num_tests
 */
[[nodiscard]] static std::pair<size_t, size_t> num_suites(filter const& filter) noexcept
{
    size_t num_suites = 0;
    size_t num_total_tests = 0;

    for (auto const& suite : suites) {
        if (filter.match_suite(suite.name)) {
            ++num_suites;
            num_total_tests += suite.num_tests(filter);
        }
    }
    return {num_suites, num_total_tests};
}

[[nodiscard]] static std::vector<suite_entry> list_suites(filter const& filter) noexcept
{
    auto r = std::ranges::to<std::vector>(std::views::filter(suites, [&](auto const& suite) {
        return filter.match_suite(suite.name);
    }));

    std::ranges::sort(r, [](auto const& a, auto const& b) {
        return a.name < b.name;
    });

    return r;
}



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




void trace::finish_suite() noexcept
{
    using namespace std::literals;

    auto const duration = clock_type::now() - _suite_tp;

}

void trace::start_check(std::string_view file, int line, std::optional<std::string> default_failure_message) noexcept
{
    _check_file = file;
    _check_line = line;
    _check_failure_message = default_failure_message;
}

bool trace::finish_check() noexcept
{
    if (_check_failure_message) {
        std::println(stdout, "{}({}): error: {}", _check_file, _check_line, *_check_failure_message);
        _test_failed = true;
        return false;
    } else {
        return true;
    }
}

bool trace::finish_check(std::optional<std::string> failure_message) noexcept
{
    _check_failure_message = failure_message;
    return finish_check();
}

} // namespace test

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
            std::println(stderr, "Unknown command line argument {}.\n", arg);
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

    for (auto &suite : suites) {
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

[[nodiscard]] static ::test::time_point_type run_tests_start(::test::filter const &filter) noexcept
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

    return ::test::clock_type::now();
}

[[nodiscard]] static void run_tests_finish(::test::filter const &filter, ::test::time_point_type time_point) noexcept
{
    using namespace std::literals;

    auto const duration = ::test::clock_type::now() - time_point;

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
}

static void run_tests(::test::filter const &filter)
{
    auto const time_point = run_tests_start(filter);

    for (auto &suite: ::test::suites) {
        if (filter.match_suite(suite.suite_name)) {
            suite.run_tests(filter);
        }
    }

    run_tests_finish(filter, time_point);    
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

static int list_tests(::test::filter const& filter) noexcept
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
    return 0;
}

int main(int argc, char *argv[])
{
    std::println(stdout, "Running main() from {}", __FILE__);
    parse_arguments(argc, argv);

    auto r = 0;
    if (option_list_tests) {
        r = list_tests(option_filter);
    } else {
        r = run_tests(option_filter);
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

    return r;
}