
#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>

namespace TTauri::Config {

struct ASTInteger : ASTExpression {
    int64_t value;

    ASTInteger(ASTLocation location, int64_t value) : ASTExpression(location), value(value) {}

    std::string str() override {
        return (boost::format("%i") % value).str();
    }
};

}
