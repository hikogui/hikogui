// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Value.hpp"
#include <optional>

namespace TTauri::Config {

struct ObjectStackItem {
    Value object = Object{};
    Value *section = nullptr;
};

struct ExecutionContext {
    Value _variableObject = Object{};
    std::vector<ObjectStackItem> objectStack;

    void pushObject() {
        objectStack.emplace_back();
    }

    void setSection(Value *section) {
        auto &item = objectStack.back();
        item.section = section;
    }

    Value popObject() {
        return pop_back(objectStack).object;
    }

    Value &currentObject() {
        auto &item = objectStack.back();
        
        if (item.section) {
            return *(item.section);
        } else {
            return item.object;
        }
    }

    Value &rootObject() {
        auto &item = objectStack.front();

        return item.object;
    }

    Value &variableObject() {
        return _variableObject;
    }
};

}
