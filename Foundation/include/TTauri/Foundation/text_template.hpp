// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"

namespace TTauri {

struct text_template {
    using statement_vector = typename std::vector<std::unique_ptr<text_template>>;

    /** Append a template-piece to the current template.
     */
    virtual bool append(std::unique_ptr<text_template> x) noexcept { return false; }

    virtual bool found_elif(std::unique_ptr<expression> expression) noexcept { return false; }
    virtual bool found_else() noexcept { return false;}
    virtual bool found_while(std::unique_ptr<expression> expression) noexcept { return false; }
};

struct text_template_string: text_template {
    std::string text;
};

struct text_template_snippet: text_template {
    statement_vector children;

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<text_template> x) noexcept override {
        statement_vector.push_back(std::move(x));
        return true;
    }
};


struct text_template_parse_context {
    using statement_stack_type = typename text_template::statement_vector;
    using const_iterator = typename std::vector<token_t>::const_iterator;
    using function_type = std::function<datum(expression_evaluation_context&, datum::vector const &)>;
    using function_table = std::unordered_map<std::string,function_type>;
    using method_type = std::function<datum(expression_evaluation_context&, datum &, datum::vector const &)>;
    using method_table = std::unordered_map<std::string,method_type>;

    function_table functions;
    static function_table global_functions;
    static method_table global_methods;

    statement_stack_type statement_stack;

    std::string_view text;
    const_iterator index;
    const_iterator end;

    text_template_parse_context(std::string_view text) :
        text(text), index(text.begin()), end(text.end())    
    {
        push<text_template_snippet>();
    }

    [[nodiscard]] char const& operator*() const noexcept {
        return *index;
    }

    [[nodiscard]] bool atEOF() const noexcept {
        return index == end;
    }

    expression_parse_context& operator++() noexcept {
        axiom_assert(!atEOF());
        ++index;
        return *this;
    }

    expression_parse_context operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    expression_parse_context& operator+=(int x) noexcept {
        index += x;
        return *this;
    }

    bool starts_with(std::string_view text) const noexcept {
        return starts_with(index, end, text.begin(), text.end());
    }

    bool starts_with_and_advance(std::string_view text) noexcept {
        if (starts_with(text)) {
            *this += text.size();
            return true;
        } else {
            return false;
        }
    }

    template<typename T, typename... Args>
    void push(Args &&... args) {
        statement_stack.emplace_back<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    [[nodiscard]] bool append(Args &&... args) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back().append(std::make_unique<T>(std::forward<Args>(args)...));
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

    [[nodiscard]] bool found_elif(std::unique_ptr<expression> expression) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_elif(std::move(expression));
        } else {
            return false;
        }
    }

    [[nodiscard]] bool found_else() noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_else();
        } else {
            return false;
        }
    }

    [[nodiscard]] bool found_while(std::unique_ptr<expression> expression) noexcept {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_while(std::move(expression));
        } else {
            return false;
        }
    }

    
    void include(std::unique_ptr<expression> expression);

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

std::unique_ptr<text_template> text_template_parser(text_template_parse_context &context);

inline std::unique_ptr<expression> text_template_parser(std::string_view text) {
    auto context = text_template_parse_context(text);
    auto e = text_template_parser(context);
    e->post_process(context);
    return e;
}


}