// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "SHA2.hpp"
#include "base_n.hpp"
#include "../utility/utility.hpp"
#include "../algorithm/algorithm.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(SHA2_suite) {

template<typename T>
static std::string test_sha2(hi::bstring value)
{
    auto hash = T();
    hash.add(value);

    return hi::to_lower(hi::base16::encode(hash.get_bytes()));
}

template<typename T>
static std::string test_sha2(std::string value)
{
    return test_sha2<T>(hi::to_bstring(value));
}

TEST_CASE(empty)
{
    REQUIRE(test_sha2<hi::SHA224>("") == "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f");
    REQUIRE(test_sha2<hi::SHA256>("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    REQUIRE(
        test_sha2<hi::SHA384>("") ==
        "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
    REQUIRE(
        test_sha2<hi::SHA512>("") ==
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327a"
        "f927da3e");
    REQUIRE(test_sha2<hi::SHA512_224>("") == "6ed0dd02806fa89e25de060c19d3ac86cabb87d6a0ddd05c333b84f4");
    REQUIRE(test_sha2<hi::SHA512_256>("") == "c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a");
}

TEST_CASE(NESSIE256Set1)
{
    REQUIRE(test_sha2<hi::SHA256>("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    REQUIRE(test_sha2<hi::SHA256>("a") == "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");

    REQUIRE(test_sha2<hi::SHA256>("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    REQUIRE(test_sha2<hi::SHA256>("message digest") == "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650");

    REQUIRE(
        test_sha2<hi::SHA256>("abcdefghijklmnopqrstuvwxyz") == "71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73");

    REQUIRE(
        test_sha2<hi::SHA256>("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
        "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    REQUIRE(
        test_sha2<hi::SHA256>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ==
        "db4bfcbd4da0cd85a60c3c37d3fbd8805c77f15fc6b1fdfe614ee0a7c8fdb4c0");

    REQUIRE(
        test_sha2<hi::SHA256>("12345678901234567890123456789012345678901234567890123456789012345678901234567890") ==
        "f371bc4a311f2b009eef952dd83ca80e2b60026c8e935592d0f9c308453c813e");

    REQUIRE(test_sha2<hi::SHA256>(std::string(1'000'000, 'a')) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

    // Same test, but with chunks of 10 characters.
    auto h = hi::SHA256();
    for (int i = 0; i != 99'999; i++) {
        h.add(hi::to_bstring("aaaaaaaaaa"), false);
    }
    h.add(hi::to_bstring("aaaaaaaaaa"));
    REQUIRE(hi::to_lower(hi::base16::encode(h.get_bytes())) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

TEST_CASE(NESSIE512Set1)
{
    REQUIRE(
        test_sha2<hi::SHA512>("") ==
        "cf83e1357eefb8bdf1542850d66d8007"
        "d620e4050b5715dc83f4a921d36ce9ce"
        "47d0d13c5d85f2b0ff8318d2877eec2f"
        "63b931bd47417a81a538327af927da3e");

    REQUIRE(
        test_sha2<hi::SHA512>("a") ==
        "1f40fc92da241694750979ee6cf582f2"
        "d5d7d28e18335de05abc54d0560e0f53"
        "02860c652bf08d560252aa5e74210546"
        "f369fbbbce8c12cfc7957b2652fe9a75");

    REQUIRE(
        test_sha2<hi::SHA512>("abc") ==
        "ddaf35a193617abacc417349ae204131"
        "12e6fa4e89a97ea20a9eeee64b55d39a"
        "2192992a274fc1a836ba3c23a3feebbd"
        "454d4423643ce80e2a9ac94fa54ca49f");

    REQUIRE(
        test_sha2<hi::SHA512>("message digest") ==
        "107dbf389d9e9f71a3a95f6c055b9251"
        "bc5268c2be16d6c13492ea45b0199f33"
        "09e16455ab1e96118e8a905d5597b720"
        "38ddb372a89826046de66687bb420e7c");

    REQUIRE(
        test_sha2<hi::SHA512>("abcdefghijklmnopqrstuvwxyz") ==
        "4dbff86cc2ca1bae1e16468a05cb9881"
        "c97f1753bce3619034898faa1aabe429"
        "955a1bf8ec483d7421fe3c1646613a59"
        "ed5441fb0f321389f77f48a879c7b1f1");

    REQUIRE(
        test_sha2<hi::SHA512>("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
        "204a8fc6dda82f0a0ced7beb8e08a416"
        "57c16ef468b228a8279be331a703c335"
        "96fd15c13b1b07f9aa1d3bea57789ca0"
        "31ad85c7a71dd70354ec631238ca3445");

    REQUIRE(
        test_sha2<hi::SHA512>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ==
        "1e07be23c26a86ea37ea810c8ec78093"
        "52515a970e9253c26f536cfc7a9996c4"
        "5c8370583e0a78fa4a90041d71a4ceab"
        "7423f19c71b9d5a3e01249f0bebd5894");

    REQUIRE(
        test_sha2<hi::SHA512>("12345678901234567890123456789012345678901234567890123456789012345678901234567890") ==
        "72ec1ef1124a45b047e8b7c75a932195"
        "135bb61de24ec0d1914042246e0aec3a"
        "2354e093d76f3048b456764346900cb1"
        "30d2a4fd5dd16abb5e30bcb850dee843");

    REQUIRE(
        test_sha2<hi::SHA512>(std::string(1'000'000, 'a')) ==
        "e718483d0ce769644e2e42c7bc15b463"
        "8e1f98b13b2044285632a803afa973eb"
        "de0ff244877ea60a4cb0432ce577c31b"
        "eb009c5c2c49aa2e4eadb217ad8cc09b");

    // Same test, but with chunks of 10 characters.
    auto h = hi::SHA512();
    for (int i = 0; i != 99'999; i++) {
        h.add(hi::to_bstring("aaaaaaaaaa"), false);
    }
    h.add(hi::to_bstring("aaaaaaaaaa"));
    REQUIRE(
        hi::to_lower(hi::base16::encode(h.get_bytes())) ==
        "e718483d0ce769644e2e42c7bc15b463"
        "8e1f98b13b2044285632a803afa973eb"
        "de0ff244877ea60a4cb0432ce577c31b"
        "eb009c5c2c49aa2e4eadb217ad8cc09b");
}

}; // TEST_SUITE(SHA2_suite)
