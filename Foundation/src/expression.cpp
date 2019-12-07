// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/expression.hpp"
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <limits>

namespace TTauri {

static datum function_float(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for float() function, got {}", args.size()));
    }

    return datum{static_cast<double>(args[0])};
}

static datum function_integer(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for integer() function, got {}", args.size()));
    }

    return datum{static_cast<long long int>(args[0])};
}

static datum function_decimal(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for decimal() function, got {}", args.size()));
    }

    return datum{static_cast<decimal>(args[0])};
}

static datum function_string(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for string() function, got {}", args.size()));
    }

    return datum{static_cast<std::string>(args[0])};
}

static datum function_boolean(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for boolean() function, got {}", args.size()));
    }

    return datum{static_cast<bool>(args[0])};
}

static datum function_url(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for url() function, got {}", args.size()));
    }

    return datum{static_cast<URL>(args[0])};
}

static datum function_size(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for size() function, got {}", args.size()));
    }

    return datum{args[0].size()};
}

static datum function_keys(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for keys() function, got {}", args.size()));
    }

    let &arg = args[0];

    datum::vector keys;
    for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
        keys.push_back(i->first);
    }
    return datum{std::move(keys)};
}

static datum function_values(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for values() function, got {}", args.size()));
    }

    let &arg = args[0];

    if (arg.is_map()) {
        datum::vector values;
        for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
            values.push_back(i->second);
        }
        return datum{std::move(values)};
    } else if (arg.is_vector()) {
        return datum{arg};
    } else {
        TTAURI_THROW(invalid_operation_error("Expecting vector or map argument for values() function, got {}", arg.type_name()));
    }
}

static datum function_items(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for items() function, got {}", args.size()));
    }

    let &arg = args[0];

    if (arg.is_map()) {
        datum::vector values;
        for (auto i = arg.map_begin(); i != arg.map_end(); i++) {
            values.emplace_back(datum::vector{i->first, i->second});
        }
        return datum{std::move(values)};

    } else {
        TTAURI_THROW(invalid_operation_error("Expecting map argument for items() function, got {}", arg.type_name()));
    }
}

static datum function_sort(expression_evaluation_context &context, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for sort() function, got {}", args.size()));
    }

    let &arg = args[0];

    if (arg.is_vector()) {
        auto r = static_cast<datum::vector>(arg);
        std::sort(r.begin(), r.end());
        return datum{r};

    } else {
        TTAURI_THROW(invalid_operation_error("Expecting vector argument for sort() function, got {}", arg.type_name()));
    }
}

expression_parse_context::function_table expression_parse_context::global_functions = {
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

static datum method_append(expression_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 1) {
        TTAURI_THROW(invalid_operation_error("Expecting 1 argument for .append() method, got {}", args.size()));
    }

    if (self.is_vector()) {
        self.push_back(args[0]);
        return {};

    } else {
        TTAURI_THROW(invalid_operation_error("Expecting vector on left hand side for .append() method, got {}", self.type_name()));
    }
}

static datum method_pop(expression_evaluation_context &context, datum &self, datum::vector const &args)
{
    if (args.size() != 0) {
        TTAURI_THROW(invalid_operation_error("Expecting 0 arguments for .pop() method, got {}", args.size()));
    }

    if (self.is_vector()) {
        auto r = self.back();
        self.pop_back();
        return r;

    } else {
        TTAURI_THROW(invalid_operation_error("Expecting vector on left hand side for .pop() method, got {}", self.type_name()));
    }
}

expression_parse_context::method_table expression_parse_context::global_methods = {
    {"append"s, method_append},
    {"push"s, method_append},
    {"pop"s, method_pop},
};

struct expression_arguments final : expression {
    expression_vector args;

    expression_arguments(index_type index, expression_vector args) :
        expression(index), args(std::move(args)) {}

