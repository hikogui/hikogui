
#pragma once

#include "ASTLocation.hpp"
#include "ExecutionContext.hpp"
#include "Value.hpp"
#include "TTauri/utils.hpp"

#include <string>

namespace TTauri::Config {

struct ASTNode {
    struct InvalidOperation : virtual boost::exception, virtual std::exception {};

    ASTLocation location;
    ASTNode(ASTLocation location) : location(location) {}

    virtual std::string str() = 0;

    virtual Value execute(ExecutionContext *context) = 0;

    virtual Value executeAssignment(ExecutionContext *context, Value other) {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
    virtual void executeStatement(ExecutionContext *context) {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
};

}
