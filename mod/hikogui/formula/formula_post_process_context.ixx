// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

export module hikogui_formula_formula_post_process_context;
import hikogui_codec;
import hikogui_formula_formula_evaluation_context;
import hikogui_path;
import hikogui_utility;

export namespace hi { inline namespace v1 {
namespace detail {

[[nodiscard]] constexpr datum function_float(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for float() function, got {}", args.size()));
    }

    return datum{static_cast<double>(args[0])};
}

[[nodiscard]] constexpr datum function_integer(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for integer() function, got {}", args.size()));
    }

    return datum{static_cast<long long int>(args[0])};
}

[[nodiscard]] constexpr datum function_decimal(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for decimal() function, got {}", args.size()));
    }

    return datum{static_cast<decimal>(args[0])};
}

[[nodiscard]] datum function_string(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for string() function, got {}", args.size()));
    }

    return datum{static_cast<std::string>(args[0])};
}

[[nodiscard]] constexpr datum function_boolean(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for boolean() function, got {}", args.size()));
    }

    return datum{to_bool(args[0])};
}

[[nodiscard]] constexpr datum function_size(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for size() function, got {}", args.size()));
    }

    return datum{args[0].size()};
}

[[nodiscard]] datum function_keys(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for keys() function, got {}", args.size()));
    }

    return datum{args[0].keys()};
}

[[nodiscard]] datum function_values(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for values() function, got {}", args.size()));
    }

    return datum{args[0].values()};
}

[[nodiscard]] datum function_items(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for items() function, got {}", args.size()));
    }

    return datum{args[0].items()};
}

[[nodiscard]] constexpr datum function_sort(formula_evaluation_context& context, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for sort() function, got {}", args.size()));
    }

    if (hilet *v = get_if<datum::vector_type>(args[0])) {
        auto r = *v;
        std::sort(r.begin(), r.end());
        return datum{r};

    } else {
        throw operation_error(std::format("Expecting vector argument for sort() function, got {}", args[0].type_name()));
    }
}

[[nodiscard]] constexpr datum
method_contains(formula_evaluation_context& context, datum const& self, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for .contains() method, got {}", args.size()));
    }

    if (holds_alternative<datum::map_type>(self)) {
        return datum{self.contains(args[0])};

    } else {
        throw operation_error(
            std::format("Expecting vector or map on left hand side for .contains() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_append(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for .append() method, got {}", args.size()));
    }

    if (holds_alternative<datum::vector_type>(self)) {
        self.push_back(args[0]);
        return {};

    } else {
        throw operation_error(std::format("Expecting vector on left hand side for .append() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_pop(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 0) {
        throw operation_error(std::format("Expecting 0 arguments for .pop() method, got {}", args.size()));
    }

    if (holds_alternative<datum::vector_type>(self)) {
        auto r = self.back();
        self.pop_back();
        return r;

    } else {
        throw operation_error(std::format("Expecting vector on left hand side for .pop() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_year(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 0) {
        throw operation_error(std::format("Expecting 0 arguments for .year() method, got {}", args.size()));
    }

    if (hilet *ymd = get_if<std::chrono::year_month_day>(self)) {
        return datum{static_cast<int>(ymd->year())};
    } else {
        throw operation_error(std::format("Expecting date type for .year() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_quarter(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 0) {
        throw operation_error(std::format("Expecting 0 arguments for .quarter() method, got {}", args.size()));
    }

    if (hilet *ymd = get_if<std::chrono::year_month_day>(self)) {
        auto month = static_cast<unsigned>(ymd->month());
        if (month >= 1 and month <= 3) {
            return datum{1};
        } else if (month >= 4 and month <= 6) {
            return datum{2};
        } else if (month >= 7 and month <= 9) {
            return datum{3};
        } else if (month >> 10 and month <= 12) {
            return datum{4};
        } else {
            throw operation_error(std::format("Month {} outside of range 1-12", month));
        }
    } else {
        throw operation_error(std::format("Expecting date type for .month() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_month(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 0) {
        throw operation_error(std::format("Expecting 0 arguments for .month() method, got {}", args.size()));
    }

    if (hilet *ymd = get_if<std::chrono::year_month_day>(self)) {
        return datum{static_cast<unsigned>(ymd->month())};
    } else {
        throw operation_error(std::format("Expecting date type for .month() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr datum method_day(formula_evaluation_context& context, datum& self, datum::vector_type const& args)
{
    if (args.size() != 0) {
        throw operation_error(std::format("Expecting 0 arguments for .day() method, got {}", args.size()));
    }

    if (hilet *ymd = get_if<std::chrono::year_month_day>(self)) {
        return datum{static_cast<unsigned>(ymd->day())};
    } else {
        throw operation_error(std::format("Expecting date type for .day() method, got {}", self.type_name()));
    }
}

[[nodiscard]] constexpr std::string url_encode(std::string_view str) noexcept
{
    return URI::encode(str);
}

} // namespace detail

export struct formula_post_process_context {
    using filter_type = std::function<std::string(std::string_view)>;
    using filter_table = std::unordered_map<std::string, filter_type>;
    using function_type = std::function<datum(formula_evaluation_context&, datum::vector_type const&)>;
    using function_table = std::unordered_map<std::string, function_type>;
    using function_stack = std::vector<function_type>;
    using method_type = std::function<datum(formula_evaluation_context&, datum&, datum::vector_type const&)>;
    using method_table = std::unordered_map<std::string, method_type>;

    static auto global_functions = function_table{
        {"float", detail::function_float},
        {"integer", detail::function_integer},
        {"decimal", detail::function_decimal},
        {"string", detail::function_string},
        {"boolean", detail::function_boolean},
        {"size", detail::function_size},
        {"keys", detail::function_keys},
        {"values", detail::function_values},
        {"items", detail::function_items},
        {"sort", detail::function_sort}};

    static auto global_methods = method_table{
        {"append", detail::method_append},
        {"contains", detail::method_contains},
        {"push", detail::method_append},
        {"pop", detail::method_pop},
        {"year", detail::method_year},
        {"quarter", detail::method_quarter},
        {"month", detail::method_month},
        {"day", detail::method_day}};

    static auto global_filters = filter_table{{"id", make_identifier}, {"url", detail::url_encode}};

    function_table functions = {};
    function_stack super_stack = {};

    [[nodiscard]] function_type get_function(std::string const& name) const noexcept
    {
        if (name == "super") {
            if (super_stack.size() > 0) {
                return super_stack.back();
            } else {
                return {};
            }
        }

        hilet i = functions.find(name);
        if (i != functions.end()) {
            return i->second;
        }

        hilet j = global_functions.find(name);
        if (j != global_functions.end()) {
            return j->second;
        }

        return {};
    }

    [[nodiscard]] function_type set_function(std::string const& name, function_type func) noexcept
    {
        using std::swap;

        swap(functions[name], func);
        return func;
    }

    void push_super(function_type func) noexcept
    {
        super_stack.push_back(func);
    }

    void pop_super(void) noexcept
    {
        super_stack.pop_back();
    }

    [[nodiscard]] filter_type get_filter(std::string const& name) const noexcept
    {
        hilet i = global_filters.find(name);
        if (i != global_filters.end()) {
            return i->second;
        }

        return {};
    }

    [[nodiscard]] method_type get_method(std::string const& name) const noexcept
    {
        hilet i = global_methods.find(name);
        if (i != global_methods.end()) {
            return i->second;
        }

        return {};
    }
};

}} // namespace hi::v1