    expression_arguments(index_type index, std::unique_ptr<expression> arg1, std::unique_ptr<expression> arg2) :
        expression(index)
    {
        args.push_back(std::move(arg1));
        args.push_back(std::move(arg2));
    }

    datum evaluate(expression_evaluation_context& context) const override {
        return {};
    };

    std::string string() const noexcept override {
        std::string s = "<args ";
        int i = 0;
        for (let &arg: args) {
            if (i++ > 0) {
                s += ", ";
            }
            s += arg->string();
        }
        return s + ">";
    }
};

struct expression_literal final : expression {
    datum value;

    expression_literal(index_type index, datum const& value) :
        expression(index), value(value) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return value;
    }

    std::string string() const noexcept override {
        return value.repr();
    }
};

struct expression_vector_literal final : expression {
    expression_vector values;

    expression_vector_literal(index_type index, expression_vector values) :
        expression(index), values(std::move(values)) {}

    void post_process(expression_parse_context& context) override {
        for (auto &value: values) {
            value->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        datum::vector r;
        for (let &value: values) {
            r.push_back(value->evaluate(context));
        }
        return datum{std::move(r)};
    }

    datum &assign(expression_evaluation_context& context, datum const &rhs) const override {
        if (!rhs.is_vector()) {
            TTAURI_THROW(invalid_operation_error("Unpacking values can only be done on vectors, got {}.", rhs));
        }
        if (values.size() < 1) {
            TTAURI_THROW(invalid_operation_error("Unpacking can only be done on 1 or more return values."));
        }
        if (values.size() != rhs.size()) {
            TTAURI_THROW(invalid_operation_error("Unpacking values can only be done on with a vector of size {} got {}.", values.size(), rhs.size()));
        }

        // Make a copy, in case of self assignment.
        let rhs_copy = rhs;

        size_t i = 0;
        while (true) {
            let &lhs_ = values[i];
            let &rhs_ = rhs_copy[i];

            if (++i < rhs.size()) {
                lhs_->assign(context, rhs_);
            } else {
                return lhs_->assign(context, rhs_);
            }
        }
    }

    std::string string() const noexcept override {
        std::string r = "[";
        int i = 0;
        for (let &value: values) {
            if (i++ > 0) {
                r += ", ";
            }
            r += value->string();
        }
        return r + "]";
    }
};

struct expression_map_literal final : expression {
    expression_vector keys;
    expression_vector values;

    expression_map_literal(index_type index, expression_vector keys, expression_vector values) :
        expression(index), keys(std::move(keys)), values(std::move(values)) {}

    void post_process(expression_parse_context& context) override {
        for (auto &key: keys) {
            key->post_process(context);
        }

        for (auto &value: values) {
            value->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        required_assert(keys.size() == values.size());

        datum::map r;
        for (size_t i = 0; i < keys.size(); i++) {
            let &key = keys[i];
            let &value = values[i];

            r[key->evaluate(context)] = value->evaluate(context);
        }
        return datum{std::move(r)};
    }

    std::string string() const noexcept override {
        required_assert(keys.size() == values.size());

        std::string r = "{";
        for (size_t i = 0; i < keys.size(); i++) {
            let &key = keys[i];
            let &value = values[i];

            if (i > 0) {
                r += ", ";
            }
            r += key->string();
            r += ": ";
            r += value->string();
        }
        return r + "}";
    }
};

struct expression_name final : expression {
    std::string name;
    mutable expression_parse_context::function_type function;

    expression_name(index_type index, std::string_view name) :
        expression(index), name(name) {}

    void resolve_function_pointer(expression_parse_context& context) override {
        function = context.get_function(name);
        if (!function) {
            TTAURI_THROW(parse_error("Could not find function {}()", name));
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        return context.get(name);
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        return context.get(name);
    }

    datum &assign(expression_evaluation_context& context, datum const &rhs) const override {
        return context.set(name, rhs);
    }

    datum call(expression_evaluation_context& context, datum::vector const &arguments) const override {
        return function(context, arguments);
    }

    std::string string() const noexcept override {
        return name;
    }
};

struct expression_call final : expression {
    std::unique_ptr<expression> lhs;
    expression_vector args;

