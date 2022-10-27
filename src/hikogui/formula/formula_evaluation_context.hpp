// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../datum.hpp"
#include "../exception.hpp"
#include <unordered_map>
#include <vector>
#include <string_view>

namespace hi::inline v1 {

struct formula_evaluation_context {
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

        loop_info(ssize_t count, ssize_t size) : count(), size(), first(), last()
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

    formula_evaluation_context() noexcept = default;

    /** Write data to the output.
     */
    void write(std::string_view text) noexcept
    {
        if (output_disable_count == 0) {
            output += text;
        }
    }

    /** Get the size of the output.
     * Used if you need to reset the output to a previous position.
     */
    ssize_t output_size() const noexcept
    {
        return ssize(output);
    }

    /** Set the size of the output.
     * Used if you need to reset the output to a previous position.
     */
    void set_output_size(ssize_t new_size) noexcept
    {
        hi_assert(new_size > 0);
        hi_assert(new_size <= output_size());
        output.resize(new_size);
    }

    void enable_output() noexcept
    {
        hi_assert(output_disable_count > 0);
        output_disable_count--;
    }

    void disable_output() noexcept
    {
        output_disable_count++;
    }

    void loop_push(ssize_t count = -1, ssize_t size = -1) noexcept
    {
        loop_stack.emplace_back(count, size);
    }

    void loop_pop() noexcept
    {
        hi_assert(ssize(loop_stack) > 0);
        loop_stack.pop_back();
    }

    void push()
    {
        local_stack.emplace_back();
        loop_push();
    }

    void pop()
    {
        hi_assert(local_stack.size() > 0);
        local_stack.pop_back();
        loop_pop();
    }

    [[nodiscard]] bool has_locals() const noexcept
    {
        return local_stack.size() > 0;
    }

    scope const &locals() const
    {
        hi_assert(has_locals());
        return local_stack.back();
    }

    scope &locals()
    {
        hi_assert(has_locals());
        return local_stack.back();
    }

    [[nodiscard]] datum const &loop_get(std::string_view name) const
    {
        hi_assert(name.size() > 0);
        if (name.back() == '$') {
            throw operation_error(std::format("Invalid loop variable '{}'", name));
        }

        std::string_view short_name = name.substr(1);
        auto i = loop_stack.crbegin();

        while (short_name[0] == '$') {
            if (i == loop_stack.crend() || holds_alternative<std::monostate>(i->count)) {
                throw operation_error(std::format("Accessing loop variable {} while not in loop", name));
            }

            short_name = short_name.substr(1);
            i++;
        }

        if (short_name == "i" || short_name == "count") {
            return i->count;
        } else if (short_name == "first") {
            return i->first;
        } else if (short_name == "size" || short_name == "length") {
            if (holds_alternative<std::monostate>(i->size)) {
                throw operation_error(std::format("Accessing loop variable {} only available in #for loops", name));
            }
            return i->size;
        } else if (short_name == "last") {
            if (holds_alternative<std::monostate>(i->last)) {
                throw operation_error(std::format("Accessing loop variable {} only available in #for loops", name));
            }
            return i->last;
        } else {
            throw operation_error(std::format("Unknown loop variable {}", name));
        }
    }

    [[nodiscard]] datum const &get(std::string const &name) const
    {
        hi_assert(name.size() > 0);

        if (name[0] == '$') {
            return loop_get(name);
        }

        if (has_locals()) {
            hilet i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        hilet j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        throw operation_error(std::format("Could not find {} in local or global scope.", name));
    }

    [[nodiscard]] datum &get(std::string const &name)
    {
        hi_assert(name.size() > 0);

        if (has_locals()) {
            hilet i = locals().find(name);
            if (i != locals().end()) {
                return i->second;
            }
        }

        hilet j = globals.find(name);
        if (j != globals.end()) {
            return j->second;
        }

        throw operation_error(std::format("Could not find {} in local or global scope.", name));
    }

    template<typename T>
    void set_local(std::string const &name, T &&value)
    {
        locals()[name] = std::forward<T>(value);
    }

    template<typename T>
    void set_global(std::string const &name, T &&value)
    {
        globals[name] = std::forward<T>(value);
    }

    datum &set(std::string const &name, datum const &value)
    {
        if (has_locals()) {
            return locals()[name] = value;
        } else {
            return globals[name] = value;
        }
    }
};

} // namespace hi::inline v1