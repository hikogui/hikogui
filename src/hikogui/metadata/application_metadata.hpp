// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "semantic_version.hpp"
#include "../algorithm/algorithm.hpp"
#include "../macros.hpp"
#include <string>
#include <stdexcept>
#include <filesystem>
#include <source_location>

hi_export_module(hikogui.metadata.metadata_application);

hi_export namespace hi { inline namespace v1 {

hi_inline std::optional<std::string> _application_name = std::nullopt;
hi_inline std::optional<std::string> _application_slug = std::nullopt;
hi_inline std::optional<std::string> _application_vendor = std::nullopt;
hi_inline std::optional<semantic_version> _application_version = std::nullopt;

hi_export [[nodiscard]] hi_inline std::string const& get_application_name()
{
    if (_application_name) {
        return *_application_name;
    } else {
        throw std::logic_error("set_application_name() should be called at application startup.");
    }
}

hi_export [[nodiscard]] hi_inline std::string const& get_application_slug()
{
    if (_application_slug) {
        return *_application_slug;
    } else {
        throw std::logic_error("set_application_name() should be called at application startup.");
    }
}

hi_export [[nodiscard]] hi_inline std::string const& get_application_vendor()
{
    if (_application_vendor) {
        return *_application_vendor;
    } else {
        throw std::logic_error("set_application_vendor() should be called at application startup.");
    }
}

hi_export [[nodiscard]] hi_inline semantic_version const& get_application_version()
{
    if (_application_version) {
        return *_application_version;
    } else {
        throw std::logic_error("set_application_version() should be called at application startup.");
    }
}

hi_export hi_inline void set_application_name(std::string_view name, std::string_view slug)
{
    if (name.empty()) {
        throw std::invalid_argument("application name must not be empty.");
    }
    if (name.contains('/') or name.contains('\\')) {
        throw std::invalid_argument("application name must not contain a slash or backslash.");
    }
    if (slug.empty()) {
        throw std::invalid_argument("application slug must not be empty.");
    }
    if (not is_slug(slug)) {
        throw std::invalid_argument("application slug must contain only 'a'-'z' '0'-'9' and '-' characters.");
    }

    _application_name = name;
    _application_slug = slug;
}

hi_export hi_inline void set_application_name(std::string_view name)
{
    return set_application_name(name, make_slug(name));
}

hi_export hi_inline void set_application_vendor(std::string_view name)
{
    if (name.empty()) {
        throw std::invalid_argument("vendor name must not be empty.");
    }
    if (name.contains('/') or name.contains('\\')) {
        throw std::invalid_argument("vendor name must not contain a slash or backslash.");
    }

    _application_vendor = name;
}

hi_export hi_inline void set_application_version(semantic_version version) noexcept
{
    _application_version = version;
}

hi_export hi_inline void set_application_version(int major, int minor = 0, int patch = 0) noexcept
{
    return set_application_version(semantic_version{major, minor, patch});
}

}} // namespace hi::inline v1
