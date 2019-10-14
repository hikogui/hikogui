
#include <vector>

namespace TTauri {

class Text_code_unit_iterator {

};

class Text_grapheme_iterator {

};

class Text {
    std::vector<char32_t> str;

    /*! Get a UTF-32 code unit.
     */
    char32_t getCodeUnit(size_t index);


    size_t numberOfGraphemes() const noexcept;

    /*! Get a grapheme.
     * A grapheme is a user editable piece of text.
     * Specificaly this excludes ligatures.
     */
    Grapheme getGrapheme(size_t index) const noexcept;

    void setGrapheme(size_t index, Grapheme grapheme) noexcept;

    void removeGrapheme(size_t index) noexcept;

    void insertGrapheme(size_t index, Grapheme grapheme) noexcept;

    /*! Get a glyph.
     * A glyph is a character that is drawn in the screen
     * Specifically this include ligatures such as:
     *    ff, fi, ffi, fl, ffl
     */
    Glyph getGlyph(size_t index);

}


}

