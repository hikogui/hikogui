// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <ranges>

export module hikogui_l10n_po_parser;
import hikogui_file;
import hikogui_i18n;
import hikogui_l10n_translation;
import hikogui_parser;

namespace hi::inline v1 {

struct po_translation {
    std::optional<std::string> msgctxt;
    std::string msgid;
    std::string msgid_plural;
    std::vector<std::string> msgstr;
};

struct po_translations {
    language_tag language;
    size_t nr_plural_forms;
    std::string plural_expression;
    std::vector<po_translation> translations;
};

namespace detail {

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::tuple<std::string, size_t, std::string> parse_po_line(It& it, ItEnd last, std::string_view path)
{
    hi_assert(it != last);

    auto name = std::string{};
    if ((*it == token::id)) {
        name = static_cast<std::string>(*it++);
    } else {
        throw parse_error(
            std::format("{}: Expecting a keyword at start of each line, got {}", token_location(it, last, path), *it));
    }

    auto index = 0_uz;
    if ((*it == '[')) {
        ++it;

        if (it != last and *it == token::integer) {
            index = static_cast<size_t>(*it++);
        } else {
            throw parse_error(std::format(
                "{}: Expecting an integer literal as an index for {}, got {}", token_location(it, last, path), name, *it));
        }

        if (it != last and *it == ']') {
            ++it;
        } else {
            throw parse_error(std::format(
                "{}: The index on {} must terminate with a bracket ']', got {}", token_location(it, last, path), name, *it));
        }
    }

    auto value = std::string{};
    if (it != last and (*it == token::sstr or *it == token::dstr)) {
        value = static_cast<std::string>(*it++);
    } else {
        throw parse_error(
            std::format("{}: Expecting a string value after {}, got {}", token_location(it, last, path), name, *it));
    }

    while (it != last and (*it == token::sstr or *it == token::dstr)) {
        // Concatenating string literals.
        value += static_cast<std::string>(*it++);
    }

    return {name, index, value};
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<po_translation> parse_po_translation(It& it, ItEnd last, std::string_view path)
{
    po_translation r;

    while (it != last) {
        if (r.msgstr.empty()) {
            // If there have been no "msgstr" keywords, then capture information in the translation.
            auto [name, index, value] = parse_po_line(it, last, path);

            if (name == "msgctxt") {
                r.msgctxt = value;

            } else if (name == "msgid") {
                r.msgid = value;

            } else if (name == "msgid_plural") {
                r.msgid_plural = value;

            } else if (name == "msgstr") {
                if (index >= r.msgstr.size()) {
                    r.msgstr.resize(index + 1);
                }
                r.msgstr[index] = value;

            } else {
                throw parse_error(
                    std::format("{}: Line starts with unexpected keyword {}", token_location(it, last, path), name));
            }

        } else if ((*it == token::id) and (*it == "msgstr")) {
            // After the first "msgstr" keyword there may be others, but another keyword will start a new translation.
            auto [name, index, value] = parse_po_line(it, last, path);

            if (index >= r.msgstr.size()) {
                r.msgstr.resize(index + 1);
            }
            r.msgstr[index] = value;

        } else {
            // The current keyword is not a msgstr, so return the translation captured.
            return r;
        }
    }

    return std::nullopt;
}

constexpr void parse_po_header(po_translations& r, std::string_view header)
{
    using namespace std::literals;

    for (hilet line : std::views::split(header, "\\n"sv)) {
        if (line.empty()) {
            // Skip empty header lines.
            continue;
        }

        auto split_line = make_vector<std::string_view>(std::views::split(line, ":"sv));
        if (split_line.size() < 2) {
            throw parse_error(std::format("Unknown header '{}'", std::string_view{line}));
        }

        hilet name = to_lower(strip(split_line.front()));
        split_line.erase(split_line.begin());
        hilet value = strip(join(split_line, ":"));

        if (name == "language") {
            r.language = language_tag{value};
        }
    }
}

} // namespace detail

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr po_translations parse_po(It it, ItEnd last, std::string_view path)
{
    po_translations r;

    auto token_it = lexer<lexer_config::sh_style()>.parse(it, last);

    while (token_it != std::default_sentinel) {
        if (auto result = detail::parse_po_translation(token_it, std::default_sentinel, path)) {
            if (not result->msgid.empty()) {
                r.translations.push_back(*result);

            } else if (result->msgstr.size() == 1) {
                // If a translation has an empty msgid, then the msgstr contain headers.
                detail::parse_po_header(r, result->msgstr.front());

            } else {
                throw parse_error(std::format("{}: Unknown .po syntax.", token_location(token_it, path)));
            }
        }
    }

    return r;
}

[[nodiscard]] constexpr po_translations parse_po(std::string_view text, std::string_view path)
{
    return parse_po(text.begin(), text.end(), path);
}

[[nodiscard]] inline po_translations parse_po(std::filesystem::path const& path)
{
    return parse_po(as_string_view(file_view{path}), path.string());
}

inline void load_translations(std::filesystem::path path)
{
    hi_log_info("Loading translation file {}.", path.string());
    return add_translations(parse_po(path));
}

inline void load_translations()
{
    if (not translations_loaded.exchange(true)) {
        // XXX Waiting for C++23 to extend life-time of temporaries in for loops.
        auto resource_paths = resource_dirs();
        for (auto &path : glob(resource_paths, "**/*.po")) {
            try {
                load_translations(path);
            } catch (std::exception const &e) {
                hi_log_error("Could not load translation file. {}", e.what());
            }
        }
    }
}

[[nodiscard]] inline std::pair<std::string_view, language_tag>
get_translation(std::string_view msgid, long long n, std::vector<language_tag> const &languages) noexcept
{
    load_translations();

    // Update only the language in each iteration.
    auto key = translation_key{std::string{msgid}, language_tag{}};

    for (hilet language : languages) {
        key.language = language;

        hilet i = translations.find(key);
        if (i != translations.cend()) {
            hilet plurality = cardinal_plural(language, n, i->second.size());
            hilet &translation = i->second[plurality];
            if (translation.size() != 0) {
                return {translation, language};
            }
        }
    }
    hi_log_debug("No translation found for '{}'", msgid);
    return {msgid, language_tag{"en-Latn-US"}};
}

} // namespace hi::inline v1
