

#include "hikotest.hpp"
#include <utility>
#include <print>
#include <optional>
#include <filesystem>
#include <expected>
#include <algorithm>

namespace test {

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

    if (inclusions.empty()) {
        inclusions.emplace_back();
    }
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
            continue;

        } else if (arg == "--gtest_list_tests") {
            option_list_tests = true;

        } else if (arg.starts_with("--gtest_filter=")) {
            try {
                option_filter = ::test::filter(arg.substr(15));

            } catch(std::runtime_error const &e) {
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