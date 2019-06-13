// Copyright 2019 Pokitec
// All rights reserved.

#include "TrueTypeParser.hpp"
#include "Fonts.hpp"
#include "TTauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri::Draw;

TEST(TrueTypeParserTest, ParseTest) {
    let font = parseTrueTypeFile(std::filesystem::path("Draw/TestFiles/Roboto-Regular.ttf"));
}