

#include "hikotest.hpp"
#include <utility>
#include <print>
#include <optional>
#include <filesystem>
#include <expected>
#include <algorithm>

namespace test {

[[nodiscard]] std::string type_name_strip(std::string type)
{
    // Remove leading `struct` or `class` keyword.
    // This is quick and possibly will allow small-string-optimization.
    if (type.starts_with("struct ")) {
        type = type.substr(7);
    } else if (type.starts_with("class ")) {
        type = type.substr(6);
    }

    // Remove `struct` and `class` keywords and remove spaces.
    for (auto i = type.find_first_of("<, "); i != type.npos; i = type.find_first_of("<, ", i)) {
        if (type[i] == ' ') {
            type.erase(i, 1);
        } else {
            ++i;
        }

        if (type.substr(i).starts_with("struct ")) {
            type.erase(i, 7);
        } else if (type.substr(i + 1).starts_with("class ")) {
            type.erase(i, 6);
        }
    }

    return type;
}

[[nodiscard]] static std::string xml_escape(std::string str, char quote_char = '\0') noexcept
{
    for (auto it = str.begin(); it != str.end(); ++it) {
        auto const c = *it;

        if (c == '"' and quote_char == '"') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&quot;"});
            it += 5;

        } else if (c == '\'' and quote_char == '\'') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&apos;"});
            it += 5;

        } else if (c == '<') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&lt;"});
            it += 3;

        } else if (c == '>') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&qt;"});
            it += 3;

        } else if (c == '&') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&amp;"});
            it += 4;
        }
    }

    return str;
}

filter::filter(std::string_view str)
{
    enum class state_type { include_start, include_suite, include_test, exclude_suite, exclude_test };

    auto suite_name = std::string{};
    auto test_name = std::string{};
    auto state = state_type::include_start;

    auto commit = [&](state_type const next_state) {
        if (suite_name == "*") {
            suite_name.clear();
        }
        if (test_name == "*") {
            test_name.clear();
        }

        if (suite_name.contains("*")) {
            throw std::runtime_error("The suite-name in a filter may be '*' but can not contain '*'.");
        }
        if (test_name.contains("*")) {
            throw std::runtime_error("The test-name in a filter may be '*' but can not contain '*'.");
        }

        if (state == state_type::include_start or state == state_type::include_suite or state == state_type::include_test) {
            this->inclusions.emplace_back(suite_name, test_name);
        } else if (state == state_type::exclude_suite or state == state_type::exclude_test) {
            this->exclusions.emplace_back(suite_name, test_name);
        } else {
            std::terminate();
        }

        suite_name.clear();
        test_name.clear();
        state = next_state;
    };

    for (auto const c : str) {
        switch (state) {
        case state_type::include_start:
            if (c == '-') {
                state = state_type::exclude_suite;
            } else if (c == '.') {
                state = state_type::include_test;
            } else if (c == ':') {
                commit(state_type::include_start);
            } else {
                suite_name += c;
                state = state_type::include_suite;
            }
            break;
        case state_type::include_suite:
            if (c == '.') {
                state = state_type::include_test;
            } else if (c == ':') {
                commit(state_type::include_start);
            } else {
                suite_name += c;
            }
            break;
        case state_type::include_test:
            if (c == '.') {
                throw std::runtime_error("dot '.' in test-name is not valid in filter.");
            } else if (c == ':') {
                commit(state_type::include_start);
            } else {
                test_name += c;
            }
            break;

        case state_type::exclude_suite:
            if (c == '.') {
                state = state_type::exclude_test;
            } else if (c == ':') {
                commit(state_type::exclude_suite);
            } else {
                suite_name += c;
            }
            break;

        case state_type::exclude_test:
            if (c == '.') {
                throw std::runtime_error("dot '.' in test-name is not valid in filter.");
            } else if (c == ':') {
                commit(state_type::exclude_suite);
            } else {
                test_name += c;
            }
            break;
        default:
            std::terminate();
        }
    }
    commit(state_type::include_start);
}

[[nodiscard]] bool filter::match_test(std::string_view suite_name, std::string_view test_name) const noexcept
{
    if (std::none_of(inclusions.begin(), inclusions.end(), [&](auto const& item) {
            return (item.suite_name.empty() or item.suite_name == suite_name) and
                (item.test_name.empty() or item.test_name == test_name);
        })) {
        return false;
    }

    if (std::any_of(exclusions.begin(), exclusions.end(), [&](auto const& item) {
            return (item.suite_name.empty() or item.suite_name == suite_name) and
                (item.test_name.empty() or item.test_name == test_name);
        })) {
        return false;
    }

    return true;
}

