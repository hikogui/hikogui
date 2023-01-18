// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "terminate.hpp"
#include "utility/module.hpp"
#include "console.hpp"
#include "dialog.hpp"
#include "log.hpp"

namespace hi { inline namespace v1 {

[[noreturn]] void terminate_handler() noexcept
{
    log_global.flush();

    auto title = std::string{};
    auto message = std::string{};

    hilet ep = std::current_exception();
    if (ep) {
        try {
            std::rethrow_exception(ep);

        } catch (std::exception const& e) {
            title = "Unhandled std::exception";
            message += e.what();

        } catch (...) {
            title = "Unhandled unknown exception";
        }

    } else {
        title = "Abnormal termination";
    }

    if (auto terminate_message_ = terminate_message.exchange(nullptr)) {
        message += *terminate_message_;
    }

    console_output(title + "\n", std::cerr);

    if (not message.empty()) {
        console_output(message, std::cerr);
    } else {
        message = "Unknown error.";
    }

    dialog_ok(title, message);

    old_terminate_handler();
}

}} // namespace hi::v1
