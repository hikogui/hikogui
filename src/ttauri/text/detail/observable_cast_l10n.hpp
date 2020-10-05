// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../../detail/observable_cast.hpp"
#include "../language.hpp"

namespace tt::detail {

template<>
class observable_cast<std::u8string, tt::l10n> final : public observable_unary<std::u8string, tt::l10n> {
private:
    size_t language_list_cbid;
    mutable int count = 0;
public:
    observable_cast(std::shared_ptr<observable_base<tt::l10n>> const &operand) noexcept :
        observable_unary<std::u8string, tt::l10n>(operand)
    {
        language_list_cbid = language::preferred_languages.add_callback([this](auto...) {
            ++count;
            notify({}, load());
        });
    }

    ~observable_cast() {
        language::preferred_languages.remove_callback(language_list_cbid);
    }

    virtual std::u8string load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<std::u8string, tt::l10n>::mutex);
        return static_cast<std::u8string>(operand_cache);
    }

    virtual bool store(std::u8string const &new_value) noexcept override {
        tt_no_default();
    }
};

}
