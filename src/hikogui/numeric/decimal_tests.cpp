// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/decimal.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace hi;

TEST(Decimal, StringConstruction)
{
    decimal x;

    ASSERT_NO_THROW(x = decimal("0"));
    ASSERT_EQ(x.mantissa(), 0);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("1"));
    ASSERT_EQ(x.mantissa(), 1);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("1000"));
    ASSERT_EQ(x.mantissa(), 1'000);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("1'000'000"));
    ASSERT_EQ(x.mantissa(), 1'000'000);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("9'999'999'999'999'999"));
    ASSERT_EQ(x.mantissa(), 9'999'999'999'999'999);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("99'999'999'999'999'999"));
    ASSERT_EQ(x.mantissa(), 9'999'999'999'999'999);
    ASSERT_EQ(x.exponent(), 1);

    ASSERT_NO_THROW(x = decimal("-0"));
    ASSERT_EQ(x.mantissa(), 0);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("-1"));
    ASSERT_EQ(x.mantissa(), -1);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("-1000"));
    ASSERT_EQ(x.mantissa(), -1'000);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("-1'000'000"));
    ASSERT_EQ(x.mantissa(), -1'000'000);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("-9'999'999'999'999'999"));
    ASSERT_EQ(x.mantissa(), -9'999'999'999'999'999);
    ASSERT_EQ(x.exponent(), 0);

    ASSERT_NO_THROW(x = decimal("-99'999'999'999'999'999"));
    ASSERT_EQ(x.mantissa(), -9'999'999'999'999'999);
    ASSERT_EQ(x.exponent(), 1);

    ASSERT_NO_THROW(x = decimal("0.421"));
    ASSERT_EQ(x.mantissa(), 421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("1.421"));
    ASSERT_EQ(x.mantissa(), 1421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("1000.421"));
    ASSERT_EQ(x.mantissa(), 1'000'421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("1'000'000.421"));
    ASSERT_EQ(x.mantissa(), 1'000'000'421);
    ASSERT_EQ(x.exponent(), -3);

    // max int64_t             9'223'372'036'854'775'808
    // max mantissa                9'999'999'999'999'999
    ASSERT_NO_THROW(x = decimal("999'999'999'999'999.421"));
    ASSERT_EQ(x.mantissa(), 9'999'999'999'999'994);
    ASSERT_EQ(x.exponent(), -1);

    ASSERT_NO_THROW(x = decimal("-0.421"));
    ASSERT_EQ(x.mantissa(), -421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("-1.421"));
    ASSERT_EQ(x.mantissa(), -1421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("-1000.421"));
    ASSERT_EQ(x.mantissa(), -1'000'421);
    ASSERT_EQ(x.exponent(), -3);

    ASSERT_NO_THROW(x = decimal("-1'000'000.421"));
    ASSERT_EQ(x.mantissa(), -1'000'000'421);
    ASSERT_EQ(x.exponent(), -3);

    // min int64_t             -9'223'372'036'854'775'809
    // min mantissa                -9'999'999'999'999'999
    ASSERT_NO_THROW(x = decimal("-999'999'999'999'999.421"));
    ASSERT_EQ(x.mantissa(), -9'999'999'999'999'994);
    ASSERT_EQ(x.exponent(), -1);
}

TEST(Decimal, ToString)
{
    ASSERT_EQ(to_string(decimal(0, 0)), "0");
    ASSERT_EQ(to_string(decimal(0, 1)), "1");
    ASSERT_EQ(to_string(decimal(0, -1)), "-1");
    ASSERT_EQ(to_string(decimal(1, 0)), "00");
    ASSERT_EQ(to_string(decimal(1, 1)), "10");
    ASSERT_EQ(to_string(decimal(1, -1)), "-10");
    ASSERT_EQ(to_string(decimal(-1, 0)), "0.0");
    ASSERT_EQ(to_string(decimal(-1, 1)), "0.1");
    ASSERT_EQ(to_string(decimal(-1, -1)), "-0.1");
    ASSERT_EQ(to_string(decimal(2, 0)), "000");
    ASSERT_EQ(to_string(decimal(2, 1)), "100");
    ASSERT_EQ(to_string(decimal(2, -1)), "-100");
    ASSERT_EQ(to_string(decimal(-2, 0)), "0.00");
    ASSERT_EQ(to_string(decimal(-2, 1)), "0.01");
    ASSERT_EQ(to_string(decimal(-2, -1)), "-0.01");
}

hi_no_inline decimal test(decimal a, decimal b)
{
    return a + b;
}

TEST(Decimal, Add)
{
    ASSERT_EQ(decimal(0, 0) + decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(2, 0) + decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(-2, 0) + decimal(0, 0), decimal(-2, 0));

    ASSERT_EQ(decimal(0, 42) + decimal(0, 55), decimal(0, 97));
    ASSERT_EQ(decimal(2, 42) + decimal(0, 55), decimal(0, 4255));
    ASSERT_EQ(decimal(-2, 42) + decimal(0, 55), decimal(-2, 5542));
    ASSERT_EQ(decimal(-2, 42) + decimal(2, 55), decimal(-2, 550042));

    ASSERT_EQ(test(decimal(2, 42), decimal(0, 55)), decimal(0, 4255));
}

TEST(Decimal, Sub)
{
    ASSERT_EQ(decimal(0, 0) - decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(2, 0) - decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(-2, 0) - decimal(0, 0), decimal(-2, 0));

    ASSERT_EQ(decimal(0, 42) - decimal(0, 55), decimal(0, -13));
    ASSERT_EQ(decimal(2, 42) - decimal(0, 55), decimal(0, 4145));
    ASSERT_EQ(decimal(-2, 42) - decimal(0, 55), decimal(-2, -5458));
    ASSERT_EQ(decimal(-2, 42) - decimal(2, 55), decimal(-2, -549958));
}

TEST(Decimal, Mul)
{
    ASSERT_EQ(decimal(0, 0) * decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(2, 0) * decimal(0, 0), decimal(0, 0));
    ASSERT_EQ(decimal(-2, 0) * decimal(0, 0), decimal(-2, 0));

    ASSERT_EQ(decimal(0, 42) * decimal(0, 55), decimal(0, 2310));
    ASSERT_EQ(decimal(2, 42) * decimal(0, 55), decimal(2, 2310));
    ASSERT_EQ(decimal(-2, 42) * decimal(0, 55), decimal(-2, 2310));
    ASSERT_EQ(decimal(-2, 42) * decimal(2, 55), decimal(0, 2310));
}

TEST(Decimal, Div)
{
    ASSERT_EQ(decimal(0, 42) / decimal(0, 55), decimal(-15, 763636363636363));
    ASSERT_EQ(decimal(2, 42) / decimal(0, 55), decimal(-13, 763636363636363));
    ASSERT_EQ(decimal(-2, 42) / decimal(0, 55), decimal(-17, 763636363636363));
    ASSERT_EQ(decimal(-2, 42) / decimal(2, 55), decimal(-19, 763636363636363));
}
