// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <concepts>
#include <memory>

export module hikogui_observer : observed_value;
import : observed;
import hikogui_concurrency;
import hikogui_utility;


export namespace hi::inline v1 {
template<std::equality_comparable>
class observer;

template<std::equality_comparable T>
class observed_value final : public observed {
public:
    using value_type = T;
    using path_type = observable_msg::path_type;

    ~observed_value() = default;

    /** Construct the shared state and default initialize the value.
     */
    constexpr observed_value() noexcept : _rcu()
    {
        _rcu.emplace(value_type{});
    }

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr observed_value(Args&&...args) noexcept : _rcu()
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    /// @privatesection
    [[nodiscard]] void const *read() const noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] void *copy(void const *ptr) const noexcept override
    {
        return _rcu.copy(static_cast<value_type const *>(ptr));
    }

    void commit(void *ptr) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
    }

    void abort(void *ptr) const noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
    }

    void read_lock() const noexcept override
    {
        _rcu.lock();
    }

    void read_unlock() const noexcept override
    {
        _rcu.unlock();
    }

    void write_lock() const noexcept override
    {
        _write_mutex.lock();
        read_lock();
    }

    void write_unlock() const noexcept override
    {
        read_unlock();
        _write_mutex.unlock();
    }
    /// @endprivatesection
private:
    rcu<value_type> _rcu;
    mutable unfair_mutex _write_mutex;
};

}
