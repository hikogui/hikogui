// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "attributes.hpp"
#include "Path.hpp"

namespace TTauri::Draw {

struct PathString {
    std::vector<Path> paths;

    Alignment alignment = Alignment::BaseLeft;

    PathString() {}

    PathString(std::initializer_list<Path> l) : paths(l) {}

    PathString(PathString const &other) : paths(other.paths), alignment(other.alignment) {}

    PathString(PathString &&other) : paths(std::move(other.paths)), alignment(other.alignment) {}

    void operator=(PathString const &other) {
        paths = other.paths;
        alignment = other.alignment;
    }

    void operator=(PathString &&other) {
        paths = std::move(other.paths);
        alignment = other.alignment;
    }

    size_t size() const {
        return paths.size();
    }

    Path const &at(size_t i) const {
        return paths.at(i);
    }

    void add(Path glyph) {
        paths.push_back(std::move(glyph));
    }

    glm::vec2 glyphAdvance(size_t i) const {
        return at(i).advance;
    }

    /*! Total width of the text.
     * can be called before positionGlyphs().
     */
    glm::vec2 advance() const;

    glm::vec2 ascender() const;

    glm::vec2 descender() const;

    glm::vec2 capHeight() const;

    /*! Find the start position with a specific alignment.
     */
    glm::vec2 getStartPosition() const;

    /*! Get the cursor position at grapheme index.
     */
    glm::vec2 cursorAdvance(size_t graphemeIndex) const;

    Path toPath(wsRGBA defaultColor={0.0, 0.0, 0.0, 1.0}) const;
};

PathString operator*(glm::mat3x3 const &lhs, PathString rhs);

PathString &operator*=(PathString &lhs, glm::mat3x3 const &rhs);

PathString operator+(Alignment lhs, PathString rhs);

PathString operator+(PathString lhs, Alignment rhs);

PathString &operator+=(PathString &lhs, Alignment rhs);

}