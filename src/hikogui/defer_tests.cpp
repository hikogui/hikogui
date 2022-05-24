


#include "defer.hpp"


TEST(defer, early_out)
{
    int a = 0;
    int b = 0;

    do {
        hilet d_a = defer([&]{ a = 42; });
        ASSERT_EQ(a, 0);

        // If will be taken.
        if (a == 0) {
            break;
        }

        hilet d_b = defer([&]{ b = a + 1; });
    } while (false);

    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 0);
}

TEST(defer, fully)
{
    int a = 0;
    int b = 0;

    do {
        hilet d_a = defer([&]{ a = 42; });
        ASSERT_EQ(a, 0);

        // If will NOT be taken.
        if (a == 42) {
            break;
        }

        hilet d_b = defer([&]{ b = a + 5; });
        ASSERT_EQ(b, 0);
    } while (false);

    // d_b destructor will be called before, d_a, this is when a is still zero.
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 5);
}

