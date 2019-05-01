// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "parser.hpp"

namespace TTauri::Config {

struct Config {
    std::system::path path;
    ASTObject *ast;
    Value root;
    std::string _error;

    Config(std::system::path const &path) : path(path) {
        try {
            ast = parseConfigFile(path);
            root = ast->execute();

        } catch (ConfigError &x) {

        }
    }

    ~Config() {
        delete ast;
    }

    std::string error() const {
        return _error;
    }

    std::string astStr() const {
        return ast->str();
    }

    std::string str() const {
        return root.str();
    }

    bool hasValue() const {
        return !root.is_type<Undefined>();
    }

    template<typename T>
    T value(std::string key) const {
        return root.get(key).value<T>();
    }

    std::map<std::string,std::any> rootObject() {
        return root.value<std::map<std::string,std::any>>();
    }

};

}