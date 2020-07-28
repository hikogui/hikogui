// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/cell_address.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

using namespace std;
using namespace tt;

TEST(cell_address, simple_literals) {
    ASSERT_EQ(to_string(""_ca),     "");
    ASSERT_EQ(to_string("L4"_ca),   "L4");
    ASSERT_EQ(to_string("L42"_ca),  "L42");
    ASSERT_EQ(to_string("L+4"_ca),  "L+4");
    ASSERT_EQ(to_string("L+42"_ca), "L+42");
    ASSERT_EQ(to_string("L-4"_ca),  "L-4");
    ASSERT_EQ(to_string("L-42"_ca), "L-42");
    ASSERT_EQ(to_string("R4"_ca),   "R4");
    ASSERT_EQ(to_string("R42"_ca),  "R42");
    ASSERT_EQ(to_string("R+4"_ca),  "R+4");
    ASSERT_EQ(to_string("R+42"_ca), "R+42");
    ASSERT_EQ(to_string("R-4"_ca),  "R-4");
    ASSERT_EQ(to_string("R-42"_ca), "R-42");
    ASSERT_EQ(to_string("B4"_ca),   "B4");
    ASSERT_EQ(to_string("B42"_ca),  "B42");
    ASSERT_EQ(to_string("B+4"_ca),  "B+4");
    ASSERT_EQ(to_string("B+42"_ca), "B+42");
    ASSERT_EQ(to_string("B-4"_ca),  "B-4");
    ASSERT_EQ(to_string("B-42"_ca), "B-42");
    ASSERT_EQ(to_string("T4"_ca),   "T4");
    ASSERT_EQ(to_string("T42"_ca),  "T42");
    ASSERT_EQ(to_string("T+4"_ca),  "T+4");
    ASSERT_EQ(to_string("T+42"_ca), "T+42");
    ASSERT_EQ(to_string("T-4"_ca),  "T-4");
    ASSERT_EQ(to_string("T-42"_ca), "T-42");
    ASSERT_EQ(to_string("L:4"_ca),  "L:4");
    ASSERT_EQ(to_string("L:42"_ca), "L:42");
    ASSERT_EQ(to_string("R:4"_ca),  "R:4");
    ASSERT_EQ(to_string("R:42"_ca), "R:42");
    ASSERT_EQ(to_string("T:4"_ca),  "T:4");
    ASSERT_EQ(to_string("T:42"_ca), "T:42");
    ASSERT_EQ(to_string("B:4"_ca),  "B:4");
    ASSERT_EQ(to_string("B:42"_ca), "B:42");
}

TEST(cell_address, complex_literals) {
    ASSERT_EQ(to_string("L23T45"_ca),   "L23T45");
    ASSERT_EQ(to_string("T45L23"_ca),   "L23T45");
    ASSERT_EQ(to_string("L+23T-45"_ca), "L+23T-45");
    ASSERT_EQ(to_string("L+23R-45"_ca), "R-45");
    ASSERT_EQ(to_string("L+23R45"_ca),  "R45");
    ASSERT_EQ(to_string("L-23R45"_ca),  "R45");
    ASSERT_EQ(to_string("L-23R+45"_ca), "R+45");

    ASSERT_EQ(to_string("L23:45"_ca),   "L23:45");
    ASSERT_EQ(to_string("L23T:45"_ca),  "L23T:45");
    ASSERT_EQ(to_string("L:45R23"_ca),  "R23:45");
    ASSERT_EQ(to_string("L45R:23"_ca),  "R45:23");

    ASSERT_EQ(to_string("L23:15T45:26"_ca),   "L23:15T45:26");

    ASSERT_EQ(to_string("L23:45:5"_ca),   "L23:45:5");
    ASSERT_EQ(to_string("L23T:45:3"_ca),  "L23T:45:3");
    ASSERT_EQ(to_string("L:45:6R23"_ca),  "R23:45:6");
    ASSERT_EQ(to_string("L45R:23:9"_ca),  "R45:23:9");

    ASSERT_EQ(to_string("L23:15:4T45:26:3"_ca),   "L23:15:4T45:26:3");
}

