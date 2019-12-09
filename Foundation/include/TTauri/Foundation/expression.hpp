// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/Location.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/tokenizer.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace TTauri {

struct expression_evaluation_context {
    using scope = std::unordered_map<std::string, datum>;
    using stack = std::vector<scope>;

    stack local_stack;
    scope globals;

    expression_evaluation_context() {};

    void push() {
        local_stack.emplace_back();
    }

    void pop() {
        required_assert(local_stack.size() > 1);
        local_stack.pop_back();
    }

    [[nodiscard]] bool has_locals() const noexcept {
        return local_stack.size() > 0;
    }

    force_inline scope const& locals() const {
        axiom_assert(has_locals());
        return local_stack.back();
    }

    force_inline scope& locals() {
        axiom_assert(has_locals());
        return local_stack.back();
    }

    

    [[nodiscard]] datum const& get(std::string const &name) const {
        if (has_locals()) {
            let i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        let j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        TTAURI_THROW(key_error("Could not find {} in local or global scope.", name));
    }

    [[nodiscard]] datum &get(std::string const &name) {
        if (has_locals()) {
            let i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        let j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        TTAURI_THROW(key_error("Could not find {} in local or global scope.", name));
    }

    void set_local(std::string const &name, datum& value) {
        locals()[name] = value;
    }

    void set_global(std::string const& name, datum& value) {
        globals[name] = value;
    }

    datum &set(std::string const &name, datum const &value) {
        if (has_locals()) {
            return locals()[name] = value;
        } else {
            return globals[name] = value;
        }
    }
};


struct expression_parse_context {
    using const_iterator = typename std::vector<token_t>::const_iterator;
    using function_type = std::function<datum(expression_evaluation_context&, datum::vector const &)>;
    using function_table = std::unordered_map<std::string,function_type>;
    using method_type = std::function<datum(expression_evaluation_context&, datum &, datum::vector const &)>;
    using method_table = std::unordered_map<std::string,method_type>;

    function_table functions;
    static function_table global_functions;
    static method_table global_methods;

    std::string_view::const_iterator first;
    std::string_view::const_iterator last;

    std::vector<token_t> tokens;
    const_iterator token_it;

    expression_parse_context(std::string_view::const_iterator first, std::string_view::const_iterator last) :
        first(first), last(last), tokens(parseTokens(first, last)), token_it(tokens.begin()) {}

    ssize_t offset() const noexcept {
        return std::distance(first, token_it->index);
    }

    [[nodiscard]] token_t const& operator*() const noexcept {
        return *token_it;
    }

    [[nodiscard]] token_t const *operator->() const noexcept {
        return &(*token_it);
    }

    expression_parse_context& operator++() noexcept {
        axiom_assert(token_it != tokens.end());
        axiom_assert(*token_it != tokenizer_name_t::End);
        ++token_it;
        return *this;
    }

    expression_parse_context operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

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

struct expression_node {
    using expression_vector = std::vector<std::unique_ptr<expression_node>>;

    ssize_t offset;

    expression_node(ssize_t offset) : offset(offset) {}

    virtual ~expression_node() {}

    /** Resolve function and method pointers.
     * At all call-expressions resolve the function pointers from the parse_context.
     */
    virtual void post_process(expression_parse_context& context) {}

    /** Resolve function and method pointers.
    * This is called on a name-expression or member-expression to set the function pointer.
    */
    virtual void resolve_function_pointer(expression_parse_context& context) {}

    /** Evaluate an rvalue.
     */
    virtual datum evaluate(expression_evaluation_context& context) const = 0;

    /** Evaluate an existing lvalue.
     */
    virtual datum &evaluate_lvalue(expression_evaluation_context& context) const {
        TTAURI_THROW(invalid_operation_error("Expression is not a modifiable value.").set<"offset"_tag>(offset));
    }

    /** Assign to a non-existing or existing lvalue.
     */
    virtual datum &assign(expression_evaluation_context& context, datum const &rhs) const {
        return evaluate_lvalue(context) = rhs;
    }

    /** Call a function with a datum::vector as arguments.
     */
    virtual datum call(expression_evaluation_context& context, datum::vector const &arguments) const {
        TTAURI_THROW(invalid_operation_error("Expression is not callable.").set<"offset"_tag>(offset));
    }

    virtual std::string string() const noexcept = 0;

    friend std::string to_string(expression_node const& rhs) noexcept {
        return rhs.string();
    }

    friend std::ostream& operator<<(std::ostream& lhs, expression_node const& rhs) noexcept {
        return lhs << to_string(rhs);
    }
};

/** Parse an expression.
    * Parses an expression until EOF, ')', ',', '}'
    */
std::unique_ptr<expression_node> parse_expression(expression_parse_context& context);

/** Parse an expression.
    * Parses an expression until EOF, ')', ',', '}'
    */
inline std::unique_ptr<expression_node> parse_expression(std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = expression_parse_context(first, last);
    auto e = parse_expression(context);
    e->post_process(context);
    return e;
}

/** Parse an expression.
* Parses an expression until EOF, ')', ',', '}'
*/
inline std::unique_ptr<expression_node> parse_expression(std::string_view text) {
    return parse_expression(text.cbegin(), text.cend());
}


/** Find the end of an expression.
    * This function will track nested brackets and strings, until the terminating_character is found.
    * @param first Iterator to the first character of the expression.
    * @param last Iterator to beyond the last character of the text.
    * @param terminating_string The string to find, which is not part of the expression.
    * @return Iterator to the terminating character if found, or last.
    */
std::string_view::const_iterator find_end_of_expression(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    std::string_view terminating_string);

}