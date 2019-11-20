// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Config/ASTObject.hpp"
#include "TTauri/Config/parser.hpp"
#include <gtest/gtest.h>

using namespace std;
using namespace TTauri;
using namespace TTauri::Config;

TEST(Config_Parser, EmptyFile) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:empty.txt")));
    ASSERT_EQ(to_string(*o), "{}");
    ASSERT_EQ(to_string(o->execute()), "{}");
}

TEST(Config_Parser, Assignments) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:assignments.txt")));
    ASSERT_EQ(to_string(*o), "{foo:\"Hello World\",bar:1}");
    ASSERT_EQ(to_string(o->execute()), "{\"bar\": 1, \"foo\": \"Hello World\"}");
}

TEST(Config_Parser, Integers) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:integers.txt")));
    ASSERT_EQ(to_string(*o), "{a:0,b:1,c:10,d:2,e:8,f:10,g:16,h:0,i:-1,j:-10,k:-2,l:-8,m:-10,n:-16,o:-10,p:-2,q:-8,r:-10,s:-16}");
    ASSERT_EQ(to_string(o->execute()), "{\"a\": 0, \"b\": 1, \"c\": 10, \"d\": 2, \"e\": 8, \"f\": 10, \"g\": 16, "s +
        "\"h\": 0, \"i\": -1, \"j\": -10, \"k\": -2, \"l\": -8, \"m\": -10, \"n\": -16, \"o\": -10, \"p\": -2, \"q\": -8, \"r\": -10, \"s\": -16}");
}

TEST(Config_Parser, Floats) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:floats.txt")));
    ASSERT_EQ(to_string(*o), "{a:0.,b:-0.,c:1.,d:-1.,e:0.,f:-0.,g:0.1,h:-0.1,i:0.,j:-0.,k:1.,l:-1.}");
    ASSERT_EQ(to_string(o->execute()), "{\"a\": 0.0, \"b\": -0.0, \"c\": 1.0, \"d\": -1.0, \"e\": 0.0, \"f\": -0.0, \"g\": 0.1, "s
        "\"h\": -0.1, \"i\": 0.0, \"j\": -0.0, \"k\": 1.0, \"l\": -1.0}");
}

TEST(Config_Parser, Colors) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:colors.txt")));

    auto parse_r = "{a:#012345ff,b:#6889abcd}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"a\": <wsRGBA #012345ff>, \"b\": <wsRGBA #6889abcd>}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, Booleans) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:booleans.txt")));
    ASSERT_EQ(to_string(*o), "{a:true,b:false,c:null}");
    ASSERT_EQ(to_string(o->execute()), "{\"a\": true, \"b\": false, \"c\": null}");
}

TEST(Config_Parser, Arrays) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:arrays.txt")));
    ASSERT_EQ(to_string(*o), "{foo:[],bar:[1],baz:[1,2],bob:[1,2],a[]:3}");
    ASSERT_EQ(to_string(o->execute()), "{\"a\": [3], \"bar\": [1], \"baz\": [1, 2], \"bob\": [1, 2], \"foo\": []}");
}

TEST(Config_Parser, ObjectsSimple) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:objects_simple.txt")));
    ASSERT_EQ(to_string(*o), "{[z],w:3}");
    ASSERT_EQ(to_string(o->execute()), "{\"z\": {\"w\": 3}}");
}

TEST(Config_Parser, Objects) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:objects.txt")));
    ASSERT_EQ(to_string(*o), "{foo:{a:1,\"b\":2},bar.baz:5,[hello],world:\"World\",[z],w:3}");
    ASSERT_EQ(to_string(o->execute()), "{\"bar\": {\"baz\": 5}, \"foo\": {\"a\": 1, \"b\": 2}, \"hello\": {\"world\": \"World\"}, \"z\": {\"w\": 3}}");
}

TEST(Config_Parser, JSON) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:json.txt")));
    ASSERT_EQ(to_string(*o), "{\"a\":1,\"b\":\"foo\",\"c\":1.1,\"d\":[1,2,3],\"e\":{\"a\":1,\"b\":1.1}}");
    ASSERT_EQ(to_string(o->execute()), "{\"a\": 1, \"b\": \"foo\", \"c\": 1.1, \"d\": [1, 2, 3], \"e\": {\"a\": 1, \"b\": 1.1}}");
}

