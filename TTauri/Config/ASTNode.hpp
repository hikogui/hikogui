// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Location.hpp"
#include "ExecutionContext.hpp"
#include "TTauri/universal_value.hpp"
#include "exceptions.hpp"
#include <boost/format.hpp>
#include <string>

namespace TTauri::Config {

/*! Abstract syntax tree node.
 */
struct ASTNode {
    //! Location of node in the source file.
    Location location;
 
    ASTNode(Location location) : location(location) {}

    virtual std::string string() const {
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("string() not implemented for %s") %
            typeid(*this).name()).str())
            << errinfo_location(location)
        );
    }

    /*! Execute the expression and return a value that can be modifed by the caller.
     */
    virtual universal_value &executeLValue(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("syntax error, expected a lvalue expression")
            << errinfo_location(location)
        );
    }

    /*! Execute the expression.
    */
    virtual universal_value execute(ExecutionContext *context) const {
        return executeLValue(context);
    }

    /*! Execute a function or method call.
     */
    virtual universal_value executeCall(ExecutionContext *context, std::vector<universal_value> const &arguments) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError("result of expression does not support being used as a function")
            << errinfo_location(location)
        );
    }

    /*! Execute an assignment of a value to an modifiable value.
     */
    virtual universal_value &executeAssignment(ExecutionContext *context, universal_value other) const {
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
