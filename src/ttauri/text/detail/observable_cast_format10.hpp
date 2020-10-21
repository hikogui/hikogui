// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../../detail/observable_cast.hpp"
#include "../language.hpp"

namespace tt::detail {

template<>
class observable_cast<std::u8string,tt::format10> final : public observable_unary<std::u8string,tt::format10> {
public:
    observable_cast(std::shared_ptr<observable_base<tt::format10>> const &operand) noexcept :
        observable_unary<std::u8string,tt::format10>(operand)
    {
        _language_list_callback = scoped_callback(language::preferred_languages, [this](auto...) {
            notify({}, load());
        });
    }

    virtual std::u8string load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<std::u8string,tt::format10>::_mutex);
        return static_cast<std::u8string>(_operand_cache);
    }

    virtual bool store(std::u8string const &new_value) noexcept override {
        tt_no_default();
    }

private:
    scoped_callback<decltype(language::preferred_languages)> _language_list_callback;
};

}
