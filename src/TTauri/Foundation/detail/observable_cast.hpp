// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_unary.hpp"

namespace tt::detail {

template<typename T,typename OT>
class observable_cast final : public observable_unary<T,OT> {
public:
    observable_cast(std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_unary<T,OT>(operand) {}

    virtual T load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<T,OT>::mutex);
        return static_cast<T>(this->operand_cache);
    }

    virtual bool store(T const &new_value) noexcept override {
        return this->operand->store(static_cast<OT>(new_value));
    }
};

}
