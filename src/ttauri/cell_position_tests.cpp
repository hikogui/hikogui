// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/cell_position.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

using namespace std;
using namespace tt;

TEST(cell_position, simple_literals) {
    ASSERT_EQ(to_string(""_cp),     "");
    ASSERT_EQ(to_string("L4"_cp),   "L4");
    ASSERT_EQ(to_string("L42"_cp),  "L42");
    ASSERT_EQ(to_string("L+4"_cp),  "L+4");
    ASSERT_EQ(to_string("L+42"_cp), "L+42");
    ASSERT_EQ(to_string("L-4"_cp),  "L-4");
    ASSERT_EQ(to_string("L-42"_cp), "L-42");
    ASSERT_EQ(to_string("R4"_cp),   "R4");
    ASSERT_EQ(to_string("R42"_cp),  "R42");
    ASSERT_EQ(to_string("R+4"_cp),  "R+4");
    ASSERT_EQ(to_string("R+42"_cp), "R+42");
    ASSERT_EQ(to_string("R-4"_cp),  "R-4");
    ASSERT_EQ(to_string("R-42"_cp), "R-42");
    ASSERT_EQ(to_string("B4"_cp),   "B4");
    ASSERT_EQ(to_string("B42"_cp),  "B42");
    ASSERT_EQ(to_string("B+4"_cp),  "B+4");
    ASSERT_EQ(to_string("B+42"_cp), "B+42");
    ASSERT_EQ(to_string("B-4"_cp),  "B-4");
    ASSERT_EQ(to_string("B-42"_cp), "B-42");
    ASSERT_EQ(to_string("T4"_cp),   "T4");
    ASSERT_EQ(to_string("T42"_cp),  "T42");
    ASSERT_EQ(to_string("T+4"_cp),  "T+4");
    ASSERT_EQ(to_string("T+42"_cp), "T+42");
    ASSERT_EQ(to_string("T-4"_cp),  "T-4");
    ASSERT_EQ(to_string("T-42"_cp), "T-42");
    ASSERT_EQ(to_string("L:4"_cp),  "L:4");
    ASSERT_EQ(to_string("L:42"_cp), "L:42");
    ASSERT_EQ(to_string("R:4"_cp),  "R:4");
    ASSERT_EQ(to_string("R:42"_cp), "R:42");
    ASSERT_EQ(to_string("T:4"_cp),  "T:4");
    ASSERT_EQ(to_string("T:42"_cp), "T:42");
    ASSERT_EQ(to_string("B:4"_cp),  "B:4");
    ASSERT_EQ(to_string("B:42"_cp), "B:42");
}

TEST(cell_position, complex_literals) {
    ASSERT_EQ(to_string("L23T45"_cp),   "L23T45");
    ASSERT_EQ(to_string("T45L23"_cp),   "L23T45");
    ASSERT_EQ(to_string("L+23T-45"_cp), "L+23T-45");
    ASSERT_EQ(to_string("L+23R-45"_cp), "R-45");
    ASSERT_EQ(to_string("L+23R45"_cp),  "R45");
    ASSERT_EQ(to_string("L-23R45"_cp),  "R45");
    ASSERT_EQ(to_string("L-23R+45"_cp), "R+45");

    ASSERT_EQ(to_string("L23:45"_cp),   "L23:45");
    ASSERT_EQ(to_string("L23T:45"_cp),  "L23T:45");
    ASSERT_EQ(to_string("L:45R23"_cp),  "R23:45");
    ASSERT_EQ(to_string("L45R:23"_cp),  "R45:23");

    ASSERT_EQ(to_string("L23:89T45:67"_cp),   "L23:89T45:67");
}

