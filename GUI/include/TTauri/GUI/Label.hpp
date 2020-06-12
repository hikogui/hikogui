
#pragma once

#include "TTauri/Foundation/URL.hpp"
#include <fmt/format.h>
#include <string>
#include <ostream>

namespace tt {

class Label {
    std::string _text;
    URL _iconURL;

public:
    Label(std::string text, URL iconURL) noexcept :
        _text(text), _iconURL(iconURL) {}

    Label(std::string text) noexcept :
        _text(text), _iconURL() {}

    Label(URL iconURL) noexcept :
        _text(), _iconURL(iconURL) {}

    Label(Label const &other) noexcept :
        _text(other._text), _iconURL(other._iconURL) {}

    Label(Label &&) noexcept = default;

    Label &operator=(Label const &other) noexcept {
        _text = other._text;
        _iconURL = other._iconURL;
        return *this;
    }

    Label &operator=(Label &&) noexcept = default;

    /** Get the text translated in the current locale.
     */
    [[nodiscard]] std::string text() const noexcept {
        return _text;
    }


    [[nodiscard]] friend std::string to_string(Label const &rhs) noexcept {
        return fmt::format("label:{}:{}", rhs._text, rhs._iconURL);
    }

    friend std::ostream &operator<<(std::ostream &lhs, Label const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
