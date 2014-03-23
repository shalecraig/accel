#include "gtest/gtest.h"

#include "../src/moving_avg_ticker.h"
#include "../src/accel.h"

// TODO: use accel_test_util.h
#include "callback_util.h"
#include "util.h"

accel_state *test_fabricate_state_with_callback(int dimensions, accel_callback callback, const int threshold) {
    accel_state *state = NULL;
    int result = accel_generate_state(&state, dimensions, 1, callback, threshold);
    EXPECT_EQ(ACCEL_SUCCESS, result);
    if (ACCEL_SUCCESS != result) {
        int *myNull = NULL;
        myNull = 0;
    }
    EXPECT_NE(VOID_NULL, state);
    if (VOID_NULL == state) {
        int *myNull = NULL;
        myNull = 0;
    }
    return state;
}

accel_state *test_fabricate_1d_state_with_callback(accel_callback callback, const int threshold) {
    return test_fabricate_state_with_callback(1, callback, threshold);
}

accel_state *test_fabricate_3d_state_with_callback(accel_callback callback, const int threshold) {
    return test_fabricate_state_with_callback(3, callback, threshold);
}

accel_state *test_fabricate_state(int dimensions) { return test_fabricate_state_with_callback(dimensions, NULL, 0); }

accel_state *test_fabricate_1d_state() { return test_fabricate_state(1); }

accel_state *test_fabricate_3d_state() { return test_fabricate_state(3); }

void test_burn_state(accel_state **state) {
    int result = accel_destroy_state(state);
    EXPECT_EQ(0, result);
    EXPECT_EQ(VOID_NULL, *state);
}

TEST(AccelFuzzTest, generate_state_null_state) {
    int result = accel_generate_state(NULL, 3, 1, NULL, 0);
    EXPECT_EQ(result, ACCEL_PARAM_ERROR);
}