TEST(Config_Parser, IntegerExpressions) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:integer_expressions.txt")));

    auto parse_r = "{a:35,b:3,c:a+b,d:a-3,e:35*b,f:35/3,g:35%3,h:35&3,i:35|3,j:35^3,k:35<<3,l:35>>3,"s + 
                    "m:35<3,n:35>3,o:35<=3,p:35>=3,q:35==3,r:35!=3,s:35 and 3,t:35 or 3,u:35 xor 3,v:~35,w:-35,x:not 35,y:--35}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"a\": 35, \"b\": 3, \"c\": 38, \"d\": 32, \"e\": 105, \"f\": 11, \"g\": 2, \"h\": 3, \"i\": 35, \"j\": 32, "s +
                  "\"k\": 280, \"l\": 4, "s +
                  "\"m\": false, \"n\": true, \"o\": false, \"p\": true, \"q\": false, \"r\": true, \"s\": true, \"t\": true, "s +
                  "\"u\": false, \"v\": -36, \"w\": -35, \"x\": false, \"y\": 35}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, StringExpressions) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:string_expressions.txt")));

    auto parse_r = "{a:\"Hello\",b:\"World\",c:a+b,d:a<b,e:a>b,f:a<=b,g:a>=b,h:a==b,i:a!=b,j:a and b,"s +
        "k:a or b,l:a xor b,m:not a,n:not \"\",o:a and \"\",p:a or \"\",q:a xor \"\"}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"a\": \"Hello\", \"b\": \"World\", \"c\": \"HelloWorld\", \"d\": true, \"e\": false, \"f\": true, \"g\": false, "s +
        "\"h\": false, \"i\": true, \"j\": true, "s +
        "\"k\": true, \"l\": false, \"m\": false, \"n\": true, \"o\": false, \"p\": true, \"q\": true}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, BooleanExpressions) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:boolean_expressions.txt")));

    auto parse_r =
        "{m:true and true,n:true and false,o:false and true,p:false and false,q:true or true,"s +
        "r:true or false,s:false or true,t:false or false,u:true xor true,"s +
        "v:true xor false,w:false xor true,x:false xor false,"s +
        "za:not true,zb:not false}"s;
  
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r =
        "{\"m\": true, \"n\": false, \"o\": false, \"p\": false, \"q\": true, \"r\": true, \"s\": true, "s
        "\"t\": false, \"u\": false, \"v\": true, \"w\": true, \"x\": false, "s +
        "\"za\": false, \"zb\": true}"s;

    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, FloatExpressions) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:float_expressions.txt")));

    auto parse_r =
        "{a:35.,b:3.,ac:a+b,ad:a-b,ae:a*b,af:a/b,ag:a%b,"s +
        "ah:a<b,ai:a>b,aj:a<=b,ak:a>=b,al:a==b,am:a!=b,"s +
        "an:a and b,ao:a or b,ap:a xor b,aq:-a,ar:not a,as:--35.,"s +
        "a:35.,b:3,bc:a+b,bd:a-b,be:a*b,bf:a/b,bg:a%b,"s +
        "bh:a<b,bi:a>b,bj:a<=b,bk:a>=b,bl:a==b,bm:a!=b,"s +
        "bn:a and b,bo:a or b,bp:a xor b,"s +
        "a:35,b:3.,cc:a+b,cd:a-b,ce:a*b,cf:a/b,cg:a%b,"s +
        "ch:a<b,ci:a>b,cj:a<=b,ck:a>=b,cl:a==b,cm:a!=b,"s +
        "cn:a and b,co:a or b,cp:a xor b}"s;

    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r =
        "{\"a\": 35, \"ac\": 38.0, \"ad\": 32.0, \"ae\": 105.0, \"af\": 11.6667, \"ag\": 2.0, "s +
        "\"ah\": false, \"ai\": true, \"aj\": false, \"ak\": true, \"al\": false, \"am\": true, "s +
        "\"an\": true, \"ao\": true, \"ap\": false, \"aq\": -35.0, \"ar\": false, \"as\": 35.0, "s +
        "\"b\": 3.0, \"bc\": 38.0, \"bd\": 32.0, \"be\": 105.0, \"bf\": 11.6667, \"bg\": 2.0, "s +
        "\"bh\": false, \"bi\": true, \"bj\": false, \"bk\": true, \"bl\": false, \"bm\": true, "s +
        "\"bn\": true, \"bo\": true, \"bp\": false, "s +
        "\"cc\": 38.0, \"cd\": 32.0, \"ce\": 105.0, \"cf\": 11.6667, \"cg\": 2.0, "s +
        "\"ch\": false, \"ci\": true, \"cj\": false, \"ck\": true, \"cl\": false, \"cm\": true, "s +
        "\"cn\": true, \"co\": true, \"cp\": false}"s;
            
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, IncludeFiles) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:includer.txt")));

    auto parse_r = "{include(\"included.txt\"),a:{include(\"included.txt\")},[b],include(file:included.txt)}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"a\": {\"foo\": \"bar\"}, \"b\": {\"foo\": \"bar\"}, \"foo\": \"bar\"}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, Variables) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:variables.txt")));

    auto parse_r = "{$.a:3,$.b.foo:5,c:$.a,d:$.b}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"c\": 3, \"d\": {\"foo\": 5}}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

TEST(Config_Parser, RootAccess) {
    auto o = std::unique_ptr<ASTObject>(parseConfigFile(URL("file:root_access.txt")));

    auto parse_r = "{a:3,b:{foo:/.a,/.a:5,bar:/.a},/.b.baz:/.b.foo}"s;
    ASSERT_EQ(to_string(*o), parse_r);

    auto exec_r = "{\"a\": 5, \"b\": {\"bar\": 5, \"baz\": 3, \"foo\": 3}}"s;
    ASSERT_EQ(to_string(o->execute()), exec_r);
}

