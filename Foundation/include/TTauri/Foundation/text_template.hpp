// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/algorithm.hpp"

namespace TTauri {

struct template_parse_context;

struct template_node {
    using statement_vector = typename std::vector<std::unique_ptr<template_node>>;

    ssize_t offset;

    template_node(ssize_t offset) :
        offset(offset) {}

    virtual ~template_node() {}

    /** Append a template-piece to the current template.
     */
    virtual bool append(std::unique_ptr<template_node> x) noexcept { return false; }

    virtual bool found_elif(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept { return false; }
    virtual bool found_else(ssize_t offset) noexcept { return false;}
    virtual bool found_while(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept { return false; }

    void post_process(template_parse_context &context) {
        no_default;
    }

    virtual std::string string() const noexcept {
        return "<template_node>";
    }

    friend std::string to_string(template_node const &lhs) noexcept {
        return lhs.string();
    }

    friend std::ostream &operator<<(std::ostream &lhs, template_node const &rhs) {
        return lhs << to_string(rhs);
    }
};


struct template_top_node: template_node {
    URL url;
    statement_vector children;

    template_top_node(ssize_t offset, URL url) :
        template_node(offset), url(std::move(url)), children() {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        let children_str = transform<std::vector<std::string>>(children, [](auto const &x) { return x->string(); });
        return fmt::format("<snippet {}>", join(children_str, ", "));
    }
};

struct template_parse_context {
    using statement_stack_type = typename template_node::statement_vector;
    using const_iterator = typename std::string_view::const_iterator;
    using function_type = std::function<datum(expression_evaluation_context&, datum::vector const &)>;
    using function_table = std::unordered_map<std::string,function_type>;
    using method_type = std::function<datum(expression_evaluation_context&, datum &, datum::vector const &)>;
    using method_table = std::unordered_map<std::string,method_type>;

    function_table functions;
    static function_table global_functions;
    static method_table global_methods;

    statement_stack_type statement_stack;

    URL url;
    const_iterator first;
    const_iterator last;
    const_iterator text_it;

    std::optional<const_iterator> text_segment_start;

    template_parse_context() = delete;
    template_parse_context(template_parse_context const &other) = delete;
    template_parse_context &operator=(template_parse_context const &other) = delete;
    template_parse_context(template_parse_context &&other) = delete;
    template_parse_context &operator=(template_parse_context &&other) = delete;
    ~template_parse_context() = default;

    template_parse_context(URL const &url, const_iterator first, const_iterator last) :
        url(url), first(first), last(last), text_it(first)
    {
        push<template_top_node>(0, url);
    }

    [[nodiscard]] ssize_t offset() const noexcept {
        return std::distance(first, text_it);
    }

    [[nodiscard]] char const& operator*() const noexcept {
        return *text_it;
    }

    [[nodiscard]] bool atEOF() const noexcept {
        return text_it == last;
    }

    template_parse_context& operator++() noexcept {
        axiom_assert(!atEOF());
        ++text_it;
        return *this;
    }

    template_parse_context& operator+=(int x) noexcept {
        text_it += x;
        return *this;
    }

    bool starts_with(std::string_view text) const noexcept {
        return ::TTauri::starts_with(text_it, last, text.begin(), text.end());
    }

    bool starts_with_and_advance_over(std::string_view text) noexcept {
        if (starts_with(text)) {
            *this += text.size();
            return true;
        } else {
            return false;
        }
    }

    bool advance_to(std::string_view text) noexcept {
        for (; text_it != last; ++text_it) {
            if (starts_with(text)) {
                return true;
            }
        }
        return false;
    }

    bool advance_over(std::string_view text) noexcept {
        if (advance_to(text)) {
            *this += text.size();
            return true;
        } else {
            return false;
        }
    }

    std::unique_ptr<expression_node> parse_expression(std::string_view end_text) {
        let expression_last = find_end_of_expression(text_it, last, end_text);

        auto context = expression_parse_context(text_it, expression_last);

        std::unique_ptr<expression_node> expression;

        try {
            expression = ::TTauri::parse_expression(context);
        } catch (error &e) {
            required_assert(e.has<"offset"_tag>());
            e.set<"offset"_tag>(offset() + e.get<"offset"_tag>());
        }

        text_it = expression_last;
        if (starts_with(end_text)) {
            *this += end_text.size();
        }

        return expression;
    }

    template<typename T, typename... Args>
    void push(Args &&... args) {
        statement_stack.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
    [[nodiscard]] bool append(Args &&... args) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->append(std::make_unique<T>(std::forward<Args>(args)...));
        } else {
            return false;
        }
    }

    [[nodiscard]] bool pop() noexcept {
        if (statement_stack.size() > 0) {
            auto tmp = std::move(statement_stack.back());
            statement_stack.pop_back();
            return statement_stack.back()->append(std::move(tmp));
        } else {
            return false;
        }
    }

    void start_of_text_segment(int back_track = 0) noexcept;
    void end_of_text_segment();

    [[nodiscard]] bool top_statement_is_do() const noexcept;

    [[nodiscard]] bool found_elif(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_elif(offset, std::move(expression));
        } else {
            return false;
        }
    }

    [[nodiscard]] bool found_else(ssize_t offset) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_else(offset);
        } else {
            return false;
        }
    }

    [[nodiscard]] bool found_while(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_while(offset, std::move(expression));
        } else {
            return false;
        }
    }


    void include(ssize_t offset, std::unique_ptr<expression_node> expression);

    [[nodiscard]] function_type get_function(std::string const &name) const noexcept {
        let i = functions.find(name);
        if (i != functions.end()) {
            return i->second;
        }

        let j = global_functions.find(name);
        if (j != global_functions.end()) {
            return j->second;
        }

        return {};
    }

    [[nodiscard]] method_type get_method(std::string const &name) const noexcept {
        let i = global_methods.find(name);
        if (i != global_methods.end()) {
            return i->second;
        }

        return {};
    }
};


std::unique_ptr<template_node> parse_template(template_parse_context &context);

inline std::unique_ptr<template_node> parse_template(URL url, std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = template_parse_context(std::move(url), first, last);
    auto e = parse_template(context);
    e->post_process(context);
    return e;
}

inline std::unique_ptr<template_node> parse_template(URL url, std::string_view text) {
    return parse_template(std::move(url), text.cbegin(), text.cend());
}



}