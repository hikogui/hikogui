
#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTFloat : ASTExpression {
    double value;

    ASTFloat(ASTLocation location, double value) : ASTExpression(location), value(value) {}

    std::string str() override {
        return (boost::format("%g") % value).str();
    }

    Value execute(ExecutionContext *context) override { 
        return value;
    } 

};

}