    expression_call(
        index_type index,
        std::unique_ptr<expression> lhs,
        std::unique_ptr<expression> rhs
    ) :
        expression(index), lhs(std::move(lhs))
    {
        auto rhs_ = dynamic_cast<expression_arguments*>(rhs.get());
        required_assert(rhs_ != nullptr);
        args = std::move(rhs_->args);
    }

    void post_process(expression_parse_context& context) override {
        lhs->resolve_function_pointer(context);
        for (auto &arg: args) {
            arg->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        let args_ = transform<datum::vector>(args, [&](let& x) {
            return x->evaluate(context);
        });

        return lhs->call(context, args_);
    }

    std::string string() const noexcept override {
        auto s = fmt::format("({}(", *lhs);
        int i = 0;
        for (let &arg: args) {
            if (i++ > 0) {
                s += ',';
                s += ' ';
            }
            s += to_string(*arg);
        }
        return s + "))";
    }
};


struct expression_unary_operator : expression {
    std::unique_ptr<expression> rhs;

    expression_unary_operator(index_type index, std::unique_ptr<expression> rhs) :
        expression(index), rhs(std::move(rhs)) {}

    void post_process(expression_parse_context& context) override {
        rhs->post_process(context);
    }

    std::string string() const noexcept override {
        return fmt::format("<unary_operator {}>", rhs);
    }
};

struct expression_binary_operator : expression {
    std::unique_ptr<expression> lhs;
    std::unique_ptr<expression> rhs;

    expression_binary_operator(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression(index), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void post_process(expression_parse_context& context) override {
        lhs->post_process(context);
        rhs->post_process(context);
    }

    std::string string() const noexcept override {
        return fmt::format("<binary_operator {}, {}>", lhs, rhs);
    }
};

struct expression_ternary_operator final : expression {
    std::unique_ptr<expression> lhs;
    std::unique_ptr<expression> rhs_true;
    std::unique_ptr<expression> rhs_false;

    expression_ternary_operator(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> pair) :
        expression(index), lhs(std::move(lhs))
    {
        expression_arguments* pair_ = dynamic_cast<expression_arguments*>(pair.get());
        required_assert(pair_ != nullptr);
        required_assert(pair_->args.size() == 2);

        rhs_true = std::move(pair_->args[0]);
        rhs_false = std::move(pair_->args[1]);
        // The unique_ptrs inside pair_ are now empty.
    }

    void post_process(expression_parse_context& context) override {
        lhs->post_process(context);
        rhs_true->post_process(context);
        rhs_false->post_process(context);
    }

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) ? rhs_true->evaluate(context) : rhs_false->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} ? {} : {})", *lhs, *rhs_true, *rhs_false);
    }
};

struct expression_plus final : expression_unary_operator {
    expression_plus(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return +rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("(+ {})", *rhs);
    }
};

struct expression_minus final : expression_unary_operator {
    expression_minus(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return -rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("(- {})", *rhs);
    }
};

struct expression_invert final : expression_unary_operator {
    expression_invert(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return ~rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("(~ {})", *rhs);
    }
};

struct expression_logical_not final : expression_unary_operator {
    expression_logical_not(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return !rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("(! {})", *rhs);
    }
};

struct expression_increment final : expression_unary_operator {
    expression_increment(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return ++(rhs->evaluate(context));
    }

    std::string string() const noexcept override {
        return fmt::format("(++ {})", *rhs);
    }
};

struct expression_decrement final : expression_unary_operator {
    expression_decrement(index_type index, std::unique_ptr<expression> rhs) :
        expression_unary_operator(index, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return --(rhs->evaluate(context));
    }

