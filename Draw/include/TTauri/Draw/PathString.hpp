// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Draw/attributes.hpp"
#include "TTauri/Draw/Path.hpp"
#include "TTauri/Required/numeric_cast.hpp"

namespace TTauri::Draw {

struct PathString {
    std::vector<Path> paths;

    Alignment alignment = Alignment::BaseLeft;

    PathString() = default;
    ~PathString() = default;

    PathString(std::initializer_list<Path> l) noexcept: paths(l) {}

    PathString(PathString const &other) noexcept : paths(other.paths), alignment(other.alignment) {}

    PathString(PathString &&other) noexcept : paths(std::move(other.paths)), alignment(other.alignment) {}

    void operator=(PathString const &other) noexcept {
        paths = other.paths;
        alignment = other.alignment;
    }

    void operator=(PathString &&other) noexcept {
        paths = std::move(other.paths);
        alignment = other.alignment;
    }

    ssize_t size() const noexcept {
        return to_signed(paths.size());
    }

    Path const &at(int i) const noexcept {
        return paths.at(i);
    }

    void add(Path glyph) noexcept {
        paths.push_back(std::move(glyph));
    }

    glm::vec2 glyphAdvance(int i) const noexcept {
        return at(i).advance;
    }

    /*! Total width of the text.
     * can be called before positionGlyphs().
     */
    glm::vec2 advance() const noexcept;

    glm::vec2 ascender() const noexcept;

    glm::vec2 descender() const noexcept;

    glm::vec2 capHeight() const noexcept;

    /*! Find the start position with a specific alignment.
     */
    glm::vec2 getStartPosition() const noexcept;

    /*! Get the cursor position at grapheme index.
     */
    glm::vec2 cursorAdvance(int graphemeIndex) const noexcept;

    Path toPath(wsRGBA defaultColor={0.0, 0.0, 0.0, 1.0}) const noexcept;
};

PathString operator*(glm::mat3x3 const &lhs, PathString rhs) noexcept;

PathString &operator*=(PathString &lhs, glm::mat3x3 const &rhs) noexcept;

PathString operator+(Alignment lhs, PathString rhs) noexcept;

PathString operator+(PathString lhs, Alignment rhs) noexcept;

PathString &operator+=(PathString &lhs, Alignment rhs) noexcept;

}
