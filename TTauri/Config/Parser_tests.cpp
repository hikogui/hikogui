
#include <gtest/gtest.h>

#include <Windows.h>

#include "Parser.hpp"

using namespace TTauri::Config;
TEST(ConfigParser, empty_file) {
    auto o = parseFile("Config/TestFiles/empty.txt");
    ASSERT_EQ(o->str(), "{}");
}

TEST(ConfigParser, assignment) {
    auto o = parseFile("Config/TestFiles/assignment.txt");
    ASSERT_EQ(o->str(), "{foo:\"Hello World\"}");
}

TEST(ConfigParser, integers) {
    auto o = parseFile("Config/TestFiles/integers.txt");
    ASSERT_EQ(o->str(), "{a:0,b:1,c:10,d:2,e:8,f:10,g:16,h:0,i:-1,j:-10,k:-2,l:-8,m:-10,n:-16,o:-10,p:-2,q:-8,r:-10,s:-16}");
}

TEST(ConfigParser, floats) {
    auto o = parseFile("Config/TestFiles/floats.txt");
    ASSERT_EQ(o->str(), "{a:0,b:-0,c:1,d:-1,e:0,f:-0,g:0.1,h:-0.1,i:0,j:-0,k:1,l:-1}");
}

TEST(ConfigParser, list) {
    auto o = parseFile("Config/TestFiles/list.txt");
    ASSERT_EQ(o->str(), "{foo:[],bar:[1],baz:[1,2],bob:[1,2]}");
}
