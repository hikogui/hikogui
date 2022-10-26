// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "packed_int_array.hpp"
#include "scoped_task.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(packed_int_array, bits_1)
{
    auto a = packed_int_array<1, 20>{0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1};
    // clang-format off
    ASSERT_EQ(a[0],  0);  ASSERT_EQ(get<0>(a), 0);
    ASSERT_EQ(a[1],  1);  ASSERT_EQ(get<1>(a), 1);
    ASSERT_EQ(a[2],  0);  ASSERT_EQ(get<2>(a), 0);
    ASSERT_EQ(a[3],  0);  ASSERT_EQ(get<3>(a), 0);
    ASSERT_EQ(a[4],  1);  ASSERT_EQ(get<4>(a), 1);
    ASSERT_EQ(a[5],  1);  ASSERT_EQ(get<5>(a), 1);
    ASSERT_EQ(a[6],  0);  ASSERT_EQ(get<6>(a), 0);
    ASSERT_EQ(a[7],  0);  ASSERT_EQ(get<7>(a), 0);
    ASSERT_EQ(a[8],  0);  ASSERT_EQ(get<8>(a), 0);
    ASSERT_EQ(a[9],  0);  ASSERT_EQ(get<9>(a), 0);
    ASSERT_EQ(a[10], 1);  ASSERT_EQ(get<10>(a), 1);
    ASSERT_EQ(a[11], 1);  ASSERT_EQ(get<11>(a), 1);
    ASSERT_EQ(a[12], 1);  ASSERT_EQ(get<12>(a), 1);
    ASSERT_EQ(a[13], 1);  ASSERT_EQ(get<13>(a), 1);
    ASSERT_EQ(a[14], 0);  ASSERT_EQ(get<14>(a), 0);
    ASSERT_EQ(a[15], 0);  ASSERT_EQ(get<15>(a), 0);
    ASSERT_EQ(a[16], 0);  ASSERT_EQ(get<16>(a), 0);
    ASSERT_EQ(a[17], 0);  ASSERT_EQ(get<17>(a), 0);
    ASSERT_EQ(a[18], 0);  ASSERT_EQ(get<18>(a), 0);
    ASSERT_EQ(a[19], 1);  ASSERT_EQ(get<19>(a), 1);
    // clang-format on
}

TEST(packed_int_array, bits_2)
{
    auto a = packed_int_array<2, 20>{0, 1, 2, 3, 0, 0, 1, 1, 2, 2, 3, 3, 1, 3, 2, 1, 0, 3, 2, 1};
    // clang-format off
    ASSERT_EQ(a[0],  0);  ASSERT_EQ(get<0>(a),  0);
    ASSERT_EQ(a[1],  1);  ASSERT_EQ(get<1>(a),  1);
    ASSERT_EQ(a[2],  2);  ASSERT_EQ(get<2>(a),  2);
    ASSERT_EQ(a[3],  3);  ASSERT_EQ(get<3>(a),  3);
    ASSERT_EQ(a[4],  0);  ASSERT_EQ(get<4>(a),  0);
    ASSERT_EQ(a[5],  0);  ASSERT_EQ(get<5>(a),  0);
    ASSERT_EQ(a[6],  1);  ASSERT_EQ(get<6>(a),  1);
    ASSERT_EQ(a[7],  1);  ASSERT_EQ(get<7>(a),  1);
    ASSERT_EQ(a[8],  2);  ASSERT_EQ(get<8>(a),  2);
    ASSERT_EQ(a[9],  2);  ASSERT_EQ(get<9>(a),  2);
    ASSERT_EQ(a[10], 3);  ASSERT_EQ(get<10>(a), 3);
    ASSERT_EQ(a[11], 3);  ASSERT_EQ(get<11>(a), 3);
    ASSERT_EQ(a[12], 1);  ASSERT_EQ(get<12>(a), 1);
    ASSERT_EQ(a[13], 3);  ASSERT_EQ(get<13>(a), 3);
    ASSERT_EQ(a[14], 2);  ASSERT_EQ(get<14>(a), 2);
    ASSERT_EQ(a[15], 1);  ASSERT_EQ(get<15>(a), 1);
    ASSERT_EQ(a[16], 0);  ASSERT_EQ(get<16>(a), 0);
    ASSERT_EQ(a[17], 3);  ASSERT_EQ(get<17>(a), 3);
    ASSERT_EQ(a[18], 2);  ASSERT_EQ(get<18>(a), 2);
    ASSERT_EQ(a[19], 1);  ASSERT_EQ(get<19>(a), 1);
    // clang-format on
}

