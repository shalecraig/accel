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

#define ACCEL_SUCCESS 0
#define ACCEL_PARAM_ERROR -1
#define ACCEL_INTERNAL_ERROR -2
#define ACCEL_MALLOC_ERROR -3
#define ACCEL_NO_VALID_GESTURE -4

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
    int **normalized_recording;

    moving_avg_values **moving_avg_values;
    int *affinities;
} accel_gesture;

// TODO: forward declare the implementation-based state so it is not exposed.
typedef struct {
    /* data */
    int dimensions;
    int window_size;

    int num_gestures_saved;
    accel_gesture **gestures;
} accel_state;

/**
 * Creates a state object, essentially a constructor.
 * @param  state       Pointer-to-pointer of the state being generated, populated
 *                     by the method.
 *                     The current value of the pointer's pointed (*state) must
 *                     be NULL.
 * @param  dimensions  The number of dimensions of input that the state
 *                     represents.
 * @param  window_size The size of the moving windows used to calculate smoothed
 *                     sensor readings.
 * @return             ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_generate_state(accel_state **state, int dimensions, int window_size);

/**
 * Destroys the state object at the pointer pointed to by the state pointer.
 * @param  state pointer to pointer of object to be destroyed. Pointer (*state)
 *               will be set to NULL after execution.
 * @return       ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_destroy_state(accel_state **state);

/**
 * Starts recording a accel gesture
 * @param  state   A pointer to a non-NULL state variable that holds recording
 *                 metadata.
 * @param  gesture Non-NULL pointer that will be populated with the gesture id.
 * @return         ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_start_record_gesture(accel_state *state, int *gesture);

/**
 * Ends recording a accel gesture
 * @param state      A pointer to a non-NULL state variable that holds recording
 *                   metadata.
 * @param gesture_id Value that corresponds to a gesture currently being
 *                   recorded.
 * @return           ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_end_record_gesture(accel_state *state, int gesture_id);

/**
 * Updates the state variable's current state based on the accel data array
 * passed in.
 * @param  state      A pointer to a non-NULL state variable that holds
 *                    recording metadata.
 * @param  accel_data An with accelerometer data.
 * @return            ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_process_timer_tick(accel_state *state, int *accel_data);

/**
 * For a given state, returns the most likely valid gesture and its distance
 * according to tick data.
 * @param  state      A pointer to a non-NULL state variable that holds
 *                    recording metadata.
 * @param  gesture_id A non-NULL pointer that will be populated with the gesture
 *                    of lowest distance.
 * @param  distance   A non-NULL pointer that will be populated with the
 *                    distance corresponding to the returned gesture.
 * @return            ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_find_most_likely_gesture(accel_state *state, int *gesture_id, int *distance);

#endif
