// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/universal_value.hpp"
#include <optional>

namespace TTauri::Config {


/*! Context used during execution.
 * This context keeps track of the active object for statements
 * in the configuration file.
 */
struct ExecutionContext {
    struct Item {
        universal_value object = Object{};
        std::vector<std::string> section;
    };

    universal_value _variableObject = Object{};
    std::vector<Item> objectStack;

    /*! Create an empty object on the stack.
     * This method is called at the start of a object-literal.
     */
    void pushObject() noexcept {
        objectStack.emplace_back();
    }

    /*! Set the active object.
     * Override the active object whenever a section-statement
     * is encountered in an object-literal.
     */
    void setSection(std::vector<std::string> section) noexcept {
        auto &item = objectStack.back();
        item.section = std::move(section);
    }

    /*! Pop object.
     * This method is caled at the end of a object-literal.
     */
    universal_value popObject() noexcept {
        return pop_back(objectStack).object;
    }

    /*! Get the current active object.
     * When assignments are done, this is the first object that
     * is accessed.
     */
    universal_value &currentObject() noexcept {
        auto &item = objectStack.back();

        gsl::not_null<universal_value *> object = &(item.object);
        for (let &key: item.section) {
            object = &((*object)[key]);
        }

        return *object;
    }

    /*! Get root object.
     * This method is called when the root-accesor operator is used.
     */
    universal_value &rootObject() noexcept {
        auto &item = objectStack.front();

        return item.object;
    }

    /*! Get variable object
     * This method is called when the variable-accessor operator is used.
     */
    universal_value &variableObject() noexcept {
        return _variableObject;
    }
};

}
