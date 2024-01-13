

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
#include <cassert>
#include <limits>
#include <typeinfo>

namespace test {

#define TEST_FORCE_SYMBOL(symbol) _Pragma("comment(linker, \"/include:\"" #symbol ")")

#if defined(_MSC_VER)
#define TEST_SUITE(id) \
    struct id; \
    inline auto _hikotest_suite_info_##id = register_suite<id>(); \
    TEST_FORCE_SYMBOL(hikotest_info_##id); \
    struct id : ::test::suite<id>

#elif defined(__GNUC__) or defined(__clang_major__)
#define TEST_SUITE(id) \
    struct id; \
    [[gnu::externally_visible]] inline auto _hikotest_test_info_##id = register_suite<id>(); \
    struct id : ::test::suite<id>

#else
#error "TEST_SUITE() not implemented."
#endif

/** Delcare a test case
 *
 * @param id The method-name of the test case.
 * @param ... The optional name of the test case as a string-literal.
 */
#define TEST_CASE(id) \
    void _hikotest_wrap_##id(::test::trace& _hikotest_trace) \
    { \
        return id(_hikotest_trace); \
    } \
    inline static bool _hikotest_wrap_registered_##id = \
        register_test(__FILE__, __LINE__, #id, &_hikotest_suite_type::_hikotest_wrap_##id); \
    void id(::test::trace& _hikotest_trace)

/** Check an expression
 *
 * @param expression A comparison or boolean expression to check.
 * @param ... The optional error value. If it is a double value than it is the
 *            absolute error value. You may also pass in a `::test::error`.
 */
#define REQUIRE(expression, ...) \
    do { \
        _hikotest_trace.start_check(__FILE__, __LINE__); \
        if (not _hikotest_trace.finish_check(::test::error{__VA_ARGS__} <=> expression)) { \
            return; \
        } \
    } while (false)

/** Check if a expression throws an exception
 *
 * @param expression The expression which causes an exception to be thrown.
 * @param exception The exception to be thrown.
 */
#define REQUIRE_THROWS(expression, exception) \
    _hikofui_trace.start_check(__FILE__, __LINE__, "Check did not throw " #exception "."); \
    do { \
        try { \
            (void)expression; \
        } catch (exception const&) { \
            _hikofui_trace.finish_check(std::nullopt); \
        } \
        if (not _hikofui_trace.finish_check()) { \
            return; \
        } \
    } while (false)

using clock_type = std::chrono::high_resolution_clock;
using duration_type = std::chrono::duration<double>;
using time_point_type = std::chrono::time_point<clock_type>;

template<std::size_t N>
struct fixed_string {
    constexpr fixed_string() noexcept = default;

    template<std::size_t O>
    constexpr fixed_string(char const (&str)[O]) noexcept : _str()
    {
        for (auto i = 0; i != O - 1; ++i) {
            _str[i] = str[i];
        }
    }

    constexpr operator std::string_view() const noexcept
    {
        return std::string_view{_str.begin(), _str.end()};
    }

    constexpr operator std::string() const noexcept
    {
        return std::string{_str.begin(), _str.end()};
    }

    std::array<char, N> _str;
};

fixed_string() -> fixed_string<0>;

template<std::size_t O>
fixed_string(char const (&str)[O]) -> fixed_string<O - 1>;

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
        return -v;
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
[[nodiscard]] constexpr std::optional<std::string>
operator==(operand<error_class::exact, LHS> const& lhs, RHS const& rhs) noexcept
{
    if constexpr (requires {
                      {
                          lhs == rhs
                      } -> std::convertible_to<bool>;
                  }) {
        if (lhs == rhs) {
            return std::nullopt;
        } else {
            return std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs));
        }

    } else if constexpr (requires { std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs); }) {
        if (std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs)) {
            return std::nullopt;
        } else {
            return std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs));
        }

    } else if constexpr (requires { std::ranges::equal(lhs, rhs); }) {
        if (std::ranges::equal(lhs, rhs)) {
            return std::nullopt;
        } else {
            return std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs));
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
[[nodiscard]] constexpr std::optional<std::string>
operator==(operand<error_class::absolute, LHS> const& lhs, RHS const& rhs) noexcept
    requires diff_ordered<LHS, RHS, double>
{
    auto const diff = lhs.v - rhs;
    if (diff >= -lhs.e and diff <= lhs.e) {
        return std::nullopt;
    } else {
        return std::format(
            "Expected equality within {} of these values:\n  {}\n  {}",
            lhs.e.v,
            operand_to_string(lhs.v),
            operand_to_string(rhs));
    }
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::optional<std::string>
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
            return std::format(
                "Expected equality within {} of these values:\n  {}\n  {}",
                +lhs.e,
                operand_to_string(lhs.v),
                operand_to_string(rhs));
        }

        ++lit;
        ++rit;
    }

    if (lit != lend or rit != rend) {
        return std::format(
            "Expected both range-values to the same size:\n  {}\n  {}", operand_to_string(lhs.v), operand_to_string(rhs));
    }

    return std::nullopt;
}

class filter {
public:
    [[nodiscard]] bool match_suite(std::string_view suite) const noexcept;
    [[nodiscard]] bool match_test(std::string_view suite, std::string_view test) const noexcept;
};

struct test_result {
    std::string_view file;
    int line;
    std::string suite_name;
    std::string test_name;
    std::function<void(test_result&)> _run_test;
    std::string failure = {};
    time_point_type time_point = {};
    duration_type duration = {};

