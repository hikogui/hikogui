// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTFloat : ASTExpression {
    double value;

    ASTFloat(Location location, double value) : ASTExpression(location), value(value) {}

    std::string str() const override {
        auto s = (boost::format("%g") % value).str();
        if (s.find('.') == s.npos) {
            return s + ".";
        } else {
            return s;
        }
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}

