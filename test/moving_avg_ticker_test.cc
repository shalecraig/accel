#include "gtest/gtest.h"

#include "../src/moving_avg_ticker.h"

// TEST(blahTest, blah1) {
//     EXPECT_EQ(1, 1);
// }

// TEST(SquareRootTest, PositiveNos) {
//     EXPECT_EQ (18.0, square_root (324.0));
//     EXPECT_EQ (25.4, square_root (645.16));
//     EXPECT_EQ (50.3321, square_root (2533.310224));
// }

const moving_avg_values * null_avg_values = NULL;

TEST(MovingAvgTicker, InvalidInputValues) {

    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 0, &allocated);
    EXPECT_EQ(NULL, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
    retval = allocate_moving_avg(0, 1, &allocated);
    EXPECT_EQ(NULL, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    // Pass in a non-null pointer-to-pointer.
    moving_avg_values *initial_non_null_value = (moving_avg_values *) 1;
    allocated = initial_non_null_value;
    retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_EQ(initial_non_null_value, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
    // TODO: find a way to test failing malloc/calloc.
}

TEST(MovingAvgTicker, ValigInputValuesAllocatesObjects) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE((moving_avg_values *) NULL, allocated);
}

int main (int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int returnValue;

    //Do whatever setup here you will need for your tests here
    //
    //

    returnValue =  RUN_ALL_TESTS();

    //Do Your teardown here if required
    //
    //

    return returnValue;
}