TEST(packed_int_array, bits_3)
{
    auto a = packed_int_array<3, 20>{0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3};
    // clang-format off
    ASSERT_EQ(a[0],  0);  ASSERT_EQ(get<0>(a),  0);
    ASSERT_EQ(a[1],  1);  ASSERT_EQ(get<1>(a),  1);
    ASSERT_EQ(a[2],  2);  ASSERT_EQ(get<2>(a),  2);
    ASSERT_EQ(a[3],  3);  ASSERT_EQ(get<3>(a),  3);
    ASSERT_EQ(a[4],  4);  ASSERT_EQ(get<4>(a),  4);
    ASSERT_EQ(a[5],  5);  ASSERT_EQ(get<5>(a),  5);
    ASSERT_EQ(a[6],  6);  ASSERT_EQ(get<6>(a),  6);
    ASSERT_EQ(a[7],  7);  ASSERT_EQ(get<7>(a),  7);
    ASSERT_EQ(a[8],  7);  ASSERT_EQ(get<8>(a),  7);
    ASSERT_EQ(a[9],  6);  ASSERT_EQ(get<9>(a),  6);
    ASSERT_EQ(a[10], 5);  ASSERT_EQ(get<10>(a), 5);
    ASSERT_EQ(a[11], 4);  ASSERT_EQ(get<11>(a), 4);
    ASSERT_EQ(a[12], 3);  ASSERT_EQ(get<12>(a), 3);
    ASSERT_EQ(a[13], 2);  ASSERT_EQ(get<13>(a), 2);
    ASSERT_EQ(a[14], 1);  ASSERT_EQ(get<14>(a), 1);
    ASSERT_EQ(a[15], 0);  ASSERT_EQ(get<15>(a), 0);
    ASSERT_EQ(a[16], 0);  ASSERT_EQ(get<16>(a), 0);
    ASSERT_EQ(a[17], 1);  ASSERT_EQ(get<17>(a), 1);
    ASSERT_EQ(a[18], 2);  ASSERT_EQ(get<18>(a), 2);
    ASSERT_EQ(a[19], 3);  ASSERT_EQ(get<19>(a), 3);
    // clang-format on
}

TEST(packed_int_array, bits_15)
{
    auto a = packed_int_array<15, 20>{0, 1, 255, 256, 1023, 1024, 8191, 8192, 16383, 16384, 32767, 16384, 16383, 8192, 8191, 1024, 1023, 256, 255, 1};
    // clang-format off
    ASSERT_EQ(a[0],      0);  ASSERT_EQ(get<0>(a),      0);
    ASSERT_EQ(a[1],      1);  ASSERT_EQ(get<1>(a),      1);
    ASSERT_EQ(a[2],    255);  ASSERT_EQ(get<2>(a),    255);
    ASSERT_EQ(a[3],    256);  ASSERT_EQ(get<3>(a),    256);
    ASSERT_EQ(a[4],   1023);  ASSERT_EQ(get<4>(a),   1023);
    ASSERT_EQ(a[5],   1024);  ASSERT_EQ(get<5>(a),   1024);
    ASSERT_EQ(a[6],   8191);  ASSERT_EQ(get<6>(a),   8191);
    ASSERT_EQ(a[7],   8192);  ASSERT_EQ(get<7>(a),   8192);
    ASSERT_EQ(a[8],  16383);  ASSERT_EQ(get<8>(a),  16383);
    ASSERT_EQ(a[9],  16384);  ASSERT_EQ(get<9>(a),  16384);
    ASSERT_EQ(a[10], 32767);  ASSERT_EQ(get<10>(a), 32767);
    ASSERT_EQ(a[11], 16384);  ASSERT_EQ(get<11>(a), 16384);
    ASSERT_EQ(a[12], 16383);  ASSERT_EQ(get<12>(a), 16383);
    ASSERT_EQ(a[13],  8192);  ASSERT_EQ(get<13>(a),  8192);
    ASSERT_EQ(a[14],  8191);  ASSERT_EQ(get<14>(a),  8191);
    ASSERT_EQ(a[15],  1024);  ASSERT_EQ(get<15>(a),  1024);
    ASSERT_EQ(a[16],  1023);  ASSERT_EQ(get<16>(a),  1023);
    ASSERT_EQ(a[17],   256);  ASSERT_EQ(get<17>(a),   256);
    ASSERT_EQ(a[18],   255);  ASSERT_EQ(get<18>(a),   255);
    ASSERT_EQ(a[19],     1);  ASSERT_EQ(get<19>(a),     1);
    // clang-format on
}