TEST(AccelFuzzTest, generate_state_negative_or_zero_dimensions) {
    accel_state *state = NULL;
    // 0 dimensions must fail
    int result = accel_generate_state(&state, 0, 1, NULL, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // -1 dimension must fail
    result = accel_generate_state(&state, -1, 1, NULL, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // 1 dimension must succeed.
    state = NULL;
    result = accel_generate_state(&state, 1, 1, NULL, 0);
    EXPECT_EQ(0, result);
    accel_destroy_state(&state);
}

TEST(AccelFuzzTest, generate_state_invalid_threshold_with_callback_params) {
    accel_state *state = NULL;
    accel_callback nonNullCallback = (accel_callback)1;

    // Fails for negatives.
    int result = accel_generate_state(&state, 1, 1, nonNullCallback, -1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Fails for zero.
    result = accel_generate_state(&state, 1, 1, nonNullCallback, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Succeeds for 1.
    result = accel_generate_state(&state, 1, 1, nonNullCallback, 1);
    EXPECT_EQ(ACCEL_SUCCESS, result);
}

TEST(AccelFuzzTest, generate_state_invalid_window_size) {
    accel_state *state = NULL;
    int result = 0;

    // Size 0 must fail
    result = accel_generate_state(&state, 1, 0, NULL, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Size -1 must fail
    result = accel_generate_state(&state, 1, -1, NULL, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Size 1 must succeed
    result = accel_generate_state(&state, 1, 1, NULL, 0);
    EXPECT_EQ(0, result);
    EXPECT_NE(VOID_NULL, state);
}

TEST(AccelFuzzTest, generate_state_threshold_fuzzing) {
    accel_state *state = NULL;

    // Threshold of 1 with a non-null callback succeeds
    EXPECT_EQ(NULL, state);
    EXPECT_EQ(ACCEL_SUCCESS, accel_generate_state(&state, 1, 1, (accel_callback)0x1, 1));
    accel_destroy_state(&state);

    // Threshold of 1 with a null callback fails
    EXPECT_EQ(NULL, state);
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_generate_state(&state, 1, 1, (accel_callback)NULL, 1));

    // Threshold of 0 with a null callback succeeds
    EXPECT_EQ(NULL, state);
    EXPECT_EQ(ACCEL_SUCCESS, accel_generate_state(&state, 1, 1, (accel_callback)NULL, 0));
    accel_destroy_state(&state);

    // Threshold of 0 with a non-null callback fails
    EXPECT_EQ(NULL, state);
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_generate_state(&state, 1, 1, (accel_callback)0x1, 0));
}

TEST(AccelFuzzTest, accel_generate_state_null_callback) {
    int result = 0;
    accel_state *state = NULL;

    // Null callback must be successful
    result = accel_generate_state(&state, 1, 1, NULL, 0);
}

TEST_CALLBACK(int, AccelFuzzTest, accel_generate_state_valid_callback, myTest, accel_state *state, int gesture_id,
              int offset_found, bool *reset_gesture)
*reset_gesture = true;
return ACCEL_SUCCESS;
}

TEST(AccelFuzzTest, accel_generate_state_valid_callback) {
    int gesture_id = 0;
    accel_state *state = NULL;

    // Non-null callback, watch it iterate over this stuff.
    state = test_fabricate_1d_state_with_callback(
        &TEST_CALLBACK_NAME(AccelFuzzTest, accel_generate_state_valid_callback, myTest), 10);

    EXPECT_EQ(ACCEL_SUCCESS, accel_start_record_gesture(state, &gesture_id));

    for (int i = 0; i < 100; ++i) {
        int data[1] = {i};
        EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    }
    EXPECT_EQ(ACCEL_SUCCESS, accel_end_record_gesture(state, gesture_id));

    for (int i = 0; i < 100; ++i) {
        int data[1] = {i};
        EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    }
    EXPECT_GT(TEST_CALLBACK_COUNTER(AccelFuzzTest, accel_generate_state_valid_callback, myTest), (uint32_t)0);
}

TEST(AccelFuzzTest, accel_destroy_state_invalid_input) {
    int result = 0;

    // Destroying x (x = NULL) will fail.
    result = accel_destroy_state(NULL);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Destroying x (*x = NULL) will fail.
    accel_state *state = NULL;
    result = accel_destroy_state(&state);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Destroying x (*x != NULL) should succeed.
    state = test_fabricate_1d_state();

    // Destroy the state
    result = accel_destroy_state(&state);
    EXPECT_EQ(0, result);
    EXPECT_EQ(VOID_NULL, state);
}

TEST(AccelFuzzTest, accel_start_record_gesture_invalid_input) {
    int result = 0;
    int gesture_id = 0;
    result = accel_start_record_gesture(NULL, &gesture_id);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    accel_state *state = test_fabricate_1d_state();

    result = accel_start_record_gesture(state, NULL);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    test_burn_state(&state);
}

TEST(AccelFuzzTest, accel_end_record_gesture_invalid_input) {
    int result = 0;
    accel_state *state = NULL;

    // Null state:
    result = accel_end_record_gesture(NULL, 1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Negative index:
    state = test_fabricate_1d_state();
    result = accel_end_record_gesture(state, -1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);
    test_burn_state(&state);

    // Unused index:
    state = test_fabricate_1d_state();
    result = accel_end_record_gesture(state, 1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);
    test_burn_state(&state);

    // Verify it works for valid indexes.
    state = test_fabricate_1d_state();
    int gesture = 1234;
    result = accel_start_record_gesture(state, &gesture);
    EXPECT_NE(1234, gesture);
    EXPECT_EQ(0, result);
    int data[1] = {1};
    EXPECT_EQ(0, accel_process_timer_tick(state, data));

    result = accel_end_record_gesture(state, gesture);
    EXPECT_EQ(0, result) << "gesture " << gesture << " couldn't be recorded correctly" << std::endl;
    test_burn_state(&state);
}

TEST(AccelFuzzTest, accel_process_timer_tick_invalid_input) {
    int result = 0;
    int accel_data = 0;
    accel_state *state = NULL;

    // Null state value.
    result = accel_process_timer_tick(NULL, &accel_data);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Null array input.
    state = test_fabricate_1d_state();
    result = accel_process_timer_tick(state, NULL);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);
    test_burn_state(&state);
}

TEST(AccelFuzzTest, accel_find_most_likely_gesture_invalid_input) {
    int result = 0;
    int gesture_id = 0;
    int affinity = 0;
    accel_state *state = NULL;

    // Null state:
    result = accel_find_most_likely_gesture(NULL, &gesture_id, &affinity);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Null gesture id is passed in.
    state = test_fabricate_1d_state();
    result = accel_find_most_likely_gesture(state, NULL, &affinity);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);
    test_burn_state(&state);

    // Null affinity is passed in.
    state = test_fabricate_1d_state();
    result = accel_find_most_likely_gesture(state, &gesture_id, NULL);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);
    test_burn_state(&state);

    // No tests exist, but otherwise valid parameters.
    state = test_fabricate_1d_state();
    result = accel_find_most_likely_gesture(state, &gesture_id, &affinity);
    EXPECT_EQ(ACCEL_NO_VALID_GESTURE, gesture_id);
    EXPECT_EQ(ACCEL_NO_VALID_GESTURE, affinity);
    EXPECT_EQ(ACCEL_NO_VALID_GESTURE, result);
}

TEST(AccelTest, accel_generate_and_destroy) {
    accel_state *state = NULL;
    for (int i = 1; i < 10; ++i) {
        EXPECT_EQ(VOID_NULL, state) << "i = " << i;
        EXPECT_EQ(0, accel_generate_state(&state, 2 * i, i, NULL, 0)) << "i = " << i;
        EXPECT_EQ(0, accel_destroy_state(&state)) << "i = " << i;
        EXPECT_EQ(VOID_NULL, state) << "i = " << i;
    }
}

TEST(AccelTest, start_recording_and_close_many_gestures) {
    accel_state *state = NULL;
    state = test_fabricate_1d_state();

    int data[1] = {0};
    for (int i = 0; i < 10; ++i) {
        int gesture = 0;
        ASSERT_EQ(0, accel_start_record_gesture(state, &gesture));
        ASSERT_EQ(i, gesture);
        ASSERT_EQ(0, accel_process_timer_tick(state, data));
    }
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(0, accel_end_record_gesture(state, i));
    }
    test_burn_state(&state);
}

