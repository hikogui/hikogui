// Copyright 2019 Pokitec
// All rights reserved.

#include "TrueTypeParser.hpp"
#include "Font.hpp"
#include "TTauri/required.hpp"
#include "TTauri/ResourceView.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
using namespace TTauri::Draw;

TEST(TrueTypeParserTest, ParseTest) {
    let view = ResourceView(URL("resource:Draw/TestFiles/Roboto-Regular.ttf"));
    let font = parseTrueTypeFile(view.bytes());
}