// Copyright 2019 Pokitec
// All rights reserved.

#include <gtest/gtest.h>
#include <Windows.h>
#include "parser.hpp"

using namespace TTauri::Config;
TEST(TTauriConfigParser, EmptyFile) {
    auto o = parseFile("Config/TestFiles/empty.txt");
    ASSERT_EQ(o->str(), "{}");
    ASSERT_EQ(o->execute().str(), "{}");
}

TEST(TTauriConfigParser, Assignments) {
    auto o = parseFile("Config/TestFiles/assignments.txt");
    ASSERT_EQ(o->str(), "{foo:\"Hello World\",bar:1}");
    ASSERT_EQ(o->execute().str(), "{bar:1,foo:\"Hello World\"}");
}

TEST(TTauriConfigParser, Integers) {
    auto o = parseFile("Config/TestFiles/integers.txt");
    ASSERT_EQ(o->str(), "{a:0,b:1,c:10,d:2,e:8,f:10,g:16,h:0,i:-1,j:-10,k:-2,l:-8,m:-10,n:-16,o:-10,p:-2,q:-8,r:-10,s:-16}");
    ASSERT_EQ(o->execute().str(), "{a:0,b:1,c:10,d:2,e:8,f:10,g:16,h:0,i:-1,j:-10,k:-2,l:-8,m:-10,n:-16,o:-10,p:-2,q:-8,r:-10,s:-16}");
}

TEST(TTauriConfigParser, Floats) {
    auto o = parseFile("Config/TestFiles/floats.txt");
    ASSERT_EQ(o->str(), "{a:0,b:-0,c:1,d:-1,e:0,f:-0,g:0.1,h:-0.1,i:0,j:-0,k:1,l:-1}");
    ASSERT_EQ(o->execute().str(), "{a:0,b:-0,c:1,d:-1,e:0,f:-0,g:0.1,h:-0.1,i:0,j:-0,k:1,l:-1}");
}

TEST(TTauriConfigParser, Colors) {
    auto o = parseFile("Config/TestFiles/colors.txt");
    ASSERT_EQ(o->str(), "{a:#012345ff,b:#6789abcd}");
    ASSERT_EQ(o->execute().str(), "{a:#012345ff,b:#6789abcd}");
}

TEST(TTauriConfigParser, Booleans) {
    auto o = parseFile("Config/TestFiles/booleans.txt");
    ASSERT_EQ(o->str(), "{a:true,b:false,c:null}");
    ASSERT_EQ(o->execute().str(), "{a:true,b:false,c:null}");
}

TEST(TTauriConfigParser, Arrays) {
    auto o = parseFile("Config/TestFiles/arrays.txt");
    ASSERT_EQ(o->str(), "{foo:[],bar:[1],baz:[1,2],bob:[1,2],a[0]:3}");
    ASSERT_EQ(o->execute().str(), "{a:[3],bar:[1],baz:[1,2],bob:[1,2],foo:[]}");
}

TEST(TTauriConfigParser, Objects) {
    auto o = parseFile("Config/TestFiles/objects.txt");
    ASSERT_EQ(o->str(), "{foo:{a:1,b:2},bar.baz:5,[hello],world:\"World\",[z],w:3}");
    ASSERT_EQ(o->execute().str(), "{bar:{baz:5},foo:{a:1,b:2},hello:{world:\"World\"},z:{w:3}}");
}