    std::string string() const noexcept override {
        return fmt::format("(-- {})", *rhs);
    }
};

struct expression_add final : expression_binary_operator {
    expression_add(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) + rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} + {})", *lhs, *rhs);
    }
};

struct expression_sub final : expression_binary_operator {
    expression_sub(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) - rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} - {})", *lhs, *rhs);
    }
};

struct expression_mul final : expression_binary_operator {
    expression_mul(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) * rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} * {})", *lhs, *rhs);
    }
};

struct expression_div final : expression_binary_operator {
    expression_div(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) / rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} / {})", *lhs, *rhs);
    }
};

struct expression_mod final : expression_binary_operator {
    expression_mod(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) % rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} % {})", *lhs, *rhs);
    }
};

struct expression_pow final : expression_binary_operator {
    expression_pow(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return pow(lhs->evaluate(context), rhs->evaluate(context));
    }

    std::string string() const noexcept override {
        return fmt::format("({} ** {})", *lhs, *rhs);
    }
};

struct expression_logical_and final : expression_binary_operator {
    expression_logical_and(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) && rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} && {})", *lhs, *rhs);
    }
};

struct expression_logical_or final : expression_binary_operator {
    expression_logical_or(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) || rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} || {})", *lhs, *rhs);
    }
};

struct expression_bit_and final : expression_binary_operator {
    expression_bit_and(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) & rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} & {})", *lhs, *rhs);
    }
};

struct expression_bit_or final : expression_binary_operator {
    expression_bit_or(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) | rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} | {})", *lhs, *rhs);
    }
};

struct expression_bit_xor final : expression_binary_operator {
    expression_bit_xor(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) ^ rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} ^ {})", *lhs, *rhs);
    }
};

struct expression_shl final : expression_binary_operator {
    expression_shl(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) << rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} << {})", *lhs, *rhs);
    }
};

struct expression_shr final : expression_binary_operator {
    expression_shr(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) >> rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} >> {})", *lhs, *rhs);
    }
};

struct expression_eq final : expression_binary_operator {
    expression_eq(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) == rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} == {})", *lhs, *rhs);
    }
};

struct expression_ne final : expression_binary_operator {
    expression_ne(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) != rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} != {})", *lhs, *rhs);
    }
};

struct expression_lt final : expression_binary_operator {
    expression_lt(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) < rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} < {})", *lhs, *rhs);
    }
};

struct expression_gt final : expression_binary_operator {
    expression_gt(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) > rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} > {})", *lhs, *rhs);
    }
};

struct expression_le final : expression_binary_operator {
    expression_le(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) <= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} <= {})", *lhs, *rhs);
    }
};

struct expression_ge final : expression_binary_operator {
    expression_ge(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) >= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} >= {})", *lhs, *rhs);
    }
};

struct expression_member final : expression_binary_operator {
    mutable expression_parse_context::method_type method;
    expression_name* rhs_name;

    expression_member(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<expression_name*>(this->rhs.get());
        if (rhs_name == nullptr) {
            TTAURI_THROW(parse_error("Expecting a name token on the right hand side of a member accessor. got {}.", rhs));
        }
    }

    void resolve_function_pointer(expression_parse_context& context) override {
        method = context.get_method(rhs_name->name);
        if (!method) {
            TTAURI_THROW(parse_error("Could not find method .{}().", rhs_name->name));
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context)[rhs_name->name];
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        let rhs_ = dynamic_cast<expression_name*>(rhs.get());
        if (rhs_ == nullptr) {
            TTAURI_THROW(invalid_operation_error("Expecting a name token on the right hand side of a member accessor. got {}.", rhs));
        }

        return lhs->evaluate_lvalue(context)[rhs_->name];
    }

    datum call(expression_evaluation_context& context, datum::vector const &arguments) const override {
        return method(context, lhs->evaluate_lvalue(context), arguments);
    }

    std::string string() const noexcept override {
        return fmt::format("({} . {})", *lhs, *rhs);
    }
};

