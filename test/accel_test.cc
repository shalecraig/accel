#include "gtest/gtest.h"

#include "../src/accel.h"

const void * void_null = NULL;

accel_state *test_fabricate_state(int dimensions) {
    accel_state *state = NULL;
    int result = accel_generate_state(&state, dimensions, 1);
    EXPECT_EQ(0, result);
    EXPECT_NE(void_null, state);
    return state;
}

accel_state *test_fabricate_1d_state() {
    return test_fabricate_state(1);
}

accel_state *test_fabricate_3d_state() {
    return test_fabricate_state(3);
}

void test_burn_state(accel_state ** state) {
    int result = accel_destroy_state(state);
    EXPECT_EQ(0, result);
    EXPECT_EQ(void_null, *state);
}


TEST(AccelFuzzTest, generate_state_null_state) {
    int result = accel_generate_state(NULL, 3, 1);
    EXPECT_EQ(result, ACCEL_PARAM_ERROR);
}

TEST(AccelFuzzTest, generate_state_negative_or_zero_dimensions) {
    accel_state *state = NULL;
    // 0 dimensions must fail
    int result = accel_generate_state(&state, 0, 1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // -1 dimension must fail
    result = accel_generate_state(&state, -1, 1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // 1 dimension must succeed.
    state = NULL;
    result = accel_generate_state(&state, 1, 1);
    EXPECT_EQ(0, result);
    // TODO: result's memory is leaked :s
}

TEST(AccelFuzzTest, generate_state_invalid_window_size) {
    accel_state *state = NULL;
    int result = 0;

    // Size 0 must fail
    result = accel_generate_state(&state, 1, 0);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Size -1 must fail
    result = accel_generate_state(&state, 1, -1);
    EXPECT_EQ(ACCEL_PARAM_ERROR, result);

    // Size 1 must succeed
    result = accel_generate_state(&state, 1, 1);
    EXPECT_EQ(0, result);
    EXPECT_NE(void_null, state);
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
    EXPECT_EQ(void_null, state);
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
    EXPECT_EQ(0, result)  << "gesture " << gesture << " couldn't be recorded correctly" << std::endl;
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
    for (int i=1; i<10; ++i) {
        EXPECT_EQ(void_null, state) << "i = " << i;
        EXPECT_EQ(0, accel_generate_state(&state, 2*i, i)) << "i = " << i;
        EXPECT_EQ(0, accel_destroy_state(&state)) << "i = " << i;
        EXPECT_EQ(void_null, state) << "i = " << i;
    }
}

TEST(AccelTest, start_recording_and_close_many_gestures) {
    accel_state *state = NULL;
    state = test_fabricate_1d_state();

    int data[1] = {0};
    for (int i=0; i<10; ++i) {
        int gesture = 0;
        ASSERT_EQ(0, accel_start_record_gesture(state, &gesture));
        ASSERT_EQ(i, gesture);
        ASSERT_EQ(0, accel_process_timer_tick(state, data));
    }
    for (int i=0; i<10; ++i) {
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
    for (int i=0; i<10000; ++i) {
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
    for (int i=0; i<10; ++i) {
        data[0] = i*100;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, gesture));

    int prev_affinity = 0;
    for (int i=0; i<10; ++i) {
        data[0] = i*100;
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
    for (int i=0; i<10; ++i) {
        data[0] = i;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, first_gesture));

    int second_gesture = 0;
    EXPECT_EQ(0, accel_start_record_gesture(state, &second_gesture));
    EXPECT_NE(first_gesture, second_gesture);

    for (int i=0; i<10; ++i) {
        data[0] = i*i;
        EXPECT_EQ(0, accel_process_timer_tick(state, data));
    }

    EXPECT_EQ(0, accel_end_record_gesture(state, second_gesture));

    int prev_affinity = 0;
    for (int i=0; i<10; ++i) {
        data[0] = i*2;
        int gesture_found = 1;
        int affinity_of_gesture = 1;
        ASSERT_EQ(0, accel_process_timer_tick(state, data));
        ASSERT_EQ(0, accel_find_most_likely_gesture(state, &gesture_found, &affinity_of_gesture));
        ASSERT_EQ(first_gesture, gesture_found);
        prev_affinity = affinity_of_gesture;
    }

    test_burn_state(&state);
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
