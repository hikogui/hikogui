// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTInteger : ASTExpression {
    int64_t value;

    ASTInteger(ASTLocation location, int64_t value) : ASTExpression(location), value(value) {}

    std::string str() const override {
        return (boost::format("%i") % value).str();
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}
