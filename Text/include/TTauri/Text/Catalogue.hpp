// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace TTauri::Text {

/** A catalogue of messages.
 */
class Catalogue {
    std::string name;

    std::unique_ptr<expression_node> plural_expression;

    // std::map can be searched using a string_view.
    std::map<std::string,std::vector<std::string>> msgstr;

public:
    /** Return the plarity index.
     */
    ssize_t plurality(unsigned long long n) const noexcept {
        // To protect against overflow make the number smaller,
        // But preserve trailing digits since language rules check for these.
        n = (n > 1000000) ? (n % 1000000) : n;

        auto context = expression_evaluation_context{};
        context.set_local("n", n);

        if (plural_expression) {
            let result = plural_expression->evaluate(context);
            if (result.is_bool()) {
                return static_cast<bool>(result) ? 1 : 0;
            } else if (result.is_integer()) {
                return static_cast<ssize_t>(result);
            } else {
                LOG_ERROR("Catalogue {}: plurality expression with value {} results in non-bool or non-integer type but {}",
                    name, n, result.type_name()
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
     * @param msgid English message used to search the catalogue. The string may include the context following a '|' character.
     * @param msgid_plural Engligh plural message, used as fallback together with the msgid. May be an empty string_view.
     * @param n Used for plurality determination. If unused set to 1.
     * @return The translated message, or the english fallback.
     */
    [[nodiscard]] char const *get(std::string_view msgid, std::string_view msgid_plural, unsigned long long n) const noexcept {
        let pl = plurality(n);

        let i = msgstr.find(msgid);
        if (i == msgstr.end()) {
            LOG_ERROR("Catalogue '{}': missing translation for msgid '{}'", name, msgid);
            if (i != 1 && ssize(msgid_plural) != 0) {
                return msgid_pural.data();
            } else {
                return msgid.data();
            }
        }

        if (pl >= ssize(*i)) {
            LOG_ERROR("Catalogue '{}': missing plurality {} for msgid '{}'", name, pl, msgid);
            ttauri_assume(ssize(*i) >= 1);
            return *i[0].data();
        }

        return *i[pl].data();

    }

};

Catalogue parseCatalogue(URL const &url);

}
