// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "parser.hpp"

namespace TTauri::Config {

/*! Configuration
 * 
 */
struct Config {
    std::filesystem::path path;
    ASTObject *ast = nullptr;
    Value root = {};

    std::string errorMessage;

    /*! Load a configuration file.
     * See the README.md file is this directory for the file format of the configuration file.
     * \param path path to the configuration file.
     */
    Config(std::filesystem::path const &path) : path(path) {
        try {
            ast = parseConfigFile(path);
            root = ast->execute();

        } catch (ConfigError &e) {
            if (let previousErrorMessage = boost::get_error_info<errinfo_previous_error_message>(e)) {
                errorMessage += *previousErrorMessage + "\n";
            }

            if (let location = boost::get_error_info<errinfo_location>(e)) {
                errorMessage += location->string() + ": ";
            }

            errorMessage += e.what();
            errorMessage += ".";
        }
    }

    ~Config() {
        delete ast;
    }

    /*! Parsing the configuration file was succesfull.
     */
    bool success() const {
        return !root.is_type<Undefined>();
    }

    /*! Retreive error message
     */
    std::string error() const {
        if (success()) {
            return "";
        } else {
            return errorMessage;
        }
    }

    /*! string representation of the abstract-syntax-tree.
     */
    std::string astString() const {
        if (ast) {
            return ast->string();
        } else {
            return "";
        }
    }

    /*! string representation of the configuration.
     */
    std::string string() const {
        if (success()) {
            return root.string();
        } else {
            return error();
        }
    }

    Value operator[](std::string const &key) const {
        let splitKey = split(key, '.');
        return root.get(splitKey);
    }

    Value &operator[](std::string const &key) {
        let splitKey = split(key, '.');
        return root.get(splitKey);
    }

    /*! Get a value from the configuration.
     * The key is a string; identifiers and integer indices
     * seperated by dots. To select items from nested objects
     * and arrays.
     *
     * The following types are supported:
     * - bool, int64_t, double, std::string, std::filesystem::path, Color_XYZ
     * - std::vector<std::any>, std::map<std::string, std::any>
     *
     * int64_t can be promoted to double.
     * std::string can be promoted to std::filesystem::path
     *
     * \param key A configuration key.
     */
    template<typename T>
    T value(std::string const &key) const {
        let obj = (*this)[key];
        return obj.value<T>();
    }

    /*! Get the root object.
     * \see value() for the different kinds of types that are supported.
     */
    std::map<std::string,std::any> rootObject() {
        return root.value<std::map<std::string,std::any>>();
    }
};

}