[[nodiscard]] bool filter::match_suite(std::string_view suite_name) const noexcept
{
    if (std::none_of(inclusions.begin(), inclusions.end(), [&](auto const& item) {
            return item.suite_name.empty() or item.suite_name == suite_name;
        })) {
        return false;
    }

    if (std::any_of(exclusions.begin(), exclusions.end(), [&](auto const& item) {
            return item.suite_name == suite_name and item.test_name.empty();
        })) {
        return false;
    }

    return true;
}

test_case::result_type::result_type(test_case const* parent) noexcept :
    parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
{
}

[[nodiscard]] std::string test_case::result_type::suite_name() const noexcept
{
    return parent->suite_name;
}

[[nodiscard]] std::string test_case::result_type::test_name() const noexcept
{
    return parent->test_name;
}

[[nodiscard]] std::string_view test_case::result_type::file() const noexcept
{
    return parent->file;
}

[[nodiscard]] int test_case::result_type::line() const noexcept
{
    return parent->line;
}

[[nodiscard]] bool test_case::result_type::success() const noexcept
{
    return completed and error_message.empty();
}

[[nodiscard]] bool test_case::result_type::failure() const noexcept
{
    return completed and not error_message.empty();
}

[[nodiscard]] bool test_case::result_type::skipped() const noexcept
{
    return not completed;
}

void test_case::result_type::set_success() noexcept
{
    duration = hr_clock_type::now() - time_point;
    completed = true;
}

void test_case::result_type::set_failure(std::string message) noexcept
{
    duration = hr_clock_type::now() - time_point;
    error_message = message;
    completed = true;
}

void test_case::result_type::junit_xml(FILE* out) const noexcept
{
    using namespace std::literals;

    std::print(
        out, "    <testcase name=\"{}\" file=\"{}\" line=\"{}\" classname=\"{}\" ", test_name(), file(), line(), suite_name());

    if (completed) {
        std::print(
            out,
            "status=\"run\" result=\"completed\" time=\"{:.3f}\" timestamp=\"{:%Y-%m-%dT%H:%M:%S}\"",
            duration / 1s,
            time_stamp);

        if (not error_message.empty()) {
            std::print(out, "      <failure message=\"{}\" type=\"\">", xml_escape(error_message, '"'));
            std::println(out, "<![CDATA[{}]]></failure>", xml_escape(error_message));
            std::println(out, "    </testcase>");

        } else {
            std::println(out, "/>");
        }
    } else {
        std::println(out, "/>");
    }
}

[[nodiscard]] test_case::result_type test_case::run_test_break() const
{
    auto r = result_type{this};
    _run_test();
    r.set_success();
    return r;
}

