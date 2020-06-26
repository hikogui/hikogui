// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace tt {

/** A catalogue of messages.
 */
class Catalogue {
    std::string language;

    std::unique_ptr<expression_node> plural_expression;

    // std::map can be searched using a string_view.
    std::map<std::string,std::vector<std::string>> translations;

public:
    void add_translation(std::string original, std::vector<std::string> translation) noexcept {
        translations[original] = translation;
    }

    /** Return the plarity index.
     */
    ssize_t plurality(unsigned long long n) const noexcept {
        // To protect against overflow make the number smaller,
        // But preserve trailing digits since language rules check for these.
        n = (n > 1000000) ? (n % 1000000) : n;

        auto context = expression_evaluation_context{};
        context.set_local("n", n);

        if (plural_expression) {
            ttlet result = plural_expression->evaluate(context);
            if (result.is_bool()) {
                return static_cast<bool>(result) ? 1 : 0;
            } else if (result.is_integer()) {
                return static_cast<ssize_t>(result);
            } else {
                LOG_ERROR("Catalogue {}: plurality expression with value {} results in non-bool or non-integer type but {}",
                    language, n, result.type_name()
                );
                // Plural expression failure, use english rules.
                return (n == 1) ? 0 : 1;
            }

        } else {
            // No plural expression available, use english rules.
            return (n == 1) ? 0 : 1;
        }
    }

    /** Get a message from the catalogue.
     * @param original English message used to search the catalogue. The string may include the context following a '|' character.
     * @param original_plural Engligh plural message, used as fallback together with the msgid. May be an empty string_view.
     * @param n Used for plurality determination. If unused set to 1.
     * @return The translated message, or the english fallback.
     */
    [[nodiscard]] std::string_view get(std::string const &original, std::string_view original_plural, unsigned long long n) const noexcept {
        ttlet pl = plurality(n);

        ttlet i = translations.find(original);
        if (i == translations.end()) {
            LOG_ERROR("Catalogue '{}': missing translation for msgid '{}'", language, original);
            if (n != 1 && ssize(original_plural) != 0) {
                return original_plural;
            } else {
                return original;
            }
        }

        if (pl >= ssize(i->second)) {
            LOG_ERROR("Catalogue '{}': missing plurality {} for msgid '{}'", language, pl, original);
            tt_assume(ssize(i->second) >= 1);
            return i->second[0];
        }

        return i->second[pl];
    }

};

Catalogue parseCatalogue(URL const &url);

}