TEST(AccelTest, record_incredibly_long_sequence) {
    accel_state *state = NULL;
    state = test_fabricate_1d_state();

    int gesture = 0;
    EXPECT_EQ(0, accel_start_record_gesture(state, &gesture));
    EXPECT_EQ(0, gesture);

    int data[] = {1};
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, gesture));
    test_burn_state(&state);
}

TEST(AccelTest, end_to_end_test_single_recording) {
    accel_state *state = NULL;
    state = test_fabricate_1d_state();

    int gesture = 0;
    EXPECT_EQ(0, accel_start_record_gesture(state, &gesture));
    EXPECT_EQ(0, gesture);

    int data[] = {1};
    for (int i = 0; i < 10; ++i) {
        data[0] = i * 100;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, gesture));

    int prev_affinity = 0;
    for (int i = 0; i < 10; ++i) {
        data[0] = i * 100;
        int gesture_found = 1;
        int affinity_of_gesture = 1;
        ASSERT_EQ(0, accel_process_timer_tick(state, data));
        ASSERT_EQ(0, accel_find_most_likely_gesture(state, &gesture_found, &affinity_of_gesture));
        ASSERT_EQ(gesture, gesture_found);
        if (i != 0) {
            ASSERT_LT(affinity_of_gesture, prev_affinity) << "i=" << i;
        }
        prev_affinity = affinity_of_gesture;
    }

    test_burn_state(&state);
}

TEST(AccelTest, end_to_end_test_multiple_recordings) {
    // g_1(x) = x, g_2(x) = x*x. Sample data is f(x) = 2x, we want to verify that g_1 is chosen over g_2.
    accel_state *state = NULL;
    state = test_fabricate_1d_state();

    int first_gesture = 0;
    EXPECT_EQ(0, accel_start_record_gesture(state, &first_gesture));
    EXPECT_EQ(0, first_gesture);

    int data[] = {1};
    for (int i = 0; i < 10; ++i) {
        data[0] = i;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, first_gesture));

    int second_gesture = 0;
    EXPECT_EQ(0, accel_start_record_gesture(state, &second_gesture));
    EXPECT_NE(first_gesture, second_gesture);

    for (int i = 0; i < 10; ++i) {
        data[0] = i * i;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, second_gesture));

    int prev_affinity = 0;
    for (int i = 0; i < 10; ++i) {
        data[0] = i * 2;
        int gesture_found = 1;
        int affinity_of_gesture = 1;
        ASSERT_EQ(0, accel_process_timer_tick(state, data));
        ASSERT_EQ(0, accel_find_most_likely_gesture(state, &gesture_found, &affinity_of_gesture));
        ASSERT_EQ(first_gesture, gesture_found);
        prev_affinity = affinity_of_gesture;
    }

    test_burn_state(&state);
}

