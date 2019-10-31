// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"
#include "TTauri/Config/ASTObject.hpp"
#include "TTauri/Config/parser.hpp"
#include "TTauri/Foundation/exceptions.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(Location location, char *name) noexcept : ASTExpression(location), name(name) {
        free(name);
    }

    std::string string() const noexcept override {
        return name;
    }

    std::vector<std::string> getFQName() override {
        return { name };
    }

    datum &executeLValue(ExecutionContext &context) const override {
        return context.currentObject()[name];
    } 

    datum &executeAssignment(ExecutionContext &context, datum other) const override {
        auto &lv = context.currentObject()[name];
        lv = std::move(other);
        return lv;
    }

    template<typename T>
    T getArgument(std::vector<datum> const &arguments, size_t i, bool lastArgument=false) const {
        if (i >= arguments.size()) {
            TTAURI_THROW(
                invalid_operation_error(
                    "syntax error, not enough arguments to function '{0}', expecting argument number {1} of type {2}",
                    name,
                    (i + 1),
                    typeid(T).name()
                )
                .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
            );
        }

        let argument = arguments.at(0);
        if (!will_cast_to<T>(argument)) {
            TTAURI_THROW(
                invalid_operation_error(
                    "syntax error, invalid argument to function '{0}', expecting argument number {1} of type {2} got {3}",
                    name,
                    (i + 1),
                    typeid(T).name(),
                    argument.type_name()
                )
                .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
            );
        }

        if (lastArgument && i != (arguments.size() - 1)) {
            TTAURI_THROW(
                invalid_operation_error(
                    "syntax error, too many arguments to function '{0}', expecting {1} arguments got {2}",
                    name,
                    (i + 1),
                    arguments.size()
                )
                .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
            );
        }

        return static_cast<T>(argument);
    }

    /*! Include a configuration file.
     */
    datum executeIncludeCall(ExecutionContext &context, std::vector<datum> const &arguments) const {
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
            if (e.has<"previous_msg"_tag>()) {
                let previousErrorMessage = static_cast<std::string>(e.get<"previous_msg"_tag>());
                errorMessage += previousErrorMessage + "\n";
            }

            if (e.has<"line"_tag>()) {
                if (e.has<"url"_tag>()) {
                    let url = static_cast<URL>(e.get<"url"_tag>());
                    errorMessage += url.string() + ":";
                }

                let line = static_cast<size_t>(e.get<"line"_tag>());
                errorMessage += std::to_string(line) + ":";

                if (e.has<"column"_tag>()) {
                    let column = static_cast<size_t>(e.get<"column"_tag>());
                    errorMessage += std::to_string(column) + ":";
                }
                errorMessage += " ";
            }

            errorMessage += e.message();
            errorMessage += ".";

            TTAURI_THROW(invalid_operation_error("Could not include file '{}'", path)
                .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
                .set<"previous_msg"_tag>(errorMessage)
            );
        }
    }

    /*! Return a absolute path relative to the directory where this configuration file is located.
    */
    datum executePathCall(ExecutionContext &context, std::vector<datum> const &arguments) const {
        if (arguments.size() == 0) {
            // Without arguments return the directory where this configuration file is located.
            return datum{location.file->urlByRemovingFilename()};
        } else {
            // Suffix the given argument with the directory where this configuration file is located.
            let path = getArgument<URL>(arguments, 0, true);

            if (path.isRelative()) {
                return datum{location.file->urlByRemovingFilename() / path};
            } else {
                return datum{path};
            }
        }
    }

    /*! Return a absolute path relative to the current working directory.
     */
    datum executeCwdCall(ExecutionContext &context, std::vector<datum> const &arguments) const {
        if (arguments.size() == 0) {
            // Without argument return the current working directory.
            return datum{URL::urlFromCurrentWorkingDirectory()};

        } else {
            // Suffix the given argument with the current working directory.
            let path = getArgument<URL>(arguments, 0, true);

            if (path.isRelative()) {
                return datum{URL::urlFromCurrentWorkingDirectory() / path};
            } else {
                TTAURI_THROW(
                    invalid_operation_error(
                        "Expecting relative path argument to function '{0}' got '{1}'",
                        name,
                        path
                    )
                    .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
                );
            }
        }
    }

    /*! A function call.
     * The expression is a identifier followed by a call; therefor this is a normal function call.
     */
    datum executeCall(ExecutionContext &context, std::vector<datum> const &arguments) const override {
        if (name == "include") {
            return executeIncludeCall(context, arguments);

        } else if (name == "path") {
            return executePathCall(context, arguments);

        } else if (name == "cwd") {
            return executeCwdCall(context, arguments);

        } else {
            TTAURI_THROW(
                invalid_operation_error(
                    "Unknown function '{0}'",
                    name
                )
                .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
            );
        }
    }

};

}
