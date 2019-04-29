
#pragma once

#include "Value.hpp"

namespace TTauri::Config {

struct ExecutionContext {
    std::vector<Value> objectStack;

    void pushObject() {
        objectStack.emplace_back(Object{});
    }

    Value popObject() {
        return pop_back(objectStack);
    }

    Value &currentObject() {
        return objectStack.back();
    }
};

}