TEST(AccelTest, test_fuzz_reset_affinities) {
    accel_state *state = NULL;

    // Null accel states.
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_reset_affinities_for_gesture(NULL, 0));

    // No recorded accelerations
    state = test_fabricate_1d_state();
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_reset_affinities_for_gesture(state, 0));

    int gesture_id = 0;
    EXPECT_EQ(ACCEL_SUCCESS, accel_start_record_gesture(state, &gesture_id));

    // A recording gesture with no data.
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_reset_affinities_for_gesture(state, gesture_id));

    int data[1] = {0};
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));

    // A recording gesture with some data.
    EXPECT_EQ(ACCEL_PARAM_ERROR, accel_reset_affinities_for_gesture(state, gesture_id));

    EXPECT_EQ(ACCEL_SUCCESS, accel_end_record_gesture(state, gesture_id));

    // No ticks have been recorded.
    EXPECT_EQ(ACCEL_SUCCESS, accel_reset_affinities_for_gesture(state, gesture_id));

    int gesture = 1;
    int initial_distance = 1;
    int after_run_distance = 1;
    int after_reset_distance = 1;

    EXPECT_EQ(ACCEL_SUCCESS, accel_find_most_likely_gesture(state, &gesture, &initial_distance));

    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_process_timer_tick(state, data));
    EXPECT_EQ(ACCEL_SUCCESS, accel_find_most_likely_gesture(state, &gesture, &after_run_distance));

    EXPECT_EQ(ACCEL_SUCCESS, accel_reset_affinities_for_gesture(state, gesture_id));
    EXPECT_EQ(ACCEL_SUCCESS, accel_find_most_likely_gesture(state, &gesture, &after_reset_distance));

    EXPECT_EQ(initial_distance, after_reset_distance);
    EXPECT_GE(after_reset_distance, after_run_distance);

    test_burn_state(&state);
}

TEST(MovingAvgTicker, InvalidInputValues) {

    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 0, &allocated);
    EXPECT_EQ(NULL, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
    retval = allocate_moving_avg(0, 1, &allocated);
    EXPECT_EQ(NULL, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    // Pass in a non-null pointer-to-pointer.
    moving_avg_values *initial_non_null_value = (moving_avg_values *)1;
    allocated = initial_non_null_value;
    retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_EQ(initial_non_null_value, allocated);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
    // TODO: find a way to test failing malloc/calloc.
}

TEST(MovingAvgTicker, AllocatesAndFreesCorrectly) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(VOID_NULL, allocated);
    EXPECT_NE(VOID_NULL, allocated->wbuf);
    EXPECT_EQ(retval, 0);

    retval = free_moving_avg(&allocated);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(VOID_NULL, allocated);
}

TEST(MovingAvgTicker, ResetsCorrectly) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(VOID_NULL, allocated);
    EXPECT_EQ(0, retval);

    retval = reset_moving_avg(NULL);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);

    retval = reset_moving_avg(allocated);
    EXPECT_EQ(0, retval);
}

TEST(MovingAvgTicker, AppendsCorrectly1_1) {
    moving_avg_values *allocated = NULL;
    int retval = allocate_moving_avg(1, 1, &allocated);
    EXPECT_NE(VOID_NULL, allocated);
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
    EXPECT_NE(VOID_NULL, allocated);
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
    EXPECT_NE(VOID_NULL, allocated);
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
    EXPECT_NE(VOID_NULL, allocated);
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
    EXPECT_EQ(VOID_NULL, allocated);
}

TEST(MovingAvgTicker, AppendToInvalid) {
    int retval = 0;
    bool tru = true;
    retval = append_to_moving_avg((moving_avg_values *)NULL, 1, &tru);
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, retval);
}

TEST(MovingAvgTicker, AppendWithInvalidAtEnd) {
    int retval = 0;
    moving_avg_values *allocated = NULL;
    retval = allocate_moving_avg(2, 2, &allocated);
    EXPECT_EQ(0, retval);
    EXPECT_NE(VOID_NULL, allocated);

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
    EXPECT_NE(VOID_NULL, allocated);

    retval = get_latest_frame_moving_avg(allocated, (int *)NULL);
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
    allocated = (moving_avg_values *)0x1;
    EXPECT_EQ(MOVING_AVG_PARAM_ERROR, allocate_moving_avg(1, 1, &allocated));

    // Test with success, to validate that there was only one difference between this and the above tests.
    allocated = NULL;
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));
    EXPECT_NE(VOID_NULL, allocated);
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(VOID_NULL, allocated);
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
    EXPECT_NE(VOID_NULL, allocated);
    free(allocated->wbuf);
    allocated->wbuf = NULL;
    // TODO: successfully completes, even with invalid input.
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(VOID_NULL, allocated);

    // Test normal path.
    EXPECT_EQ(0, allocate_moving_avg(1, 1, &allocated));
    EXPECT_NE(VOID_NULL, allocated);
    EXPECT_EQ(0, free_moving_avg(&allocated));
    EXPECT_EQ(VOID_NULL, allocated);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int returnValue;

    // Do whatever setup here you will need for your tests here
    //
    //

    returnValue = RUN_ALL_TESTS();

    // Do Your teardown here if required
    //
    //

    return returnValue;
}
