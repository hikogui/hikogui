// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_file.hpp"
#include "theme_value.hpp"
#include "../font/font_book.hpp"
#include "../codec/JSON.hpp"
#include "../log.hpp"
#include <algorithm>
#include <ranges>

namespace hi { inline namespace v1 {

[[nodiscard]] static color parse_theme_color(datum const& data)
{
    if (auto list = get_if<datum::vector_type>(&data)) {
        hi_check(list->size() == 3 or list->size() == 4, "Color values must be 3 or 4 elements, got {}.", list->size());

        hilet r = data[0];
        hilet g = data[1];
        hilet b = data[2];
        hilet a = data.size() == 4 ? data[3] : (holds_alternative<long long>(r) ? datum{255} : datum{1.0});

        if (holds_alternative<long long>(r) and holds_alternative<long long>(g) and holds_alternative<long long>(b) and
            holds_alternative<long long>(a)) {
            hilet r_ = get<long long>(r);
            hilet g_ = get<long long>(g);
            hilet b_ = get<long long>(b);
            hilet a_ = get<long long>(a);

            hi_check(r_ >= 0 and r_ <= 255, "integer red-color value not within 0 and 255");
            hi_check(g_ >= 0 and g_ <= 255, "integer green-color value not within 0 and 255");
            hi_check(b_ >= 0 and b_ <= 255, "integer blue-color value not within 0 and 255");
            hi_check(a_ >= 0 and a_ <= 255, "integer alpha-color value not within 0 and 255");

            return color_from_sRGB(
                static_cast<uint8_t>(r_), static_cast<uint8_t>(g_), static_cast<uint8_t>(b_), static_cast<uint8_t>(a_));

        } else if (
            holds_alternative<double>(r) and holds_alternative<double>(g) and holds_alternative<double>(b) and
            holds_alternative<double>(a)) {
            hilet r_ = static_cast<float>(get<double>(r));
            hilet g_ = static_cast<float>(get<double>(g));
            hilet b_ = static_cast<float>(get<double>(b));
            hilet a_ = static_cast<float>(get<double>(a));
            hi_check(a_ >= 0.0 and a_ <= 1.0, "float alpha-color vlaue not within 0.0 and 1.0");

            return hi::color(r_, g_, b_, a_);

        } else {
            throw parse_error(std::format("Expect all integers or all floating point numbers in a color, got {}.", data));
        }

    } else if (auto string = get_if<std::string>(&data)) {
        hi_check(not string->empty() and string->front() == '#', "Color string value must start with '#', got {}.", data);
        return color_from_sRGB(*string);
    } else {
        throw parse_error(std::format("Unexepected color value type, got {}.", data));
    }
}

[[nodiscard]] static std::vector<color> parse_theme_colors(datum const& data)
{
    hi_assert(holds_alternative<datum::vector_type>(data));
    hilet& list = get<datum::vector_type>(data);
    hi_assert(not list.empty());

    if (holds_alternative<long long>(list.front()) or holds_alternative<double>(list.front())) {
        return {parse_theme_color(data)};
    } else {
        auto r = std::vector<color>{};
        for (hilet& item : list) {
            r.push_back(parse_theme_color(item));
        }
        return r;
    }
}

[[nodiscard]] static font_weight parse_theme_font_weight(datum const& data)
{
    if (auto i = get_if<long long>(&data)) {
        return font_weight_from_int(*i);
    } else if (auto s = get_if<std::string>(&data)) {
        return font_weight_from_string(*s);
    } else {
        throw parse_error(std::format("Unable to parse font weight, got {}.", data));
    }
}

[[nodiscard]] static text_style parse_theme_text_style(datum const& data)
{
    hi_check(holds_alternative<datum::map_type>(data), "Expect a text-style to be an object, got '{}'", data);

    auto language = iso_639{};
    auto country = iso_3166{};
    auto script = iso_15924{};
    auto phrasing_mask = hi::text_phrasing_mask{};
    auto family_id = font_family_id{};
    auto size = 10;
    auto weight = font_weight::Regular;
    auto style = font_style::normal;
    auto color = hi::color{};
    for (hilet & [ name, value ] : get<datum::map_type>(data)) {
        hi_check(holds_alternative<std::string>(name), "Expect the keys of a text-style to be integers, got {}", name);

        if (name == "language") {
            hi_check(holds_alternative<std::string>(value), "Expect the language of a text-style to be a string.");
            language = iso_639{get<std::string>(value)};

        } else if (name == "country") {
            hi_check(holds_alternative<std::string>(value), "Expect the country of a text-style to be a string.");
            country = iso_3166{get<std::string>(value)};

        } else if (name == "script") {
            hi_check(holds_alternative<std::string>(value), "Expect the script of a text-style to be a string.");
            script = iso_15924{get<std::string>(value)};

        } else if (name == "phrasing") {
            hi_check(holds_alternative<std::string>(value), "Expect the phrasing mask of a text-style to be a string.");
            phrasing_mask = to_text_phrasing_mask(get<std::string>(value));

        } else if (name == "family") {
            hi_check(holds_alternative<std::string>(value), "Expect the font-family name to be a string, got {}", value);
            family_id = find_font_family(get<std::string>(value));

        } else if (name == "size") {
            hi_check(holds_alternative<long long>(value), "Expect the font-size to be a integer, got {}", value);
            size = static_cast<int>(value);

        } else if (name == "weight") {
            weight = parse_theme_font_weight(value);

        } else if (name == "italic") {
            hi_check(holds_alternative<bool>(value), "Expect italic to be a boolean, got {}", value);
            style = get<bool>(value) ? font_style::italic : font_style::normal;

        } else if (name == "color") {
            color = parse_theme_color(value);
        }
    }

    auto variant = font_variant{weight, style};
    return {phrasing_mask, language, script, country, family_id, variant, size, color};
}

[[nodiscard]] static std::vector<text_style> parse_theme_text_styles(datum const& data)
{
    hi_assert(holds_alternative<datum::vector_type>(data));

    auto r = std::vector<text_style>{};
    for (auto& value : get<datum::vector_type>(data)) {
        r.push_back(parse_theme_text_style(value));
    }
    return r;
}

[[nodiscard]] static theme_file::value_type parse_theme_value(datum const& data)
{
    if (holds_alternative<long long>(data) or holds_alternative<double>(data)) {
        // A size value.
        return static_cast<float>(data);

    } else if (auto string_value = get_if<std::string>(&data)) {
        hi_check(not string_value->empty(), "Unexpected empty string as theme_file value.");
        if (string_value->front() == '#') {
            // An sRGB hex-color.
            return std::vector<hi::color>{color_from_sRGB(*string_value)};
        } else if (string_value->front() == '$') {
            // A reference.
            return string_value->substr(1);
        } else {
            throw parse_error(std::format("Unexpected '{}' as theme_file value.", *string_value));
        }

    } else if (auto list_value = get_if<datum::vector_type>(&data)) {
        hi_check(not list_value->empty(), "Unexpected empty list as theme_file value.");
        if (holds_alternative<datum::map_type>(list_value->front())) {
            return parse_theme_text_styles(data);
        } else {
            return parse_theme_colors(data);
        }

    } else {
        throw parse_error(std::format("Unexpected '{}' as theme_file value.", data));
    }
}

void static resolve_theme_references(theme_file::container_type& items)
{
    constexpr auto max_recursion = 256_uz;

    // Sort items alphabetically.
    std::sort(items.begin(), items.end(), [](hilet& a, hilet& b) {
        return a.first < b.first;
    });

    for (auto& [name, value] : items) {
        auto retry = max_recursion;
        while (hilet ref = std::get_if<std::string>(&value)) {
            hi_check(--retry != 0, "Maximum recursion depth reached when resolving reference '{}", *ref);

            auto it = std::lower_bound(items.begin(), items.end(), *ref, [](hilet& item, hilet& value) {
                return item.first < value;
            });
            hi_check(it != items.end() or it->first != *ref, "Could not find reference '{}' in theme_file file", *ref);
            value = it->second;
        }
    }
}

void static order_by_specificity(theme_file::container_type& items)
{
    // Sort by name where:
    // - names with a star '*' go first.
    // - by the number of dots '.' in the name.
    std::sort(items.begin(), items.end(), [](hilet& a, hilet& b) {
        hilet& a_ = a.first;
        hilet& b_ = b.first;

        hilet a_score = a_.find('*') != a_.npos ? std::numeric_limits<size_t>::max() : std::count(a_.begin(), a_.end(), '.');
        hilet b_score = b_.find('*') != b_.npos ? std::numeric_limits<size_t>::max() : std::count(b_.begin(), b_.end(), '.');
        // Sort in reverse order.
        return a_score < b_score;
    });
}

theme_file::theme_file(std::filesystem::path const& path)
{
    try {
        hi_log_info("Parsing theme_file at {}", path.string());
        hilet data = parse_JSON(path);
        parse(data);
    } catch (std::exception const& e) {
        throw io_error(std::format("{}: Could not load theme_file.\n{}", path.string(), e.what()));
    }
}

[[nodiscard]] void theme_file::parse_data(datum const& data)
{
    hi_check(holds_alternative<datum::map_type>(data), "Expecting an object as the top level of a theme_file file.");
    for (hilet & [ item_name, item_value ] : get<datum::map_type>(data)) {
        hi_check(holds_alternative<std::string>(item_name), "Expecting a string as keys in the theme_file file, got {}", name);

        if (item_name == "name") {
            hi_check(
                holds_alternative<std::string>(item_value),
                "Expecting a string as the value for 'name' in theme_file file, got {}",
                item_value);

            name = get<std::string>(item_value);

        } else if (item_name == "mode") {
            hi_check(
                holds_alternative<std::string>(item_value),
                "Expecting a string as the value for 'mode' in theme_file file, got {}",
                item_value);

            if (item_value == "light") {
                mode = theme_mode::light;
            } else if (item_value == "dark") {
                mode = theme_mode::dark;
            } else {
                throw parse_error(std::format(
                    "Expecting either 'dark' or 'light' as values for 'mode' in the theme_file file, got {}", item_value));
            }

        } else {
            // All other names are for theme_file values.
            // Use insert here so that exceptions from parse_theme_value do not cause the item to be created.
            _items.push_back({get<std::string>(item_name), parse_theme_value(item_value)});
        }
    }
}

void theme_file::activate() const noexcept
{
    hi_log_info("Activating theme {}", *this);

    detail::theme_value_base<hi::color>::reset();
    detail::theme_value_base<float>::reset();
    detail::theme_value_base<text_theme>::reset();

    // The items are sorted by least specific first.
    // That way more specific items will override the more specific theme_file values.
    //(hilet & [ item_name, item_value ] : _items) {
    //if (auto colors = std::get_if<std::vector<hi::color>>(&item_value)) {
    //    for (auto& theme_value : detail::theme_value_base<std::vector<hi::color>>::get(item_name)) {
    //        hi_not_implemented();
    //    }
    //
    //} else if (auto size = std::get_if<float>(&item_value)) {
    //    for (auto& theme_value : detail::theme_value_base<float>::get(item_name)) {
    //        theme_value.set(*size);
    //    }
    //
    //} else if (auto text_styles = std::get_if<std::vector<text_style>>(&item_value)) {
    //    hi_not_implemented();
    //
    //} else {
    //    hi_no_default();
    //}
    //}

    detail::theme_value_base<hi::color>::log();
    detail::theme_value_base<float>::log();
    detail::theme_value_base<text_theme>::log();
}

void theme_file::parse(datum const& data)
{
    parse_data(data);
    resolve_theme_references(_items);
    order_by_specificity(_items);
}

}} // namespace hi::v1
