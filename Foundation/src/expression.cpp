// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/expression.hpp"
#include "TTauri/Foundation/operator.hpp"
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

expression_post_process_context::function_table expression_post_process_context::global_functions = {
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

expression_post_process_context::method_table expression_post_process_context::global_methods = {
    {"append"s, method_append},
    {"push"s, method_append},
    {"pop"s, method_pop},
};

/** A temporary node used during parsing.
 */
struct expression_arguments final : expression_node {
    expression_vector args;

    expression_arguments(ssize_t offset, expression_vector args) :
        expression_node(offset), args(std::move(args)) {}

    expression_arguments(ssize_t offset, std::unique_ptr<expression_node> arg1, std::unique_ptr<expression_node> arg2) :
        expression_node(offset)
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

struct expression_literal_node final : expression_node {
    datum value;

    expression_literal_node(ssize_t offset, datum const& value) :
        expression_node(offset), value(value) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return value;
    }

    std::string string() const noexcept override {
        return value.repr();
    }
};

struct expression_vector_literal_node final : expression_node {
    expression_vector values;

    expression_vector_literal_node(ssize_t offset, expression_vector values) :
        expression_node(offset), values(std::move(values)) {}

    void post_process(expression_post_process_context& context) override {
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
            TTAURI_THROW(invalid_operation_error("Unpacking values can only be done on vectors, got {}.", rhs).set<"offset"_tag>(offset));
        }
        if (values.size() < 1) {
            TTAURI_THROW(invalid_operation_error("Unpacking can only be done on 1 or more return values.").set<"offset"_tag>(offset));
        }
        if (values.size() != rhs.size()) {
            TTAURI_THROW(invalid_operation_error("Unpacking values can only be done on with a vector of size {} got {}.", values.size(), rhs.size()).set<"offset"_tag>(offset));
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

struct expression_map_literal_node final : expression_node {
    expression_vector keys;
    expression_vector values;

    expression_map_literal_node(ssize_t offset, expression_vector keys, expression_vector values) :
        expression_node(offset), keys(std::move(keys)), values(std::move(values)) {}

    void post_process(expression_post_process_context& context) override {
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

struct expression_name_node final : expression_node {
    std::string name;
    mutable expression_post_process_context::function_type function;

    expression_name_node(ssize_t offset, std::string_view name) :
        expression_node(offset), name(name) {}

    void resolve_function_pointer(expression_post_process_context& context) override {
        function = context.get_function(name);
        if (!function) {
            TTAURI_THROW(parse_error("Could not find function {}()", name).set<"offset"_tag>(offset));
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        let &const_context = context;

        try {
            return const_context.get(name);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        try {
            return context.get(name);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum &assign(expression_evaluation_context& context, datum const &rhs) const override {
        try {
            return context.set(name, rhs);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum call(expression_evaluation_context& context, datum::vector const &arguments) const override {
        try {
            return function(context, arguments);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string get_name() const noexcept override {
        return name;
    }

    std::string string() const noexcept override {
        return name;
    }
};

struct expression_call_node final : expression_node {
    std::unique_ptr<expression_node> lhs;
    expression_vector args;

    expression_call_node(
        ssize_t offset,
        std::unique_ptr<expression_node> lhs,
        std::unique_ptr<expression_node> rhs
    ) :
        expression_node(offset), lhs(std::move(lhs))
    {
        auto rhs_ = dynamic_cast<expression_arguments*>(rhs.get());
        required_assert(rhs_ != nullptr);
        args = std::move(rhs_->args);
    }

    void post_process(expression_post_process_context& context) override {
        lhs->resolve_function_pointer(context);
        for (auto &arg: args) {
            arg->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        let args_ = transform<datum::vector>(args, [&](let& x) {
            return x->evaluate(context);
        });

        try {
            return lhs->call(context, args_);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::vector<std::string> get_name_and_argument_names() const override {
        std::vector<std::string> r;

        try {
            r.push_back(lhs->get_name());
        } catch (parse_error &) {
            TTAURI_THROW(parse_error("Function definition does not have a name, got {})", lhs));
        }

        for (let &arg: args) {
            try {
                r.push_back(arg->get_name());
            } catch (parse_error &) {
                TTAURI_THROW(parse_error("Definition of function {}() has a non-name argument {})", lhs, arg));
            }
        }

        return r;
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


struct expression_unary_operator_node : expression_node {
    std::unique_ptr<expression_node> rhs;

    expression_unary_operator_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_node(offset), rhs(std::move(rhs)) {}

    void post_process(expression_post_process_context& context) override {
        rhs->post_process(context);
    }

    std::string string() const noexcept override {
        return fmt::format("<unary_operator {}>", rhs);
    }
};

struct expression_binary_operator_node : expression_node {
    std::unique_ptr<expression_node> lhs;
    std::unique_ptr<expression_node> rhs;

    expression_binary_operator_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_node(offset), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void post_process(expression_post_process_context& context) override {
        lhs->post_process(context);
        rhs->post_process(context);
    }

    std::string string() const noexcept override {
        return fmt::format("<binary_operator {}, {}>", lhs, rhs);
    }
};

struct expression_ternary_operator_node final : expression_node {
    std::unique_ptr<expression_node> lhs;
    std::unique_ptr<expression_node> rhs_true;
    std::unique_ptr<expression_node> rhs_false;

    expression_ternary_operator_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> pair) :
        expression_node(offset), lhs(std::move(lhs))
    {
        expression_arguments* pair_ = dynamic_cast<expression_arguments*>(pair.get());
        required_assert(pair_ != nullptr);
        required_assert(pair_->args.size() == 2);

        rhs_true = std::move(pair_->args[0]);
        rhs_false = std::move(pair_->args[1]);
        // The unique_ptrs inside pair_ are now empty.
    }

    void post_process(expression_post_process_context& context) override {
        lhs->post_process(context);
        rhs_true->post_process(context);
        rhs_false->post_process(context);
    }

    datum evaluate(expression_evaluation_context& context) const override {
        let lhs_ = lhs->evaluate(context);
        if (lhs_) {
            return rhs_true->evaluate(context);
        } else {
            return rhs_false->evaluate(context);
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} ? {} : {})", *lhs, *rhs_true, *rhs_false);
    }
};

struct expression_plus_node final : expression_unary_operator_node {
    expression_plus_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return +rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(+ {})", *rhs);
    }
};

struct expression_minus_node final : expression_unary_operator_node {
    expression_minus_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return -rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(- {})", *rhs);
    }
};

struct expression_invert_node final : expression_unary_operator_node {
    expression_invert_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return ~rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(~ {})", *rhs);
    }
};

struct expression_logical_not_node final : expression_unary_operator_node {
    expression_logical_not_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return !rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(! {})", *rhs);
    }
};

struct expression_increment_node final : expression_unary_operator_node {
    expression_increment_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto &rhs_ = rhs->evaluate_lvalue(context);
        try {
            return ++rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(++ {})", *rhs);
    }
};

struct expression_decrement_node final : expression_unary_operator_node {
    expression_decrement_node(ssize_t offset, std::unique_ptr<expression_node> rhs) :
        expression_unary_operator_node(offset, std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto &rhs_ = rhs->evaluate_lvalue(context);
        try {
            return --rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(-- {})", *rhs);
    }
};

struct expression_add_node final : expression_binary_operator_node {
    expression_add_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ + rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} + {})", *lhs, *rhs);
    }
};

struct expression_sub_node final : expression_binary_operator_node {
    expression_sub_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ - rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} - {})", *lhs, *rhs);
    }
};

struct expression_mul_node final : expression_binary_operator_node {
    expression_mul_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ * rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} * {})", *lhs, *rhs);
    }
};

struct expression_div_node final : expression_binary_operator_node {
    expression_div_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ / rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} / {})", *lhs, *rhs);
    }
};

struct expression_mod_node final : expression_binary_operator_node {
    expression_mod_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ % rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} % {})", *lhs, *rhs);
    }
};