[[nodiscard]] test_case::result_type test_case::run_test_catch() const
{
    auto r = result_type{this};
    try {
        _run_test();
        r.set_success();
    } catch (require_error const &e) {
        r.set_failure(e.what());
    } catch (std::exception const &e) {
        r.set_failure(std::format("{}({}): error: Unexpected exception thrown: {}.", file, line, e.what());
    } catch (...) {
        r.set_failure(std::format("{}({}): error: Unexpected unknown-exception thrown.", file, line));
    }
    return r;
}

[[nodiscard]] test_case::result_type test_case::run_test() const
{
    using namespace std::literals;

    std::println(stdout, "[ RUN      ] {}.{}", suite_name, test_name);
    std::fflush(stdout);

    auto r = break_on_failure ? run_test_break() : run_test_catch();

    auto result_str = r ? "[       OK ]" : "[  FAILED  ]";
    std::println(stdout, "{} {}.{} ({:.0f} ms)", result_str, suite_name, test_name, r.duration / 1ms);
    std::fflush(stdout);

    return r;
}

[[nodiscard]] test_case::result_type test_case::layout() const noexcept
{
    return result_type{this};
}

test_suite::result_type::result_type(test_suite const* parent) noexcept :
    parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
{
}

[[nodiscard]] std::string test_suite::result_type::suite_name() const noexcept
{
    return parent->suite_name;
}

[[nodiscard]] size_t test_suite::result_type::num_tests() const noexcept
{
    return test_results.size();
}

[[nodiscard]] size_t test_suite::result_type::num_failures() const noexcept
{
    return std::count_if(test_results.begin(), test_results.end(), [](auto const& item) {
        return item.failure();
    });
}

[[nodiscard]] size_t test_suite::result_type::num_success() const noexcept
{
    return std::count_if(test_results.begin(), test_results.end(), [](auto const& item) {
        return item.success();
    });
}

[[nodiscard]] size_t test_suite::result_type::num_skipped() const noexcept
{
    return 0;
}

[[nodiscard]] size_t test_suite::result_type::num_disabled() const noexcept
{
    return 0;
}

[[nodiscard]] size_t test_suite::result_type::num_errors() const noexcept
{
    return 0;
}

[[nodiscard]] test_suite::result_type::const_iterator test_suite::result_type::begin() const noexcept
{
    return test_results.begin();
}

[[nodiscard]] test_suite::result_type::const_iterator test_suite::result_type::end() const noexcept
{
    return test_results.end();
}

void test_suite::result_type::push_back(test_case::result_type test_result) noexcept
{
    test_results.push_back(std::move(test_result));
}

void test_suite::result_type::finish() noexcept
{
    duration = hr_clock_type::now() - time_point;
    completed = true;
}

void test_suite::result_type::junit_xml(FILE* out) const noexcept
{
    using namespace std::literals;

    std::print(out, "  <testsuite name=\"{}\" tests=\"{}\" ", suite_name(), num_tests());

    if (completed) {
        std::println(
            out,
            "failures=\"{}\" disabled=\"{}\" skipped=\"{}\" errors=\"{}\" time=\"{:.3f}\" "
            "timestamp=\"{:%Y-%m-%dT%H:%M:%S}\">",
            num_failures(),
            num_disabled(),
            num_skipped(),
            num_errors(),
            duration / 1s,
            time_stamp);
    } else {
        std::println(out, ">");
    }

    for (auto& test_result : test_results) {
        test_result.junit_xml(out);
    }

    std::println(out, "  </testsuite>");
}

[[nodiscard]] test_suite::result_type test_suite::layout(filter const& filter) const noexcept
{
    auto r = result_type{this};
    for (auto& test : tests) {
        if (filter.match_test(test.suite_name, test.test_name)) {
            r.push_back(test.layout());
        }
    }
    return r;
}

[[nodiscard]] test_suite::result_type test_suite::run_tests(filter const& filter) const
{
    using namespace std::literals;

    auto const stats = layout(filter);
    auto const num_tests = stats.num_tests();

    std::println(stdout, "[----------] {} {} from {}", num_tests, num_tests == 1 ? "test" : "tests", suite_name);
    std::fflush(stdout);

    auto r = result_type{this};
    for (auto& test : tests) {
        if (filter.match_test(test.suite_name, test.test_name)) {
            r.push_back(test.run_test());
        }
    }
    r.finish();

    std::println(
        stdout,
        "[----------] {} {} from {} ({:.0f} ms total)",
        num_tests,
        num_tests == 1 ? "test" : "tests",
        suite_name,
        r.duration / 1ms);
    std::println(stdout, "");
    std::fflush(stdout);
    return r;
}

all_tests::result_type::result_type(all_tests* parent) noexcept :
    parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
{
}

void all_tests::result_type::finish() noexcept
{
    duration = hr_clock_type::now() - time_point;
    completed = true;
}

[[nodiscard]] size_t all_tests::result_type::num_suites() const noexcept
{
    return suite_results.size();
}

[[nodiscard]] size_t all_tests::result_type::num_tests() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_tests();
    });
}

[[nodiscard]] size_t all_tests::result_type::num_failures() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_failures();
    });
}

[[nodiscard]] size_t all_tests::result_type::num_success() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_success();
    });
}

[[nodiscard]] size_t all_tests::result_type::num_disabled() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_disabled();
    });
}

[[nodiscard]] size_t all_tests::result_type::num_skipped() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_skipped();
    });
}

[[nodiscard]] size_t all_tests::result_type::num_errors() const noexcept
{
    return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const& acc, auto const& item) {
        return acc + item.num_errors();
    });
}

[[nodiscard]] std::vector<std::string> all_tests::result_type::fqnames_of_failed_tests() const noexcept
{
    auto r = std::vector<std::string>{};
    for (auto const& suite_result : suite_results) {
        for (auto const& test_result : suite_result) {
            if (test_result.failure()) {
                r.push_back(std::format("{}.{}", test_result.suite_name(), test_result.test_name()));
            }
        }
    }
    return r;
}

[[nodiscard]] all_tests::result_type::const_iterator all_tests::result_type::begin() const noexcept
{
    return suite_results.begin();
}

[[nodiscard]] all_tests::result_type::const_iterator all_tests::result_type::end() const noexcept
{
    return suite_results.end();
}

void all_tests::result_type::push_back(test_suite::result_type suite_result) noexcept
{
    suite_results.push_back(suite_result);
}

