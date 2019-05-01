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

    /*! Get a modifiable value from an expression.
     */
    virtual Value &executeLValue(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << boost::errinfo_file_name(location.file->string())
            << boost::errinfo_at_line(location.line)
            << errinfo_at_column(location.column)
            << errinfo_message("syntax error, expected a lvalue expression")
        );
    }

    /*! Get a read-only value from an expression.
    */
    virtual Value execute(ExecutionContext *context) const {
        return executeLValue(context);
    }

    /*! Execute a function or method call.
     */
    virtual Value executeCall(ExecutionContext *context, std::vector<Value> const &arguments) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << boost::errinfo_file_name(location.file->string())
            << boost::errinfo_at_line(location.line)
            << errinfo_at_column(location.column)
            << errinfo_message("result of expression does not support being used as a function")
        );
    }

    /*! Execute an assignment of a value to an modifiable value.
     */
    virtual Value &executeAssignment(ExecutionContext *context, Value other) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << boost::errinfo_file_name(location.file->string())
            << boost::errinfo_at_line(location.line)
            << errinfo_at_column(location.column)
            << errinfo_message("result of expression does not support assignment")
        );
    }

    /*! Execute a object-statement.
     */
    virtual void executeStatement(ExecutionContext *context) const {
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << boost::errinfo_file_name(location.file->string())
            << boost::errinfo_at_line(location.line)
            << errinfo_at_column(location.column)
            << errinfo_message("syntax error, expression can not be used as a statement inside an object")
        );
    }
};

}
