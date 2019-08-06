// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "parser.hpp"
#include "TTauri/universal_value.hpp"
#include "ASTObject.hpp"

namespace TTauri::Config {

/*! Configuration
 * 
 */
struct Config {
    boost::filesystem::path path;
    ASTObject *ast = nullptr;
    universal_value root = Undefined{};

    std::string errorMessage;

    /*! Load a configuration file.
     * See the README.md file is this directory for the file format of the configuration file.
     * \param path path to the configuration file.
     */
    Config(boost::filesystem::path path) : path(std::move(path)) {
        try {
            ast = parseConfigFile(this->path);
            root = ast->execute();

        } catch (Error &e) {
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
        return !holds_alternative<Undefined>(root);
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

    universal_value operator[](std::string const &key) const {
        let splitKey = split(key, '.');
        return root.get_by_path(splitKey);
    }

    universal_value &operator[](std::string const &key) {
        let splitKey = split(key, '.');
        return root.get_by_path(splitKey);
    }

    /*! Get a value from the configuration.
     * The key is a string; identifiers and integer indices
     * seperated by dots. To select items from nested objects
     * and arrays.
     *
     * The following types are supported:
     * - bool, int64_t, double, std::string, boost::filesystem::path, Color_XYZ
     * - std::vector<std::any>, std::map<std::string, std::any>
     *
     * int64_t can be promoted to double.
     * std::string can be promoted to boost::filesystem::path
     *
     * \param key A configuration key.
     */
    template<typename T>
    T value(std::string const &key) const {
        let obj = (*this)[key];
        return get_and_promote<T>(obj);
    }

    /*! Get the root object.
     * \see value() for the different kinds of types that are supported.
     */
    Object rootObject() {
        return get<Object>(root);
    }
};

/*! string representation of the configuration.
*/
inline std::string to_string(Config const &config) {
    if (config.success()) {
        return to_string(config.root);
    } else {
        return config.error();
    }
}


}