void all_tests::result_type::junit_xml(FILE* out) const noexcept
{
    using namespace std::literals;

    std::println(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    std::println(out, "<testsuites tests=\"{}\" name=\"AllTests\" ", num_tests());

    if (completed) {
        std::println(
            out,
            "failures=\"{}\" disabled=\"{}\" skipped=\"{}\" errors=\"{}\" time=\"{:.3f}\" "
            "timestamp=\"{:%Y-%m-%dT%H:%M:%S}\">",
            num_failures(),
            num_disabled(),
            num_skipped(),
            num_errors(),
            duration / 1s,
            time_stamp);
    } else {
        std::println(out, ">");

        for (auto const& suite_result : suite_results) {
            suite_result.junit_xml(out);
        }

        std::println(out, "</testsuites>");
    }
}

[[nodiscard]] all_tests::result_type all_tests::layout(::test::filter const& filter) noexcept
{
    auto r = result_type{this};

    for (auto& suite : suites) {
        if (filter.match_suite(suite.suite_name)) {
            r.push_back(suite.layout(filter));
        }
    }

    return r;
}

[[nodiscard]] all_tests::result_type all_tests::list_tests(::test::filter const& filter) noexcept
{
    auto const r = layout(filter);
    for (auto const& suite_result : r) {
        std::println(stdout, "{}.", suite_result.suite_name());
        for (auto const& test_result : suite_result) {
            std::println(stdout, "  {}", test_result.test_name());
        }
    }
    return r;
}

[[nodiscard]] all_tests::result_type all_tests::run_tests(::test::filter const& filter)
{
    using namespace std::literals;

    auto const stats = layout(filter);
    auto const num_tests = stats.num_tests();
    auto const num_suites = stats.num_suites();

    std::println(
        stdout,
        "[==========] Running {} {} from {} test {}.",
        num_tests,
        num_tests == 1 ? "test" : "tests",
        num_suites,
        num_suites == 1 ? "suite" : "suites");
    std::println(stdout, "[----------] Global test environment set-up.");
    std::fflush(stdout);

    auto r = result_type{this};
    for (auto& suite : suites) {
        if (filter.match_suite(suite.suite_name)) {
            r.push_back(suite.run_tests(filter));
        }
    }
    r.finish();

    std::println(stdout, "[----------] Global test environment tear-down");
    std::println(
        stdout,
        "[==========] {} {} from {} test {} ran. ({} ms total)",
        num_tests,
        num_tests == 1 ? "test" : "tests",
        num_suites,
        num_suites == 1 ? "suite" : "suites",
        r.duration / 1ms);

    auto const num_success = r.num_success();
    if (num_success != 0) {
        std::println(stdout, "[  PASSED  ] {} {}.", num_success, num_success == 1 ? "test" : "tests");
    }

    auto const num_failures = r.num_failures();
    if (num_failures != 0) {
        std::println(stdout, "[  FAILED  ] {} {}, listed below:", num_failures, num_failures == 1 ? "test" : "tests");

        for (auto const& failed_test : r.fqnames_of_failed_tests()) {
            std::println(stdout, "[  FAILED  ] {}", failed_test);
        }
    }

    return r;
}

all_tests::result_type list_tests(filter const& filter) noexcept
{
    return all.list_tests(filter);
}

[[nodiscard]] all_tests::result_type run_tests(filter const& filter)
{
    return all.run_tests(filter);
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

static void parse_arguments(int argc, char* argv[]) noexcept
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
            ::test::break_on_failure = true;
            continue;

        } else if (arg == "--gtest_list_tests") {
            option_list_tests = true;

        } else if (arg.starts_with("--gtest_filter=")) {
            try {
                option_filter = ::test::filter(arg.substr(15));

            } catch (std::runtime_error const& e) {
                std::println(stderr, "error: {}.\n", e.what());
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

int main(int argc, char* argv[])
{
    std::println(stdout, "Running main() from {}", __FILE__);
    parse_arguments(argc, argv);

    auto result = option_list_tests ? ::test::list_tests(option_filter) : ::test::run_tests(option_filter);

    FILE* xml_output = nullptr;
    if (option_xml_output_path) {
        xml_output = fopen(option_xml_output_path->string().c_str(), "w");
        if (xml_output == nullptr) {
            std::println(stdout, "Could not open xml-file {}", option_xml_output_path->string());
            print_help(2);
        }

        result.junit_xml(xml_output);

        if (fclose(xml_output) != 0) {
            std::println(stdout, "Could not close xml-file {}", option_xml_output_path->string());
            print_help(1);
        }
    }

    return result.num_failures() == 0 ? 0 : 1;
}
