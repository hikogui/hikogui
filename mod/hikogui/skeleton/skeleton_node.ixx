// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <memory>
#include <string_view>
#include <optional>

export module hikogui_skeleton : node;
import hikogui_algorithm;
import hikogui_formula;

export namespace hi::inline v1 {

struct skeleton_node {
    using statement_vector = typename std::vector<std::unique_ptr<skeleton_node>>;

    parse_location location;

    skeleton_node(parse_location location) : location(std::move(location)) {}

    virtual ~skeleton_node() {}

    /** Append a template-piece to the current template.
     */
    [[nodiscard]] virtual bool append(std::unique_ptr<skeleton_node> x) noexcept
    {
        return false;
    }

    /** Should any spaces on the left side of a statement be removed?
     */
    [[nodiscard]] virtual bool should_left_align() const noexcept
    {
        return true;
    }

    /** Remove any trailing spaces or tabs after a new-line.
     */
    virtual void left_align() noexcept {}

    [[nodiscard]] virtual bool found_elif(parse_location _location, std::unique_ptr<formula_node> expression) noexcept
    {
        return false;
    }
    [[nodiscard]] virtual bool found_else(parse_location _location) noexcept
    {
        return false;
    }
    [[nodiscard]] virtual bool found_while(parse_location _location, std::unique_ptr<formula_node> expression) noexcept
    {
        return false;
    }

    virtual void post_process(formula_post_process_context &context) {}

    /** Evaluate the template.
     * Text in the template is added to the context.output_text.
     * @param context Data used by expressions inside the template statements. .output_text will
     *        contain textual data from the template.
     * @return datum::undefined when the skeleton_node generated textual data into context.output_text.
     *         datum::break when a \#break statement was encountered. datum::continue when a \#continue statement
     *         was encountered. Otherwise data returned from a \#return statement.
     */
    [[nodiscard]] virtual datum evaluate(formula_evaluation_context &context)
    {
        hi_no_default();
    }

    [[nodiscard]] std::string evaluate_output(formula_evaluation_context &context)
    {
        auto tmp = evaluate(context);
        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else if (tmp.is_undefined()) {
            return std::move(context.output);

        } else {
            throw operation_error(std::format("{}: Found #return not inside a function.", location));
        }
    }

    [[nodiscard]] std::string evaluate_output()
    {
        auto context = formula_evaluation_context{};
        return evaluate_output(context);
    }

    [[nodiscard]] virtual std::string string() const noexcept
    {
        return "<skeleton_node>";
    }

    [[nodiscard]] friend std::string to_string(skeleton_node const &lhs) noexcept
    {
        return lhs.string();
    }

    friend std::ostream &operator<<(std::ostream &lhs, skeleton_node const &rhs)
    {
        return lhs << to_string(rhs);
    }

    static void append_child(statement_vector &children, std::unique_ptr<skeleton_node> new_child)
    {
        if (ssize(children) > 0 && new_child->should_left_align()) {
            children.back()->left_align();
        }
        children.push_back(std::move(new_child));
    }

    [[nodiscard]] static datum evaluate_formula_without_output(
        formula_evaluation_context &context,
        formula_node const &expression,
        parse_location const &location)
    {
        try {
            return expression.evaluate_without_output(context);

        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Could not evaluate.\n{}", location, e.what()));
        }
    }

    [[nodiscard]] static datum
    evaluate_expression(formula_evaluation_context &context, formula_node const &expression, parse_location const &location)
    {
        try {
            return expression.evaluate(context);

        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Could not evaluate expression.\n{}", location, e.what()));
        }
    }

    static void
    post_process_expression(formula_post_process_context &context, formula_node &expression, parse_location const &location)
    {
        try {
            return expression.post_process(context);

        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Could not post-process expression.\n{}", location, e.what()));
        }
    }

    [[nodiscard]] static datum evaluate_children(formula_evaluation_context &context, statement_vector const &children)
    {
        for (hilet &child : children) {
            hilet tmp = child->evaluate(context);
            if (!tmp.is_undefined()) {
                return tmp;
            }
        }
        return {};
    }
};

} // namespace hi::inline v1
