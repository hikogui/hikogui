

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
#include <stdexcept>

namespace test {

/** Declare a test suite
 *
 * @note It is recommended to use the suffix `_test_suite` on the id,
 *       to reduce collisions with symbols being tested.
 *       The suffixed `_test_suite`, `_suite`, `_test`, `_tests` will
 *       be stripped from the name.
 * @param id The struct-name of the test suite.
 */
#define TEST_SUITE(id) \
    struct id; \
    inline auto _hikotest_registered_##id = std::addressof(::test::register_suite<id>()); \
    struct id : ::test::suite<id>

/** Delcare a test case
 *
 * @note It is recommended to use the suffix `_test_case` on the id,
 *       to reduce collisions with symbols being tested.
 *       The suffixed `_test_case`, `_case`, `_test` will
 *       be stripped from the name.
 * @param id The method-name of the test case.
 */
#define TEST_CASE(id) \
    void _hikotest_wrap_##id() \
    { \
        return id(); \
    } \
    inline static auto _hikotest_registered_##id = \
        std::addressof(::test::register_test(&_hikotest_suite_type::_hikotest_wrap_##id, __FILE__, __LINE__, #id)); \
    void id()

/** Check an expression
 *
 * @param expression A comparison or boolean expression to check.
 * @param ... The optional error value. If it is a double value than it is the
 *            absolute error value. You may also pass in a `::test::error`.
 */
#define REQUIRE(expression, ...) ::test::require(__FILE__, __LINE__, expression <=> ::test::error{__VA_ARGS__})

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
            ::test::require(__FILE__, __LINE__, std::unexpected{std::string{#expression " did not throw " #exception "."}}); \
        } \
    } while (false)

/** Force inline a function.
 *
 * Some compilers will inline even in debug mode, this will allow a breakpoint
 * to appear at the call site of an forced-inlined function.
 */
#if defined(_MSC_VER)
#define TEST_FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#define TEST_FORCE_INLINE __attribute__((always_inline))
#else
#error "TEST_FORCE_INLINE not implemented"
#endif

/** A breakpoint.
 *
 * The debugger will break at the position of this instruction.
 */
#if defined(_MSC_VER)
#define TEST_BREAKPOINT() __debugbreak()
#elif defined(__GNUC__)
#define TEST_BREAKPOINT() __builtin_debugtrap()
#else
#error "BREAKPOINT not implemented"
#endif

using hr_clock_type = std::chrono::high_resolution_clock;
using hr_duration_type = std::chrono::duration<double>;
using hr_time_point_type = std::chrono::time_point<hr_clock_type>;
using utc_clock_type = std::chrono::utc_clock;
using utc_time_point_type = utc_clock_type::time_point;

/** Break the unit-test on failure.
 *
 * When:
 *  - true: On failure a break point is set, and the tests are terminated..
 *  - false: Errors are catched and the tests continue.
 */
inline bool break_on_failure = false;

/** Strip a type-name.
 *
 * This removes the following from a type-name:
 *  - spaces
 *  - `struct`
 *  - `class`
 */
[[nodiscard]] std::string type_name_strip(std::string type);

/** Get the type-name of a type.
 *
 * @tparam T The type to query.
 * @return The name of the type @a T.
 */
template<typename T>
[[nodiscard]] std::string type_name() noexcept
{
#if defined(_MSC_VER)
    auto signature = std::string_view{__FUNCSIG__};
#elif defined(__GNUC__)
    auto signature = std::string_view{__PRETTY_FUNCTION__};
#else
#error "type_name() not implemented"
#endif

    // constexpr std::string class_name() [with T = foo<bar>; std::string_view = std::basic_string_view<char>]
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

    // class std::basic_string<char,struct std::char_traits<char> > __cdecl test::type_name<struct foo<struct bar>>(void) noexcept
    if (auto first = signature.find("::type_name<"); first != signature.npos) {
        first += 12;
        auto const last = signature.rfind(">(void)");
        return type_name_strip(std::string{signature.substr(first, last - first)});
    }

    std::println(stderr, "{}({}): error: Could not parse type_name from '{}'", __FILE__, __LINE__, signature);
    std::terminate();
}

/** Make a string representation of a value.
 *
 * @param arg The value to convert to a represenation.
 * @return The string representation.
 */
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

/** The class of comparison error.
 */
enum class error_class {
    /** The operands can be compared exactly.
     */
    exact,

    /** The operands can be compared with an absolute error epsilon.
     */
    absolute,

    /** The operands can be compared with an relative error epsilon.
     */
    relative
};

/** The comparison error.
 *
 * The comparison error will apear on the right side of the
 * spaceship operator.
 *
 * This error will bind to the right hand side operand of a comparison.
 */
template<error_class ErrorClass = error_class::exact>
struct error {
    double v = 0.0;

    /** Get the error value as a positive number.
     */
    [[nodiscard]] constexpr double operator+() const noexcept
    {
        return v;
    }

    /** Get the error value as a negative  number.
     */
    [[nodiscard]] constexpr double operator-() const noexcept
    {
        return -v;
    }
};

/** By default an error with a value is an absolute-error.
 */
error(double) -> error<error_class::absolute>;

/** Operand of a comparison, bound to an error-value.
 *
 * An `operand` is the result of the spaceship-operator between:
 *  - lhs: The rhs of a comparison operator.
 *    (the operand becomes the rhs of the comparison operator).
 *  - rhs: Any `error<>` value.
 */
template<error_class ErrorClass, typename T>
struct operand {
    using value_type = T;

    ::test::error<ErrorClass> e;
    value_type const& v;

    operand(error<ErrorClass> error, value_type const &value) noexcept : e(error), v(value) {}

};

/** Result of the boolean expression.
 *
 * The `operand` is the result of a spaceship-operator between:
 *  - lhs: A boolean expression.
 *  - rhs: An `error<error_class::exact>` value.
 */
template<>
struct operand<error_class::exact, bool> {
    using value_type = bool;

    value_type const& v;

    operand(error<error_class::exact>, value_type const &value) noexcept : v(value) {}

    /** Convert the boolean result as an std::expected.
     *
     * This allows an expression to be directly used as an argument to `check()`.
     */
    operator std::expected<void, std::string>() const noexcept
    {
        if (v) {
            return {};
        } else {
            return std::unexpected{std::string{"expression was false"}};
        }
    }
};


/** The spaceship operator is used to split a comparison-expression.
 *
 * The spaceship operator has a slightly higher precedence then the
 * comparison operators: ==, !=, <, >, <=, =>.
 *
 * The spaceship operator also is not often used in unit-test expression,
 * which means that this is the perfect operator to wrap to the rhs
 * comparison expression.
 *
 * By wrapping the operand we can now overload the normal comparison operators.
 * So that the unit-test framework can do a custom comparison.
 *
 * @param lhs The left-hand-side operand.
 * @return The wrapped left-hand-side operand.
 */
template<typename RHS, error_class ErrorClass>
[[nodiscard]] constexpr operand<ErrorClass, RHS> operator<=>(RHS const& rhs, error<ErrorClass> e) noexcept
{
    return {e, rhs};
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::expected<void, std::string>
operator==(LHS const& lhs, operand<error_class::exact, RHS> const& rhs) noexcept
{
    // clang-format off
    if constexpr (requires { { lhs == rhs.v } -> std::convertible_to<bool>; }) {
        if (lhs == rhs.v) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs.v))};
        }

    } else if constexpr (requires { std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs.v); }) {
        if (std::equal_to<std::common_type_t<LHS, RHS>>{}(lhs, rhs.v)) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs.v))};
        }

    } else if constexpr (requires { std::ranges::equal(lhs, rhs.v); }) {
        if (std::ranges::equal(lhs, rhs.v)) {
            return {};
        } else {
            return std::unexpected{
                std::format("Expected equality of these values:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs.v))};
        }

    } else {
        []<bool Flag = false>() {
            static_assert(Flag, "hikotest: Unable to equality-compare two values.");
        }();
    }
    // clang-format on
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
operator==(LHS const& lhs, operand<error_class::absolute, RHS> const& rhs) noexcept requires diff_ordered<LHS, RHS, double>
{
    auto const diff = lhs - rhs.v;
    if (diff >= -rhs.e and diff <= +rhs.e) {
        return {};
    } else {
        return std::unexpected{std::format(
            "Expected equality within {} of these values:\n  {}\n  {}",
            +rhs.e,
            operand_to_string(lhs),
            operand_to_string(rhs.v))};
    }
}

template<typename LHS, typename RHS>
[[nodiscard]] constexpr std::expected<void, std::string>
operator==(LHS const& lhs, operand<error_class::absolute, RHS> const& rhs) noexcept
    requires(not diff_ordered<LHS, RHS, double>) and range_diff_ordered<LHS, RHS, double>
{
    auto lit = lhs.begin();
    auto rit = rhs.v.begin();

    auto const lend = lhs.end();
    auto const rend = rhs.v.end();

    while (lit != lend and rit != rend) {
        auto const diff = *lit - *rit;
        if (diff < -rhs.e or diff > +rhs.e) {
            return std::unexpected{std::format(
                "Expected equality within {} of these values:\n  {}\n  {}",
                +rhs.e,
                operand_to_string(lhs),
                operand_to_string(rhs.v))};
        }

        ++lit;
        ++rit;
    }

    if (lit != lend or rit != rend) {
        return std::unexpected{std::format(
            "Expected both range-values to the same size:\n  {}\n  {}", operand_to_string(lhs), operand_to_string(rhs.v))};
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

class require_error : public std::logic_error {
    using std::logic_error::logic_error;
};

TEST_FORCE_INLINE void require(char const* file, int line, std::expected<void, std::string> result)
{
    if (result) {
        return;

    } else if (not break_on_failure) {
        throw require_error(std::format("{}({}): error: {}", file, line, result.error()));

    } else {
        TEST_BREAKPOINT();
        std::terminate();
    }
}

struct test_case {
    std::string_view file;
    int line;
    std::string suite_name;
    std::string test_name;
    std::function<void()> _run_test;

    struct result_type {
        test_case const* parent;
        utc_time_point_type time_stamp;
        hr_time_point_type time_point;
        hr_duration_type duration = {};
        std::string error_message = {};
        bool completed = false;

        result_type(result_type const&) noexcept = default;
        result_type(result_type&&) noexcept = default;
        result_type& operator=(result_type const&) noexcept = default;
        result_type& operator=(result_type&&) noexcept = default;
        result_type(test_case const* parent) noexcept;
        [[nodiscard]] std::string suite_name() const noexcept;
        [[nodiscard]] std::string test_name() const noexcept;
        [[nodiscard]] std::string_view file() const noexcept;
        [[nodiscard]] int line() const noexcept;
        [[nodiscard]] bool success() const noexcept;
        [[nodiscard]] bool failure() const noexcept;
        [[nodiscard]] bool skipped() const noexcept;
        void set_success() noexcept;
        void set_failure(std::string message) noexcept;
        void junit_xml(FILE* out) const noexcept;
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
        void (Suite::*test)()) noexcept :
        file(file), line(line), suite_name(std::move(suite_name)), test_name(std::move(test_name)), _run_test([test]() {
            return (Suite{}.*test)();
        })
    {
    }

    [[nodiscard]] result_type run_test_break() const;
    [[nodiscard]] result_type run_test_catch() const;
    [[nodiscard]] result_type run_test() const;
    [[nodiscard]] result_type layout() const noexcept;
};

struct test_suite {
    std::string suite_name;
    std::vector<test_case> tests;

    test_suite(std::string suite_name) noexcept : suite_name(std::move(suite_name)) {}

    struct result_type {
        using const_iterator = std::vector<test_case::result_type>::const_iterator;

        test_suite const* parent;
        utc_time_point_type time_stamp = {};
        hr_time_point_type time_point = {};
        hr_duration_type duration = {};
        std::vector<test_case::result_type> test_results;
        bool completed = false;

        result_type(result_type const&) noexcept = default;
        result_type(result_type&&) noexcept = default;
        result_type& operator=(result_type const&) noexcept = default;
        result_type& operator=(result_type&&) noexcept = default;
        result_type(test_suite const* parent) noexcept;
        [[nodiscard]] std::string suite_name() const noexcept;
        [[nodiscard]] size_t num_tests() const noexcept;
        [[nodiscard]] size_t num_failures() const noexcept;
        [[nodiscard]] size_t num_success() const noexcept;
        [[nodiscard]] size_t num_skipped() const noexcept;
        [[nodiscard]] size_t num_disabled() const noexcept;
        [[nodiscard]] size_t num_errors() const noexcept;
        [[nodiscard]] const_iterator begin() const noexcept;
        [[nodiscard]] const_iterator end() const noexcept;
        void push_back(test_case::result_type test_result) noexcept;
        void finish() noexcept;
        void junit_xml(FILE* out) const noexcept;
    };

    [[nodiscard]] result_type layout(filter const& filter) const noexcept;
    [[nodiscard]] result_type run_tests(filter const& filter) const;
};

struct all_tests {
    struct result_type {
        using const_iterator = std::vector<test_suite::result_type>::const_iterator;

        all_tests* parent;
        utc_time_point_type time_stamp = {};
        hr_time_point_type time_point = {};
        hr_duration_type duration = {};
        std::vector<test_suite::result_type> suite_results;
        bool completed = false;

        result_type(all_tests* parent) noexcept;
        void finish() noexcept;
        [[nodiscard]] size_t num_suites() const noexcept;
        [[nodiscard]] size_t num_tests() const noexcept;
        [[nodiscard]] size_t num_failures() const noexcept;
        [[nodiscard]] size_t num_success() const noexcept;
        [[nodiscard]] size_t num_disabled() const noexcept;
        [[nodiscard]] size_t num_skipped() const noexcept;
        [[nodiscard]] size_t num_errors() const noexcept;
        [[nodiscard]] std::vector<std::string> fqnames_of_failed_tests() const noexcept;
        [[nodiscard]] const_iterator begin() const noexcept;
        [[nodiscard]] const_iterator end() const noexcept;
        void push_back(test_suite::result_type suite_result) noexcept;
        void junit_xml(FILE* out) const noexcept;
    };

    std::vector<test_suite> suites;
    mutable size_t last_registered_suite = 0;

    template<typename Suite>
    [[nodiscard]] inline test_suite& register_suite() noexcept
    {
        auto name = type_name<Suite>();

        // Remove common suffixes.
        if (name.ends_with("_test_suite")) {
            name = name.substr(0, name.size() - 11);
        } else if (name.ends_with("_suite")) {
            name = name.substr(0, name.size() - 6);
        } else if (name.ends_with("_tests")) {
            name = name.substr(0, name.size() - 6);
        } else if (name.ends_with("_test")) {
            name = name.substr(0, name.size() - 5);
        }

        // Remove namespaces.
        if (auto i = name.rfind(':'); i != name.npos) {
            name = name.substr(i + 1);
        }

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
    register_test(void (Suite::*test)(), std::string_view file, int line, std::string name) noexcept
    {
        if (name.ends_with("_test_case")) {
            name = name.substr(0, name.size() - 10);
        } else if (name.ends_with("_case")) {
            name = name.substr(0, name.size() - 5);
        } else if (name.ends_with("_test")) {
            name = name.substr(0, name.size() - 5);
        }

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

        return *tests.emplace(it, file, line, suite.suite_name, name, test);
    }

    [[nodiscard]] result_type layout(::test::filter const& filter) noexcept;
    [[nodiscard]] result_type list_tests(::test::filter const& filter) noexcept;
    [[nodiscard]] result_type run_tests(::test::filter const& filter);
};

inline auto all = all_tests{};

template<typename Suite>
[[nodiscard]] inline test_suite& register_suite() noexcept
{
    return all.template register_suite<Suite>();
}

template<typename Suite>
[[nodiscard]] inline test_case& register_test(void (Suite::*test)(), std::string_view file, int line, std::string name) noexcept
{
    return all.template register_test<Suite>(test, file, line, std::move(name));
}

inline all_tests::result_type list_tests(filter const& filter) noexcept;
[[nodiscard]] inline all_tests::result_type run_tests(filter const& filter);

template<typename Suite>
struct suite {
    using _hikotest_suite_type = Suite;
};

} // namespace test
