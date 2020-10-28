// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../../detail/observable_cast.hpp"
#include "../language.hpp"

namespace tt::detail {

template<>
class observable_cast<std::u8string, tt::l10n> final : public observable_unary<std::u8string, tt::l10n> {
public:
    observable_cast(std::shared_ptr<observable_base<tt::l10n>> const &operand) noexcept :
        observable_unary<std::u8string, tt::l10n>(operand)
    {
        _language_list_callback = language::preferred_languages.subscribe([this](auto...) {
            notify({}, load());
        });
    }

    virtual std::u8string load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<std::u8string, tt::l10n>::_mutex);
        return static_cast<std::u8string>(_operand_cache);
    }

    virtual bool store(std::u8string const &new_value) noexcept override {
        tt_no_default();
    }

private:
    typename decltype(language::preferred_languages)::callback_ptr_type _language_list_callback;
};

}
