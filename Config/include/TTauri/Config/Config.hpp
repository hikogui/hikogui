// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/parser.hpp"
#include "TTauri/Config/ASTObject.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"

namespace TTauri::Config {

/*! Configuration
 * 
 */
struct Config {
    URL path;
    ASTObject *ast = nullptr;
    datum root;

    std::string _errorMessage;

    /*! Load a configuration file.
     * See the README.md file is this directory for the file format of the configuration file.
     * \param path path to the configuration file.
     */
    Config(URL path) noexcept : path(std::move(path)) {
        try {
            ast = parseConfigFile(this->path);
            root = ast->execute();

        } catch (error &e) {
            if (e.has<"previous_msg"_tag>()) {
                let previousErrorMessage = static_cast<std::string>(e.get<"previous_msg"_tag>());
                _errorMessage += previousErrorMessage + "\n";
            }

            if (e.has<"location"_tag>()) {
                let location = static_cast<Location>(e.get<"location"_tag>());
                _errorMessage += location.string() + ": ";
            }

            _errorMessage += e.message();
            _errorMessage += ".";
        }
    }

    ~Config() {
        delete ast;
    }

    /*! Parsing the configuration file was succesfull.
     */
    bool success() const noexcept {
        return !root.is_undefined();
    }

    /*! Retreive error message
     */
    std::string errorMessage() const noexcept {
        if (success()) {
            return "";
        } else {
            return _errorMessage;
        }
    }

    /*! string representation of the abstract-syntax-tree.
     */
    std::string astString() const noexcept {
        if (ast) {
            return ast->string();
        } else {
            return "";
        }
    }

    datum operator[](std::string const &key) const {
        let splitKey = split(key, '.');
        return root.get_by_path(splitKey);
    }

    datum &operator[](std::string const &key) {
        let splitKey = split(key, '.');
        return root.get_by_path(splitKey);
    }

    /*! Get a value from the configuration.
     * The key is a string; identifiers and integer indices
     * seperated by dots. To select items from nested objects
     * and arrays.
     *
     * The following types are supported:
     * - bool, int64_t, double, std::string, URL, Color_XYZ
     * - std::vector<std::any>, std::map<std::string, std::any>
     *
     * int64_t can be promoted to double.
     * std::string can be promoted to URL
     *
     * \param key A configuration key.
     */
    template<typename T>
    T value(std::string const &key) const {
        let obj = (*this)[key];
        return static_cast<T>(obj);
    }

    /*! Get the root object.
     * \see value() for the different kinds of types that are supported.
     */
    datum::map rootObject() noexcept {
        return static_cast<datum::map>(root);
    }
};

/*! string representation of the configuration.
*/
inline std::string to_string(Config const &config) {
    if (config.success()) {
        return config.root.repr();
    } else {
        return config.errorMessage();
    }
}


}
