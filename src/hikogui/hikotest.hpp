

#include <concepts>
#include <utility>
#include <type_traits>
#include <functional>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <chrono>

namespace test {

/** Declare a test suite.
 *
 * @param id The class-name of the test suite.
 * @param ... The optional name of the test suite as a string-literal.
 */
#define TEST_SUITE(id, ...) class id : public ::test::suite<id __VA_OPT__(, ) __VA_ARGS__>

/** Delcare a test case
 *
 * @param id The method-name of the test case.
 * @param ... The optional name of the test case as a string-literal.
 */
#define TEST_CASE(id, ...) \
    void hikotest_##id##_wrapper(::test::result& hikotest_result) \
    { \
        return id(hikotest_result); \
    } \
    inline static bool hikotest_##id##_dummy = \
        register_test(__FILE__, __LINE__, #id, &suite_type::hikotest_##id##_wrapper __VA_OPT__(, ) __VA_ARGS__); \
    void id(::test::result& hikotest_result)

/** Check an expression
 *
 * @param expression A comparison or boolean expression to check.
 */
#define CHECK(expression) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression); \
    hikotest_result.check(::test::expression_tag{} <=> expression); \
    hikotest_result.finish_check();

/** Check a floating point comparison
 *
 * @param expression A floating point comparison expression.
 * @param error The absolute error of the values in the comparison to still be valid.
 */
#define CHECK_NEAR(expression, error) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression); \
    hikotest_result.check_near(error, ::test::expression_tag{} <=> expression) hikotest_result.finish_check();

/** Check if a expression throws an exception
 *
 * @param expression The expression which causes an exception to be thrown.
 * @param exception The exception to be thrown.
 */
#define CHECK_THROWS(expression, exception) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression, "Check did not throw '" #exception "'."); \
    try { \
        (void)expression; \
    } catch (exception const&) { \
        hikotest_result.throw_success(); \
    } \
    hikotest_result.finish_check();

/** Check an expression
 *
 * @param expression A comparison or boolean expression to check.
 */
#define REQUIRE(expression) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression); \
    hikotest_result.check(::test::expression_tag{} <=> expression); \
    do { \
        if (not hikotest_result.finish_check()) { \
            return; \
        } \
    } while (false)

/** Check a floating point comparison
 *
 * @param expression A floating point comparison expression.
 * @param error The absolute error of the values in the comparison to still be valid.
 */
#define REQUIRE_NEAR(expression, error) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression); \
    hikotest_result.check_near(error, ::test::expression_tag{} <=> expression); \
    do { \
        if (not hikotest_result.finish_check()) { \
            return; \
        } \
    } while (false)

/** Check if a expression throws an exception
 *
 * @param expression The expression which causes an exception to be thrown.
 * @param exception The exception to be thrown.
 */
#define REQUIRE_THROWS(expression, exception) \
    hikotest_result.start_check(__FILE__, __LINE__, #expression, "Check did not throw '" #exception "'."); \
    try { \
        (void)expression; \
    } catch (exception const&) { \
        hikotest_result.throw_success(); \
    } \
    do { \
        if (not hikotest_result.finish_check()) { \
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

    std::array<char, N> _str;
};

fixed_string() -> fixed_string<0>;

template<std::size_t O>
fixed_string(char const (&str)[O]) -> fixed_string<O - 1>;

template<typename LHS>
struct left_operand {
    constexpr left_operand(LHS const& lhs) : _lhs(lhs) {}

    LHS const& _lhs;
};

enum class comparator { eq, ne, lt, gt, le, ge };

template<comparator Comparator, typename LHS, typename RHS>
struct compare_expression {
    using common_type = std::common_type_t<LHS, RHS>;
    constexpr comparator comparator = Comparator;

    constexpr check_expression(LHS const& lhs, RHS const& rhs) : _lhs(lhs), _rhs(rhs) {}

    LHS const& _lhs;
    RHS const& _rhs;
};

class expression_tag {};

template<typename LHS>
[[nodiscard]] constexpr left_operand<LHS> operator<=>(expression_tag, LHS const& lhs)
{
    using value_type = std::remove_reference_t<LHS>;
    return {lhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare_expression<comparator::eq, LHS, RHS> operator==(left_operand<LHS> const& lhs, RHS const& rhs)
{
    return {lhs._lhs, rhs};
}

class result {
public:
    using clock_type = std::chrono::high_resolution_clock;
    using time_point_type = std::chrono::time_point<clock_type>;

    void start_global(size_t num_suites, size_t num_total_tests) noexcept
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

    void finish_global() noexcept
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

    void start_suite(char const *file, int line, char const *suite_name, size_t num_tests) noexcept
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

    bool finish_suite() noexcept
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

    void start_test(char const *file, int line, char const *test_name) noexcept
    {
        _test_file = file;
        _test_line = line;
        _test_name = test_name;
        _test_failed = false;

        std::println(stdout, "[ RUN      ] {}.{}", _suite_name, _test_name);
        std::fflush(stdout);
        _test_tp = clock_type::now();
    }

    bool finish_test() noexcept
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

    void start_check(
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

    bool finish_check() noexcept
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

    template<typename Arg>
    [[nodiscard]] std::string make_string(Arg const& arg) noexcept
    {
        std::stringbuf buf;

        if constexpr (requires { std::format("{}", arg); }) {
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
            return std::format("<{}>", bytes);
        }
    }

    template<comparator Comparator, typename LHS, typename RHS>
    void check(compare_expression<Comparator, LHS, RHS> const& expression) noexcept
    {
        if (expression.success()) {
            _check_failure = std::nullopt;
        } else {
            _check_failure = std::format(
                "Expected {} of these values:\n  {}\n  {}",
                expression.value_relation(),
                make_string(expression._lhs),
                make_string(expression._rhs));
        }
    }

    void check(left_operand<bool> const& expression) noexcept
    {
        if (expression.success()) {
            _check_failure = std::nullopt;
        } else {
            _check_failure = std::string("Expected expression to be true");
        }
    }

    template<comparator Comparator, typename LHS, typename RHS>
    void check_near(double error, compare_expression<Comparator, LHS, RHS> const& expression) noexcept
    {
        if (expression.difference() <= error) {
            _check_failure = std::nullopt;
        } else {
            _check_failure = std::format(
                "Expected {} of these values be within {}:\n  {}\n  {}",
                expression.value_relation(),
                error,
                make_string(expression._lhs),
                make_string(expression._rhs));
        }
    }

    void throw_success()
    {
        _check_failure = std::nullopt;
    }

private:
    /** The failure message of the check.
     *
     * The failure message can have the following values:
     *  - std::nullopt   The check was successfully completed.
     *  - std::string{}  The check has not completed.
     *  - otherwise      The failure message from the completed check.
     */
    std::optional<std::string> _failure_message = std::string{};

    size_t _num_total_tests = 0;
    std::vector<std::string> _failed_tests_summary = {};

    char const *_check_file = nullptr;
    int _check_line = 0;
    char const *_check_expression_str = nullptr;

    char const *_test_file = nullptr;
    int _test_line = 0;
    char const *_test_name = nullptr;
    time_point_type _test_start_tp = {};

    char const *_suite_file = nullptr;
    int _suite_line = 0;
    char const *_suite_name = nullptr;
    time_point_type _suite_start_tp = {};
};

class suite_base {
public:
    constexpr suite_base() noexcept = default;
    ~suite_base() = default;
};

template<typename Suite, fixed_string SuiteName = "">
class suite : public suite_base {
    using suite_type = Suite;
};

} // namespace test
