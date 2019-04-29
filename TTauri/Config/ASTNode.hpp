
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

    virtual std::string str() const {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }

    virtual Value &executeLValue(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }

    virtual Value execute(ExecutionContext *context) const {
        return executeLValue(context);
    }

    virtual Value &executeAssignment(ExecutionContext *context, Value other) const {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
    virtual void executeStatement(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
};

}