TEST(packed_int_array, bits_16)
{
    auto a = packed_int_array<16, 20>{0,     1,     255,   256,  1023, 1024, 8191, 8192, 16383, 16384,
                                      32767, 32768, 65535, 32768, 32767, 16384, 16383, 8192,  8191, 1024};
    // clang-format off
    ASSERT_EQ(a[0],      0);  ASSERT_EQ(get<0>(a),      0);
    ASSERT_EQ(a[1],      1);  ASSERT_EQ(get<1>(a),      1);
    ASSERT_EQ(a[2],    255);  ASSERT_EQ(get<2>(a),    255);
    ASSERT_EQ(a[3],    256);  ASSERT_EQ(get<3>(a),    256);
    ASSERT_EQ(a[4],   1023);  ASSERT_EQ(get<4>(a),   1023);
    ASSERT_EQ(a[5],   1024);  ASSERT_EQ(get<5>(a),   1024);
    ASSERT_EQ(a[6],   8191);  ASSERT_EQ(get<6>(a),   8191);
    ASSERT_EQ(a[7],   8192);  ASSERT_EQ(get<7>(a),   8192);
    ASSERT_EQ(a[8],  16383);  ASSERT_EQ(get<8>(a),  16383);
    ASSERT_EQ(a[9],  16384);  ASSERT_EQ(get<9>(a),  16384);
    ASSERT_EQ(a[10], 32767);  ASSERT_EQ(get<10>(a), 32767);
    ASSERT_EQ(a[11], 32768);  ASSERT_EQ(get<11>(a), 32768);
    ASSERT_EQ(a[12], 65535);  ASSERT_EQ(get<12>(a), 65535);
    ASSERT_EQ(a[13], 32768);  ASSERT_EQ(get<13>(a), 32768);
    ASSERT_EQ(a[14], 32767);  ASSERT_EQ(get<14>(a), 32767);
    ASSERT_EQ(a[15], 16384);  ASSERT_EQ(get<15>(a), 16384);
    ASSERT_EQ(a[16], 16383);  ASSERT_EQ(get<16>(a), 16383);
    ASSERT_EQ(a[17],  8192);  ASSERT_EQ(get<17>(a),  8192);
    ASSERT_EQ(a[18],  8191);  ASSERT_EQ(get<18>(a),  8191);
    ASSERT_EQ(a[19],  1024);  ASSERT_EQ(get<19>(a),  1024);
    // clang-format on
}

TEST(packed_int_array, bits_17)
{
    auto a = packed_int_array<17, 20>{0,     1,     255,   256,   1023,  1024,  8191,  8192, 16383, 16384,
                                      32767, 32768, 65535, 65536, 131071, 65536, 65535, 32768, 32767, 16384};
    // clang-format off
    ASSERT_EQ(a[0],       0);  ASSERT_EQ(get<0>(a),       0);
    ASSERT_EQ(a[1],       1);  ASSERT_EQ(get<1>(a),       1);
    ASSERT_EQ(a[2],     255);  ASSERT_EQ(get<2>(a),     255);
    ASSERT_EQ(a[3],     256);  ASSERT_EQ(get<3>(a),     256);
    ASSERT_EQ(a[4],    1023);  ASSERT_EQ(get<4>(a),    1023);
    ASSERT_EQ(a[5],    1024);  ASSERT_EQ(get<5>(a),    1024);
    ASSERT_EQ(a[6],    8191);  ASSERT_EQ(get<6>(a),    8191);
    ASSERT_EQ(a[7],    8192);  ASSERT_EQ(get<7>(a),    8192);
    ASSERT_EQ(a[8],   16383);  ASSERT_EQ(get<8>(a),   16383);
    ASSERT_EQ(a[9],   16384);  ASSERT_EQ(get<9>(a),   16384);
    ASSERT_EQ(a[10],  32767);  ASSERT_EQ(get<10>(a),  32767);
    ASSERT_EQ(a[11],  32768);  ASSERT_EQ(get<11>(a),  32768);
    ASSERT_EQ(a[12],  65535);  ASSERT_EQ(get<12>(a),  65535);
    ASSERT_EQ(a[13],  65536);  ASSERT_EQ(get<13>(a),  65536);
    ASSERT_EQ(a[14], 131071);  ASSERT_EQ(get<14>(a), 131071);
    ASSERT_EQ(a[15],  65536);  ASSERT_EQ(get<15>(a),  65536);
    ASSERT_EQ(a[16],  65535);  ASSERT_EQ(get<16>(a),  65535);
    ASSERT_EQ(a[17],  32768);  ASSERT_EQ(get<17>(a),  32768);
    ASSERT_EQ(a[18],  32767);  ASSERT_EQ(get<18>(a),  32767);
    ASSERT_EQ(a[19],  16384);  ASSERT_EQ(get<19>(a),  16384);
    // clang-format on
}
