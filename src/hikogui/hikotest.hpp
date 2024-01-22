

#include <concepts>
#include <utility>
#include <type_traits>
#include <functional>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <chrono>
#include <compare>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>
#include <print>
#include <expected>
#include <optional>

namespace test {

/** Declare a test suite
 *
 * @param id The method-name of the test case.
 */
#define TEST_SUITE(id) \
    struct id; \
    inline auto _hikotest_suite_info_##id = std::addressof(::test::register_suite<id>()); \
    struct id : ::test::suite<id>

/** Delcare a test case
 *
 * The test function that is declared returns a `std::expected<void, std::string>`.
 * On success the test function should: `return {};`.
 *
 * @param id The method-name of the test case.
 */
#define TEST_CASE(id) \
    std::expected<void, std::string> _hikotest_wrap_##id() \
    { \
        return id(); \
    } \
    inline static auto _hikotest_wrap_registered_##id = \
        std::addressof(::test::register_test(&_hikotest_suite_type::_hikotest_wrap_##id, __FILE__, __LINE__, #id)); \
    std::expected<void, std::string> id()

/** Check an expression
 *
 * @param expression A comparison or boolean expression to check.
 * @param ... The optional error value. If it is a double value than it is the
 *            absolute error value. You may also pass in a `::test::error`.
 */
#define REQUIRE(expression, ...) \
    do { \
        if (auto _hikotest_result = ::test::error{__VA_ARGS__} <=> expression; not _hikotest_result) { \
            std::println(stdout, "{}({}): error: {}", __FILE__, __LINE__, _hikotest_result.error()); \
            return _hikotest_result; \
        } \
    } while (false)

/** Check if a expression throws an exception
 *
 * @param expression The expression which causes an exception to be thrown.
 * @param exception The exception to be thrown.
 */
#define REQUIRE_THROWS(expression, exception) \
    do { \
        bool _hikotest_throws = false; \
        try { \
            (void)expression; \
        } catch (exception const&) { \
            _hikotest_throws = true; \
        } \
        if (not _hikotest_throws) { \
            std::println(stdout, "{}({}): error: {}", __FILE__, __LINE__, #expression " did not throw " #exception "."); \
            return std::unexpected{#expression " did not throw " #exception "."}; \
        } \
    } while (false)

using hr_clock_type = std::chrono::high_resolution_clock;
using hr_duration_type = std::chrono::duration<double>;
using hr_time_point_type = std::chrono::time_point<hr_clock_type>;
using utc_clock_type = std::chrono::utc_clock;
using utc_time_point_type = utc_clock_type::time_point;

[[nodiscard]] constexpr std::string type_name_strip(std::string type)
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

template<typename T>
[[nodiscard]] constexpr std::string type_name() noexcept
{
#if defined(_MSC_VER)
    auto signature = std::string_view{__FUNCSIG__};
#elif defined(__GNUC__)
    auto signature = std::string_view{__PRETTY_FUNCTION__};
#else
#error "type_name() not implemented"
#endif

    // constexpr std::string_view class_name() [with T = foo<bar>; std::string_view = std::basic_string_view<char>]
    if (auto first = signature.find("::type_name() [with T = "); first != signature.npos) {
        first += 24;
        auto const last = signature.find("; ", first);
        if (last == signature.npos) {
            std::println(stderr, "{}({}): error: Could not parse type_name from '{}'", __FILE__, __LINE__, signature);
            std::terminate();
        }
        return type_name_strip(std::string{signature.substr(first, last - first)});
    }

    // __cdecltest::type_name(void)[T=foo<bar>]
    if (auto first = signature.find("::type_name(void) [T = "); first != signature.npos) {
        first += 23;
        auto const last = signature.find("]", first);
        if (last == signature.npos) {
            std::println(stderr, "{}({}): error: Could not parse type_name from '{}'", __FILE__, __LINE__, signature);
            std::terminate();
        }
        return type_name_strip(std::string{signature.substr(first, last - first)});
    }

    // class std::basic_string_view<char,struct std::char_traits<char> > __cdecl class_name<struct foo<struct bar>>(void) noexcept
    if (auto first = signature.find(" type_name<"); first != signature.npos) {
        first += 12;
        auto const last = signature.rfind(">(void)");
        return type_name_strip(std::string{signature.substr(first, last - first)});
    }

    std::println(stderr, "{}({}): error: Could not parse type_name from '{}'", __FILE__, __LINE__, signature);
    std::terminate();
}

template<char QuoteChar = '\0'>
[[nodiscard]] constexpr std::string xml_escape(std::string str) noexcept
{
    for (auto it = str.begin(); it != str.end(); ++it) {
        auto const c = *it;

        if (c == '"' and QuoteChar == '"') {
            it = str.erase(it);
            it = str.insert_range(it, std::string_view{"&quot;"});
            it += 5;

        } else if (c == '\'' and QuoteChar == '\'') {
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

template<typename Arg>
[[nodiscard]] std::string operand_to_string(Arg const& arg) noexcept
{
    std::stringbuf buf;

    if constexpr (requires { std::formatter<Arg, char>{}; }) {
        return std::format("{}", arg);

    } else if constexpr (requires { buf << arg; }) {
        buf << arg;
        return std::move(buf).str();

    } else if constexpr (requires { to_string(arg); }) {
        return to_string(arg);

    } else if constexpr (requires { arg.string(); }) {
        return arg.string();

    } else if constexpr (requires { arg.str(); }) {
        return arg.str();

    } else {
        std::array<std::byte, sizeof(Arg)> bytes;
        std::memcpy(bytes.data(), std::addressof(arg), sizeof(Arg));

        auto r = std::string{};
        for (auto const byte : bytes) {
            if (r.empty()) {
                r += std::format("<{:02x}", std::to_underlying(byte));
            } else {
                r += std::format(" {:02x}", std::to_underlying(byte));
            }
        }
        return r + '>';
    }
}

template<typename LHS, typename RHS>
struct compare {
    using lhs_type = LHS;
    using rhs_type = RHS;

    constexpr compare(bool trace, std::string lhs, std::string rhs) noexcept :
        trace(trace), lhs(std::move(lhs)), rhs(std::move(rhs))
    {
    }

    bool trace;
    std::string lhs;
    std::string rhs;
};

enum class error_class { exact, absolute, relative };

/** expression-tag used on the left side of the spaceship operator.
 *
 * This tag is used to cause the operator<=> to bind to any other type.
 */
template<error_class ErrorClass = error_class::exact>
struct error {
    double v = 0.0;

    [[nodiscard]] constexpr double operator+() const noexcept
    {
        return v;
    }

    [[nodiscard]] constexpr double operator-() const noexcept
    {
        return -v;
    }
};

error(double) -> error<error_class::absolute>;

/** Left operand of a comparison, or a boolean.
 */
template<error_class ErrorClass, typename T>
struct operand {
    using value_type = T;

    error<ErrorClass> e;
    value_type const& v;
};

/** The spaceship operator is used to split a comparison in an expression.
 *
 * The spaceship operator has a slightly higher precedence then the
 * comparison operators: ==, !=, <, >, <=, =>.
 *
 * The spaceship operator also is not often used in unit-test expression,
 * which means that this is the perfect operator to wrap to one of the operands.
 *
 * By wrapping the operand we can now overload the normal comparison operators.
 * So that the unit-test framework can do a custom comparison.
 *
 * @param lhs The left-hand-side operand.
 * @return The wrapped left-hand-side operand.
 */
template<error_class ErrorClass, typename LHS>
[[nodiscard]] constexpr operand<ErrorClass, LHS> operator<=>(error<ErrorClass> e, LHS const& lhs) noexcept
{
    return {e, lhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::expected<void, std::string>
operator==(operand<error_class::exact, LHS> const& lhs, RHS const& rhs) noexcept
{
    if constexpr (requires {
                      {
                          lhs == rhs
                      } -> std::convertible_to<bool>;
                  }) {
        if (lhs == rhs) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs))};
        }

    } else if constexpr (requires { std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs); }) {
        if (std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs)) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs))};
        }

    } else if constexpr (requires { std::ranges::equal(lhs, rhs); }) {
        if (std::ranges::equal(lhs, rhs)) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs))};
        }

    } else {
        []<bool Flag = false>()
        {
            static_assert(Flag, "hikotest: Unable to equality-compare two values.");
        }
        ();
    }
}

