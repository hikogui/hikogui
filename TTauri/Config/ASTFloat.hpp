// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTFloat : ASTExpression {
    double value;

    ASTFloat(Location location, double value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        auto s = (boost::format("%g") % value).str();
        if (s.find('.') == s.npos) {
            return s + ".";
        } else {
            return s;
        }
    }

    universal_value execute(ExecutionContext &context) const noexcept override { 
        return value;
    } 

};

}

