// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTObject.hpp"
#include "parser.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(Location location, char *name) : ASTExpression(location), name(name) {
        free(name);
    }

    std::string str() const override {
        return name;
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return context->currentObject()[name];
    } 

    Value &executeAssignment(ExecutionContext *context, Value other) const override {
        auto &lv = context->currentObject()[name];
        lv = std::move(other);
        return lv;
    }

    template<typename T>
    T getArgument(std::vector<Value> const &arguments, size_t i, bool lastArgument=false) const {
        if (i >= arguments.size()) {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column)
                << errinfo_message((boost::format("syntax error, not enough arguments to function '%', expecting argument number %i of type %s")
                    % name % (i + 1) % typeid(T).name()).str())
            );
        }

        auto const argument = arguments.at(0);
        if (!argument.is_promotable_to<T>()) {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column)
                << errinfo_message((boost::format("syntax error, invalid argument to function '%s', expecting argument number %i of type %s got %s")
                    % name % (i + 1) % typeid(T).name() % argument.type().name()).str())
            );
        }

        if (lastArgument && i != (arguments.size() - 1)) {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column)
                << errinfo_message((boost::format("syntax error, too many arguments to function '%', expecting %i arguments got %i")
                    % name % (i + 1) % arguments.size()).str())
            );
        }

        return argument.value<T>();
    }

    Value executeIncludeCall(ExecutionContext *context, std::vector<Value> const &arguments) const {
        auto path = getArgument<std::filesystem::path>(arguments, 0, true);

        if (path.is_relative()) {
            path = location.file->parent_path() / path;
        }

        auto const ast = std::unique_ptr<ASTObject>{parseConfigFile(path)};
        return ast->execute();
    }

    Value executePathCall(ExecutionContext *context, std::vector<Value> const &arguments) const {
        if (arguments.size() == 0) {
            return location.file->parent_path();
        } else {
            auto const path = getArgument<std::filesystem::path>(arguments, 0, true);

            if (path.is_relative()) {
                return location.file->parent_path() / path;
            } else {
                return path;
            }
        }
    }

    Value executeCwdCall(ExecutionContext *context, std::vector<Value> const &arguments) const {
        if (arguments.size() == 0) {
            return std::filesystem::current_path();
        } else {
            auto const path = getArgument<std::filesystem::path>(arguments, 0, true);

            if (path.is_relative()) {
                return std::filesystem::current_path() / path;
            } else {
                BOOST_THROW_EXCEPTION(InvalidOperationError()
                    << boost::errinfo_file_name(location.file->string())
                    << boost::errinfo_at_line(location.line)
                    << errinfo_at_column(location.column)
                    << errinfo_message((boost::format("Expecting relative path argument to function '%s' got '%s'")
                        % name % path.string()).str())
                );
            }
        }
    }

    Value executeCall(ExecutionContext *context, std::vector<Value> const &arguments) const override {
        if (name == "include") {
            return executeIncludeCall(context, arguments);

        } else if (name == "path") {
            return executePathCall(context, arguments);

        } else if (name == "cwd") {
            return executeCwdCall(context, arguments);

        } else {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column)
                << errinfo_message((boost::format("Unknown function '%'") % name).str())
            );
        }
    }

};

}
