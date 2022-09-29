// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "formula_post_process_context.hpp"
#include "../file/URI.hpp"

namespace hi::inline v1 {

static datum function_float(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for float() function, got {}", args.size()));
    }

    return datum{static_cast<double>(args[0])};
}

static datum function_integer(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for integer() function, got {}", args.size()));
    }

    return datum{static_cast<long long int>(args[0])};
}

static datum function_decimal(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for decimal() function, got {}", args.size()));
    }

    return datum{static_cast<decimal>(args[0])};
}

static datum function_string(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for string() function, got {}", args.size()));
    }

    return datum{static_cast<std::string>(args[0])};
}

static datum function_boolean(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for boolean() function, got {}", args.size()));
    }

    return datum{to_bool(args[0])};
}

static datum function_size(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for size() function, got {}", args.size()));
    }

    return datum{args[0].size()};
}

static datum function_keys(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for keys() function, got {}", args.size()));
    }

    return datum{args[0].keys()};
}

static datum function_values(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for values() function, got {}", args.size()));
    }

    return datum{args[0].values()};
}

static datum function_items(formula_evaluation_context &context, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for items() function, got {}", args.size()));
    }

    return datum{args[0].items()};
}

static datum function_sort(formula_evaluation_context &context, datum::vector_type const &args)
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

static datum method_contains(formula_evaluation_context &context, datum const &self, datum::vector_type const &args)
{
    if (args.size() != 1) {
        throw operation_error(std::format("Expecting 1 argument for .contains() method, got {}", args.size()));
    }

    if (holds_alternative<datum::map_type>(self)) {
        return datum{self.contains(args[0])};

    } else {
        throw operation_error(std::format("Expecting vector or map on left hand side for .contains() method, got {}", self.type_name()));
    }
}

static datum method_append(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

static datum method_pop(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

static datum method_year(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

static datum method_quarter(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

static datum method_month(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

static datum method_day(formula_evaluation_context &context, datum &self, datum::vector_type const &args)
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

formula_post_process_context::function_table formula_post_process_context::global_functions = {
    {"float", function_float},
    {"integer", function_integer},
    {"decimal", function_decimal},
    {"string", function_string},
    {"boolean", function_boolean},
    {"size", function_size},
    {"keys", function_keys},
    {"values", function_values},
    {"items", function_items},
    {"sort", function_sort}};
formula_post_process_context::method_table formula_post_process_context::global_methods = {
    {"append", method_append},
    {"contains", method_contains},
    {"push", method_append},
    {"pop", method_pop},
    {"year", method_year},
    {"quarter", method_quarter},
    {"month", method_month},
    {"day", method_day},
};

static std::string url_encode(std::string_view str) noexcept
{
    return URI::encode(str);
}

formula_post_process_context::filter_table formula_post_process_context::global_filters = {
    {"id", make_identifier},
    {"url", url_encode}};

} // namespace hi::inline v1