struct expression_index final : expression_binary_operator {
    expression_index(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context)[rhs->evaluate(context)];
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context)[rhs->evaluate(context)];
    }

    std::string string() const noexcept override {
        return fmt::format("({}[{}])", *lhs, *rhs);
    }
};

struct expression_assign final : expression_binary_operator {
    expression_assign(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->assign(context, rhs->evaluate(context));
    }

    std::string string() const noexcept override {
        return fmt::format("({} = {})", *lhs, *rhs);
    }
};

struct expression_inplace_add final : expression_binary_operator {
    expression_inplace_add(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) += rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} += {})", *lhs, *rhs);
    }
};

struct expression_inplace_sub final : expression_binary_operator {
    expression_inplace_sub(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) -= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} -= {})", *lhs, *rhs);
    }
};

struct expression_inplace_mul final : expression_binary_operator {
    expression_inplace_mul(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) *= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} *= {})", *lhs, *rhs);
    }
};

struct expression_inplace_div final : expression_binary_operator {
    expression_inplace_div(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) /= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} /= {})", *lhs, *rhs);
    }
};

struct expression_inplace_mod final : expression_binary_operator {
    expression_inplace_mod(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) %= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} %= {})", *lhs, *rhs);
    }
};

struct expression_inplace_shl final : expression_binary_operator {
    expression_inplace_shl(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) <<= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} <<= {})", *lhs, *rhs);
    }
};

struct expression_inplace_shr final : expression_binary_operator {
    expression_inplace_shr(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) >>= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} >>= {})", *lhs, *rhs);
    }
};

struct expression_inplace_and final : expression_binary_operator {
    expression_inplace_and(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) &= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} &= {})", *lhs, *rhs);
    }
};

struct expression_inplace_or final : expression_binary_operator {
    expression_inplace_or(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) |= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} |= {})", *lhs, *rhs);
    }
};

struct expression_inplace_xor final : expression_binary_operator {
    expression_inplace_xor(index_type index, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression_binary_operator(index, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate_lvalue(context) ^= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} ^= {})", *lhs, *rhs);
    }
};

[[nodiscard]] constexpr uint32_t operator_to_int(char const* str) noexcept {
    uint32_t v = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        v <<= 8;
        v |= str[i];
    }
    return v;
}


/** Operator Precedence according to C++.
    */
[[nodiscard]] uint8_t binary_operator_precedence(token_t const& token) noexcept {
    if (token != tokenizer_name_t::Literal) {
        return std::numeric_limits<uint8_t>::max();
    }

    switch (operator_to_int(token.value.data())) {
    case operator_to_int("::"): return 1;
    case operator_to_int("("): return 2;
    case operator_to_int("["): return 2;
    case operator_to_int("."): return 2;
    case operator_to_int("->"): return 2;
    case operator_to_int(".*"): return 4;
    case operator_to_int("->*"): return 4;
    case operator_to_int("**"): return 4;
    case operator_to_int("*"): return 5;
    case operator_to_int("/"): return 5;
    case operator_to_int("%"): return 5;
    case operator_to_int("+"): return 6;
    case operator_to_int("-"): return 6;
    case operator_to_int("<<"): return 7;
    case operator_to_int(">>"): return 7;
    case operator_to_int("<=>"): return 8;
    case operator_to_int("<"): return 9;
    case operator_to_int(">"): return 9;
    case operator_to_int("<="): return 9;
    case operator_to_int(">="): return 9;
    case operator_to_int("=="): return 10;
    case operator_to_int("!="): return 10;
    case operator_to_int("&"): return 11;
    case operator_to_int("^"): return 12;
    case operator_to_int("|"): return 13;
    case operator_to_int("&&"): return 14;
    case operator_to_int("||"): return 15;
    case operator_to_int("?"): return 16;
    case operator_to_int("="): return 16;
    case operator_to_int("+="): return 16;
    case operator_to_int("-="): return 16;
    case operator_to_int("*="): return 16;
    case operator_to_int("/="): return 16;
    case operator_to_int("%="): return 16;
    case operator_to_int("<<="): return 16;
    case operator_to_int(">>="): return 16;
    case operator_to_int("&="): return 16;
    case operator_to_int("^="): return 16;
    case operator_to_int("|="): return 16;
    case operator_to_int(","): return 17;
    case operator_to_int("]"): return 17;
    case operator_to_int(")"): return 17;
    default: return std::numeric_limits<uint8_t>::max();
    }
}

