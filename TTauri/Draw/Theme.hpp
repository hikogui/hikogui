// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace TTauri::Draw {

struct Theme {
	std::string name;


	static void loadAllThemes(const std::filesystem::path &fontDirectory, const std::filesystem::path &iconDirectory);
};

extern std::shared_ptr<Theme> theme;
extern std::vector<std::shared_ptr<Theme>> themes;

}