    void start() noexcept
    {
        std::println(stdout, "[ RUN      ] {}.{}", suite_name, test_name);
        std::fflush(stdout);
        failure = {};
        time_point = clock_type::now();
    }

    void finish() noexcept
    {
        duration = clock_type::now() - time_point;
        if (failure.empty()) {
            std::println(stdout, "[       OK ] {}.{} ({} ms)", suite_name, test_name, duration / 1ms);
        } else {
            std::println(stdout, "[  FAILED  ] {}.{} ({} ms)", suite_name, test_name, duration / 1ms);
        }
        std::fflush(stdout);
    }

    void run_test() noexcept
    {
        start();
        _run_test(*this);
        finish();
    }

    void generate_junit_xml(FILE *out) noexcept
    {
        using namespace std::literals;

        std::print(out, "    <testcase name=\"{}\" file=\"{}\" line=\"{}\" classname=\"{}\"", test_name, file, line, suite_name);

        if (timestamp != {}) {
            std::print(
                out,
                "status=\"run\" result=\"completed\" time=\"{}\" timestamp=\"{:%Y-%m-%dT%H:%M:%S.000}\"",
                time / 1s,
                timestamp);

            if (not failure.empty()) {
                std::print(out, "      <failure message=\"{}\" type=\"\">", html_attribute_encode(failure));
                std::println(out, "![CDATA[{}]]</failure>", html_cdata_encode(failure));
                std::println(out, "    </testcase>");
            } else {
                std::println(out, " />");
            }
        } else {
            std::println(out, " />");
        }
    }
};

struct suite_result {
    std::string suite_name;
    std::vector<test_result> tests;
    time_point_type time_point = {};
    duration_type duration = {};

    struct collect_stats_result {
        size_t num_tests = 0;
        size_t num_failures = 0;
        size_t num_disabled = 0;
        size_t num_skipped = 0;
        size_t num_errors = 0;
        std::vector<std::string> failed_tests;
    };

    [[nodiscard]] stats_result collect_stats(filter const& filter) noexcept
    {
        collect_stats_result r = {};
        for (auto& test : tests) {
            if (filter.match_test(test.suite_name, test.test_name)) {
                ++r.num_tests;
                if (test.time_point != {}) {
                    if (not test.failure.empty()) {
                        ++r.num_failures;
                        r.failed_tests.push_back(std::format("{}.{}", test.suite_name, test.test_name));
                    }
                }
            }
        }
        return r;
    }

    void start(size_t num_tests) noexcept
    {
        std::println(stdout, "[----------] {} {} from {}", num_tests, num_tests == 1 ? "test" : "tests", suite_name);
        std::fflush(stdout);
        time_point = clock_type::now();
    }

    void finish(size_t num_tests) noexcept
    {
        duration = clock_type::now() - time_point;

        std::println(
            stdout,
            "[----------] {} {} from {} ({} ms total)",
            num_tests,
            num_tests == 1 ? "test" : "tests",
            suite_name,
            duration / 1ms);
        std::fflush(stdout);
    }

    void run_tests(filter const &filter)
    {
        auto const stats = collect_stats(filter);
        start(stats.num_tests);
        for (auto &test: tests) {
            if (filter.match_test(test.suite_name, test.test_name)) {
                test.run_test();
            }
        }
        finish(stats.num_tests);
    }

    void generate_junit_xml(filter const& filter, FILE *out) noexcept
    {
        using namespace std::literals;

        auto const stats = collect_stats(filter);

        std::print(out, "  <testsuite name=\"{}\" tests=\"{}\"", suite_name, stats.num_tests);

        if (time_point != {}) {
            std::print(
                out,
                "failures=\"{}\" disabled=\"{}\" skipped=\"{}\" errors=\"{}\" time=\"{:.3}\" "
                "timestamp=\"{%Y-%m=%dT%H:%M:%S.000}\">",
                suite_name,
                stats.num_tests,
                stats.num_failures,
                stats.num_disabled,
                stats.num_skipped,
                stats.num_errors,
                duration / 1s,
                time_point);
        } else {
            std::println(out, ">");
        }

        for (auto& test : tests) {
            if (filter.match_test(test.suite_name, test.test_name)) {
                test.generate_junit_xml(out);
            }
        }

        if (generate_junit_xml) {
            std::println(out, "  </testsuite>");
        }
    }
};

inline std::vector<suite_result> suites;
inline size_t last_registered_suite = 0;

template<typename Suite>
[[nodiscard]] inline suite_result& register_suite() noexcept
{
    auto const name = class_name<Suite>();

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

    return *suites.insert(it, suite_result{name});
}

template<typename Suite>
[[nodiscard]] inline test_result&
register_test(void (Suite::*test)(test_result&), std::string_view file, int line, std::string name) noexcept
{
    auto& suite = register_suite<Suite>();
    auto& tests = suite.tests;

    auto const it = std::lower_bound(tests.begin(), tests.end(), name, [](auto const& item, auto const& name) {
        return item.test_name < name;
    });

    if (it != suites.end() and it->test_name == name) {
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

    return *tests.insert(it, test_result{file, line, suite.suite_name, name, [](test_result& r) -> void {
        (Suite{}.*test)(r);
    }};
}

template<typename Suite>
struct suite {
    using _hikotest_suite_type = Suite;
};

} // namespace test
