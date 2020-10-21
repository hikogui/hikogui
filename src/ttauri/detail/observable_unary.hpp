// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_base.hpp"

namespace tt::detail {

template<typename T, typename OT>
class observable_unary : public observable_base<T> {
public:
    using operand_type = observable_base<OT>;

    observable_unary(std::shared_ptr<operand_type> const &operand) noexcept :
        observable_base<T>(),
        _operand(operand),
        _operand_cache(operand->load())
    {
        _operand_callback = scoped_callback(*_operand, [this](OT const &value) {
            ttlet old_value = this->load();
            {
                ttlet lock = std::scoped_lock(observable_base<T>::_mutex);
                _operand_cache = value;
            }
            ttlet new_value = this->load();
            observable_base<T>::notify(old_value, new_value);
        });
    }

protected:
    std::shared_ptr<operand_type> _operand;
    OT _operand_cache;
    scoped_callback<operand_type> _operand_callback;
};

}