struct expression_pow_node final : expression_binary_operator_node {
    expression_pow_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return pow(lhs_, rhs_);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} ** {})", *lhs, *rhs);
    }
};

struct expression_logical_and_node final : expression_binary_operator_node {
    expression_logical_and_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        if (lhs_) {
            return rhs->evaluate(context);
        } else {
            return lhs_;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} && {})", *lhs, *rhs);
    }
};

struct expression_logical_or_node final : expression_binary_operator_node {
    expression_logical_or_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        if (lhs_) {
            return lhs_;
        } else {
            return rhs->evaluate(context);
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} || {})", *lhs, *rhs);
    }
};

struct expression_bit_and_node final : expression_binary_operator_node {
    expression_bit_and_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ & rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} & {})", *lhs, *rhs);
    }
};

struct expression_bit_or_node final : expression_binary_operator_node {
    expression_bit_or_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ | rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} | {})", *lhs, *rhs);
    }
};

struct expression_bit_xor_node final : expression_binary_operator_node {
    expression_bit_xor_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ ^ rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} ^ {})", *lhs, *rhs);
    }
};

struct expression_shl_node final : expression_binary_operator_node {
    expression_shl_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ << rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} << {})", *lhs, *rhs);
    }
};

struct expression_shr_node final : expression_binary_operator_node {
    expression_shr_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_ >> rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} >> {})", *lhs, *rhs);
    }
};