[[nodiscard]] uint8_t operator_precedence(token_t const& token, bool binary) noexcept {
    return binary ? binary_operator_precedence(token) : 3;
}

static std::unique_ptr<expression> parse_operation_expression(
    expression_parse_context& constext, std::unique_ptr<expression> lhs, token_t const& op, std::unique_ptr<expression> rhs
) {
    if (lhs) {
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("."): return std::make_unique<expression_member>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("**"): return std::make_unique<expression_pow>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("*"): return std::make_unique<expression_mul>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("/"): return std::make_unique<expression_div>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("%"): return std::make_unique<expression_mod>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("+"): return std::make_unique<expression_add>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("-"): return std::make_unique<expression_sub>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("<<"): return std::make_unique<expression_shl>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int(">>"): return std::make_unique<expression_shr>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("<"): return std::make_unique<expression_lt>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int(">"): return std::make_unique<expression_gt>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("<="): return std::make_unique<expression_le>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int(">="): return std::make_unique<expression_ge>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("=="): return std::make_unique<expression_eq>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("!="): return std::make_unique<expression_ne>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("&"): return std::make_unique<expression_bit_and>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("^"): return std::make_unique<expression_bit_xor>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("|"): return std::make_unique<expression_bit_or>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("&&"): return std::make_unique<expression_logical_and>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("||"): return std::make_unique<expression_logical_or>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("?"): return std::make_unique<expression_ternary_operator>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("["): return std::make_unique<expression_index>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("("): return std::make_unique<expression_call>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("="): return std::make_unique<expression_assign>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("+="): return std::make_unique<expression_inplace_add>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("-="): return std::make_unique<expression_inplace_sub>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("*="): return std::make_unique<expression_inplace_mul>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("/="): return std::make_unique<expression_inplace_div>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("%="): return std::make_unique<expression_inplace_mod>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("<<="): return std::make_unique<expression_inplace_shl>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int(">>="): return std::make_unique<expression_inplace_shr>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("&="): return std::make_unique<expression_inplace_and>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("|="): return std::make_unique<expression_inplace_or>(op.index, std::move(lhs), std::move(rhs));
        case operator_to_int("^="): return std::make_unique<expression_inplace_xor>(op.index, std::move(lhs), std::move(rhs));
        default: TTAURI_THROW(parse_error("Unexpected binary operator {}", op));
        }
    } else {
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("+"): return std::make_unique<expression_plus>(op.index, std::move(rhs));
        case operator_to_int("-"): return std::make_unique<expression_minus>(op.index, std::move(rhs));
        case operator_to_int("~"): return std::make_unique<expression_invert>(op.index, std::move(rhs));
        case operator_to_int("!"): return std::make_unique<expression_logical_not>(op.index, std::move(rhs));
        case operator_to_int("++"): return std::make_unique<expression_increment>(op.index, std::move(rhs));
        case operator_to_int("--"): return std::make_unique<expression_decrement>(op.index, std::move(rhs));
        default: TTAURI_THROW(parse_error("Unexpected unary operator {}", op));
        }
    }
}

