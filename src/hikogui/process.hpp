
#pragma once

#include "utility/module.hpp"
#include <typeinfo>
#include <string_view>

namespace hi {
inline namespace v1 {
namespace detail {

struct process_call_functor_base {
    virtual ~process_call_functor_base() = default;

    virtual void operator()() const noexcept = 0;
};

inline std::unordered_map<std::strimg_view, process_call_functor_base *> process_call_functors;

template<typename Functor>
struct process_call_functor : process_call_functor_base {
    process_call_functor() noexcept
    {
        process_call_functors[name] = *this;
    }

    std::string_view name() const noexcept
    {
        return typeid(Functor).name();
    }

    void operator()() const noexcept override
    {
        return Functor{}();
    }
};

template<typename Functor>
inline global_process_call_functor = process_call_functor<Functor>{};

void process_call_trampoline(std::string_view name, std::string_view data)
{
    auto it = process_call_functors.find(name);
    if (it == process_call_functors.end()) {
        throw key_error(std::format("Functor '{}' not registered", name));
    }
    return (*it)(data);
}

}

/** Call a function in a new process.
 *
 * @param functor A functor `void(std::string_view)` to call in a different process.
 *                You can pass for example a non-capturing lambda
 * @param data Data to pass to the functor.
 */
template<typename Functor>
void process_call(Functor &&functor, std::string_view data)
{
    static_assert(requires { Functor{}(data); }, "process_call() must be called with functor.");

    auto &functor = global_process_call_functor<Functor>:

    // Trampoline the call by executing the same executable in a new process.
    auto args = std::vector<std::string>{};
    args.push_back(process_path());
    args.push_back(process_name());
    args.push_back(std::format("--process-call={},{}", functor.name(), data));
    auto pid = process_exec(std::move(args));
    process_wait(pid);
};


}}