TEST(cell_address, transform) {
    ASSERT_EQ(""_ca    * "L4B5"_ca, "L4B5"_ca);
    ASSERT_EQ(""_ca    * "L4:2B5:3"_ca, "L4B5"_ca);

    ASSERT_EQ("L+1"_ca * "L4B5"_ca, "L5B5"_ca);
    ASSERT_EQ("R+1"_ca * "L4B5"_ca, "L3B5"_ca);
    ASSERT_EQ("T+1"_ca * "L4B5"_ca, "L4B4"_ca);
    ASSERT_EQ("B+1"_ca * "L4B5"_ca, "L4B6"_ca);
    ASSERT_EQ("L-1"_ca * "L4B5"_ca, "L3B5"_ca);
    ASSERT_EQ("R-1"_ca * "L4B5"_ca, "L5B5"_ca);
    ASSERT_EQ("T-1"_ca * "L4B5"_ca, "L4B6"_ca);
    ASSERT_EQ("B-1"_ca * "L4B5"_ca, "L4B4"_ca);

    ASSERT_EQ("L1"_ca * "L4B5"_ca, "L1B5"_ca);
    ASSERT_EQ("R1"_ca * "L4B5"_ca, "R1B5"_ca);
    ASSERT_EQ("T1"_ca * "L4B5"_ca, "L4T1"_ca);
    ASSERT_EQ("B1"_ca * "L4B5"_ca, "L4B1"_ca);

    ASSERT_EQ("L+1"_ca * "L+4B-5"_ca, "L+5B-5"_ca);
    ASSERT_EQ("R+1"_ca * "L+4B-5"_ca, "L+3B-5"_ca);
    ASSERT_EQ("T+1"_ca * "L+4B-5"_ca, "L+4B-6"_ca);
    ASSERT_EQ("B+1"_ca * "L+4B-5"_ca, "L+4B-4"_ca);

    ASSERT_EQ("L:2"_ca * "L4B5"_ca, "L4:2B5"_ca);
    ASSERT_EQ("R:2"_ca * "L4B5"_ca, "L4:2B5"_ca);
    ASSERT_EQ("T:2"_ca * "L4B5"_ca, "L4B5:2"_ca);
    ASSERT_EQ("B:2"_ca * "L4B5"_ca, "L4B5:2"_ca);
    ASSERT_EQ("L:3:2"_ca * "L4B5"_ca, "L4:3:2B5"_ca);
    ASSERT_EQ("R:3:2"_ca * "L4B5"_ca, "L4:3:2B5"_ca);
    ASSERT_EQ("T:3:2"_ca * "L4B5"_ca, "L4B5:3:2"_ca);
    ASSERT_EQ("B:3:2"_ca * "L4B5"_ca, "L4B5:3:2"_ca);

    ASSERT_EQ("L+1:2"_ca * "L4B5"_ca, "L5:2B5"_ca);
    ASSERT_EQ("R+1:2"_ca * "L4B5"_ca, "L3:2B5"_ca);
    ASSERT_EQ("T+1:2"_ca * "L4B5"_ca, "L4B4:2"_ca);
    ASSERT_EQ("B+1:2"_ca * "L4B5"_ca, "L4B6:2"_ca);
    ASSERT_EQ("L+1:3:2"_ca * "L4B5"_ca, "L5:3:2B5"_ca);
    ASSERT_EQ("R+1:3:2"_ca * "L4B5"_ca, "L3:3:2B5"_ca);
    ASSERT_EQ("T+1:3:2"_ca * "L4B5"_ca, "L4B4:3:2"_ca);
    ASSERT_EQ("B+1:3:2"_ca * "L4B5"_ca, "L4B6:3:2"_ca);
}
