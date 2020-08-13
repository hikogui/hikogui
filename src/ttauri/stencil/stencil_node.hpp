// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "../formula/formula.hpp"
#include "../strings.hpp"
#include "../algorithm.hpp"
#include "../ResourceView.hpp"
#include <memory>
#include <string_view>
#include <optional>

namespace tt {

struct stencil_node {
    using statement_vector = typename std::vector<std::unique_ptr<stencil_node>>;

    parse_location location;

    stencil_node(parse_location location) :
        location(std::move(location)) {}

    virtual ~stencil_node() {}

    /** Append a template-piece to the current template.
    */
    [[nodiscard]] virtual bool append(std::unique_ptr<stencil_node> x) noexcept { return false; }

    /** Should any spaces on the left side of a statement be removed?
    */
    [[nodiscard]] virtual bool should_left_align() const noexcept { return true; } 

    /** Remove any trailing spaces or tabs after a new-line.
    */
    virtual void left_align() noexcept {}

    [[nodiscard]] virtual bool found_elif(parse_location _location, std::unique_ptr<formula_node> expression) noexcept { return false; }
    [[nodiscard]] virtual bool found_else(parse_location _location) noexcept { return false;}
    [[nodiscard]] virtual bool found_while(parse_location _location, std::unique_ptr<formula_node> expression) noexcept { return false; }

    virtual void post_process(formula_post_process_context &context) {}

    /** Evaluate the template.
    * Text in the template is added to the context.output_text.
    * @param context Data used by expressions inside the template statements. .output_text will
    *        contain textual data from the template.
    * @return datum::undefined when the stencil_node generated textual data into context.output_text.
    *         datum::break when a \#break statement was encountered. datum::continue when a \#continue statement
    *         was encountered. Otherwise data returned from a \#return statement.
    */
    [[nodiscard]] virtual datum evaluate(formula_evaluation_context &context) {
        tt_no_default;
    }

    [[nodiscard]] std::string evaluate_output(formula_evaluation_context &context) {
        auto tmp = evaluate(context);
        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else if (tmp.is_undefined()) {
            return std::move(context.output);

        } else {
            TTAURI_THROW(invalid_operation_error("Found #return not inside a function.").set_location(location));
        }
    }

    [[nodiscard]] std::string evaluate_output() {
        auto context = formula_evaluation_context{};
        return evaluate_output(context);
    }

    [[nodiscard]] virtual std::string string() const noexcept {
        return "<stencil_node>";
    }

    [[nodiscard]] friend std::string to_string(stencil_node const &lhs) noexcept {
        return lhs.string();
    }

    friend std::ostream &operator<<(std::ostream &lhs, stencil_node const &rhs) {
        return lhs << to_string(rhs);
    }

    static void append_child(statement_vector &children, std::unique_ptr<stencil_node> new_child) {
        if (std::ssize(children) > 0 && new_child->should_left_align()) {
            children.back()->left_align();
        }
        children.push_back(std::move(new_child));
    }

    [[nodiscard]] static datum evaluate_formula_without_output(formula_evaluation_context &context, formula_node const &expression, parse_location const &location) {
        try {
            return expression.evaluate_without_output(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    [[nodiscard]] static datum evaluate_expression(formula_evaluation_context &context, formula_node const &expression, parse_location const &location) {
        try {
            return expression.evaluate(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    static void post_process_expression(formula_post_process_context &context, formula_node &expression, parse_location const &location) {
        try {
            return expression.post_process(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }


    [[nodiscard]] static datum evaluate_children(formula_evaluation_context &context, statement_vector const &children) {
        for (ttlet &child: children) {
            ttlet tmp = child->evaluate(context);
            if (!tmp.is_undefined()) {
                return tmp;
            }
        }
        return {};
    }
};

}