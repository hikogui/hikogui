// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;


export module hikogui_concurrency;
export import hikogui_concurrency_atomic;
export import hikogui_concurrency_callback;
export import hikogui_concurrency_callback_flags;
export import hikogui_concurrency_global_state;
export import hikogui_concurrency_id_factory;
export import hikogui_concurrency_rcu;
export import hikogui_concurrency_subsystem;
export import hikogui_concurrency_thread;
export import hikogui_concurrency_unfair_mutex;
export import hikogui_concurrency_unfair_recursive_mutex;
export import hikogui_concurrency_wfree_idle_count;

export namespace hi {
inline namespace v1 {

/** @defgroup concurrency Concurrency.

Types and functions for handling multiple threads of execution.
*/

}}
