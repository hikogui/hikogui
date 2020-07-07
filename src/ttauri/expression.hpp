// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/datum.hpp"
#include "ttauri/parse_location.hpp"
#include "ttauri/exceptions.hpp"
#include "ttauri/tokenizer.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <tuple>


namespace tt {

struct expression_evaluation_context {
    using scope = std::unordered_map<std::string, datum>;
    using stack = std::vector<scope>;

    ssize_t output_disable_count = 0;
    std::string output;

    stack local_stack;

    struct loop_info {
        datum count;
        datum size;
        datum first;
        datum last;

        loop_info(ssize_t count, ssize_t size) :
            count(), size(), first(), last()
        {
            if (count >= 0) {
                this->count = count;
                this->first = count == 0;
                if (size >= 0) {
                    this->size = size;
                    this->last = count == (size - 1);
                }
            }
        }
    };
    std::vector<loop_info> loop_stack;
    scope globals;

    expression_evaluation_context() {};

    /** Write data to the output.
     */
    void write(std::string_view text) noexcept {
        if (output_disable_count == 0) {
            output += text;
        }
    }

    /** Get the size of the output.
     * Used if you need to reset the output to a previous position.
     */
    ssize_t output_size() const noexcept {
        return ssize(output);
    }

    /** Set the size of the output.
     * Used if you need to reset the output to a previous position.
     */
    void set_output_size(ssize_t new_size) noexcept {
        tt_assert(new_size > 0);
        tt_assert(new_size <= output_size());
        output.resize(new_size);
    }

    void enable_output() noexcept {
        tt_assert(output_disable_count > 0);
        output_disable_count--;
    }

    void disable_output() noexcept {
        output_disable_count++;
    }

    void loop_push(ssize_t count = -1, ssize_t size = -1) noexcept {
        loop_stack.emplace_back(count, size);
    }

    void loop_pop() noexcept {
        tt_assert(ssize(loop_stack) > 0);
        loop_stack.pop_back();
    }

    void push() {
        local_stack.emplace_back();
        loop_push();
    }

    void pop() {
        tt_assert(local_stack.size() > 0);
        local_stack.pop_back();
        loop_pop();
    }
    
    [[nodiscard]] bool has_locals() const noexcept {
        return local_stack.size() > 0;
    }

    tt_force_inline scope const& locals() const {
        tt_assume(has_locals());
        return local_stack.back();
    }

    tt_force_inline scope& locals() {
        tt_assume(has_locals());
        return local_stack.back();
    }

    [[nodiscard]] datum const &loop_get(std::string_view name) const {
        tt_assume(name.size() > 0);
        if (name.back() == '$') {
            TTAURI_THROW(invalid_operation_error("Invalid loop variable '{}'", name));
        }

        std::string_view short_name = name.substr(1);
        auto i = loop_stack.crbegin();

        while (short_name[0] == '$') {
            if (i == loop_stack.crend() || i->count.is_undefined()) {
                TTAURI_THROW(invalid_operation_error("Accessing loop variable {} while not in loop", name));
            }

            short_name = short_name.substr(1);
            i++;
        }

        if (short_name == "i" || short_name == "count") {
            return i->count;
        } else if (short_name == "first") {
            return i->first;
        } else if (short_name == "size" || short_name == "length") {
            if (i->size.is_undefined()) {
                TTAURI_THROW(invalid_operation_error("Accessing loop variable {} only available in #for loops", name));
            }
            return i->size;
        } else if (short_name == "last") {
            if (i->last.is_undefined()) {
                TTAURI_THROW(invalid_operation_error("Accessing loop variable {} only available in #for loops", name));
            }
            return i->last;
        } else {
            TTAURI_THROW(invalid_operation_error("Unknown loop variable {}", name));
        }
    }