/** Parse a lhs or rhs part of an expression.
    * This should expect any off:
    *  - leaf node: literal
    *  - leaf node: name
    *  - vector literal: '[' ( parse_expression() ( ',' parse_expression() )* ','? )? ']'
    *  - map literal: '{' ( parse_expression() ':' parse_expression() ( ',' parse_expression() ':' parse_expression() )* ','? )? '}'
    *  - subexpression:  '(' parse_expression() ')'
    *  - unary operator: op parse_expression()
    */
static std::unique_ptr<expression> parse_primary_expression(expression_parse_context& context)
{
    let index = context->index;
    switch (context->name) {
    case tokenizer_name_t::IntegerLiteral:
        return std::make_unique<expression_literal>(index, static_cast<long long>(*context++));

    case tokenizer_name_t::FloatLiteral:
        return std::make_unique<expression_literal>(index, static_cast<double>(*context++));

    case tokenizer_name_t::StringLiteral:
        return std::make_unique<expression_literal>(index, static_cast<std::string>(*context++));

    case tokenizer_name_t::Name:
        if (*context == "true") {
            ++context;
            return std::make_unique<expression_literal>(index, true);

        } else if (*context == "false") {
            ++context;
            return std::make_unique<expression_literal>(index, false);

        } else if (*context == "null") {
            ++context;
            return std::make_unique<expression_literal>(index, datum::null{});

        } else if (*context == "undefined") {
            ++context;
            return std::make_unique<expression_literal>(index, datum{});

        } else {
            return std::make_unique<expression_name>(index, (context++)->value);
        }

    case tokenizer_name_t::Literal:
        if (*context == "(") {
            ++context;
            auto subexpression = parse_expression(context);

            if ((*context == tokenizer_name_t::Literal) && (*context == ")")) {
                ++context;
            } else {
                TTAURI_THROW(parse_error("Expected ')' token for function call got {}", *context));
            }

            return subexpression;

        } else if (*context == "[") {
            ++context;

            expression::expression_vector values;

            // ',' is between each expression, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Literal) && (*context == "]"))) {
                values.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Literal) && (*context == "]")) {
                    ++context;
                    break;
                } else {
                    TTAURI_THROW(parse_error("Expected ']' or ',' after a vector sub-expression. got {}", *context));
                }
            }

            return std::make_unique<expression_vector_literal>(index, std::move(values));

        } else if (*context == "{") {
            ++context;

            expression::expression_vector keys;
            expression::expression_vector values;

            // ',' is between each expression, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Literal) && (*context == "}"))) {
                keys.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ":")) {
                    ++context;
                } else {
                    TTAURI_THROW(parse_error("Expected ':' after a map key. got {}", *context));
                }

                values.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Literal) && (*context == "}")) {
                    ++context;
                    break;
                } else {
                    TTAURI_THROW(parse_error("Expected ']' or ',' after a vector sub-expression. got {}", *context));
                }
            }

            return std::make_unique<expression_map_literal>(index, std::move(keys), std::move(values));

        } else {
            // Found a unary operator, this should be handled using operator_precedence.
            return {};
        }

    default:
        return {};
        TTAURI_THROW(parse_error("Unexpected token in primary expression {}", *context));
    }
}

/** Parse the rhs of an index operator, including the closing bracket.
    */
