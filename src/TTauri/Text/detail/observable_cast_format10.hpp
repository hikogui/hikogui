// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/detail/observable_cast.hpp"
#include "TTauri/Foundation/language.hpp"

namespace tt::detail {

template<>
class observable_cast<std::string,tt::format10> final : public observable_unary<std::string,tt::format10> {
private:
    size_t language_list_cbid;
    mutable int count = 0;
public:
    observable_cast(std::shared_ptr<observable_base<tt::format10>> const &operand) noexcept :
        observable_unary<std::string,tt::format10>(operand)
    {
        language_list_cbid = language_list.add_callback([this](auto...) {
            ++count;
            notify({}, load());
        });
    }

    ~observable_cast() {
          language_list.remove_callback(language_list_cbid);
    }

    virtual std::string load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<std::string,tt::format10>::mutex);
        return fmt::format("boogly {}", count);
    }

    virtual bool store(std::string const &new_value) noexcept override {
        tt_no_default;
    }
};

}
