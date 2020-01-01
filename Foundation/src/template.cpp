// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/template.hpp"

namespace TTauri {

struct template_top_node final: template_node {
    statement_vector children;

    template_top_node(parse_location location) :
        template_node(std::move(location)), children() {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        for (let &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        try {
            return evaluate_children(context, children);

        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        let children_str = transform<std::vector<std::string>>(children, [](auto const &x) { return x->string(); });
        return fmt::format("<top {}>", join(children_str));
    }
};

struct template_string_node final: template_node {
    std::string text;

    template_string_node(parse_location location, std::string text) :
        template_node(std::move(location)), text(std::move(text)) {}

    [[nodiscard]] bool should_left_align() const noexcept override { return false; }

    void left_align() noexcept override {
        // first check if there are only spaces and tabs after the last line feed.
        auto new_text_length = ssize(text);
        for (auto i = text.crbegin(); i != text.crend(); ++i, --new_text_length) {
            if (*i == ' ' || *i == '\t') {
                // Strip spaces and tabs.
                continue;
            } else if (*i == '\n') {
                // Stop here, we are stripping a line upto the last line feed.
                break;
            } else {
                // If there are characters after the last line feed, we do not want to strip the line.
                return;
            }
        }
        text.resize(new_text_length);
    }

    std::string string() const noexcept override {
        return fmt::format("<text {}>", text);
    }

    datum evaluate(expression_evaluation_context &context) override {
        context.write(text);
        return {};
    }

};

struct template_placeholder_node final: template_node {
    std::unique_ptr<expression_node> expression;

    template_placeholder_node(parse_location location, std::unique_ptr<expression_node> expression) :
        template_node(std::move(location)), expression(std::move(expression)) {}

    [[nodiscard]] bool should_left_align() const noexcept override { return false; }

    void post_process(expression_post_process_context &context) override {
        try {
            expression->post_process(context);
        } catch (parse_error &e) {
            e.merge_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("<placeholder {}>", *expression);
    }

    datum evaluate(expression_evaluation_context &context) override {
        let output_size = context.output_size();

        let tmp = evaluate_expression(context, *expression, location);
        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            // When a function returns, it should not have written data to the output.
            context.set_output_size(output_size);
            context.write(static_cast<std::string>(tmp));
            return {};
        }
    }
};

struct template_expression_node final: template_node {
    std::unique_ptr<expression_node> expression;

    template_expression_node(parse_location location, std::unique_ptr<expression_node> expression) :
        template_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(expression_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<expression {}>", *expression);
    }

    datum evaluate(expression_evaluation_context &context) override {
        let tmp = evaluate_expression_without_output(context, *expression, location);
        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else {
            return {};
        }
    }
};

struct template_if_node final: template_node {
    std::vector<statement_vector> children_groups;
    std::vector<std::unique_ptr<expression_node>> expressions;
    std::vector<parse_location> expression_locations;

    template_if_node(parse_location location, std::unique_ptr<expression_node> expression) noexcept :
        template_node(location)
    {
        expressions.push_back(std::move(expression));
        expression_locations.push_back(location);
        children_groups.emplace_back();
    }

    bool found_elif(parse_location location, std::unique_ptr<expression_node> expression) noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        expressions.push_back(std::move(expression));
        expression_locations.push_back(std::move(location));
        children_groups.emplace_back();
        return true;
    }

    bool found_else(parse_location location) noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        children_groups.emplace_back();
        return true;
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        append_child(children_groups.back(), std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        ttauri_assert(ssize(expressions) == ssize(expression_locations));
        for (ssize_t i = 0; i != ssize(expressions); ++i) {
            post_process_expression(context, *expressions[i], expression_locations[i]);
        }

        for (let &children: children_groups) {
            if (ssize(children) > 0) {
                children.back()->left_align();
            }

            for (let &child: children) {
                child->post_process(context);
            }
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        ttauri_axiom(ssize(expressions) == ssize(expression_locations));
        for (ssize_t i = 0; i != ssize(expressions); ++i) {
            if (evaluate_expression_without_output(context, *expressions[i], expression_locations[i])) {
                return evaluate_children(context, children_groups[i]);
            }
        }
        if (ssize(children_groups) > ssize(expressions)) {
            return evaluate_children(context, children_groups[ssize(expressions)]);
        }
        return {};
    }

    std::string string() const noexcept override {
        ttauri_assert(expressions.size() > 0);
        std::string s = "<if ";
        s += to_string(*expressions[0]);
        s += join(transform<std::vector<std::string>>(children_groups[0], [](auto &x) { return to_string(*x); }));

        for (size_t i = 1; i != expressions.size(); ++i) {
            s += "elif ";
            s += to_string(*expressions[i]);
            s += join(transform<std::vector<std::string>>(children_groups[i], [](auto &x) { return to_string(*x); }));
        }

        if (children_groups.size() != expressions.size()) {
            s += "else ";
            s += join(transform<std::vector<std::string>>(children_groups.back(), [](auto &x) { return to_string(*x); }));
        }

        s += ">";
        return s;
    }
};

struct template_while_node final: template_node {
    statement_vector children;
    std::unique_ptr<expression_node> expression;

    template_while_node(parse_location location, std::unique_ptr<expression_node> expression) noexcept :
        template_node(std::move(location)), expression(std::move(expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        post_process_expression(context, *expression, location);
        for (let &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        let output_size = context.output_size();

        ssize_t loop_count = 0;
        while (evaluate_expression_without_output(context, *expression, location)) {
            context.loop_push(loop_count++);
            auto tmp = evaluate_children(context, children);
            context.loop_pop();
            if (tmp.is_break()) {
                break;
            } else if (tmp.is_continue()) {
                continue;
            } else if (!tmp.is_undefined()) {
                context.set_output_size(output_size);
                return tmp;
            }
        }
        return {};
    }

    std::string string() const noexcept override {
        std::string s = "<while ";
        s += to_string(*expression);
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};

struct template_do_node final: template_node {
    statement_vector children;
    std::unique_ptr<expression_node> expression;
    parse_location expression_location;

    template_do_node(parse_location location) noexcept :
        template_node(std::move(location)) {}


    bool found_while(parse_location location, std::unique_ptr<expression_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            expression = std::move(x);
            expression_location = std::move(location);
            return true;
        }
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            append_child(children, std::move(x));
            return true;
        }
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        post_process_expression(context, *expression, location);

        for (let &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        let output_size = context.output_size();

        ssize_t loop_count = 0;
        do {
            context.loop_push(loop_count++);
            auto tmp = evaluate_children(context, children);
            context.loop_pop();

            if (tmp.is_break()) {
                break;
            } else if (tmp.is_continue()) {
                continue;
            } else if (!tmp.is_undefined()) {
                context.set_output_size(output_size);
                return tmp;
            }

        } while (evaluate_expression_without_output(context, *expression, expression_location));
        return {};
    }

    std::string string() const noexcept override {
        ttauri_assert(expression);
        std::string s = "<do ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += to_string(*expression);
        s += ">";
        return s;
    }
};

struct template_for_node final: template_node {
    std::unique_ptr<expression_node> name_expression;
    std::unique_ptr<expression_node> list_expression;
    bool has_else = false;
    statement_vector children;
    statement_vector else_children;

    template_for_node(parse_location location, std::unique_ptr<expression_node> name_expression, std::unique_ptr<expression_node> list_expression) noexcept :
        template_node(std::move(location)), name_expression(std::move(name_expression)), list_expression(std::move(list_expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        if (has_else) {
            append_child(else_children, std::move(x));
        } else {
            append_child(children, std::move(x));
        }
        return true;
    }

    bool found_else(parse_location location) noexcept override {
        if (has_else) {
            return false;
        } else {
            has_else = true;
            return true;
        }
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }
        if (ssize(else_children) > 0) {
            else_children.back()->left_align();
        }

        post_process_expression(context, *name_expression, location);
        post_process_expression(context, *list_expression, location);

        for (let &child: children) {
            child->post_process(context);
        }
        for (let &child: else_children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        auto list_data = evaluate_expression_without_output(context, *list_expression, location);

        if (!list_data.is_vector()) {
            TTAURI_THROW(invalid_operation_error("Expecting expression returns a vector, got {}", list_data).set_location(location));
        }

        let output_size = context.output_size();
        if (list_data.size() > 0) {
            let loop_size = ssize(list_data);
            ssize_t loop_count = 0;
            for (auto i = list_data.vector_begin(); i != list_data.vector_end(); ++i) {
                let &item = *i;
                try {
                    name_expression->assign_without_output(context, item);
                } catch (invalid_operation_error &e) {
                    e.merge_location(location);
                }

                context.loop_push(loop_count++, loop_size);
                auto tmp = evaluate_children(context, children);
                context.loop_pop();

                if (tmp.is_break()) {
                    break;
                } else if (tmp.is_continue()) {
                    continue;
                } else if (!tmp.is_undefined()) {
                    context.set_output_size(output_size);
                    return tmp;
                }
            }

        } else {
            auto tmp = evaluate_children(context, else_children);
            if (tmp.is_break() || tmp.is_continue()) {
                return tmp;
            } else if (!tmp.is_undefined()) {
                context.set_output_size(output_size);
                return tmp;
            }
        }
        return {};
    }

    std::string string() const noexcept override {
        std::string s = "<for ";
        s += to_string(*name_expression);
        s += ": ";
        s += to_string(*list_expression);
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        if (has_else) {
            s += "else ";
            s += join(transform<std::vector<std::string>>(else_children, [](auto &x) { return to_string(*x); }));
        }
        s += ">";
        return s;
    }
};

struct template_function_node final: template_node {
    std::string name;
    std::vector<std::string> argument_names;
    statement_vector children;

    expression_post_process_context::function_type super_function;

    template_function_node(parse_location location, expression_post_process_context &context, std::unique_ptr<expression_node> function_declaration_expression) noexcept :
        template_node(std::move(location))
    {
        auto name_and_arguments = function_declaration_expression->get_name_and_argument_names();
        ttauri_assert(name_and_arguments.size() >= 1);

        name = name_and_arguments[0];
        name_and_arguments.erase(name_and_arguments.begin());
        argument_names = std::move(name_and_arguments);

        super_function = context.set_function(name,
            [this,&location](expression_evaluation_context &context, datum::vector const &arguments) {
                try {
                    return this->evaluate_call(context, arguments);
                } catch (error &e) {
                    TTAURI_THROW(invalid_operation_error("Failed during handling of function call")
                        .caused_by(e)
                        .set_location(location)
                    );
                }
            }
        );
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        context.push_super(super_function);
        for (let &child: children) {
            child->post_process(context);
        }
        context.pop_super();
    }

    datum evaluate(expression_evaluation_context &context) override {
        return {};
    }

    datum evaluate_call(expression_evaluation_context &context, datum::vector const &arguments) {
        context.push();
        if (ssize(argument_names) != ssize(arguments)) {
            TTAURI_THROW(invalid_operation_error("Invalid number of arguments to function {}() expecting {} got {}.", name, argument_names.size(), arguments.size()).set_location(location));
        }

        for (ssize_t i = 0; i != ssize(argument_names); ++i) {
            context.set(argument_names[i], arguments[i]);
        }

        let output_size = context.output_size();
        auto tmp = evaluate_children(context, children);
        context.pop();

        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            // When a function returns, it should not have written data to the output.
            context.set_output_size(output_size);
            return tmp;
        }
    }

    std::string string() const noexcept override {
        std::string s = "<function ";
        s += name;
        s += "(";
        s += join(argument_names, ",");
        s += ")";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }

};

struct template_block_node final: template_node {
    std::string name;
    statement_vector children;

    expression_post_process_context::function_type function;
    expression_post_process_context::function_type super_function;

    template_block_node(parse_location location, expression_post_process_context &context, std::unique_ptr<expression_node> name_expression) noexcept :
        template_node(std::move(location)), name(name_expression->get_name())
    {
        name = name_expression->get_name();

        super_function = context.set_function(name,
            [&](expression_evaluation_context &context, datum::vector const &arguments) {
                return this->evaluate_call(context, arguments);
            }
        );
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        auto function = context.get_function(name);
        ttauri_assert(function);

        context.push_super(super_function);
        for (let &child: children) {
            child->post_process(context);
        }
        context.pop_super();
    }

    datum evaluate(expression_evaluation_context &context) override {
        datum tmp;
        try {
            tmp = function(context, datum::vector{});
        } catch (invalid_operation_error &e) {
            e.merge_location(location);
            throw;
        }

        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            TTAURI_THROW(invalid_operation_error("Can not use a #return statement inside a #block.").set_location(location));
        }
    }

    datum evaluate_call(expression_evaluation_context &context, datum::vector const &arguments) {
        context.push();
        auto tmp = evaluate_children(context, children);
        context.pop();

        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            TTAURI_THROW(invalid_operation_error("Can not use a #return statement inside a #block.").set_location(location));
        }
    }

    std::string string() const noexcept override {
        std::string s = "<block ";
        s += name;
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};


struct template_break_node final: template_node {
    template_break_node(parse_location location) noexcept : template_node(std::move(location)) {}

    datum evaluate(expression_evaluation_context &context) override {
        return datum::_break{};
    }

    std::string string() const noexcept override {
        return "<break>";
    }

};

struct template_continue_node final: template_node {
    template_continue_node(parse_location location) noexcept : template_node(std::move(location)) {}

    datum evaluate(expression_evaluation_context &context) override {
        return datum::_continue{};
    }

    std::string string() const noexcept override {
        return "<continue>";
    }
};

struct template_return_node final: template_node {
    std::unique_ptr<expression_node> expression;

    template_return_node(parse_location location, std::unique_ptr<expression_node> expression) noexcept :
        template_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(expression_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    datum evaluate(expression_evaluation_context &context) override {
        return evaluate_expression_without_output(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<return {}>", *expression);
    }
};

[[nodiscard]] bool template_parse_context::append(std::unique_ptr<template_node> x) noexcept
{
    return statement_stack.back()->append(std::move(x));
}

template_parse_context::template_parse_context(URL const &url, const_iterator first, const_iterator last) :
    location(url), index(first), last(last)
{
    push<template_top_node>(location);
}

std::unique_ptr<expression_node> template_parse_context::parse_expression(std::string_view end_text) {
    let expression_last = find_end_of_expression(index, last, end_text);

    auto context = expression_parse_context(index, expression_last);

    std::unique_ptr<expression_node> expression;

    try {
        expression = ::TTauri::parse_expression(context);
    } catch (error &e) {
        e.merge_location(location);
        throw;
    }

    (*this) += std::distance(index, expression_last);
    return expression;
}

std::unique_ptr<expression_node> template_parse_context::parse_expression_and_advance_over(std::string_view end_text) {
    auto expression = parse_expression(end_text);

    if (!starts_with_and_advance_over(end_text)) {
        TTAURI_THROW(parse_error("Could not find '{}' after expression", end_text).set_location(location));
    }

    return expression;
}

[[nodiscard]] bool template_parse_context::pop() noexcept {
    if (statement_stack.size() > 0) {
        auto tmp = std::move(statement_stack.back());
        statement_stack.pop_back();
        return statement_stack.back()->append(std::move(tmp));
    } else {
        return false;
    }
}

[[nodiscard]] bool template_parse_context::top_statement_is_do() const noexcept
{
    if (statement_stack.size() < 1) {
        return false;
    }

    auto ptr = dynamic_cast<template_do_node*>(statement_stack.back().get());
    return ptr != nullptr;
}

void template_parse_context::start_of_text_segment(int back_track) noexcept
{
    text_segment_start = index - back_track;
}

void template_parse_context::end_of_text_segment()
{
    if (text_segment_start) {
        if (index > *text_segment_start) {
            if (!append<template_string_node>(location, std::string(*text_segment_start, index))) {
                TTAURI_THROW(parse_error("Unexpected text segment.").set_location(location));
            }
        }

        text_segment_start = {};
    }
}

[[nodiscard]] bool template_parse_context::found_elif(parse_location location, std::unique_ptr<expression_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_elif(std::move(location), std::move(expression));
    } else {
        return false;
    }
}

[[nodiscard]] bool template_parse_context::found_else(parse_location location) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_else(std::move(location));
    } else {
        return false;
    }
}

[[nodiscard]] bool template_parse_context::found_while(parse_location location, std::unique_ptr<expression_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_while(std::move(location), std::move(expression));
    } else {
        return false;
    }
}

void template_parse_context::include(parse_location location, std::unique_ptr<expression_node> expression)
{
    auto post_process_context = expression_post_process_context();
    expression->post_process(post_process_context);

    auto evaluation_context = expression_evaluation_context();
    let argument = expression->evaluate(evaluation_context);

    let current_template_directory = location.has_file() ?
        location.file().urlByRemovingFilename() :
        URL::urlFromCurrentWorkingDirectory();

    let new_template_path = current_template_directory.urlByAppendingPath(static_cast<std::string>(argument));

    if (ssize(statement_stack) > 0) {
        if (!statement_stack.back()->append(parse_template(new_template_path))) {
            TTAURI_THROW(parse_error("Unexpected #include statement").set_location(location));
        }
    } else {
        TTAURI_THROW(parse_error("Unexpected #include statement, missing top-level").set_location(location));
    }
}

void parse_template_hash(template_parse_context &context)
{
    let &location = context.location;

    if (context.starts_with("end")) {
        context.advance_over("\n");

        if (!context.pop()) {
            TTAURI_THROW(parse_error("Unexpected #end statement.").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("#")) {
        if (!context.append<template_expression_node>(location, context.parse_expression_and_advance_over("\n"))) {
            TTAURI_THROW(parse_error("Unexpected ## (expression) statement.").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("if ")) {
        context.push<template_if_node>(location, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("elif ")) {
        if (!context.found_elif(location, context.parse_expression_and_advance_over("\n"))) {
            TTAURI_THROW(parse_error("Unexpected #elif statement.").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("else")) {
        context.advance_over("\n");

        if (!context.found_else(location)) {
            TTAURI_THROW(parse_error("Unexpected #else statement.").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("for ")) {
        auto name_expression = context.parse_expression_and_advance_over(":");
        auto list_expression = context.parse_expression_and_advance_over("\n");

        context.push<template_for_node>(location, std::move(name_expression), std::move(list_expression));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("while ")) {
        auto expression = context.parse_expression_and_advance_over("\n");

        if (context.top_statement_is_do()) {
            if (!context.found_while(location, std::move(expression))) {
                TTAURI_THROW(parse_error("Unexpected #while statement; missing #do.").set_location(location));
            }

            ttauri_assert(context.pop());
        } else {
            context.push<template_while_node>(location, std::move(expression));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("do")) {
        context.advance_over("\n");

        context.push<template_do_node>(location);

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("function ")) {
        context.push<template_function_node>(location, context.post_process_context, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("block ")) {
        context.push<template_block_node>(location, context.post_process_context, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("break")) {
        context.advance_over("\n");

        if (!context.append<template_break_node>(location)) {
            TTAURI_THROW(parse_error("Unexpected #break statement").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("continue")) {
        context.advance_over("\n");

        if (!context.append<template_continue_node>(location)) {
            TTAURI_THROW(parse_error("Unexpected #continue statement").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("return ")) {
        if (!context.append<template_return_node>(location, context.parse_expression_and_advance_over("\n"))) {
            TTAURI_THROW(parse_error("Unexpected #return statement").set_location(location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("include ")) {
        context.include(location, context.parse_expression_and_advance_over("\n"));
        context.start_of_text_segment();

    } else { // Add '#' and the current character to text.
        ++context;
        context.start_of_text_segment(-2);
    }
}

void parse_template_dollar(template_parse_context &context)
{
    let &location = context.location;

    if (*context == '{') {
        ++context;
        if (!context.append<template_placeholder_node>(location, context.parse_expression_and_advance_over("}"))) {
            TTAURI_THROW(parse_error("Unexpected placeholder."));
        }

        context.start_of_text_segment();

    } else {
        ++context;
        context.start_of_text_segment(-2);
    }
}

void parse_template_escape(template_parse_context &context)
{
    for (; !context.atEOF(); ++context) {
        let c = *context;

        switch (c) {
        case '\n': // Skip over line-feed
            ++context;
            context.start_of_text_segment();
            return;

        case '\r': // Skip over carriage return and potential line-feed.
            ++context;
            break;

        default: // Add character to text.
            ++context;
            context.start_of_text_segment(-2);
            return;
        }
    }
    TTAURI_THROW(parse_error("Unexpected end-of-file after escape '\' character.").set_location(context.location));
}

std::unique_ptr<template_node> parse_template(template_parse_context &context)
{
    context.start_of_text_segment();

    while (!context.atEOF()) {
        switch (*context) {
        case '#':
            context.end_of_text_segment();
            ++context;
            parse_template_hash(context);
            break;

        case '$':
            context.end_of_text_segment();
            ++context;
            parse_template_dollar(context);
            break;

        case '\\': // Skip the backslash.
            context.end_of_text_segment();
            ++context;
            parse_template_escape(context);
            break;

        default:
            ++context;
        } 
    }
    context.end_of_text_segment();

    if (context.statement_stack.size() < 1) {
        TTAURI_THROW(parse_error("Found to many #end statements.").set_location(context.location));
    } else if (context.statement_stack.size() > 1) {
        TTAURI_THROW(parse_error("Missing #end statement.").set_location(context.location));
    }

    auto top = std::move(context.statement_stack.back());
    context.statement_stack.pop_back();

    top->post_process(context.post_process_context);
    return top;
}

}
