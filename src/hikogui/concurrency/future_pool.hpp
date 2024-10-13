

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.concurrency : future_pool);

hi_export namespace hi::inline v1 {

template<typename T>
class future_pool {
public:
    using future_type = std::future<T>;

    constexpr future_pool() = default;

    void add(future_type&& cb)
    {
        cleanup();
        _list.push_back(std::move(cb));
    }

private:
    std::vector<future_type> _list;

    void cleanup()
    {
        std::erase_if(_list, [](auto const &f) {
            if (not f.valid()) {
                return true;
            }
            return f.wait_for(0ms) != std::future_status::timeout;
        });
    }
};

}

