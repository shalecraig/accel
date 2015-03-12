#ifndef ACCEL_H
#define ACCEL_H

#include <stdbool.h>

#define ACCEL_SUCCESS 0
#define ACCEL_PARAM_ERROR -1
#define ACCEL_INTERNAL_ERROR -2
#define ACCEL_MALLOC_ERROR -3
#define ACCEL_NO_VALID_GESTURE -4

#define ACCEL_VERSION_GEN(major, minor, point, isBeta, isAlpha) \
    (4*(100*((100*major)+minor)+point) + 3 - (isAlpha?2:0) - (isBeta?1:0))

#define ACCEL_VERSION_CODE ACCEL_VERSION_GEN(1, 0, 0, true, false)

struct internalAccelState;

typedef struct {
    int dimensions;

    struct internalAccelState *state;
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
