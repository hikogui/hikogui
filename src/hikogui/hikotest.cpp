

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
[[nodiscard]] static std::pair<size_t, size_t> num_suites(filter const &filter) noexcept
{
    size_t num_suites = 0;
    size_t num_total_tests = 0;

    for (auto const &suite: suites) {
        if (filter.match_suite(suite.name)) {
            ++num_suites;
            num_total_tests += suite.num_tests(filter);
        }
    }
    return {num_suites, num_total_tests};
}

[[nodiscard]] static std::vector<suite_entry> list_suites(filter const &filter) noexcept
{
    auto r = std::ranges::to<std::vector>(std::views::filter(suites, [&](auto const &suite) {
        return filter.match_suite(suite.name);
    }));

    std::ranges::sort(r, [](auto const &a, auto const &b) {
        return a.name < b.name;
    });

    return r;
}

static void run_suites(trace &r, filter const &filter)
{
    auto const [num_suites, num_total_tests] = ::test::num_suites(filter);

    r.start_global(num_suites, num_total_tests);

    for (auto const &suite: list_suites(filter)) {
        suite.run_tests(r, filter);
    }

    r.finish_global();
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



void trace::start_global(size_t num_suites, size_t num_total_tests) noexcept
{
    _num_suites = num_suites;
    _num_total_tests = num_total_tests;

    std::println(
        stdout,
        "[==========] Running {} {} from {} test {}.",
        _num_total_tests,
        _num_total_tests == 1 ? "test" : "tests",
        _num_suites,
        _num_suites == 1 ? "suite" : "suites");
    std::println(stdout, "[----------] Global test environment set-up.");
    std::fflush(stdout);

    _global_tp = clock_type::now();
}

void trace::finish_global() noexcept
{
    using namespace std::literals;

    auto const duration = clock_type::now() - _global_tp;

    std::println(stdout, "[----------] Global test environment tear-down.");
    std::println(
        stdout,
        "[==========] {} {} from {} test {} ran. ({} ms total)",
        _num_total_tests,
        _num_total_tests == 1 ? "test" : "tests",
        _num_suites,
        _num_suites == 1 ? "suite" : "suites",
        duration / 1ms);

    auto const num_total_tests_passed = _num_total_tests - _failed_tests_summary.size();
    if (num_total_tests_passed != 0) {
        std::println(stdout, "[  PASSED  ] {} {}.", num_total_tests_passed, num_total_tests_passed == 1 ? "test" : "tests");
    }

    if (not _failed_tests_summary.empty()) {
        std::println(
            stdout,
            "[  FAILED  ] {} {}, listed below:",
            _failed_tests_summary.size(),
            _failed_tests_summary.size() == 1 ? "test" : "tests");

        for (auto const& failed_test : _failed_tests_summary) {
            std::println(stdout, "[  FAILED  ] {}", failed_test);
        }
    }

    std::fflush(stdout);
}

void trace::start_suite(std::string_view suite_name, size_t num_tests) noexcept
{
    _suite_name = suite_name;
    _suite_num_tests = num_tests;

    std::println(
        stdout, "[----------] {} {} from {}", _suite_num_tests, _suite_num_tests == 1 ? "test" : "tests", _suite_name);
    std::fflush(stdout);
    _suite_tp = clock_type::now();
}

void trace::finish_suite() noexcept
{
    using namespace std::literals;

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

void trace::start_test(std::string_view file, int line, std::string_view test_name) noexcept
{
    _test_file = file;
    _test_line = line;
    _test_name = test_name;
    _test_failed = false;

    std::println(stdout, "[ RUN      ] {}.{}", _suite_name, _test_name);
    std::fflush(stdout);
    _test_tp = clock_type::now();
}

void trace::finish_test() noexcept
{
    using namespace std::literals;

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

void trace::start_check(
    std::string_view file,
    int line,
    std::optional<std::string> default_failure_message) noexcept
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

static int list_tests(::test::filter const &filter, FILE *output_xml) noexcept
{
    if (output_xml) {
        std::println(output_xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        std::println(output_xml, "<testsuites test=\"{}\" name=\"AllTests\">", ::test::num_suites(filter).second);
    }

    for (auto const &test_suite : ::test::list_suites(filter)) {
        std::println(stdout, "{}.", test_suite.name);
        if (output_xml) {
            std::println(output_xml, "  <testsuite name=\"{}\" tests=\"{}\">", test_suite.name, test_suite.num_tests(filter));
        }

        for (auto const &test : test_suite.list_tests(filter)) {
            std::println(stdout, "  {}", test.name);
            if (output_xml) {
                std::println(output_xml, "    <testcase name=\"{}\" file=\"{}\" line=\"{}\" />", test.name, test.file, test.line);
            }
        }

        if (output_xml) {
            std::println(output_xml, "  </testsuite>");
        }
    }

    if (output_xml) {
        std::println(output_xml, "</testsuites>");
    }
    return 0;
}

static int run_tests(::test::filter filter, FILE *output_xml) noexcept
{
    auto trace = ::test::trace{output_xml};
    if (::test::run_suites(trace, filter)) {
        return 0;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[])
{
    std::println(stdout, "Running main() from {}", __FILE__);
    parse_arguments(argc, argv);

    FILE *xml_output = nullptr;
    if (option_xml_output_path) {
        xml_output = fopen(option_xml_output_path->string().c_str(), "w");
        if (xml_output == nullptr) {
            std::println(stdout, "Could not open xml-file {}", option_xml_output_path->string());
            print_help(2);
        }
    }

    auto r = 0;
    if (option_list_tests) {
        r = list_tests(option_filter, xml_output);
    } else {
        r = run_tests(option_filter, xml_output);
    }

    if (xml_output != nullptr) {
        if (fclose(xml_output) != 0) {
            std::println(stdout, "Could not close xml-file {}", option_xml_output_path->string());
            print_help(1);
        }
    }

    return r;
}