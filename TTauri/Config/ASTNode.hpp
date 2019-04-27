
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
    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) = 0;
    virtual std::shared_ptr<ValueBase> executeAssignment(ExecutionContext *context, const std::shared_ptr<ValueBase> &other) {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
    void executeStatement(ExecutionContext *context) {
        BOOST_THROW_EXCEPTION(InvalidOperation());
    }
};

}
