
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

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override { 
        return std::make_shared<ValueFloat>(value);
    } 

};

}

