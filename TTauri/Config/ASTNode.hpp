// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Location.hpp"
#include "ExecutionContext.hpp"
#include "TTauri/universal_value.hpp"
#include "TTauri/exceptions.hpp"
#include <boost/format.hpp>
#include <string>

namespace TTauri::Config {

/*! Abstract syntax tree node.
 */
struct ASTNode {
    //! Location of node in the source file.
    Location location;
 
    ASTNode(Location location) noexcept : location(location) {}

    ASTNode() = delete;
    virtual ~ASTNode() {}
    ASTNode(ASTNode const &node) = delete;
    ASTNode(ASTNode &&node) = delete;
    ASTNode &operator=(ASTNode const &node) = delete;
    ASTNode &operator=(ASTNode &&node) = delete;

    virtual std::string string() const noexcept = 0;

    /*! Execute the expression and return a value that can be modifed by the caller.
     */
    virtual universal_value &executeLValue(ExecutionContext &context) const {
        TTAURI_THROW(invalid_operation_error("syntax error, expected a lvalue expression")
            << error_info("location", location)
        );
    }

    /*! Execute the expression.
    */
    virtual universal_value execute(ExecutionContext &context) const {
        return executeLValue(context);
    }

    /*! Execute a function or method call.
     */
    virtual universal_value executeCall(ExecutionContext &context, std::vector<universal_value> const &arguments) const {
        TTAURI_THROW(invalid_operation_error("result of expression does not support being used as a function")
            << error_info("location", location)
        );
    }

    /*! Execute an assignment of a value to an modifiable value.
     */
    virtual universal_value &executeAssignment(ExecutionContext &context, universal_value other) const {
        TTAURI_THROW(invalid_operation_error("result of expression does not support assignment")
            << error_info("location", location)
        );
    }

    /*! Execute an object-statement.
     */
    virtual void executeStatement(ExecutionContext &context) const {
        TTAURI_THROW(invalid_operation_error("syntax error, expression can not be used as a statement inside an object")
            << error_info("location", location)
        );
    }
};

}
