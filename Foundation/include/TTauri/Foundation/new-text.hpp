
#include "TTauri/Foundation/globals.hpp"
#include <string>
#include <vector>

namespace TTauri {

class grapheme {
    uint64_t value;

public:
    grapheme() noexcept : value(0) {}

    grapheme(grapheme const &other) noexcept {
        if (other.size() <= 1) {
            value = other.value;
        } else {
            auto ptr = new char32_t[other.size()];
            std::memcpy(ptr, other.getPointer(), other.size() * sizeof(char32_t));

            auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
            value = (other.size() << 48) | (iptr & 0x0000ffff'ffffffff);
        }
    }

    grapheme(grapheme &&other) noexcept {
        value = other.value;
        other.value = 0;
    }

    grapheme &operator=(grapheme const &other) noexcept {
        deletePointer();
        if (other.size() <= 1) {
            value = other.value;
        } else {
            auto ptr = new char32_t[other.size()];
            std::memcpy(ptr, other.getPointer(), other.size() * sizeof(char32_t));

            auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
            value = (other.size() << 48) | (iptr & 0x0000ffff'ffffffff);
        }
        return *this;
    }

    grapheme(grapheme &&other) noexcept {
        value = other.value;
        other.value = 0;
    }

    grapheme &operator=(grapheme &&other) noexcept {
        deletePointer();
        value = other.value;
        other.value = 0;
        return *this;
    }

    ~grapheme() {
        deletePointer();
    }

    explicit grapheme(std::u32string_view other) noexcept {
        if (other.size() == 0) {
            value = 0;
        } else if (other.size() == 1) {
            value = (1ULL << 48) | other[0];
        } else {
            axiom_assert(other.size() <= std::numeric_limits<uint16_t>::max());

            auto ptr = new char32_t[other.size()];
            std::memcpy(ptr, other.data(), other.size() * sizeof(char32_t));

            auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
            value = (other.size() << 48) | (iptr & 0x0000ffff'ffffffff);
        }
    }

    explicit operator std::u32string () const noexcept {
        if (size() == 0) {
            return {};
        } else if (size() == 1) {
            return {1, static_cast<char32_t>(value)};
        } else {
            return {getPointer(), size()};
        }
    }

    size_t size() const noexcept {
        return value >> 48;
    }

    std::u32string toNFD() const noexcept {
        return Foundation_globals->unicodeData->toNFD(static_cast<std::u32string>(*this));
    }

    std::u32string toNFC() const noexcept {
        return Foundation_globals->unicodeData->toNFC(static_cast<std::u32string>(*this));
    }

    std::u32string toNFKD() const noexcept {
        return Foundation_globals->unicodeData->toNFKD(static_cast<std::u32string>(*this));
    }

    std::u32string toNFKC() const noexcept {
        return Foundation_globals->unicodeData->toNFKC(static_cast<std::u32string>(*this));
    }

private:
    bool hasPointer() noexcept {
        return size() >= 2;
    }

    char32_t *getPointer() noexcept {
        auto iptr = (static_cast<int64_t>(value << 16) >> 16);
        return std::launder(reinterpret_cast<char32_t *>(iptr));
    }

    void deletePointer() noexcept {
        if (hasPointer()) {
            auto ptr = getPointer();
            delete [] ptr;
        }
    }
};

class text_style {
    int id;
    Font font;
    float fontSize;
    wsRGBA color;
    bool inverse;
    bool underlined;
    bool strike-through;
    bool blink;
};

class theme {
    Font fallbackFont;
    std::array<text_style,256> text_style;

};

/*! Editable text for GUI Widgets.
 *
 * The upper 11 bits of the first code point of a grapheme are used
 * as the style index for the style to use to render the grapheme.
 */
class text {
    std::vector<grapheme> graphemes;

    auto begin() noexcept {
        return graphemes.begin();
    }

    auto end() noexcept {
        return graphemes.end();
    }

    size_t size() const noexcept {
        return graphemes.size();
    }

    grapheme &operator[](size_t i) noexcept {
        return graphemes[i];
    }

    /*! Find the nearest character at position and return it's index.
     */
    size_t characterIndexAtPosition(glm::vec2 position) const noexcept;

    /*! Find the nearest break between characters at position and return the index of the character after the break.
     */
    size_t breakIndexAtPosition(glm::vec2 position) const noexcept;

    /*! Return the position of the character.
     */
    glm::vec2 positionAtIndex(size_t index) const noexcept;
};


}

