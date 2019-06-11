// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTIndex : ASTExpression {
    ASTExpression *object;
    ASTExpression *index;

    ASTIndex(Location location, ASTExpression *object) : ASTExpression(location), object(object), index(nullptr) {}

    ASTIndex(Location location, ASTExpression *object, ASTExpression *index) : ASTExpression(location), object(object), index(index) {}

    ~ASTIndex() {
        delete object;
        delete index;
    }

    std::string string() const override {
        if (index) {
            return object->string() + "[" + index->string() + "]";
        } else {
            return object->string() + "[]";
        }
    }

    /*! Index an object or array.
     * An object can be indexed by a std::string.
     * An array can be indexed by a int64_t.
     * An non-index can be used to append to an array.
     */
    Value &executeLValue(ExecutionContext *context) const override {
        auto &object_ = object->executeLValue(context);

        if (index) {
            let index_ = index->execute(context);

            if ((object_.is_type<Undefined>() || object_.is_type<Object>()) && index_.is_type<std::string>()) {
                // Use a string to index into an object.
                let index__ = index_.value<std::string>();
                try {
                    return object_[index__];
                } catch (boost::exception &e) {
                    e << errinfo_location(location);
                    throw;
                }

            } else if ((object_.is_type<Undefined>() || object_.is_type<Array>()) && index_.is_type<int64_t>()) {
                // Use a integer to index into an array.
                size_t const index__ = index_.value<size_t>();
                try {
                    return object_[index__];
                } catch (boost::exception &e) {
                    e << errinfo_location(location);
                    throw;
                }

            } else {
                BOOST_THROW_EXCEPTION(InvalidOperationError(
                    (boost::format("Can not index object of type %s with index of type %s") %
                        object_.type_name() %
                        index_.type_name()
                    ).str())
                    << errinfo_location(location)
                );
            }

        } else if (object_.is_type<Undefined>() || object_.is_type<Array>()) {
            // Append to an array because no index was specified.
            try {
                return object_.append();
            } catch (boost::exception &e) {
                e << errinfo_location(location);
                throw;
            }

        } else {
            BOOST_THROW_EXCEPTION(InvalidOperationError(
                (boost::format("Can not append to object of type %s") % object_.type_name()).str())
                << errinfo_location(location)
            );
        }
    }

    Value &executeAssignment(ExecutionContext *context, Value other) const override {
        auto &lv = executeLValue(context);
        lv = std::move(other);
        return lv;
    }
};

}
