// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_member_node;
import hikogui_formula_formula_binary_operator_node;

export namespace hi { inline namespace v1 {

export struct formula_member_node final : formula_binary_operator_node {
    mutable formula_post_process_context::method_type method;
    formula_name_node *rhs_name;

    formula_member_node(size_t line_nr, size_t column_nr, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(line_nr, column_nr, std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<formula_name_node *>(this->rhs.get());
        if (rhs_name == nullptr) {
            throw parse_error(std::format("{}:{}: Expecting a name token on the right hand side of a member accessor. got {}.", line_nr, column_nr, *rhs));
        }
    }

    void resolve_function_pointer(formula_post_process_context &context) override
    {
        method = context.get_method(rhs_name->name);
        if (!method) {
            throw parse_error(std::format("{}:{}: Could not find method .{}().", line_nr, column_nr, rhs_name->name));
        }
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        if (lhs->has_evaluate_xvalue()) {
            hilet &lhs_ = lhs->evaluate_xvalue(context);

            if (!lhs_.contains(rhs_name->name)) {
                throw operation_error(std::format("{}:{}: Unknown attribute .{}", line_nr, column_nr, rhs_name->name));
            }
            try {
                return lhs_[rhs_name->name];
            } catch (std::exception const &e) {
                throw operation_error(std::format("{}:{}: Can not evaluate member selection.\n{}", line_nr, column_nr, e.what()));
            }

        } else {
            hilet lhs_ = lhs->evaluate(context);

            if (!lhs_.contains(rhs_name->name)) {
                throw operation_error(std::format("{}:{}: Unknown attribute .{}", line_nr, column_nr, rhs_name->name));
            }
            try {
                return lhs_[rhs_name->name];
            } catch (std::exception const &e) {
                throw operation_error(std::format("{}:{}: Can not evaluate member selection.\n{}", line_nr, column_nr, e.what()));
            }
        }
    }

    datum &evaluate_lvalue(formula_evaluation_context &context) const override
    {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return lhs_[rhs_name->name];
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}:{}: Can not evaluate member-selection.\n{}", line_nr, column_nr, e.what()));
        }
    }

    datum call(formula_evaluation_context &context, datum::vector_type const &arguments) const override
    {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return method(context, lhs_, arguments);
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}:{}: Can not evaluate call-of-method.\n{}", line_nr, column_nr, e.what()));
        }
    }

    std::string string() const noexcept override
    {
        return std::format("({} . {})", *lhs, *rhs);
    }
};

}} // namespace hi::inline v1
