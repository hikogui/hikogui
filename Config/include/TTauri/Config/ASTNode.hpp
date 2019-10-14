// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/Location.hpp"
#include "TTauri/Config/ExecutionContext.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/exceptions.hpp"
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
    virtual datum &executeLValue(ExecutionContext &context) const {
        TTAURI_THROW(invalid_operation_error("syntax error, expected a lvalue expression")
            .set<"location"_tag>(location)
        );
    }

    /*! Execute the expression.
    */
    virtual datum execute(ExecutionContext &context) const {
        return executeLValue(context);
    }

    /*! Execute a function or method call.
     */
    virtual datum executeCall(ExecutionContext &context, std::vector<datum> const &arguments) const {
        TTAURI_THROW(invalid_operation_error("result of expression does not support being used as a function")
            .set<"location"_tag>(location)
        );
    }

    /*! Execute an assignment of a value to an modifiable value.
     */
    virtual datum &executeAssignment(ExecutionContext &context, datum other) const {
        TTAURI_THROW(invalid_operation_error("result of expression does not support assignment")
            .set<"location"_tag>(location)
        );
    }

    /*! Execute an object-statement.
     */
    virtual void executeStatement(ExecutionContext &context) const {
        TTAURI_THROW(invalid_operation_error("syntax error, expression can not be used as a statement inside an object")
            .set<"location"_tag>(location)
        );
    }
};

}
