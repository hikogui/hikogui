

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
    inline static bool hikotest_##id##_dummy = register_test(__FILE__, __LINE__, #id, &suite_type::_hikotest_wrap_##id); \
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
        for (auto const byte: bytes) {
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
            "Expected equality within {} of these values:\n  {}\n  {}", lhs.e.v, operand_to_string(lhs.v), operand_to_string(rhs));
    }
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::optional<std::string>
operator==(operand<error_class::absolute, LHS> const& lhs, RHS const& rhs) noexcept
    requires (not diff_ordered<LHS, RHS, double>) and range_diff_ordered<LHS, RHS, double>
{
    auto lit = lhs.v.begin();
    auto rit = rhs.begin();

    auto const lend = lhs.v.end();
    auto const rend = rhs.end();

    while (lit != lend and rit != rend) {
        auto const diff = *lit - *rit;
        if (diff < -lhs.e or diff > +lhs.e) {
            return std::format(
            "Expected equality within {} of these values:\n  {}\n  {}", +lhs.e, operand_to_string(lhs.v), operand_to_string(rhs));
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

class trace {
public:
    using clock_type = std::chrono::high_resolution_clock;
    using duration_type = std::chrono::duration<double>;
    using time_point_type = std::chrono::time_point<clock_type>;

    [[nodiscard]] size_t num_total_failures() const noexcept;
    [[nodiscard]] size_t num_total_passed() const noexcept;

    /** Start the global testing.
     *
     * @post Store arguments.
     * @post Start timer.
     * @post Print to std::out
     * @param num_suites The number of suites that will be run.
     * @param num_total_test The total number of tests that will be run.
     */
    void start_global(size_t num_suites, size_t num_total_tests) noexcept;

    /** Finish the global testing.
     *
     * @post Callculate duration.
     * @post Print to std::out
     */
    void finish_global() noexcept;

    /** Start a suite of tests.
     *
     * @post Store arguments.
     * @post Start timer.
     * @post Print to std::out
     * @param file The source file where the test is located (__FILE__).
     * @param line The line number where the test is located (__LINE__).
     * @param suite_name The name of the suite.
     * @param num_test The number of tests in this suite that will be run.
     */
    void start_suite(std::string_view suite_name, size_t num_tests) noexcept;

    /** Finish the suite of tests.
     *
     * @post Callculate duration.
     * @post Print to std::out
     */
    void finish_suite() noexcept;

    /** Start a test.
     *
     * @post Store arguments.
     * @post Start timer.
     * @post Print to std::out
     * @param file The source file where the test is located (__FILE__).
     * @param line The line number where the test is located (__LINE__).
     * @param test_name The name of the test.
     */
    void start_test(std::string_view file, int line, std::string_view test_name) noexcept;

    /** Finish the test.
     *
     * @post Callculate duration.
     * @post Print to std::out
     */
    void finish_test() noexcept;

    /** Start a check.
     *
     * @post Start timer.
     * @param file The source file where the check is located (__FILE__).
     * @param line The line number where the check is located (__LINE__).
     * @param default_failure_message The message to show if the check does not complete (used for checking throwing).
     */
    void start_check(
        std::string_view file,
        int line,
        std::optional<std::string> default_failure_message = std::string{"check did not run."}) noexcept;

    /** Finish a check.
     *
     * @post Callculate duration.
     * @post Print to std::out
     * @return true if the check was succesful.
     */
    bool finish_check() noexcept;

    /** Finish a check.
     *
     * @post Callculate duration.
     * @post Print to std::out
     * @param failure_message An optional string for the error message to display, or std::nullopt if there was no error.
     * @return true if the check was succesful.
     */
    bool finish_check(std::optional<std::string> failure_message) noexcept;

private:
    struct test_result_type {
        std::string_view name;
        std::string_view file;
        int line;
        duration_type time = {};
        time_point_type timestamp = {};
        std::optional<std::string> failure = std::nullopt;

        void output_xml(FILE *out, std::string_view suite_name) noexcept
        {
            using namespace std::literals;
            
            std::print(
                out,
                "    <testcase name=\"{}\" file=\"{}\" line=\"{}\" status=\"run\" result=\"completed\" time=\"{}\" timestamp=\"{:%Y-%m-%dT%H:%M:%S.000}\" classname=\"{}\"",
                name,
                file,
                line,
                time / 1s,
                timestamp);

            if (failure) {

            }
        }
    };

    struct suite_result_type {
        std::string_view name;
        size_t num_tests = 0;
        size_t num_failures = 0;
        duration_type time = {};
        time_point_type timestamp = {};
        std::vector<test_result_type> test_results = {};

        void output_xml(FILE *out) noexcept
        {
            using namespace std::literals;

            std::println(
                out,
                "  <testsuite name=\"{}\" tests=\"{}\" failures=\"{}\" disabled=\"0\" skipped=\"0\" errors=\"0\" time=\"{:.3}\" timestamp=\"{%Y-%m=%dT%H:%M:%S.000}\">",
                name,
                num_tests,
                num_failures,
                time / 1s,
                timestamp);

            for (auto const &test_result : test_results) {
                test_result.output_xml(out, name);
            }

            std::println(out, "  </testsuite>");
        }
    };

    std::string_view _check_file = {};
    int _check_line = 0;
    std::optional<std::string> _check_failure_message = {};

    std::string_view _test_file = {};
    int _test_line = 0;
    std::string_view _test_name = {};
    time_point_type _test_tp = {};
    bool _test_failed = false;

    std::string_view _suite_name = {};
    time_point_type _suite_tp = {};
    size_t _suite_num_tests = 0;

    size_t _num_suites = 0;
    size_t _num_total_tests = 0;
    time_point_type _global_tp = {};

    std::vector<std::string> _failed_tests_summary = {};
    std::vector<suite_result_type> _suite_results = {};
};

class filter {
public:
    [[nodiscard]] bool match_suite(std::string_view suite) const noexcept;
    [[nodiscard]] bool match_test(std::string_view suite, std::string_view test) const noexcept;
};

struct test_entry {
    std::string_view file;
    int line;
    std::string_view name;
};

struct suite_entry {
    std::string_view name;
    std::function<size_t(filter const&)> num_tests;
    std::function<std::vector<test_entry>(filter const&)> list_tests;
    std::function<void(trace&, filter const&)> run_tests;
};

inline std::vector<suite_entry> suites;

template<typename Suite>
[[nodiscard]] inline bool register_suite(std::string_view name) noexcept
{
    auto num_tests = [](filter const& filter) -> size_t {
        return Suite::num_tests(filter);
    };

    auto list_tests = [](filter const& filter) -> std::vector<test_entry> {
        auto r = std::vector<test_entry>{}; 
        for (auto const &test : Suite::list_tests(filter)) {
            r.emplace_back(test.file, test.line, test.name);
        }
        return r;
    };

    auto run_tests = [](trace &r, filter const& filter) -> void {
        return Suite::run_tests(r, filter);
    };

    suites.emplace_back(name, std::move(num_tests), std::move(list_tests), std::move(run_tests));
    return true;
}

template<typename Suite>
class suite {
public:
    using suite_type = Suite;
    inline static std::string_view suite_file = {};
    inline static int suite_line = std::numeric_limits<int>::max();

    using test_ptr = void (suite_type::*)(trace&);
    struct test_type {
        std::string_view file;
        int line;
        std::string_view name;
        test_ptr test;
    };

    [[nodiscard]] static std::string_view suite_name() noexcept
    {
        return typeid(Suite).name();
    }

    [[nodiscard]] static size_t num_tests(filter const& filter) noexcept
    {
        return std::ranges::count_if(_hikotest_tests, [&filter](auto const &test) {
            return filter.match_test(suite_name(), test.name);
        });
    }

    [[nodiscard]] static std::vector<test_type> list_tests(filter const& filter) noexcept
    {
        auto r = std::ranges::to<std::vector>(std::views::filter(_hikotest_tests, [&filter](auto const& test) {
            return filter.match_test(suite_name(), test.name);
        }));

        std::ranges::sort(r, [](auto const& a, auto const& b) {
            return a.name < b.name;
        });

        return r;
    }

    static void run_tests(trace& r, filter const& filter)
    {
        auto const tests_to_run = list_tests(filter);

        r.start_suite(suite_name(), tests_to_run.size());
        for (auto const& test : tests_to_run) {
            r.start_test(test.file, test.line, test.name);
            (suite_type{}.*test.test)(r);
            r.finish_test();
        }
        r.finish_suite();
    }

    static bool register_test(std::string_view file, int line, std::string_view name, test_ptr test) noexcept
    {
        _hikotest_tests.emplace_back(file, line, name, test);
        return true;
    }

private:
    inline static bool _hikotest_registered = register_suite<suite_type>(suite_name());
    inline static std::vector<test_type> _hikotest_tests = {};
};

} // namespace test
