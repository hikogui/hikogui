// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Value.hpp"
#include <optional>

namespace TTauri::Config {


/*! Context used during execution.
 * This context keeps track of the active object for statements
 * in the configuration file.
 */
struct ExecutionContext {
    struct Item {
        Value object = Object{};
        Value *section = nullptr;
    };

    Value _variableObject = Object{};
    std::vector<Item> objectStack;

    /*! Create an empty object on the stack.
     * This method is called at the start of a object-literal.
     */
    void pushObject() {
        objectStack.emplace_back();
    }

    /*! Set the active object.
     * Override the active object whenever a section-statement
     * is encountered in an object-literal.
     */
    void setSection(Value *section) {
        auto &item = objectStack.back();
        item.section = section;
    }

    /*! Pop object.
     * This method is caled at the end of a object-literal.
     */
    Value popObject() {
        return pop_back(objectStack).object;
    }

    /*! Get the current active object.
     * When assignments are done, this is the first object that
     * is accessed.
     */
    Value &currentObject() {
        auto &item = objectStack.back();
        
        if (item.section) {
            return *(item.section);
        } else {
            return item.object;
        }
    }

    /*! Get root object.
     * This method is called when the root-accesor operator is used.
     */
    Value &rootObject() {
        auto &item = objectStack.front();

        return item.object;
    }

    /*! Get variable object
     * This method is called when the variable-accessor operator is used.
     */
    Value &variableObject() {
        return _variableObject;
    }
};

}
