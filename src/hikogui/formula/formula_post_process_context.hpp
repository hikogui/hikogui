// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_evaluation_context.hpp"
#include "../utility/module.hpp"
#include "../datum.hpp"
#include "../strings.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

namespace hi::inline v1 {

struct formula_post_process_context {
    using filter_type = std::function<std::string(std::string_view)>;
    using filter_table = std::unordered_map<std::string, filter_type>;
    using function_type = std::function<datum(formula_evaluation_context &, datum::vector_type const &)>;
    using function_table = std::unordered_map<std::string, function_type>;
    using function_stack = std::vector<function_type>;
    using method_type = std::function<datum(formula_evaluation_context &, datum &, datum::vector_type const &)>;
    using method_table = std::unordered_map<std::string, method_type>;

    function_table functions;
    function_stack super_stack;
    static function_table global_functions;
    static method_table global_methods;
    static filter_table global_filters;

    [[nodiscard]] function_type get_function(std::string const &name) const noexcept
    {
        if (name == "super") {
            if (super_stack.size() > 0) {
                return super_stack.back();
            } else {
                return {};
            }
        }

        hilet i = functions.find(name);
        if (i != functions.end()) {
            return i->second;
        }

        hilet j = global_functions.find(name);
        if (j != global_functions.end()) {
            return j->second;
        }

        return {};
    }

    [[nodiscard]] function_type set_function(std::string const &name, function_type func) noexcept
    {
        using std::swap;

        swap(functions[name], func);
        return func;
    }

    void push_super(function_type func) noexcept
    {
        super_stack.push_back(func);
    }

    void pop_super(void) noexcept
    {
        super_stack.pop_back();
    }

    [[nodiscard]] filter_type get_filter(std::string const &name) const noexcept
    {
        hilet i = global_filters.find(name);
        if (i != global_filters.end()) {
            return i->second;
        }

        return {};
    }

    [[nodiscard]] method_type get_method(std::string const &name) const noexcept
    {
        hilet i = global_methods.find(name);
        if (i != global_methods.end()) {
            return i->second;
        }

        return {};
    }
};

} // namespace hi::inline v1
