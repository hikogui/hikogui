// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "terminate.hpp"
#include "../console/module.hpp"
#include "../utility/utility.hpp"
#include "../telemetry/module.hpp"
#include "../macros.hpp"



namespace hi { inline namespace v1 {

[[noreturn]] void terminate_handler() noexcept
{
    using namespace std::literals;

    log_global.flush();

    auto title = std::string_view{};
    auto message = std::string_view{};

    hilet ep = std::current_exception();
    if (ep) {
        try {
            std::rethrow_exception(ep);

        } catch (std::exception const& e) {
            title = "Unhandled std::exception."sv;
            message = e.what();

        } catch (...) {
            title = "Unhandled unknown exception."sv;
            message = "<no data>"sv;
        }

        std::print(std::cerr, "{}\n{}", title, message);

    } else {
        title = "Abnormal termination."sv;
        message = debug_message.exchange(nullptr, std::memory_order::relaxed);
    }

    dialog_ok(title, message);

    old_terminate_handler();
}

}} // namespace hi::v1
