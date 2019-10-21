// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/PathString.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/required.hpp"

#include <string>
#include <vector>
#include <memory>

namespace TTauri {

enum class ThemeFontStyle {
    DEFAULT,
};

struct FontStyle {
    std::shared_ptr<Font> font;
    float fontSize;
    wsRGBA color;
};

struct Theme {
	std::string name;

    std::vector<FontStyle> fontStyles;

    PathString getGlyhs(gstring const &text, ThemeFontStyle font) noexcept;
    //void render(PixelMap<uint32_t> &pixels, gstring const &text, ThemeFontStyle font, float dpi, glm::vec2 position, float angle);

	static void loadAllThemes(const URL &fontDirectory, const URL &iconDirectory);
};

extern Theme *selectedTheme;
extern std::vector<std::shared_ptr<Theme>> themes;

}