template<typename LHS, typename RHS, typename Error>
concept diff_ordered = requires(LHS lhs, RHS rhs) {
    {
        lhs - rhs
    } -> std::totally_ordered_with<Error>;
};

template<typename LHS, typename RHS, typename Error>
concept range_diff_ordered = std::ranges::range<LHS> and std::ranges::range<RHS> and
    diff_ordered<std::ranges::range_value_t<LHS>, std::ranges::range_value_t<RHS>, Error>;

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::expected<void, std::string>
operator==(operand<error_class::absolute, LHS> const& lhs, RHS const& rhs) noexcept requires diff_ordered<LHS, RHS, double>
{
    auto const diff = lhs.v - rhs;
    if (diff >= -lhs.e and diff <= lhs.e) {
        return {};
    } else {
        return std::unexpected{std::format(
            "Expected equality within {} of these values:\n  {}\n  {}",
            lhs.e.v,
            operand_to_string(lhs.v),
            operand_to_string(rhs))};
    }
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::expected<void, std::string>
operator==(operand<error_class::absolute, LHS> const& lhs, RHS const& rhs) noexcept
    requires(not diff_ordered<LHS, RHS, double>) and range_diff_ordered<LHS, RHS, double>
{
    auto lit = lhs.v.begin();
    auto rit = rhs.begin();

    auto const lend = lhs.v.end();
    auto const rend = rhs.end();

    while (lit != lend and rit != rend) {
        auto const diff = *lit - *rit;
        if (diff < -lhs.e or diff > +lhs.e) {
            return std::unexpected{std::format(
                "Expected equality within {} of these values:\n  {}\n  {}",
                +lhs.e,
                operand_to_string(lhs.v),
                operand_to_string(rhs))};
        }

        ++lit;
        ++rit;
    }

    if (lit != lend or rit != rend) {
        return std::unexpected{std::format(
            "Expected both range-values to the same size:\n  {}\n  {}", operand_to_string(lhs.v), operand_to_string(rhs))};
    }

    return {};
}

class filter {
public:
    constexpr filter() noexcept : inclusions{test_filter_type{}}, exclusions() {}
    constexpr filter(filter const&) noexcept = default;
    constexpr filter(filter&&) noexcept = default;
    constexpr filter& operator=(filter const&) noexcept = default;
    constexpr filter& operator=(filter&&) noexcept = default;

    /** Create a filter from the string representation
     *
     * @param str A string in the following format [ inclusion ]['-' exclusion]
     * @throws std::runtime_error On parse error.
     */
    filter(std::string_view str);
    [[nodiscard]] bool match_suite(std::string_view suite) const noexcept;
    [[nodiscard]] bool match_test(std::string_view suite, std::string_view test) const noexcept;

private:
    struct test_filter_type {
        std::string suite_name;
        std::string test_name;
    };

    std::vector<test_filter_type> inclusions;
    std::vector<test_filter_type> exclusions;
};

struct test_case {
    std::string_view file;
    int line;
    std::string suite_name;
    std::string test_name;
    std::function<std::expected<void, std::string>()> _run_test;

    struct result_type {
        test_case* parent;
        utc_time_point_type time_stamp;
        hr_time_point_type time_point;
        hr_duration_type duration = {};
        std::string error_message = {};
        bool completed = false;

        result_type(result_type const&) noexcept = default;
        result_type(result_type&&) noexcept = default;
        result_type& operator=(result_type const&) noexcept = default;
        result_type& operator=(result_type&&) noexcept = default;

        result_type(test_case* parent) noexcept :
            parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
        {
        }

        [[nodiscard]] std::string suite_name() const noexcept
        {
            return parent->suite_name;
        }

        [[nodiscard]] std::string test_name() const noexcept
        {
            return parent->test_name;
        }

        [[nodiscard]] std::string_view file() const noexcept
        {
            return parent->file;
        }

        [[nodiscard]] int line() const noexcept
        {
            return parent->line;
        }

        [[nodiscard]] bool success() const noexcept
        {
            return completed and error_message.empty();
        }

        [[nodiscard]] bool failure() const noexcept
        {
            return completed and not error_message.empty();
        }

        [[nodiscard]] bool skipped() const noexcept
        {
            return not completed;
        }

        void set_success() noexcept
        {
            duration = hr_clock_type::now() - time_point;
            completed = true;
        }

        void set_failure(std::string message) noexcept
        {
            duration = hr_clock_type::now() - time_point;
            error_message = message;
            completed = true;
        }

        void junit_xml(FILE* out) const noexcept
        {
            using namespace std::literals;

            std::print(
                out,
                "    <testcase name=\"{}\" file=\"{}\" line=\"{}\" classname=\"{}\" ",
                test_name(),
                file(),
                line(),
                suite_name());

            if (completed) {
                std::print(
                    out,
                    "status=\"run\" result=\"completed\" time=\"{:.3f}\" timestamp=\"{:%Y-%m-%dT%H:%M:%S}\"",
                    duration / 1s,
                    time_stamp);

                if (not error_message.empty()) {
                    std::print(out, "      <failure message=\"{}\" type=\"\">", xml_escape<'"'>(error_message));
                    std::println(out, "<![CDATA[{}]]></failure>", xml_escape(error_message));
                    std::println(out, "    </testcase>");

                } else {
                    std::println(out, "/>");
                }
            } else {
                std::println(out, "/>");
            }
        }
    };

    test_case(test_case const&) = default;
    test_case(test_case&&) = default;
    test_case& operator=(test_case const&) = default;
    test_case& operator=(test_case&&) = default;

    template<typename Suite>
    [[nodiscard]] test_case(
        std::string_view file,
        int line,
        std::string suite_name,
        std::string test_name,
        std::expected<void, std::string> (Suite::*test)()) noexcept :
        file(file),
        line(line),
        suite_name(std::move(suite_name)),
        test_name(std::move(test_name)),
        _run_test([test]() -> auto {
            return (Suite{}.*test)();
        })
    {
    }

    [[nodiscard]] result_type run_test() noexcept
    {
        using namespace std::literals;

        std::println(stdout, "[ RUN      ] {}.{}", suite_name, test_name);
        std::fflush(stdout);

        auto r = result_type{this};

        if (auto result = _run_test()) {
            r.set_success();
            std::println(stdout, "[       OK ] {}.{} ({:.0f} ms)", suite_name, test_name, r.duration / 1ms);
        } else {
            r.set_failure(result.error());
            std::println(stdout, "[  FAILED  ] {}.{} ({:.0f} ms)", suite_name, test_name, r.duration / 1ms);
        }

        std::fflush(stdout);
        return r;
    }

    [[nodiscard]] result_type layout() noexcept
    {
        return result_type{this};
    }
};

struct test_suite {
    std::string suite_name;
    std::vector<test_case> tests;

    test_suite(std::string suite_name) noexcept : suite_name(std::move(suite_name)) {}

    struct result_type {
        test_suite* parent;
        utc_time_point_type time_stamp = {};
        hr_time_point_type time_point = {};
        hr_duration_type duration = {};
        std::vector<test_case::result_type> test_results;
        bool completed = false;

        result_type(result_type const&) noexcept = default;
        result_type(result_type&&) noexcept = default;
        result_type& operator=(result_type const&) noexcept = default;
        result_type& operator=(result_type&&) noexcept = default;

        result_type(test_suite* parent) noexcept :
            parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
        {
        }

        [[nodiscard]] std::string suite_name() const noexcept
        {
            return parent->suite_name;
        }

        [[nodiscard]] size_t num_tests() const noexcept
        {
            return test_results.size();
        }

        [[nodiscard]] size_t num_failures() const noexcept
        {
            return std::count_if(test_results.begin(), test_results.end(), [](auto const& item) {
                return item.failure();
            });
        }

        [[nodiscard]] size_t num_success() const noexcept
        {
            return std::count_if(test_results.begin(), test_results.end(), [](auto const& item) {
                return item.success();
            });
        }

        [[nodiscard]] size_t num_skipped() const noexcept
        {
            return 0;
        }

        [[nodiscard]] size_t num_disabled() const noexcept
        {
            return 0;
        }

        [[nodiscard]] size_t num_errors() const noexcept
        {
            return 0;
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return test_results.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return test_results.end();
        }

        void push_back(test_case::result_type test_result) noexcept
        {
            test_results.push_back(std::move(test_result));
        }

        void finish() noexcept
        {
            duration = hr_clock_type::now() - time_point;
            completed = true;
        }

        void junit_xml(FILE* out) const noexcept
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
    };

    [[nodiscard]] result_type layout(filter const& filter) noexcept
    {
        auto r = result_type{this};
        for (auto& test : tests) {
            if (filter.match_test(test.suite_name, test.test_name)) {
                r.push_back(test.layout());
            }
        }
        return r;
    }

    [[nodiscard]] result_type run_tests(filter const& filter)
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
};

struct all_tests {
    struct result_type {
        all_tests* parent;
        utc_time_point_type time_stamp = {};
        hr_time_point_type time_point = {};
        hr_duration_type duration = {};
        std::vector<test_suite::result_type> suite_results;
        bool completed = false;

        result_type(all_tests* parent) noexcept :
            parent(parent), time_stamp(utc_clock_type::now()), time_point(hr_clock_type::now())
        {
        }

        void finish() noexcept
        {
            duration = hr_clock_type::now() - time_point;
            completed = true;
        }

        [[nodiscard]] size_t num_suites() const noexcept
        {
            return suite_results.size();
        }

        [[nodiscard]] size_t num_tests() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_tests();
            });
        }

        [[nodiscard]] size_t num_failures() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_failures();
            });
        }

        [[nodiscard]] size_t num_success() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_success();
            });
        }

        [[nodiscard]] size_t num_disabled() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_disabled();
            });
        }

        [[nodiscard]] size_t num_skipped() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_skipped();
            });
        }

        [[nodiscard]] size_t num_errors() const noexcept
        {
            return std::accumulate(suite_results.begin(), suite_results.end(), size_t{0}, [](auto const &acc, auto const& item) {
                return acc + item.num_errors();
            });
        }

        [[nodiscard]] std::vector<std::string> fqnames_of_failed_tests() const noexcept
        {
            auto r = std::vector<std::string>{};
            for (auto const &suite_result: suite_results) {
                for (auto const &test_result: suite_result) {
                    if (test_result.failure()) {
                        r.push_back(std::format("{}.{}", test_result.suite_name(), test_result.test_name()));
                    }
                }
            }
            return r;
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return suite_results.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return suite_results.end();
        }

        void push_back(test_suite::result_type suite_result) noexcept
        {
            suite_results.push_back(suite_result);
        }

        void junit_xml(FILE* out) const noexcept
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
    };

    std::vector<test_suite> suites;
    mutable size_t last_registered_suite = 0;

    template<typename Suite>
    [[nodiscard]] inline test_suite& register_suite() noexcept
    {
        auto const name = type_name<Suite>();

        // Skip binary search if possible.
        if (last_registered_suite < suites.size() and suites[last_registered_suite].suite_name == name) {
            return suites[last_registered_suite];
        }

        auto const it = std::lower_bound(suites.begin(), suites.end(), name, [](auto const& item, auto const& name) {
            return item.suite_name < name;
        });

        last_registered_suite = std::distance(suites.begin(), it);

        if (it != suites.end() and it->suite_name == name) {
            return *it;
        }

        return *suites.emplace(it, name);
    }

    template<typename Suite>
    [[nodiscard]] inline test_case&
    register_test(std::expected<void, std::string> (Suite::*test)(), std::string_view file, int line, std::string name) noexcept
    {
        auto& suite = register_suite<Suite>();
        auto& tests = suite.tests;

        auto const it = std::lower_bound(tests.begin(), tests.end(), name, [](auto const& item, auto const& name) {
            return item.test_name < name;
        });

        if (it != tests.end() and it->test_name == name) {
            std::println(
                stderr,
                "{}({}): error: Test {}.{} is already registered at {}({}).",
                file,
                line,
                it->suite_name,
                it->test_name,
                it->file,
                it->line);
            std::terminate();
        }

        // clang-format off
        return *tests.emplace(it,
            file,
            line,
            suite.suite_name,
            name,
            test);
        // clang-format on
    }

    [[nodiscard]] result_type layout(::test::filter const& filter) noexcept
    {
        auto r = result_type{this};

        for (auto& suite : suites) {
            if (filter.match_suite(suite.suite_name)) {
                r.push_back(suite.layout(filter));
            }
        }

        return r;
    }

    [[nodiscard]] result_type list_tests(::test::filter const& filter) noexcept
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

    [[nodiscard]] result_type run_tests(::test::filter const& filter)
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
};

inline auto all = all_tests{};

template<typename Suite>
[[nodiscard]] inline test_suite& register_suite() noexcept
{
    return all.template register_suite<Suite>();
}

template<typename Suite>
[[nodiscard]] inline test_case&
register_test(std::expected<void, std::string> (Suite::*test)(), std::string_view file, int line, std::string name) noexcept
{
    return all.template register_test<Suite>(test, file, line, std::move(name));
}

inline all_tests::result_type list_tests(filter const& filter) noexcept
{
    return all.list_tests(filter);
}

[[nodiscard]] inline all_tests::result_type run_tests(filter const& filter)
{
    return all.run_tests(filter);
}

template<typename Suite>
struct suite {
    using _hikotest_suite_type = Suite;
};

} // namespace test