struct expression_eq_node final : expression_binary_operator_node {
    expression_eq_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) == rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} == {})", *lhs, *rhs);
    }
};

struct expression_ne_node final : expression_binary_operator_node {
    expression_ne_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) != rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} != {})", *lhs, *rhs);
    }
};

struct expression_lt_node final : expression_binary_operator_node {
    expression_lt_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) < rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} < {})", *lhs, *rhs);
    }
};

struct expression_gt_node final : expression_binary_operator_node {
    expression_gt_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) > rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} > {})", *lhs, *rhs);
    }
};

struct expression_le_node final : expression_binary_operator_node {
    expression_le_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) <= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} <= {})", *lhs, *rhs);
    }
};

struct expression_ge_node final : expression_binary_operator_node {
    expression_ge_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        return lhs->evaluate(context) >= rhs->evaluate(context);
    }

    std::string string() const noexcept override {
        return fmt::format("({} >= {})", *lhs, *rhs);
    }
};

struct expression_member_node final : expression_binary_operator_node {
    mutable expression_post_process_context::method_type method;
    expression_name_node* rhs_name;

    expression_member_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<expression_name_node*>(this->rhs.get());
        if (rhs_name == nullptr) {
            TTAURI_THROW(parse_error("Expecting a name token on the right hand side of a member accessor. got {}.", rhs).set<"offset"_tag>(offset));
        }
    }

    void resolve_function_pointer(expression_post_process_context& context) override {
        method = context.get_method(rhs_name->name);
        if (!method) {
            TTAURI_THROW(parse_error("Could not find method .{}().", rhs_name->name).set<"offset"_tag>(offset));
        }
    }

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        try {
            return lhs_[rhs_name->name];
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return lhs_[rhs_name->name];
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum call(expression_evaluation_context& context, datum::vector const &arguments) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return method(context, lhs_, arguments);
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} . {})", *lhs, *rhs);
    }
};

struct expression_index_node final : expression_binary_operator_node {
    expression_index_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_[rhs_];
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    datum &evaluate_lvalue(expression_evaluation_context& context) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        auto rhs_ = rhs->evaluate(context);
        try {
            return lhs_[rhs_];
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({}[{}])", *lhs, *rhs);
    }
};

struct expression_assign_node final : expression_binary_operator_node {
    expression_assign_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        return lhs->assign(context, rhs_);
    }

    std::string string() const noexcept override {
        return fmt::format("({} = {})", *lhs, *rhs);
    }
};

struct expression_inplace_add_node final : expression_binary_operator_node {
    expression_inplace_add_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ += rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} += {})", *lhs, *rhs);
    }
};

struct expression_inplace_sub_node final : expression_binary_operator_node {
    expression_inplace_sub_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ -= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} -= {})", *lhs, *rhs);
    }
};

struct expression_inplace_mul_node final : expression_binary_operator_node {
    expression_inplace_mul_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ *= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} *= {})", *lhs, *rhs);
    }
};

struct expression_inplace_div_node final : expression_binary_operator_node {
    expression_inplace_div_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ /= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} /= {})", *lhs, *rhs);
    }
};

struct expression_inplace_mod_node final : expression_binary_operator_node {
    expression_inplace_mod_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ %= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} %= {})", *lhs, *rhs);
    }
};

struct expression_inplace_shl_node final : expression_binary_operator_node {
    expression_inplace_shl_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ <<= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} <<= {})", *lhs, *rhs);
    }
};

struct expression_inplace_shr_node final : expression_binary_operator_node {
    expression_inplace_shr_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ >>= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} >>= {})", *lhs, *rhs);
    }
};

struct expression_inplace_and_node final : expression_binary_operator_node {
    expression_inplace_and_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ &= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} &= {})", *lhs, *rhs);
    }
};

struct expression_inplace_or_node final : expression_binary_operator_node {
    expression_inplace_or_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ |= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} |= {})", *lhs, *rhs);
    }
};

struct expression_inplace_xor_node final : expression_binary_operator_node {
    expression_inplace_xor_node(ssize_t offset, std::unique_ptr<expression_node> lhs, std::unique_ptr<expression_node> rhs) :
        expression_binary_operator_node(offset, std::move(lhs), std::move(rhs)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ ^= rhs_;
        } catch (error &e) {
            e.set<"offset"_tag>(offset);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} ^= {})", *lhs, *rhs);
    }
};

