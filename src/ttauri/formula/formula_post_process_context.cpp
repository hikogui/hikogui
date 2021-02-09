// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "formula_post_process_context.hpp"
#include "../url_parser.hpp"

namespace tt {

static datum function_float(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for float() function, got {}", args.size());
    }

    return datum{static_cast<double>(args[0])};
}

static datum function_integer(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for integer() function, got {}", args.size());
    }

    return datum{static_cast<long long int>(args[0])};
}

static datum function_decimal(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for decimal() function, got {}", args.size());
    }

    return datum{static_cast<decimal>(args[0])};
}

static datum function_string(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for string() function, got {}", args.size());
    }

    return datum{static_cast<std::string>(args[0])};
}

static datum function_boolean(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for boolean() function, got {}", args.size());
    }

    return datum{static_cast<bool>(args[0])};
}

static datum function_url(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for url() function, got {}", args.size());
    }

    return datum{static_cast<URL>(args[0])};
}

static datum function_size(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for size() function, got {}", args.size());
    }

    return datum{args[0].size()};
}

static datum function_keys(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for keys() function, got {}", args.size());
    }

    ttlet &arg = args[0];

    datum::vector keys;
    for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
        keys.push_back(i->first);
    }
    return datum{std::move(keys)};
}

static datum function_values(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for values() function, got {}", args.size());
    }

    ttlet &arg = args[0];

    if (arg.is_map()) {
        datum::vector values;
        for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
            values.push_back(i->second);
        }
        return datum{std::move(values)};
    } else if (arg.is_vector()) {
        return datum{arg};
    } else {
        throw operation_error("Expecting vector or map argument for values() function, got {}", arg.type_name());
    }
}

static datum function_items(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for items() function, got {}", args.size());
    }

    ttlet &arg = args[0];

    if (arg.is_map()) {
        datum::vector values;
        for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
            values.emplace_back(datum::vector{i->first, i->second});
        }
        return datum{std::move(values)};

    } else {
        throw operation_error("Expecting map argument for items() function, got {}", arg.type_name());
    }
}

static datum function_sort(formula_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for sort() function, got {}", args.size());
    }

    ttlet &arg = args[0];

    if (arg.is_vector()) {
        auto r = static_cast<datum::vector>(arg);
        std::sort(r.begin(), r.end());
        return datum{r};

    } else {
        throw operation_error("Expecting vector argument for sort() function, got {}", arg.type_name());
    }
}



static datum method_contains(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for .contains() method, got {}", args.size());
    }

    if (self.is_vector() || self.is_map()) {
        return self.contains(args[0]);

    } else {
        throw operation_error("Expecting vector or map on left hand side for .contains() method, got {}", self.type_name());
    }
}

static datum method_append(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 1) {
        throw operation_error("Expecting 1 argument for .append() method, got {}", args.size());
    }

    if (self.is_vector()) {
        self.push_back(args[0]);
        return {};

    } else {
        throw operation_error("Expecting vector on left hand side for .append() method, got {}", self.type_name());
    }
}

static datum method_pop(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        throw operation_error("Expecting 0 arguments for .pop() method, got {}", args.size());
    }

    if (self.is_vector()) {
        auto r = self.back();
        self.pop_back();
        return r;

    } else {
        throw operation_error("Expecting vector on left hand side for .pop() method, got {}", self.type_name());
    }
}

static datum method_year(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        throw operation_error("Expecting 0 arguments for .year() method, got {}", args.size());
    }

    return self.year();
}

static datum method_quarter(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        throw operation_error("Expecting 0 arguments for .quarter() method, got {}", args.size());
    }

    return self.quarter();
}

static datum method_month(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        throw operation_error("Expecting 0 arguments for .month() method, got {}", args.size());
    }

    return self.month();
}

static datum method_day(formula_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        throw operation_error("Expecting 0 arguments for .day() method, got {}", args.size());
    }

    return self.day();
}

formula_post_process_context::function_table formula_post_process_context::global_functions = {
    {"float"s, function_float},
    {"integer"s, function_integer},
    {"decimal"s, function_decimal},
    {"string"s, function_string},
    {"boolean"s, function_boolean},
    {"url"s, function_url},
    {"size"s, function_size},
    {"keys"s, function_keys},
    {"values"s, function_values},
    {"items"s, function_items},
    {"sort"s, function_sort}
};
formula_post_process_context::method_table formula_post_process_context::global_methods = {
    {"append"s, method_append},
    {"contains"s, method_contains},
    {"push"s, method_append},
    {"pop"s, method_pop},
    {"year"s, method_year},
    {"quarter"s, method_quarter},
    {"month"s, method_month},
    {"day"s, method_day},
};

formula_post_process_context::filter_table formula_post_process_context::global_filters = {
    {"id"s, id_encode},
    {"url"s, url_encode}
};

}
