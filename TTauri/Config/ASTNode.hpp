// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Location.hpp"
#include "ExecutionContext.hpp"
#include "Value.hpp"
#include "exceptions.hpp"
#include "TTauri/utils.hpp"
#include <string>

namespace TTauri::Config {

struct ASTNode {

    Location location;
    ASTNode(Location location) : location(location) {}

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