    [[nodiscard]] datum const& get(std::string const &name) const {
        tt_assert(name.size() > 0);

        if (name[0] == '$') {
            return loop_get(name);
        }

        if (has_locals()) {
            ttlet i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        ttlet j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        TTAURI_THROW(invalid_operation_error("Could not find {} in local or global scope.", name));
    }

    [[nodiscard]] datum &get(std::string const &name) {
        tt_assert(name.size() > 0);

        if (has_locals()) {
            ttlet i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        ttlet j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        TTAURI_THROW(invalid_operation_error("Could not find {} in local or global scope.", name));
    }

    template<typename T>
    void set_local(std::string const &name, T &&value) {
        locals()[name] = std::forward<T>(value);
    }

    template<typename T>
    void set_global(std::string const& name, T &&value) {
        globals[name] = std::forward<T>(value);
    }

    datum &set(std::string const &name, datum const &value) {
        if (has_locals()) {
            return locals()[name] = value;
        } else {
            return globals[name] = value;
        }
    }
};

struct expression_post_process_context {
    using filter_type = std::function<std::string(std::string_view)>;
    using filter_table = std::unordered_map<std::string,filter_type>;
    using function_type = std::function<datum(expression_evaluation_context&, datum::vector const &)>;
    using function_table = std::unordered_map<std::string,function_type>;
    using function_stack = std::vector<function_type>;
    using method_type = std::function<datum(expression_evaluation_context&, datum &, datum::vector const &)>;
    using method_table = std::unordered_map<std::string,method_type>;

    function_table functions;
    function_stack super_stack;
    static function_table global_functions;
    static method_table global_methods;
    static filter_table global_filters;

    [[nodiscard]] function_type get_function(std::string const &name) const noexcept {
        if (name == "super") {
            if (super_stack.size() > 0) {
                return super_stack.back();
            } else {
                return {};
            }
        }

        ttlet i = functions.find(name);
        if (i != functions.end()) {
            return i->second;
        }

        ttlet j = global_functions.find(name);
        if (j != global_functions.end()) {
            return j->second;
        }

        return {};
    }

    [[nodiscard]] function_type set_function(std::string const &name, function_type func) noexcept {
        using namespace std;

        swap(functions[name], func);
        return func;
    }

    void push_super(function_type func) noexcept {
        super_stack.push_back(func);
    }

    void pop_super(void) noexcept {
        super_stack.pop_back();
    }

    [[nodiscard]] filter_type get_filter(std::string const &name) const noexcept {
        ttlet i = global_filters.find(name);
        if (i != global_filters.end()) {
            return i->second;
        }

        return {};
    }

    [[nodiscard]] method_type get_method(std::string const &name) const noexcept {
        ttlet i = global_methods.find(name);
        if (i != global_methods.end()) {
            return i->second;
        }

        return {};
    }
};

struct expression_parse_context {
    using const_iterator = typename std::vector<token_t>::const_iterator;

    std::string_view::const_iterator first;
    std::string_view::const_iterator last;

    std::vector<token_t> tokens;
    const_iterator token_it;

    expression_parse_context(std::string_view::const_iterator first, std::string_view::const_iterator last) :
        first(first), last(last), tokens(parseTokens(first, last)), token_it(tokens.begin()) {}

    [[nodiscard]] token_t const& operator*() const noexcept {
        return *token_it;
    }

    [[nodiscard]] token_t const *operator->() const noexcept {
        return &(*token_it);
    }

    expression_parse_context& operator++() noexcept {
        tt_assume(token_it != tokens.end());
        tt_assume(*token_it != tokenizer_name_t::End);
        ++token_it;
        return *this;
    }

    expression_parse_context operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
};

struct expression_node {
    using expression_vector = std::vector<std::unique_ptr<expression_node>>;

    parse_location location;

    expression_node(parse_location location) : location(location) {}

    virtual ~expression_node() {}

    /** Resolve function and method pointers.
     * At all call-expressions resolve the function pointers from the parse_context.
     */
    virtual void post_process(expression_post_process_context& context) {}

    /** Resolve function and method pointers.
    * This is called on a name-expression or member-expression to set the function pointer.
    */
    virtual void resolve_function_pointer(expression_post_process_context& context) {}

    /** Evaluate an rvalue.
     */
    virtual datum evaluate(expression_evaluation_context& context) const = 0;

    datum evaluate_without_output(expression_evaluation_context& context) const {
        context.disable_output();
        auto r = evaluate(context);
        context.enable_output();
        return r;
    }

    /** Evaluate an existing lvalue.
     */
    virtual datum &evaluate_lvalue(expression_evaluation_context& context) const {
        TTAURI_THROW(invalid_operation_error("Expression is not a modifiable value.").set_location(location));
    }

    virtual bool has_evaluate_xvalue() const {
        return false;
    }
    
    /** Evaluate an existing xvalue.
    */
    virtual datum const &evaluate_xvalue(expression_evaluation_context const& context) const {
        TTAURI_THROW(invalid_operation_error("Expression is not a xvalue.").set_location(location));
    }

    /** Assign to a non-existing or existing lvalue.
     */
    virtual datum &assign(expression_evaluation_context& context, datum const &rhs) const {
        return evaluate_lvalue(context) = rhs;
    }

    datum &assign_without_output(expression_evaluation_context& context, datum const &rhs) const {
        context.disable_output();
        auto &r = assign(context, rhs);
        context.enable_output();
        return r;
    }

    /** Call a function with a datum::vector as arguments.
     */
    virtual datum call(expression_evaluation_context& context, datum::vector const &arguments) const {
        TTAURI_THROW(invalid_operation_error("Expression is not callable.").set_location(location));
    }

    /** Get the name of a expression_name_node.
    */
    virtual std::string get_name() const {
        TTAURI_THROW(parse_error("Expect a name got {})", *this).set_location(location));
    }

    /** Get name and argument names from a function declaration.
     * This is only implemented on the expression_call_node.
     */
    virtual std::vector<std::string> get_name_and_argument_names() const {
        TTAURI_THROW(parse_error("Expect a function definition got {})", *this).set_location(location));
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
    auto parse_context = expression_parse_context(first, last);
    auto e = parse_expression(parse_context);

    auto post_process_context = expression_post_process_context();
    e->post_process(post_process_context);
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