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

/*! Abstract syntax tree node.
 */
struct ASTNode {
    //! Location of node in the source file.
    Location location;
 
    ASTNode(Location location) : location(location) {}

    virtual std::string str() const {
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("str() not implemented for %s") %
            typeid(*this).name()).str())
            << errinfo_location(location)
        );
    }

    /*! Execute the expression and return a value that can be modifed by the caller.
     */
    virtual Value &executeLValue(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("syntax error, expected a lvalue expression")
            << errinfo_location(location)
        );
    }

    /*! Execute the expression.
    */
    virtual Value execute(ExecutionContext *context) const {
        return executeLValue(context);
    }

    /*! Execute a function or method call.
     */
    virtual Value executeCall(ExecutionContext *context, std::vector<Value> const &arguments) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("result of expression does not support being used as a function")
            << errinfo_location(location)
        );
    }

    /*! Execute an assignment of a value to an modifiable value.
     */
    virtual Value &executeAssignment(ExecutionContext *context, Value other) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("result of expression does not support assignment")
            << errinfo_location(location)
        );
    }

    /*! Execute an object-statement.
     */
    virtual void executeStatement(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("syntax error, expression can not be used as a statement inside an object")
            << errinfo_location(location)
        );
    }
};

}
