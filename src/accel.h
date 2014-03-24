#ifndef ACCEL_H
#define ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define ACCEL_SUCCESS 0
#define ACCEL_PARAM_ERROR -1
#define ACCEL_INTERNAL_ERROR -2
#define ACCEL_MALLOC_ERROR -3
#define ACCEL_NO_VALID_GESTURE -4

#define ACCEL_VERSION_GEN(major, minor, point, isBeta, isAlpha)                                                        \
    (4 * (100 * ((100 * major) + minor) + point) + 3 - (isAlpha ? 2 : 0) - (isBeta ? 1 : 0))

#define ACCEL_VERSION_CODE ACCEL_VERSION_GEN(1, 1, 0, false, true)

struct internalAccelState;
struct accelState;

/**
 * Callback called whenever a given gesture drops below the offset/length
 * threshold specified when the state is initialized.
 *
 * A simple accel_callback is as follows:
 *
 * int my_callback(accel_state *state, const int gesture_id, const int offset_found, bool *reset_gesture) {
 *     int retval = ACCEL_SUCCESS;
 *     if (gesture_id == 1) {
 *         *reset_gesture = true;
 *         ...
 *     } else {
 *         logger->info("unrecognized gesture %i ", gesture_id);
 *         retval = ACCEL_MIN_RESERVED - 1;
 *     }
 *     return retval;
 * }
 *
 * For the callback method, the documentation is as follows:
 * @param  state            A non-NULL pointer to a state variable that holds
 *                          recording metadata.
 * @param  gesture_id       The identifier of the gesture that has been
 *                          triggered.
 * @param  offset_found   The offset of the triggered gesture_id to the
 *                          recorded gesture.
 * @param  reset_gesture    Setting reset_gesture to be true will result in the
 *                          gesture being reset after the callback is triggered,
 *                          and setting it to false will prevent the gesture
 *                          from being reset. No default value is promised.
 * @return int              Returns ACCEL_SUCCESS if successful. Values that are
 *                          not ACCEL_SUCCESS will cause the calling method to
 *                          immediately abort and proxy-return the value
 *                          returned by the callback.
 *                          Implementers wishing to return a custom value should
 *                          refer to the ACCEL_MIN_RESERVED definition inside
 *                          their implementations.
 */
typedef int (*accel_callback)(struct accelState *state, const int16_t gesture_id, const int32_t offset_found,
                              bool *reset_gesture);

typedef struct accelState {
    uint16_t dimensions;

    accel_callback callback;
    struct internalAccelState *state;
} accel_state;

/**
 * Creates a state object, essentially a constructor.
 * @param  state       Pointer-to-pointer of the state being generated,
 *                     populated by the method.
 *                     The current value of the pointer's pointed (*state) must
 *                     be NULL.
 * @param  dimensions  The number of dimensions of input that the state
 *                     represents.
 * @param  window_size The size of the moving windows used to calculate smoothed
 *                     sensor readings.
 * @param  callback    A callback that is triggered whenever a gesture passes a
 *                     threshold. See the ``accel_callback`` typedef for more
 *                     information.
 * @param  threshold   The minimum threshold offset (divided by length) that all
 *                     gestures must be before the callback is called.
 * @return             ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_generate_state(accel_state **state, uint16_t dimensions, int window_size, accel_callback callback,
                         const int threshold);

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
int accel_find_most_likely_gesture(accel_state *state, int *gesture_id, int32_t *distance);

/**
 * For a given state and recorded gesture, resets the gesture's offset state
 * entirely.
 * @param state      A pointer to a non-NULL state variable that holds recording
 *                   metadata.
 * @param gesture_id Value that corresponds to a gesture currently being reset.
 * @return           ACCEL_SUCCESS if successful, an error code otherwise.
 */
int accel_reset_affinities_for_gesture(accel_state *state, int gesture_id);

#ifdef __cplusplus
}
#endif

#endif
