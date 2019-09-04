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
    datum &executeLValue(ExecutionContext &context) const override {
        auto &object_ = object->executeLValue(context);

        try {
            if (index) {
                let index_ = index->execute(context);
                return object_[index_];

            } else {
                return object_.append();
            }
        } catch (error &e) {
            e.set<"location"_tag>(location);
            throw;
        }
    }

    datum &executeAssignment(ExecutionContext &context, datum other) const override {
        auto &lv = executeLValue(context);
        lv = std::move(other);
        return lv;
    }
};

}
