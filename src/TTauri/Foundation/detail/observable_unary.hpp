// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_base.hpp"

namespace tt::detail {

template<typename T, typename OT>
class observable_unary : public observable_base<T> {
protected:
    std::shared_ptr<observable_base<OT>> operand;
    OT operand_cache;
    size_t operand_cb_id;

public:
    observable_unary(std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_base<T>(),
        operand(operand),
        operand_cache(operand->load())
    {
        operand_cb_id = this->operand->add_callback([this](OT const &value) {
            ttlet old_value = this->load();
            {
                ttlet lock = std::scoped_lock(observable_base<T>::mutex);
                operand_cache = value;
            }
            ttlet new_value = this->load();
            notify(old_value, new_value);
        });
    }

    ~observable_unary() {
        operand->remove_callback(operand_cb_id);
    }
};

}