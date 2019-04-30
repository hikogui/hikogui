
#pragma once

#include "Location.hpp"
#include "ExecutionContext.hpp"
#include "Value.hpp"
#include "TTauri/utils.hpp"

#include <string>

namespace TTauri::Config {

struct ASTNode {
    struct InvalidOperationError : virtual boost::exception, virtual std::exception {};

    ASTLocation location;
    ASTNode(ASTLocation location) : location(location) {}

    virtual std::string str() const {
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    virtual Value &executeLValue(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    virtual Value execute(ExecutionContext *context) const {
        return executeLValue(context);
    }

    virtual Value &executeAssignment(ExecutionContext *context, Value other) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }
    virtual void executeStatement(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }
};

}
