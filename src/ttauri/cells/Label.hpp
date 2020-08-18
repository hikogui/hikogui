
#pragma once

#include "../URL.hpp"
#include "Image.hpp"
#include <fmt/format.h>
#include <string>
#include <ostream>

namespace tt {

class Label {
    std::u8string _text;
    Image _icon;

public:
    Label(std::u8string text, Image &&icon = Image{}) noexcept :
        _text(text), _icon(std::move(icon)) {}

    Label(Image &&icon) noexcept :
        _text(), _icon(std::move(icon)) {}

    Label(Label const &other) noexcept:
        _text(other._text), _icon(other._icon) {}

    Label(Label &&) noexcept = default;

    Label &operator=(Label const &other) {
        _text = other._text;
        _icon = other._icon;
        return *this;
    }

    Label &operator=(Label &&) noexcept = default;

    /** Get the text translated in the current locale.
     */
    [[nodiscard]] std::u8string text() const noexcept
    {
        return _text;
    }

    /** Get the text translated in the current locale.
    */
    [[nodiscard]] Image const &icon() const noexcept {
        return _icon;
    }

    [[nodiscard]] friend std::string to_string(Label const &rhs) noexcept
    {
        return fmt::format("label:{}", tt::to_string(rhs._text));
    }

    friend std::ostream &operator<<(std::ostream &lhs, Label const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
