
#include "utility.hpp"
#include "exception.hpp"
#include "console.hpp"
#include "dialog.hpp"

namespace hi {
inline namespace v1 {

[[noreturn]] void terminate_handler() noexcept
{
    auto title = std::string{};
    auto message = std::string{};

    hilet ep = std::current_exception();
    if (ep) {
        try {
            std::rethrow_exception(ep);

        } catch (std::exception const &e) {
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

}}

