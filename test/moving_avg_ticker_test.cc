#include "gtest/gtest.h"

#include "../src/moving_avg_ticker.h"

const void * void_null = NULL;

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

TEST(MovingAvgTicker, AllocatesAndFreesCorrectly) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_NE(void_null, allocated->wbuf);
    EXPECT_EQ(retval, 0);

    retval = free_moving_avg(&allocated);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(void_null, allocated);
}

TEST(MovingAvgTicker, ResetsCorrectly) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, retval);

    retval = reset_moving_avg(NULL);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    retval = reset_moving_avg(allocated);
    EXPECT_EQ(0, retval);
}

TEST(MovingAvgTicker, AppendsCorrectly1_1) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, retval);

    bool is_at_end = false;
    retval = append_to_moving_avg(allocated, 1, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    int frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(1, frame);

    is_at_end = false;
    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(2, frame);
}

TEST(MovingAvgTicker, AppendsCorrectly2_1) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(2, 1, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, retval);

    bool is_at_end = false;
    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    int frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(1, frame);

    is_at_end = false;
    retval = append_to_moving_avg(allocated, 4, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(3, frame);

    is_at_end = false;
    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(3, frame);
}

TEST(MovingAvgTicker, AppendsCorrectly1_2) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 2, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, retval);

    bool is_at_end = false;
    retval = append_to_moving_avg(allocated, 1, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_FALSE(is_at_end);

    retval = append_to_moving_avg(allocated, 1, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    int frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(2, frame);

    is_at_end = false;
    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_FALSE(is_at_end);

    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(4, frame);
}

TEST(MovingAvgTicker, AppendsCorrectly2_2) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(2, 2, &allocated);
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, retval);

    bool is_at_end = false;
    retval = append_to_moving_avg(allocated, 1, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_FALSE(is_at_end);

    retval = append_to_moving_avg(allocated, 1, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    int frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(1, frame);

    is_at_end = false;
    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_FALSE(is_at_end);

    retval = append_to_moving_avg(allocated, 2, &is_at_end);
    EXPECT_EQ(0, retval);
    EXPECT_TRUE(is_at_end);

    frame = 0;
    retval = get_latest_frame_moving_avg(allocated, &frame);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(3, frame);

    retval = free_moving_avg(&allocated);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(void_null, allocated);
}

TEST(MovingAvgTicker, AppendToInvalid) {
    int retval = 0;
    bool tru = true;
    retval = append_to_moving_avg((moving_avg_values *) NULL, 1, &tru);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
}

TEST(MovingAvgTicker, AppendWithInvalidAtEnd) {
    int retval = 0;
    moving_avg_values *allocated = NULL;
    retval = allocate_moving_avg(2, 2, &allocated);
    EXPECT_EQ(0, retval);
    EXPECT_NE(void_null, allocated);

    retval = append_to_moving_avg(allocated, 1, (bool *)NULL);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    EXPECT_EQ(0, free_moving_avg(&allocated));
}

TEST(MovingAvgTicker, InvalidLatestFrameParams) {
    int retval = 0;
    int frame = 0;
    retval = get_latest_frame_moving_avg(NULL, &frame);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    moving_avg_values *allocated = NULL;
    retval = allocate_moving_avg(2, 2, &allocated);
    EXPECT_EQ(0, retval);
    EXPECT_NE(void_null, allocated);

    retval = get_latest_frame_moving_avg(allocated, (int *) NULL);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    EXPECT_EQ(0, free_moving_avg(&allocated));
}

TEST(MovingAvgTickerFuzzTest, allocate_moving_avg) {
    moving_avg_values *allocated = NULL;

    // Test with negative num_wbuf
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(-1, 1, &allocated));

    // Test with zero num_wbuf
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(0, 1, &allocated));

    // Test with negative subtotal_size
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(1, -1, &allocated));

    // Test with zero subtotal_size
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(1, 0, &allocated));

    // Test with NULL pointer
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(1, 0, NULL));

    // Test with non-NULL pointer-pointer
    allocated = (moving_avg_values *) 0x1;
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(1, 1, &allocated));

    // Test with success, to validate that there was only one difference between this and the above tests.
    allocated = NULL;
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(void_null, allocated);
}

TEST(MovingAvgTickerFuzzTest, reset_moving_avg) {
    // Test with null pointer
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, reset_moving_avg(NULL));
}

TEST(MovingAvgTickerFuzzTest, append_to_moving_avg) {
    // Setup:
    bool is_at_end = false;
    moving_avg_values *allocated = NULL;
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));

    // Test with null pointer
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, append_to_moving_avg(NULL, 1, &is_at_end));

    // Test with null isAtEnd
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, append_to_moving_avg(allocated, 1, NULL));

    // Test with all valid input
    EXPECT_EQ(0, append_to_moving_avg(allocated, 1, &is_at_end));

    // Cleanup:
    EXPECT_EQ(0, free_moving_avg(&allocated));
}

TEST(MovingAvgTickerFuzzTest, get_latest_frame_moving_avg) {
    int frame = 0;
    moving_avg_values *allocated = NULL;
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));

    // Both are NULL
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, get_latest_frame_moving_avg(NULL, NULL));

    // First is NULL
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, get_latest_frame_moving_avg(NULL, &frame));

    // Second is NULL
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, get_latest_frame_moving_avg(allocated, NULL));

    // Validate it works to justify the above unit tests
    EXPECT_EQ(0, get_latest_frame_moving_avg(allocated, &frame));

    // Cleanup
    EXPECT_EQ(0, free_moving_avg(&allocated));
}

TEST(MovingAvgTickerFuzzTest, free_moving_avg) {
    // Test with null pointer-pointer
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, free_moving_avg(NULL));

    // Test with null pointer
    moving_avg_values *allocated = NULL;
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, free_moving_avg(&allocated));

    // Test with null wbuf
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));
    EXPECT_NE(void_null, allocated);
    free(allocated->wbuf);
    allocated->wbuf = NULL;
    // TODO: successfully completes, even with invalid input.
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(void_null, allocated);

    // Test normal path.
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));
    EXPECT_NE(void_null, allocated);
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(void_null, allocated);
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
