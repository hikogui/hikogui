// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTObject.hpp"
#include "parser.hpp"
#include "TTauri/exceptions.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(Location location, char *name) noexcept : ASTExpression(location), name(name) {
        free(name);
    }

    std::string string() const noexcept override {
        return name;
    }

    virtual std::vector<std::string> getFQName() {
        return { name };
    }

    universal_value &executeLValue(ExecutionContext &context) const override {
        return context.currentObject()[name];
    } 

    universal_value &executeAssignment(ExecutionContext &context, universal_value other) const override {
        auto &lv = context.currentObject()[name];
        lv = std::move(other);
        return lv;
    }

    template<typename T>
    T getArgument(std::vector<universal_value> const &arguments, size_t i, bool lastArgument=false) const {
        if (i >= arguments.size()) {
            TTAURI_THROW(invalid_operation_error((boost::format("syntax error, not enough arguments to function '%', expecting argument number %i of type %s")
                % name % (i + 1) % typeid(T).name()).str())
                << error_info<"location"_tag>(location)
            );
        }

        let argument = arguments.at(0);
        if (!argument.is_promotable_to<T>()) {
            TTAURI_THROW(invalid_operation_error((boost::format("syntax error, invalid argument to function '%s', expecting argument number %i of type %s got %s")
                % name % (i + 1) % typeid(T).name() % argument.type().name()).str())
                << error_info<"location"_tag>(location)
            );
        }

        if (lastArgument && i != (arguments.size() - 1)) {
            TTAURI_THROW(invalid_operation_error((boost::format("syntax error, too many arguments to function '%', expecting %i arguments got %i")
                % name % (i + 1) % arguments.size()).str())
                << error_info<"location"_tag>(location)
            );
        }

        return get_and_promote<T>(argument);
    }

    /*! Include a configuration file.
     */
    universal_value executeIncludeCall(ExecutionContext &context, std::vector<universal_value> const &arguments) const {
        auto path = getArgument<URL>(arguments, 0, true);

        // The included file is relative to the directory of this configuration file.
        if (path.isRelative()) {
            path = location.file->urlByRemovingFilename() / path;
        }

        try {
            let ast = std::unique_ptr<ASTObject>{parseConfigFile(path)};
            return ast->execute();

        } catch (error &e) {
            // An error was captured from recursive parsing.
            // Assemble the error message from this error and throw it.
            std::string errorMessage;
            if (let previousErrorMessage = e.get<std::string, "previous-msg"_tag>()) {
                errorMessage += *previousErrorMessage + "\n";
            }

            if (let location = e.get<Location, "location"_tag>()) {
                errorMessage += location->string() + ": ";
            }

            errorMessage += e.message();
            errorMessage += ".";

            TTAURI_THROW(invalid_operation_error((boost::format("Could not include file '%s'") % path).str())
                << error_info<"location"_tag>(location)
                << error_info<"previous-msg"_tag>(errorMessage)
            );
        }
    }

    /*! Return a absolute path relative to the directory where this configuration file is located.
    */
    universal_value executePathCall(ExecutionContext &context, std::vector<universal_value> const &arguments) const {
        if (arguments.size() == 0) {
            // Without arguments return the directory where this configuration file is located.
            return location.file->urlByRemovingFilename();
        } else {
            // Suffix the given argument with the directory where this configuration file is located.
            let path = getArgument<URL>(arguments, 0, true);

            if (path.isRelative()) {
                return location.file->urlByRemovingFilename() / path;
            } else {
                return path;
            }
        }
    }

    /*! Return a absolute path relative to the current working directory.
     */
    universal_value executeCwdCall(ExecutionContext &context, std::vector<universal_value> const &arguments) const {
        if (arguments.size() == 0) {
            // Without argument return the current working directory.
            return URL::urlFromCurrentWorkingDirectory();

        } else {
            // Suffix the given argument with the current working directory.
            let path = getArgument<URL>(arguments, 0, true);

            if (path.isRelative()) {
                return URL::urlFromCurrentWorkingDirectory() / path;
            } else {
                TTAURI_THROW(invalid_operation_error((boost::format("Expecting relative path argument to function '%s' got '%s'")
                    % name % path).str())
                    << error_info<"location"_tag>(location)
                );
            }
        }
    }

    /*! A function call.
     * The expression is a identifier followed by a call; therefor this is a normal function call.
     */
    universal_value executeCall(ExecutionContext &context, std::vector<universal_value> const &arguments) const override {
        if (name == "include") {
            return executeIncludeCall(context, arguments);

        } else if (name == "path") {
            return executePathCall(context, arguments);

        } else if (name == "cwd") {
            return executeCwdCall(context, arguments);

        } else {
            TTAURI_THROW(invalid_operation_error((boost::format("Unknown function '%'") % name).str())
                << error_info<"location"_tag>(location)
            );
        }
    }

};

}
