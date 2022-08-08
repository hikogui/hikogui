

#pragma once

#include "../rcu.hpp"
#include "../notifier.hpp"
#include <string_view>
#include <map>

namespace hi::inline v1 {

class shared_state_base {
public:
    using notifier_type = notifier<>;
    using token_type = notifier_type::token_type;
    using function_type = notifier_type::callback_type;

    constexpr ~shared_state_base() = default;
    shared_state_base(shared_state_base const&) = delete;
    shared_state_base(shared_state_base&&) = delete;
    shared_state_base& operator=(shared_state_base const&) = delete;
    shared_state_base& operator=(shared_state_base&&) = delete;
    constexpr shared_state_base() noexcept = default;

private:
    std::map<std::string, notifier_type> _notifiers;

    [[nodiscard]] virtual void *read() noexcept = 0;
    [[nodiscard]] virtual void *copy() noexcept = 0;
    virtual void commit(void *ptr, std::string const &path) noexcept = 0;
    virtual void abort(void *ptr) noexcept = 0;
    virtual void lock() noexcept = 0;
    virtual void unlock() noexcept = 0;

    [[nodiscard]] token_type subscribe(std::string const& path, callback_flags flags, function_type function) noexcept
    {
        auto [it, inserted] = _notifiers.emplace(path, {});
        return it->second.subscribe(flags, std::move(function));
    }

    void notify(std::string const &path) noexcept
    {
        for (auto it = _notifiers.lower_bound(path); it != _notifiers.end() and it->first.starts_with(path); ++it) {
            it->second();
        }
    }

    template<typename O>
    friend class shared_state_cursor;
};

template<typename T>
class shared_state : public shared_state_base {
public:
    using value_type = T;

    ~shared_state() = default;
    constexpr shared_state() noexcept = default;

private:
    rcu<value_type> _rcu;

    [[nodiscard]] const *read() noexcept override
    {
        return _rcu.read();
    }

    [[nodiscard]] void *copy() noexcept override
    {
        return _rcu.copy();
    }

    void commit(void *ptr, std::string const *path) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
        notify(path);
    }

    void abort(void *ptr) noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
    }

    void lock() noexcept override
    {
        _rcu.lock();
    }

    void unlock() noexcept override
    {
        _rcu.unlock();
    }
};

} // namespace hi::inline v1
