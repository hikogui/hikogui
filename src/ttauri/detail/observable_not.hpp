// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "observable_unary.hpp"

namespace tt::detail {

template<typename OT>
class observable_not final : public observable_unary<bool,OT> {
public:
    observable_not(std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_unary<bool,OT>(operand) {}

    virtual bool load() const noexcept override {
        return not this->_operand->load();
    }

    virtual bool store(bool const &new_value) noexcept override {
        return this->_operand->store(static_cast<OT>(not new_value));
    }
};

}
