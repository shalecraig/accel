#ifndef ACCEL_H
#define ACCEL_H

#include <stdbool.h>

#include "accel_consts.c"

#ifndef ACCEL_QUAN_POINT_SIZE
#define ACCEL_QUAN_POINT_SIZE ((uint32_t)ACCEL_FREQUENCY_HZ/25)
#endif

/*
 * At 100Hz, this should be 20s
 */
#ifndef ACCEL_RECORD_MAX_LENGTH
#define ACCEL_RECORD_MAX_LENGTH ACCEL_FREQUENCY_HZ*ACCEL_MAX_RECORDING_TIME_S
#endif

#define ACCEL_PARAM_ERROR -1
#define ACCEL_INTERNAL_ERROR -2
#define ACCEL_MALLOC_ERROR -3

#define ACCEL_NO_VALID_GESTURE -4
#define ACCEL_NO_VALID_AFFINITY -5

/* TODO: hide these in the implementation */
typedef struct {
    // Circular buffer
    int *wbuf;
    int wbuf_end;
    int wbuf_len;

    int subtotal;
    int subtotal_size;
    int max_subtotal_size;
} moving_avg_values;

/* TODO: hide these in the implementation */
typedef struct {
    bool is_recording;
    bool is_recorded;

    int recording_size;
    int **raw_recording;
    int **normalized_recording;

    moving_avg_values **moving_avg_values;
    int *affinities;
} accel_gesture;

// TODO: forward declare the implementation-based state so it is not exposed.
typedef struct {
    bool error_bit;

    /* data */
    int dimensions;
    int window_size;

    int num_gestures_saved;
    accel_gesture **gestures;
} accel_state;

// TODO: rewrite the docs, I wrote them waaaaaaaaaay too early

// Creation and deletion of accel state objects.
int accel_generate_state(accel_state **state, int dimensions, int window_size);
int accel_destroy_state(accel_state **state);

/**
 * Starts recording a accel gesture
 * @param  state state being recorded
 * @param  gesture gesture being recorded
 * @return       id the gesture corresponds to
 */
int accel_start_record_gesture(accel_state *state, int *gesture);

/**
 * Ends recording a accel gesture
 * @param state      state being recorded
 * @param gesture_id id the gesture corresponds to
 */
int accel_end_record_gesture(accel_state *state, int gesture_id);

/**
 * Updates the uWave algorithm's DTW-compare step across all gestures.
 * @param state    state being recorded
 * @param accel_data a array with accelerometer data
 */
int accel_process_timer_tick(accel_state *state, int *accel_data);

/**
 * At a given state, returns the most likely gesture and its affinity.
 * @param state      state being recorded
 * @param gesture_id id the gesture corresponds to
 * @param affinity   affinity of the gesture to the accelerometer input.
 */
int accel_find_most_likely_gesture(accel_state *state, int *gesture_id, int *affinity);

#endif
