// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Font.hpp"
#include "Path.hpp"
#include "PathString.hpp"
#include "TTauri/Color.hpp"
#include "TTauri/required.hpp"

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace TTauri::Draw {

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

    PathString getGlyhs(gstring const &text, ThemeFontStyle font);
    //void render(PixelMap<uint32_t> &pixels, gstring const &text, ThemeFontStyle font, float dpi, glm::vec2 position, float angle);

	static void loadAllThemes(const std::filesystem::path &fontDirectory, const std::filesystem::path &iconDirectory);
};

extern Theme *selectedTheme;
extern std::vector<std::shared_ptr<Theme>> themes;

}