[[nodiscard]] uint8_t operator_precedence(token_t const &token, bool binary) noexcept {
    if (token != tokenizer_name_t::Literal) {
        return 0;
    } else {
        return std::numeric_limits<uint8_t>::max() - operator_precedence(token.value.data(), binary);
    }
}

static std::unique_ptr<expression_node> parse_operation_expression(
    expression_parse_context& context, std::unique_ptr<expression_node> lhs, token_t const& op, std::unique_ptr<expression_node> rhs
) {
    let offset = std::distance(context.first, op.index);

    if (lhs) {
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("."): return std::make_unique<expression_member_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("**"): return std::make_unique<expression_pow_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("*"): return std::make_unique<expression_mul_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("/"): return std::make_unique<expression_div_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("%"): return std::make_unique<expression_mod_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("+"): return std::make_unique<expression_add_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("-"): return std::make_unique<expression_sub_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("<<"): return std::make_unique<expression_shl_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int(">>"): return std::make_unique<expression_shr_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("<"): return std::make_unique<expression_lt_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int(">"): return std::make_unique<expression_gt_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("<="): return std::make_unique<expression_le_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int(">="): return std::make_unique<expression_ge_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("=="): return std::make_unique<expression_eq_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("!="): return std::make_unique<expression_ne_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("&"): return std::make_unique<expression_bit_and_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("^"): return std::make_unique<expression_bit_xor_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("|"): return std::make_unique<expression_bit_or_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("&&"): return std::make_unique<expression_logical_and_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("||"): return std::make_unique<expression_logical_or_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("?"): return std::make_unique<expression_ternary_operator_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("["): return std::make_unique<expression_index_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("("): return std::make_unique<expression_call_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("="): return std::make_unique<expression_assign_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("+="): return std::make_unique<expression_inplace_add_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("-="): return std::make_unique<expression_inplace_sub_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("*="): return std::make_unique<expression_inplace_mul_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("/="): return std::make_unique<expression_inplace_div_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("%="): return std::make_unique<expression_inplace_mod_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("<<="): return std::make_unique<expression_inplace_shl_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int(">>="): return std::make_unique<expression_inplace_shr_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("&="): return std::make_unique<expression_inplace_and_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("|="): return std::make_unique<expression_inplace_or_node>(offset, std::move(lhs), std::move(rhs));
        case operator_to_int("^="): return std::make_unique<expression_inplace_xor_node>(offset, std::move(lhs), std::move(rhs));
        default: TTAURI_THROW(parse_error("Unexpected binary operator {}", op).set<"offset"_tag>(offset));
        }
    } else {
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("+"): return std::make_unique<expression_plus_node>(offset, std::move(rhs));
        case operator_to_int("-"): return std::make_unique<expression_minus_node>(offset, std::move(rhs));
        case operator_to_int("~"): return std::make_unique<expression_invert_node>(offset, std::move(rhs));
        case operator_to_int("!"): return std::make_unique<expression_logical_not_node>(offset, std::move(rhs));
        case operator_to_int("++"): return std::make_unique<expression_increment_node>(offset, std::move(rhs));
        case operator_to_int("--"): return std::make_unique<expression_decrement_node>(offset, std::move(rhs));
        default: TTAURI_THROW(parse_error("Unexpected unary operator {}", op).set<"offset"_tag>(offset));
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
static std::unique_ptr<expression_node> parse_primary_expression(expression_parse_context& context)
{
    let offset = context.offset();
    switch (context->name) {
    case tokenizer_name_t::IntegerLiteral:
        return std::make_unique<expression_literal_node>(offset, static_cast<long long>(*context++));

    case tokenizer_name_t::FloatLiteral:
        return std::make_unique<expression_literal_node>(offset, static_cast<double>(*context++));

    case tokenizer_name_t::StringLiteral:
        return std::make_unique<expression_literal_node>(offset, static_cast<std::string>(*context++));

    case tokenizer_name_t::Name:
        if (*context == "true") {
            ++context;
            return std::make_unique<expression_literal_node>(offset, true);

        } else if (*context == "false") {
            ++context;
            return std::make_unique<expression_literal_node>(offset, false);

        } else if (*context == "null") {
            ++context;
            return std::make_unique<expression_literal_node>(offset, datum::null{});

        } else if (*context == "undefined") {
            ++context;
            return std::make_unique<expression_literal_node>(offset, datum{});

        } else {
            return std::make_unique<expression_name_node>(offset, (context++)->value);
        }

    case tokenizer_name_t::Literal:
        if (*context == "(") {
            ++context;
            auto subexpression = parse_expression(context);

            if ((*context == tokenizer_name_t::Literal) && (*context == ")")) {
                ++context;
            } else {
                TTAURI_THROW(parse_error("Expected ')' token for function call got {}", *context).set<"offset"_tag>(offset));
            }

            return subexpression;

        } else if (*context == "[") {
            ++context;

            expression_node::expression_vector values;

            // ',' is between each expression, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Literal) && (*context == "]"))) {
                values.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Literal) && (*context == "]")) {
                    ++context;
                    break;
                } else {
                    TTAURI_THROW(parse_error("Expected ']' or ',' after a vector sub-expression. got {}", *context).set<"offset"_tag>(offset));
                }
            }

            return std::make_unique<expression_vector_literal_node>(offset, std::move(values));

        } else if (*context == "{") {
            ++context;

            expression_node::expression_vector keys;
            expression_node::expression_vector values;

            // ',' is between each expression, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Literal) && (*context == "}"))) {
                keys.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ":")) {
                    ++context;
                } else {
                    TTAURI_THROW(parse_error("Expected ':' after a map key. got {}", *context).set<"offset"_tag>(offset));
                }

                values.push_back(parse_expression(context));

                if ((*context == tokenizer_name_t::Literal) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Literal) && (*context == "}")) {
                    ++context;
                    break;
                } else {
                    TTAURI_THROW(parse_error("Expected ']' or ',' after a vector sub-expression. got {}", *context).set<"offset"_tag>(offset));
                }
            }

            return std::make_unique<expression_map_literal_node>(offset, std::move(keys), std::move(values));

        } else {
            // Found a unary operator, this should be handled using operator_precedence.
            return {};
        }

    default:
        TTAURI_THROW(parse_error("Unexpected token in primary expression {}", *context).set<"offset"_tag>(offset));
    }
}

