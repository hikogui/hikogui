// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/ResourceView.hpp"

namespace tt {

struct template_node;

struct template_parse_context {
    using statement_stack_type = std::vector<std::unique_ptr<template_node>>;
    using const_iterator = typename std::string_view::const_iterator;

    statement_stack_type statement_stack;

    parse_location location;
    const_iterator index;
    const_iterator last;

    std::optional<const_iterator> text_segment_start;

    /** Post process context is used to record functions that are defined
    * in the template being parsed.
    */
    expression_post_process_context post_process_context;

    template_parse_context() = delete;
    template_parse_context(template_parse_context const &other) = delete;
    template_parse_context &operator=(template_parse_context const &other) = delete;
    template_parse_context(template_parse_context &&other) = delete;
    template_parse_context &operator=(template_parse_context &&other) = delete;
    ~template_parse_context() = default;

    template_parse_context(URL const &url, const_iterator first, const_iterator last);

    [[nodiscard]] char const& operator*() const noexcept {
        return *index;
    }

    [[nodiscard]] bool atEOF() const noexcept {
        return index == last;
    }

    template_parse_context& operator++() noexcept {
        ttauri_assume(!atEOF());
        location += *index;
        ++index;
        return *this;
    }

    template_parse_context& operator+=(ssize_t x) noexcept {
        for (ssize_t i = 0; i != x; ++i) {
            ++(*this);
        }
        return *this;
    }

    bool starts_with(std::string_view text) const noexcept {
        return ::tt::starts_with(index, last, text.begin(), text.end());
    }

    bool starts_with_and_advance_over(std::string_view text) noexcept {
        if (starts_with(text)) {
            *this += ssize(text);
            return true;
        } else {
            return false;
        }
    }

    bool advance_to(std::string_view text) noexcept {
        while (!atEOF()) {
            if (starts_with(text)) {
                return true;
            }
            ++(*this);
        }
        return false;
    }

    bool advance_over(std::string_view text) noexcept {
        if (advance_to(text)) {
            *this += ssize(text);
            return true;
        } else {
            return false;
        }
    }

    std::unique_ptr<expression_node> parse_expression(std::string_view end_text);

    std::unique_ptr<expression_node> parse_expression_and_advance_over(std::string_view end_text);

    template<typename T, typename... Args>
    void push(Args &&... args) {
        statement_stack.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    [[nodiscard]] bool append(std::unique_ptr<template_node> x) noexcept;

    template<typename T, typename... Args>
    [[nodiscard]] bool append(Args &&... args) noexcept {
        if (statement_stack.size() > 0) {
            return append(std::make_unique<T>(std::forward<Args>(args)...));
        } else {
            return false;
        }
    }

    /** Handle #end statement.
     * This will pop the current statement of the stack and append it
     * to the statement that is now at the top of the stack.
     */
    [[nodiscard]] bool pop() noexcept;

    void start_of_text_segment(int back_track = 0) noexcept;
    void end_of_text_segment();

    [[nodiscard]] bool top_statement_is_do() const noexcept;

    [[nodiscard]] bool found_elif(parse_location location, std::unique_ptr<expression_node> expression) noexcept;

    [[nodiscard]] bool found_else(parse_location location) noexcept;

    [[nodiscard]] bool found_while(parse_location location, std::unique_ptr<expression_node> expression) noexcept;

    void include(parse_location location, std::unique_ptr<expression_node> expression);
};

struct template_node {
    using statement_vector = typename std::vector<std::unique_ptr<template_node>>;

    parse_location location;

    template_node(parse_location location) :
        location(std::move(location)) {}

    virtual ~template_node() {}

    /** Append a template-piece to the current template.
     */
    [[nodiscard]] virtual bool append(std::unique_ptr<template_node> x) noexcept { return false; }

    /** Should any spaces on the left side of a statement be removed?
     */
    [[nodiscard]] virtual bool should_left_align() const noexcept { return true; } 

    /** Remove any trailing spaces or tabs after a new-line.
     */
    virtual void left_align() noexcept {}

    [[nodiscard]] virtual bool found_elif(parse_location _location, std::unique_ptr<expression_node> expression) noexcept { return false; }
    [[nodiscard]] virtual bool found_else(parse_location _location) noexcept { return false;}
    [[nodiscard]] virtual bool found_while(parse_location _location, std::unique_ptr<expression_node> expression) noexcept { return false; }

    virtual void post_process(expression_post_process_context &context) {}

    /** Evaluate the template.
     * Text in the template is added to the context.output_text.
     * @param context Data used by expressions inside the template statements. .output_text will
     *        contain textual data from the template.
     * @return datum::undefined when the template_node generated textual data into context.output_text.
     *         datum::break when a #break statement was encountered. datum::continue when a #continue statement
     *         was encountered. Otherwise data returned from a #return statement.
     */
    [[nodiscard]] virtual datum evaluate(expression_evaluation_context &context) {
        no_default;
    }

    [[nodiscard]] std::string evaluate_output(expression_evaluation_context &context) {
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
        auto context = expression_evaluation_context{};
        return evaluate_output(context);
    }

    [[nodiscard]] virtual std::string string() const noexcept {
        return "<template_node>";
    }

    [[nodiscard]] friend std::string to_string(template_node const &lhs) noexcept {
        return lhs.string();
    }

    friend std::ostream &operator<<(std::ostream &lhs, template_node const &rhs) {
        return lhs << to_string(rhs);
    }

    static void append_child(statement_vector &children, std::unique_ptr<template_node> new_child) {
        if (ssize(children) > 0 && new_child->should_left_align()) {
            children.back()->left_align();
        }
        children.push_back(std::move(new_child));
    }

    [[nodiscard]] static datum evaluate_expression_without_output(expression_evaluation_context &context, expression_node const &expression, parse_location const &location) {
        try {
            return expression.evaluate_without_output(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    [[nodiscard]] static datum evaluate_expression(expression_evaluation_context &context, expression_node const &expression, parse_location const &location) {
        try {
            return expression.evaluate(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    static void post_process_expression(expression_post_process_context &context, expression_node &expression, parse_location const &location) {
        try {
            return expression.post_process(context);
        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }


    [[nodiscard]] static datum evaluate_children(expression_evaluation_context &context, statement_vector const &children) {
        for (ttlet &child: children) {
            ttlet tmp = child->evaluate(context);
            if (!tmp.is_undefined()) {
                return tmp;
            }
        }
        return {};
    }
};


std::unique_ptr<template_node> parse_template(template_parse_context &context);

inline std::unique_ptr<template_node> parse_template(URL url, std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = template_parse_context(std::move(url), first, last);
    auto e = parse_template(context);
    return e;
}

inline std::unique_ptr<template_node> parse_template(URL url, std::string_view text) {
    return parse_template(std::move(url), text.cbegin(), text.cend());
}

inline std::unique_ptr<template_node> parse_template(URL url) {
    ttlet fv = ResourceView::loadView(url);
    ttlet sv = fv->string_view();

    return parse_template(std::move(url), sv.cbegin(), sv.cend());
}



}
