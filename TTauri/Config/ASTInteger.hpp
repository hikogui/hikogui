// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTInteger : ASTExpression {
    int64_t value;

    ASTInteger(Location location, int64_t value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        return (boost::format("%i") % value).str();
    }

    universal_value execute(ExecutionContext *context) const noexcept override { 
        return value;
    } 

};

}
