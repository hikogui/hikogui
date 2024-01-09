

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

template<typename Arg>
[[nodiscard]] std::string operand_to_string(Arg const& arg) noexcept
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

/** Left operand of a comparison, or a boolean.
 */
template<typename T>
struct operand {
    constexpr left_operand(T const& v) noexcept : v(v) {}

    T const& v;
};

enum class comparator { eq, ne, lt, gt, le, ge };

template<comparator Comparator, typename LHS, typename RHS>
struct compare {
    using lhs_type = LHS;
    using rhs_type = RHS;
    constexpr static comparator comparator = Comparator;

    constexpr compare(LHS const& lhs, RHS const& rhs) noexcept : lhs(lhs), rhs(rhs) {}

    LHS const& lhs;
    RHS const& rhs;

    [[nodiscard]] constexpr std::string left() noexcept
    {
        return operand_to_string(lhs);
    }

    [[nodiscard]] constexpr std::string right() noexcept
    {
        return operand_to_string(rhs);
    }

    [[nodiscard]] constexpr bool compare_eq() noexcept
    {
        if constexpr (requires { std::equal_to<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::equal_to<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else constexpr (requires { lhs == rhs -> std::convertible_to<bool>; }) {
            return lhs == rhs;
        } else constexpr (requires { std::ranges::equal(lhs, rhs); }) {
            return std::ranges::equal(lhs, rhs);
        }
    }

    [[nodiscard]] constexpr bool compare_ne() noexcept
    {
        if constexpr (requires { std::not_equal_to<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::not_equal_to<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else constexpr (requires { lhs != rhs -> std::convertible_to<bool>; }) {
            return lhs != rhs;
        } else {
            return not compare_eq();
        }
    }

    [[nodiscard]] constexpr bool compare_lt() noexcept
    {
        if constexpr (requires { std::less<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::less<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else {
            return lhs < rhs;
        }
    }

    [[nodiscard]] constexpr bool compare_gt() noexcept
    {
        if constexpr (requires { std::greater<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::greater<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else {
            return lhs > rhs;
        }
    }

    [[nodiscard]] constexpr bool compare_le() noexcept
    {
        if constexpr (requires { std::less_equal<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::less_equal<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else {
            return lhs <= rhs;
        }
    }

    [[nodiscard]] constexpr bool compare_ge() noexcept
    {
        if constexpr (requires { std::greater_equal<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs); }) {
            return std::greater_equal<std::common_type_t<lhs_type, rhs_type>>{}(lhs, rhs);
        } else {
            return lhs >= rhs;
        }
    }

    [[nodiscard]] constexpr bool compare() noexcept
    {
        if constexpr (comparator == comparator::eq) {
            return compare_eq();
        } else if constexpr (comparator == comparator::ne) {
            return compare_ne();
        } else if constexpr (comparator == comparator::lt) {
            return compare_lt();
        } else if constexpr (comparator == comparator::gt) {
            return compare_gt();
        } else if constexpr (comparator == comparator::le) {
            return compare_le();
        } else if constexpr (comparator == comparator::ge) {
            return compare_ge();
        } else {
            std::println(stderr, "Unknown comparator {}", std::to_underlying(comparator));
            std::terminate();
        }
    }
};

/** expression-tag used on the left side of the spaceship operator.
 *
 * This tag is used to cause the operator<=> to bind to any other type.
 */
class expression_tag {};

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
template<typename LHS>
[[nodiscard]] constexpr operand<LHS> operator<=>(expression_tag, LHS const& lhs) noexcept
{
    using value_type = std::remove_reference_t<LHS>;
    return {lhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::eq, LHS, RHS> operator==(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::ne, LHS, RHS> operator!=(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::lt, LHS, RHS> operator<(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::gt, LHS, RHS> operator>(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::le, LHS, RHS> operator<=(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr compare<comparator::ge, LHS, RHS> operator>=(operand<LHS> const& lhs, RHS const& rhs) noexcept
{
    return {lhs.v, rhs};
}


class result {
public:
    using clock_type = std::chrono::high_resolution_clock;
    using time_point_type = std::chrono::time_point<clock_type>;

    /** Start the global testing.
     *
     * @side Store arguments.
     * @side Start timer. 
     * @side Print to std::out
     * @param num_suites The number of suites that will be run.
     * @param num_total_test The total number of tests that will be run.
     */
    void start_global(size_t num_suites, size_t num_total_tests) noexcept;

    /** Finish the global testing.
     *
     * @side Callculate duration. 
     * @side Print to std::out
     * @return true if all test where succesful.
     */
    bool finish_global() noexcept;

    /** Start a suite of tests.
     *
     * @side Store arguments.
     * @side Start timer. 
     * @side Print to std::out
     * @param file The source file where the test is located (__FILE__).
     * @param line The line number where the test is located (__LINE__).
     * @param suite_name The name of the suite.
     * @param num_test The number of tests in this suite that will be run.
     */
    void start_suite(char const *file, int line, char const *suite_name, size_t num_tests) noexcept;

    /** Finish the suite of tests.
     *
     * @side Callculate duration. 
     * @side Print to std::out
     * @return true if all test of a suite where succesful.
     */
    bool finish_suite() noexcept;

    /** Start a test.
     *
     * @side Store arguments.
     * @side Start timer. 
     * @side Print to std::out
     * @param file The source file where the test is located (__FILE__).
     * @param line The line number where the test is located (__LINE__).
     * @param test_name The name of the test.
     */
    void start_test(char const *file, int line, char const *test_name) noexcept;

    /** Finish the test.
     *
     * @side Callculate duration. 
     * @side Print to std::out
     * @return true if all checks of the test where succesful.
     */
    bool finish_test() noexcept;

    /** Start a check.
     *
     * @side Start timer.
     * @param file The source file where the check is located (__FILE__).
     * @param line The line number where the check is located (__LINE__).
     * @param expression_str The expression being checked as a string.
     * @param default_failure_message The message to show if the check does not complete (used for checking throwing).
     */
    void start_check(
        char const *file,
        int line,
        char const *expression_str,
        std::optional<std::string> default_failure_message = std::string{"check did not run."}) noexcept;

    /** Finish a check.
     *
     * @side Callculate duration. 
     * @side Print to std::out
     * @return true if the check was succesful.
     */
    bool finish_check() noexcept;


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

class suite_base;

struct suite_entry {
    std::string_view file;
    int line;
    std::string_view name;
    std::function<std::unique_ptr<suite_base>()> make_suite;
};

inline std::vector<suite_entry> suites;

template<typename Suite>
[[nodiscard]] inline bool register_suite(std::string_view file, int line, std::string_view name) noexcept
{
    auto const it = std::lower_bound(suites.cbegin(), suites.cend(), name, [](auto const &item, auto const &name) {
            return item.name < name;
    });

    if (it != suites.end() and it->name == name) {
        std::println(stderr, "Suite '{}' already exists.", SuiteName);
        std::terminate();
    }

    suites.insert(it, suite_entry{file, line, name, []() -> std::unique_ptr<suite_base> { return std::make_unique<Suite>(); });
}

[[nodiscard]] std::vector<suite_entry> filter_suites(std::vector<std::string> filters) noexcept;

/** Interface for a suite.
 */
class suite_intf {
public:
    struct test_type {
        std::string_view file;
        int line;
        std::string_view name;
    };

    constexpr suite_intf() noexcept = default;
    ~suite_intf() = default;

    /** Get a list of tests.
     *
     * This is used by the IDE to show a list of test and the file and line numbers.
     *
     * @note The returned list is alphabetically sorted by the name.
     * @return A list of tests for this suite.
     */
    [[nodiscard]] virtual std::vector<test_type> list_of_tests() const noexcept = 0;

    /** Run the tests matching the filter.
     */
    [[nodiscard]] virtual void run_tests(result &r, filter const &filter) = 0;
};

template<typename Suite, fixed_string SuiteName = "">
class suite : public suite_intf {
public:
    constexpr static fixed_string suite_name = SuiteName;
    using suite_type = Suite;

    [[nodiscard]] std::vector<test_type> list_of_tests() const noexcept override
    {
        auto r = std::vector<test_type>{};
        r.reserve(_hikotest_tests.size());

        for (auto const &test: _hikotest_tests) {
            r.emplace_back(test.file, test.line, test.name);
        }

        return r;
    }

    void run_test(result &r, std::string_view name) override
    {
        auto const it = std::lower_bound(_hikotest_tests.cbegin(), _hikotest_tests.cend(), name, [](auto const &item, auto const &name) {
            return item.name < name;
        });

        if (it == _hikotest_tests.end() or it->name != name) {
            std::println(stderr, "Could not find test '{}' in suite '{}'", name, suite_name);
            std::terminate();
        }

        r.start_test(it->file, it->line, it->name);
        it->test(r);
        r.finish_test();
    }

    bool register_test(std::string_view file, int line, std::string_view name, test_ptr test) noexcept
    {
        auto const it = std::lower_bound(_hikotest_tests.cbegin(), _hikotest_tests.cend(), name, [](auto const &item, auto const &name) {
            return item.name < name;
        });

        if (it != _hikotest_tests.end() and it->name == name) {
            std::println(stderr, "Test '{}' in suite '{}' already exists.", name, suite_name);
            std::terminate();
        }

        _hikotest_test.insert(it, _hikotest_test_type{file, line, name, test});
    }

private:
    using test_ptr = void (suite_type::*)(result &);

    struct _hikotest_test_type {
        std::string_view file;
        int line;
        std::string_view *name;
        test_ptr test; 
    };

    inline static bool _hikotest_suite_dummy = register_suite<suite_type, SuiteName>();
    inline static std::vector<_hikotest_test_type> _hikotest_tests = {};
};

} // namespace test
