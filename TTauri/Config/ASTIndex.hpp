// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTIndex : ASTExpression {
    ASTExpression *object;
    ASTExpression *index;

    ASTIndex(Location location, ASTExpression *object) noexcept : ASTExpression(location), object(object), index(nullptr) {}

    ASTIndex(Location location, ASTExpression *object, ASTExpression *index) noexcept : ASTExpression(location), object(object), index(index) {}

    ~ASTIndex() {
        delete object;
        delete index;
    }

    std::string string() const noexcept override {
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
    universal_value &executeLValue(ExecutionContext &context) const override {
        auto &object_ = object->executeLValue(context);

        if (index) {
            let index_ = index->execute(context);

            if ((holds_alternative<Undefined>(object_) || holds_alternative<Object>(object_)) && holds_alternative<std::string>(index_)) {
                // Use a string to index into an object.
                let index__ = get<std::string>(index_);
                try {
                    return object_[index__];
                } catch (error &e) {
                    e << error_info<"location"_tag>(location);
                    throw;
                }

            } else if ((holds_alternative<Undefined>(object_) || holds_alternative<Array>(object_)) && holds_alternative<int64_t>(index_)) {
                // Use a integer to index into an array.
                int64_t const index__ = get<int64_t>(index_);
                try {
                    return object_[index__];
                } catch (error &e) {
                    e << error_info<"location"_tag>(location);
                    throw;
                }

            } else {
                TTAURI_THROW(invalid_operation_error(
                    (boost::format("Can not index object of type %s with index of type %s") %
                        object_.type_name() %
                        index_.type_name()
                    ).str())
                    << error_info<"location"_tag>(location)
                );
            }

        } else if (holds_alternative<Undefined>(object_) || holds_alternative<Array>(object_)) {
            // Append to an array because no index was specified.
            try {
                return object_.append();
            } catch (error &e) {
                e << error_info<"location"_tag>(location);
                throw;
            }

        } else {
            TTAURI_THROW(invalid_operation_error(
                (boost::format("Can not append to object of type %s") % object_.type_name()).str())
                << error_info<"location"_tag>(location)
            );
        }
    }

    universal_value &executeAssignment(ExecutionContext &context, universal_value other) const override {
        auto &lv = executeLValue(context);
        lv = std::move(other);
        return lv;
    }
};

}