/** Parse the rhs of an index operator, including the closing bracket.
    */
static std::unique_ptr<expression_node> parse_index_expression(expression_parse_context& context)
{
    auto rhs = parse_expression(context);

    if ((*context == tokenizer_name_t::Literal) && (*context == "]")) {
        ++context;
    } else {
        TTAURI_THROW(parse_error("Expected ']' token at end of indexing operator got {}", *context).set<"offset"_tag>(context.offset()));
    }
    return rhs;
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<expression_node> parse_ternary_argument_expression(expression_parse_context& context)
{
    auto rhs_true = parse_expression(context);

    if ((*context == tokenizer_name_t::Literal) && (*context == ":")) {
        ++context;
    } else {
        TTAURI_THROW(parse_error("Expected ':' token in ternary expression {}", *context).set<"offset"_tag>(context.offset()));
    }

    auto rhs_false = parse_expression(context);

    return std::make_unique<expression_arguments>(context.offset(), std::move(rhs_true), std::move(rhs_false));
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<expression_node> parse_call_argument_expression(expression_parse_context& context)
{
    expression_node::expression_vector args;

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
            TTAURI_THROW(parse_error("Expected ',' or ')' After a function argument {}", *context).set<"offset"_tag>(context.offset()));
        }
    }

    return std::make_unique<expression_arguments>(context.offset(), std::move(args));
}

static bool parse_expression_is_at_end(expression_parse_context& context)
{
    if (*context == tokenizer_name_t::End) {
        return true;
    }

    if (*context != tokenizer_name_t::Literal) {
        TTAURI_THROW(parse_error("Expecting an operator token got {}", *context).set<"offset"_tag>(context.offset()));
    }

    return
        *context == ")" ||
        *context == "}" ||
        *context == "]" ||
        *context == ":" ||
        *context == ",";
}


/** Parse an expression.
    * Parses an expression until EOF, ')', '}', ']', ':', ','
    */
static std::unique_ptr<expression_node> parse_expression_1(expression_parse_context& context, std::unique_ptr<expression_node> lhs, uint8_t precedence)
{
    auto lookahead = *context;
    auto lookahead_precedence = operator_precedence(lookahead, static_cast<bool>(lhs));

    while (!parse_expression_is_at_end(context) && lookahead_precedence >= precedence) {
        let op = lookahead;
        let op_precedence = lookahead_precedence;
        ++context;

        std::unique_ptr<expression_node> rhs;
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
        while (!parse_expression_is_at_end(context) && lookahead_precedence > op_precedence) {
            rhs = parse_expression_1(context, std::move(rhs), op_precedence);

            lookahead = *context;
            lookahead_precedence = operator_precedence(lookahead, static_cast<bool>(rhs));
        }
        lhs = parse_operation_expression(context, std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<expression_node> parse_expression(expression_parse_context& context)
{
    return parse_expression_1(context, parse_primary_expression(context), 0);
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