static std::unique_ptr<expression> parse_index_expression(expression_parse_context& context)
{
    auto rhs = parse_expression(context);

    if ((*context == tokenizer_name_t::Literal) && (*context == "]")) {
        ++context;
    } else {
        TTAURI_THROW(parse_error("Expected ']' token at end of indexing operator got {}", *context));
    }
    return rhs;
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<expression> parse_ternary_argument_expression(expression_parse_context& context)
{
    auto rhs_true = parse_expression(context);

    if ((*context == tokenizer_name_t::Literal) && (*context == ":")) {
        ++context;
    } else {
        TTAURI_THROW(parse_error("Expected ':' token in ternary expression {}", *context));
    }

    auto rhs_false = parse_expression(context);

    return std::make_unique<expression_arguments>(context->index, std::move(rhs_true), std::move(rhs_false));
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<expression> parse_call_argument_expression(expression_parse_context& context)
{
    expression::expression_vector args;

    if ((*context == tokenizer_name_t::Literal) && (*context == ")")) {
        ++context;

    } else while (true) {
        args.push_back(parse_expression(context));

        if ((*context == tokenizer_name_t::Literal) && (*context == ",")) {
            ++context;
            continue;

        } else if ((*context == tokenizer_name_t::Literal) && (*context == ")")) {
            ++context;
            break;

        } else {
            TTAURI_THROW(parse_error("Expected ',' or ')' After a function argument {}", *context));
        }
    }

    return std::make_unique<expression_arguments>(context->index, std::move(args));
}

static bool parse_expression_is_at_end(token_t const& lookahead)
{
    if (lookahead == tokenizer_name_t::End) {
        return true;
    }

    if (lookahead != tokenizer_name_t::Literal) {
        TTAURI_THROW(parse_error("Expecting an operator token got {}", lookahead));
    }

    return
        lookahead == ")" ||
        lookahead == "}" ||
        lookahead == "]" ||
        lookahead == ":" ||
        lookahead == ",";
}


/** Parse an expression.
    * Parses an expression until EOF, ')', '}', ']', ':', ','
    */
static std::unique_ptr<expression> parse_expression_1(expression_parse_context& context, std::unique_ptr<expression> lhs, uint8_t precedence)
{
    auto lookahead = *context;
    auto lookahead_precedence = operator_precedence(lookahead, static_cast<bool>(lhs));

    while (!parse_expression_is_at_end(lookahead) && lookahead_precedence <= precedence) {
        let op = lookahead;
        let op_precedence = lookahead_precedence;
        ++context;

        std::unique_ptr<expression> rhs;
        if (op == tokenizer_name_t::Literal && op == "[") {
            rhs = parse_index_expression(context);
        } else if (op == tokenizer_name_t::Literal && op == "(") {
            rhs = parse_call_argument_expression(context);
        } else if (op == tokenizer_name_t::Literal && op == "?") {
            rhs = parse_ternary_argument_expression(context);
        } else {
            rhs = parse_primary_expression(context);
        }

        lookahead = *context;
        lookahead_precedence = operator_precedence(lookahead, static_cast<bool>(rhs));
        while (!parse_expression_is_at_end(lookahead) && lookahead_precedence < op_precedence) {
            rhs = parse_expression_1(context, std::move(rhs), op_precedence);

            lookahead = *context;
            lookahead_precedence = operator_precedence(lookahead, static_cast<bool>(rhs));
        }
        lhs = parse_operation_expression(context, std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<expression> parse_expression(expression_parse_context& context)
{
    return parse_expression_1(context, parse_primary_expression(context), std::numeric_limits<uint8_t>::max());
}

std::string_view::const_iterator find_end_of_expression(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    std::string_view terminating_string)
{
    std::string bracket_stack;
    char in_string = 0;
    bool in_escape = false;

    for (auto i = first; i != last; i++) {
        if (in_escape) {
            in_escape = false;

        } else if (in_string) {
            if (*i == in_string) {
                in_string = 0;
            } else if (*i == '\\') {
                in_escape = true;
            }

        } else {
            switch (*i) {
            case '"': in_string = '"'; break;
            case '\'': in_string = '\''; break;
            case '{': bracket_stack += '}'; break;
            case '[': bracket_stack += ']'; break;
            case '(': bracket_stack += ')'; break;
            case '\\': in_escape = true; break; // It is possible to escape any character, including the terminating_character.
            default:
                if (bracket_stack.size() > 0) {
                    if (*i == bracket_stack.back()) {
                        bracket_stack.pop_back();
                    }

                } else if (starts_with(i, last, terminating_string.begin(), terminating_string.end())) {
                    return i;
                }
            }
        }
    }
    return last;
}

}
