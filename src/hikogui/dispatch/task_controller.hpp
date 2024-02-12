// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "task.hpp"
#include "async_task.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.dispatch.async_controller);

hi_export namespace hi { inline namespace v1 {


class task_controller {
public:
    using result_type = Result;
    using progress_type = float;

    constexpr task_controller() noexcept = default;

    [[nodiscard]] bool has_task() const noexcept;

    [[nodiscard]] bool running() const noexcept;

    [[nodiscard]] progress_type progress() const noexcept;

    [[nodiscard]] result_type value() const;

    template<typename F, typename... Args>
    void set_task(F &&f, Args &&...args)
    {



    }

    void run();

    void cancel();

private:
};


}}