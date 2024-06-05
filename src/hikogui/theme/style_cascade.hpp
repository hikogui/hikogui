


#pragma once

#include "../macros.hpp"


hi_export_module(hikogui.theme : style_cascade);

hi_export namespace hi::inline v1 {
namespace detail {

struct style_property_element {
    style_selector selector;
    style_properties properties;
};

[[nodiscard]] auto initial_style_properties_init() noexcept
{
    auto r = std::vector<style_property_element>{};

    return r;
}

auto initial_style_properties = initial_style_properties_init();
auto user_style_properties = std::vector<style_property_element>{};
auto theme_style_properties = std::vector<style_property_element>{};
auto author_style_properties = std::vector<style_property_element>{};

generator<style_property_element const&> all_style_properties() noexcept
{
    for (auto const& element : initial_style_properties) {
        co_yield element;
    }
    for (auto const& element : user_style_properties) {
        co_yield element;
    }
    for (auto const& element : theme_style_properties) {
        co_yield element;
    }
    for (auto const& element : author_style_properties) {
        co_yield element;
    }
}

}


[[nodiscard]] style_properties get_properties(style_path const& path, style_pseudo_class pseudo_class, size_t depth) noexcept
{
    auto pseudo_classes = make_pseudo_classes_strings(pseudo_class);
    auto nesting_depth = size_t{0};

    auto r = style_properties{};

    for (auto const& element : detail::all_style_properties()) {
        if (matches(element.selector, path, pseudo_classes) {
            r.apply(element.properties); 
        }
    }

    return r;
}

}

