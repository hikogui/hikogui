

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
        if (auto _hikotest_result = (expression <=> ::test::error{__VA_ARGS__}); not _hikotest_result) { \
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

[[nodiscard]] std::string type_name_strip(std::string type);

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

struct test_case {
    std::string_view file;
    int line;
    std::string suite_name;
    std::string test_name;
    std::function<std::expected<void, std::string>()> _run_test;

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
[[nodiscard]] inline test_case&
register_test(std::expected<void, std::string> (Suite::*test)(), std::string_view file, int line, std::string name) noexcept
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
