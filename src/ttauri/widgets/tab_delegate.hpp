// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace tt::inline v1 {
class tab_widget;

class tab_delegate {
public:
    using callback_type = std::function<void()>;
    using callback_ptr_type = std::shared_ptr<callback_type>;

    virtual ~tab_delegate() = default;
    virtual void init(tab_widget &sender) noexcept {}
    virtual void deinit(tab_widget &sender) noexcept {}

    virtual callback_ptr_type subscribe(tab_widget &sender, callback_ptr_type const &callback_ptr) noexcept
    {
        return callback_ptr;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<typename Callback>
    requires(std::is_invocable_v<Callback>) [[nodiscard]] callback_ptr_type
        subscribe(tab_widget &sender, Callback &&callback) noexcept
    {
        return subscribe(sender, std::make_shared<callback_type>(std::forward<Callback>(callback)));
    }

    virtual void add_tab(tab_widget &sender, std::size_t key, std::size_t index) noexcept {}

    virtual ssize_t index(tab_widget &sender) noexcept
    {
        return -1;
    }
};

} // namespace tt::inline v1
