#ifndef MUWAVE_H
#define MUWAVE_H

#include <stdbool.h>

#include "muwave_consts.c"

#ifndef MUWAVE_QUAN_POINT_SIZE
#define MUWAVE_QUAN_POINT_SIZE ((uint32_t)MUWAVE_FREQUENCY_HZ/25)
#endif

/*
 * At 100Hz, this should be 20s
 */
#ifndef MUWAVE_RECORD_MAX_LENGTH
#define MUWAVE_RECORD_MAX_LENGTH MUWAVE_FREQUENCY_HZ*MUWAVE_MAX_RECORDING_TIME_S
#endif

/* TODO: hide this in the implementation */
typedef struct {
    bool is_recording;
    bool is_recorded;

    int recording_size;
    int** raw_recording;
    int** normalized_recording;
} muwave_gesture;

// TODO: forward declare the implementation-based state so it is not exposed.
typedef struct {
    bool error_bit;

    /* data */
    int dimensions;
    int window_size;

    int num_gestures_saved;
    muwave_gesture **gestures;
} muwave_state;

// Creation and deletion of muwave state objects.
muwave_state* muwave_generate_state(int dimensions, int window_size);
void muwave_destroy_state(muwave_state* state);

/**
 * Starts recording a muwave gesture
 * @param  state state being recorded
 * @return       id the gesture corresponds to
 */
int muwave_start_record_gesture(muwave_state* state);

/**
 * Ends recording a muwave gesture
 * @param state      state being recorded
 * @param gesture_id id the gesture corresponds to
 */
void muwave_end_record_gesture(muwave_state* state, int gesture_id);

/**
 * Updates the uWave algorithm's DTW-compare step across all gestures.
 * @param state    state being recorded
 * @param accel_data a array with accelerometer data
 */
void muwave_process_timer_tick(muwave_state* state, int* accel_data);

/**
 * At a given state, returns the most likely gesture and its affinity.
 * @param state      state being recorded
 * @param gesture_id id the gesture corresponds to
 * @param affinity   affinity of the gesture to the accelerometer input.
 */
void muwave_find_most_likely_gesture(muwave_state* state, int *gesture_id, int *affinity);

#endif
