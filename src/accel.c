#ifndef IS_NOT_PEBBLE
#ifndef PEBBLE
#define PEBBLE
#endif
#endif

#ifdef PEBBLE
#include <pebble.h>
#include <pebble_makeup.h>
#else
#define my_realloc(a, b, c) (realloc(a, b))
#define my_calloc(a, b) (calloc(a, b))
#endif

// cbrt is defined and importable for everybody!
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "accel.h"
#include "moving_avg_ticker.h"

typedef struct {
    bool is_recording;
    bool is_recorded;

    // TODO: uint32_t needed instead?
    uint16_t recording_size;
    int32_t **normalized_recording;

    moving_avg_values **moving_avg_values;
    int32_t *offsets;
} accel_gesture;

typedef struct internalAccelState {
    // TODO: uint32_t needed instead?
    uint16_t num_gestures_saved;
    uint16_t window_size;
    uint32_t threshold;

    accel_gesture **gestures;
} internal_accel_state;

#define PRECONDITION_NOT_NULL(INPUT)                                                                                   \
    if (INPUT == NULL) {                                                                                               \
        return ACCEL_PARAM_ERROR;                                                                                      \
    }

#define PRECONDITION_NULL(INPUT)                                                                                       \
    if (INPUT != NULL) {                                                                                               \
        return ACCEL_PARAM_ERROR;                                                                                      \
    }

#define PRECONDITION_VALID_STATE(INPUT_STATE)                                                                          \
    if (INPUT_STATE == NULL) {                                                                                         \
        return ACCEL_PARAM_ERROR;                                                                                      \
    }                                                                                                                  \
    if (INPUT_STATE->state == NULL) {                                                                                  \
        return ACCEL_INTERNAL_ERROR;                                                                                   \
    }                                                                                                                  \
    if (INPUT_STATE->dimensions <= 0) {                                                                                \
        return ACCEL_INTERNAL_ERROR;                                                                                   \
    }                                                                                                                  \
    if (INPUT_STATE->state->window_size <= 0) {                                                                        \
        return ACCEL_INTERNAL_ERROR;                                                                                   \
    }                                                                                                                  \
    if (INPUT_STATE->state->gestures == NULL && INPUT_STATE->state->num_gestures_saved != 0) {                         \
        return ACCEL_INTERNAL_ERROR;                                                                                   \
    }                                                                                                                  \
    if (INPUT_STATE->state->gestures != NULL && INPUT_STATE->state->num_gestures_saved == 0) {                         \
        return ACCEL_INTERNAL_ERROR;                                                                                   \
    }
// if (INPUT_STATE->state->num_gestures_saved < 0) {
//     return ACCEL_INTERNAL_ERROR;
// }

#define PRECONDITION_TRUE_PARAM(TRUE_COND)                                                                             \
    {                                                                                                                  \
        if (!TRUE_COND) {                                                                                              \
            return ACCEL_PARAM_ERROR;                                                                                  \
        }                                                                                                              \
    }

// Decay rate of values we choose to keep. 1.0 is no decay, 2.0 is a doubling every time we keep them.
// TODO: should we store the offsets as floats instead?
#define ALPHA ((float)1.0)

// TODO: include this from a header file?
// TODO: include as a static inline function?
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
// #define MAX(a,b) (((a)>(b))?(a):(b))

void accel_destroy_gesture(accel_gesture **gesture, uint32_t dimensions) {
    if (gesture == NULL || *gesture == NULL) {
        return;
    }

    accel_gesture *gest = *gesture;

    if (gest->moving_avg_values != NULL) {
        for (uint32_t i = 0; i < dimensions; ++i) {
            free_moving_avg(&(gest->moving_avg_values[i]));
        }
    }
    if (gest->normalized_recording != NULL) {
        for (uint16_t i = 0; i < gest->recording_size; ++i) {
            if (gest->normalized_recording[i] != NULL) {
                free(gest->normalized_recording[i]);
                gest->normalized_recording[i] = NULL;
            }
        }
        free(gest->normalized_recording);
        gest->normalized_recording = NULL;
    }
    if (gest->offsets != NULL) {
        free(gest->offsets);
        gest->offsets = NULL;
    }

    free(*gesture);
    *gesture = NULL;
}

int accel_generate_gesture(accel_state *state, accel_gesture **gesture) {
    PRECONDITION_VALID_STATE(state);
    PRECONDITION_NOT_NULL(gesture);

    // TODO: write a test for this value.
    PRECONDITION_NULL((*gesture));

    size_t gesture_size = sizeof(accel_gesture);
    *gesture = (accel_gesture *)malloc(gesture_size);
    if (*gesture == NULL) {
        return ACCEL_MALLOC_ERROR;
    }
    memset(*gesture, 0, gesture_size);
    (*gesture)->is_recording = false;
    (*gesture)->is_recorded = false;
    (*gesture)->normalized_recording = NULL;

    (*gesture)->moving_avg_values = (moving_avg_values **)my_calloc(state->dimensions, sizeof(moving_avg_values *));
    if ((*gesture)->moving_avg_values == NULL) {
        free((*gesture));
        *gesture = NULL;
        return ACCEL_MALLOC_ERROR;
    }
    for (uint32_t i = 0; i < state->dimensions; ++i) {
        // TODO: these two shouldn't both be the same....
        int result = allocate_moving_avg(state->state->window_size, state->state->window_size,
                                         &((*gesture)->moving_avg_values[i]));

        if (result != ACCEL_SUCCESS) {
            for (uint32_t j = 0; j < i; ++j) {
                free_moving_avg(&((*gesture)->moving_avg_values[i]));
            }
            accel_destroy_gesture(gesture, state->dimensions);
            return result;
        }
    }
    return ACCEL_SUCCESS;
}

int accel_generate_state(accel_state **state, uint32_t dimensions, uint16_t window_size, accel_callback callback,
                         const uint32_t threshold) {
    PRECONDITION_NOT_NULL(state);

    // TODO: write a test for this value.
    PRECONDITION_NULL(*state);

    if (dimensions == 0) {
        return ACCEL_PARAM_ERROR;
    }
    if (window_size <= 0) {
        return ACCEL_PARAM_ERROR;
    }
    if (threshold == 0 && callback != NULL) {
        return ACCEL_PARAM_ERROR;
    }
    if (threshold != 0 && callback == NULL) {
        return ACCEL_PARAM_ERROR;
    }

    size_t state_size = sizeof(accel_state);
    size_t internal_state_size = sizeof(internal_accel_state);

    internal_accel_state *internal_state = (internal_accel_state *)malloc(internal_state_size);
    *state = (accel_state *)malloc(state_size);
    if (*state == NULL || internal_state == NULL) {
        if (state != NULL) {
            free(*state);
            *state = NULL;
        }
        if (internal_state != NULL) {
            free(internal_state);
            internal_state = NULL;
        }
        return ACCEL_MALLOC_ERROR;
    }

    memset((*state), 0, state_size);
    memset(internal_state, 0, internal_state_size);

    (*state)->state = internal_state;

    (*state)->callback = callback;
    (*state)->dimensions = dimensions;
    (*state)->state->window_size = window_size > 0 ? window_size : 2;
    (*state)->state->threshold = threshold;
    return ACCEL_SUCCESS;
}

// TODO: needs testing with invalid objects.
int accel_destroy_state(accel_state **state) {
    PRECONDITION_NOT_NULL(state);
    PRECONDITION_NOT_NULL(*state);

    uint32_t dimensions = (*state)->dimensions;
    if ((*state)->state != NULL) {
        internal_accel_state *istate = (*state)->state;
        if (istate->gestures != NULL) {
            /* TODO: remove all additional fields inside the accel_state variable */
            for (uint16_t i = 0; i < istate->num_gestures_saved; ++i) {
                accel_gesture *gest = (istate->gestures[i]);
                accel_destroy_gesture(&(gest), dimensions);
            }
            free(istate->gestures);
            istate->gestures = NULL;
        }
        free((*state)->state);
        (*state)->state = NULL;
    }

    free((*state));
    *state = NULL;

    return ACCEL_SUCCESS;
}

int accel_start_record_gesture(accel_state *state, uint16_t *gesture) {
    PRECONDITION_VALID_STATE(state);
    PRECONDITION_NOT_NULL(gesture);

    if (state->state->num_gestures_saved != 0) {
        accel_gesture **tmp = (accel_gesture **)my_realloc(
            state->state->gestures, (state->state->num_gestures_saved + 1) * sizeof(accel_gesture *),
            (state->state->num_gestures_saved) * sizeof(accel_gesture *));
        if (tmp == NULL) {
            return ACCEL_MALLOC_ERROR;
        }
        state->state->gestures = tmp;
    } else {
        state->state->gestures = (accel_gesture **)malloc(sizeof(accel_gesture *));
        if (state->state->gestures == NULL) {
            return ACCEL_MALLOC_ERROR;
        }
    }
    *gesture = (state->state->num_gestures_saved)++;

    state->state->gestures[*gesture] = NULL;

    int result = accel_generate_gesture(state, &(state->state->gestures[*gesture]));
    if (result != ACCEL_SUCCESS) {
        if (state->state->num_gestures_saved == 1) {
            free(state->state->gestures);
            state->state->gestures = NULL;
        } else {
            accel_gesture **tmp = (accel_gesture **)my_realloc(
                state->state->gestures, state->state->num_gestures_saved - 1, state->state->num_gestures_saved);
            if (tmp != NULL) {
                // If tmp is null, we don't really care that realloc failed, since a future use of realloc will help us.
                state->state->gestures = tmp;
            }
        }
        --(state->state->num_gestures_saved);
        return result;
    }
    state->state->gestures[*gesture]->is_recording = true;
    return ACCEL_SUCCESS;
}

// Taken from hackers delight
// http://www.hackersdelight.org/hdcodetxt/icbrt.c.txt
uint32_t icbrt1(uint32_t x) {
    int32_t s;
    uint32_t y, b;

    y = 0;
    for (s = 30; s >= 0; s = s - 3) {
        y = 2 * y;
        b = (3 * y * (y + 1) + 1) << s;
        if (x >= b) {
            x = x - b;
            y = y + 1;
        }
    }
    return y;
}

// The uWave paper suggests a mapping from [-20, 20]->[-15, 15], but cube root
// should to work better for variable ranges.
// TODO: revisit this decision.
int32_t normalize(int32_t sum) {
    if (sum < 0) {
        return (int)-cbrtf(-((float)sum));
    }
    return -(int)-cbrtf(((float)sum));
}

int reset_gesture(accel_gesture *gest, const uint32_t dimensions) {
    PRECONDITION_NOT_NULL(gest);
    for (uint16_t i = 0; i < gest->recording_size; ++i) {
        gest->offsets[i] = INT16_MAX;
    }
    for (uint32_t d = 0; d < dimensions; ++d) {
        reset_moving_avg(gest->moving_avg_values[d]);
    }
    return ACCEL_SUCCESS;
}

// TODO: does this work for zero recorded timestamps?
int accel_end_record_gesture(accel_state *state, uint16_t gesture_id) {
    PRECONDITION_VALID_STATE(state);

    internal_accel_state *istate = state->state;
    // TODO: gesture_id == istate->num_gestures_saved is incorrect.
    // Write a test for it and fix it.
    if (gesture_id > istate->num_gestures_saved) {
        return ACCEL_PARAM_ERROR;
    }
    if (istate->gestures[gesture_id] == NULL) {
        return ACCEL_INTERNAL_ERROR;
    }
    accel_gesture *gesture = istate->gestures[gesture_id];
    if (!(gesture->is_recording)) {
        return ACCEL_PARAM_ERROR;
    }
    if (gesture->is_recorded) {
        return ACCEL_INTERNAL_ERROR;
    }
    if (gesture->recording_size == 0) {
        return ACCEL_PARAM_ERROR;
    }

    gesture->offsets = (int32_t *)malloc(gesture->recording_size * sizeof(int32_t));
    if (gesture->offsets == NULL) {
        return ACCEL_MALLOC_ERROR;
    }

    int reset_result = reset_gesture(gesture, state->dimensions);
    if (reset_result != ACCEL_SUCCESS) {
        free(gesture->offsets);
        gesture->offsets = NULL;
    } else {
        gesture->is_recording = false;
        gesture->is_recorded = true;
    }

    for (uint16_t i = 0; i < gesture->recording_size; ++i) {
        gesture->offsets[i] = INT16_MAX;
    }
    for (uint32_t d = 0; d < state->dimensions; ++d) {
        reset_moving_avg(gesture->moving_avg_values[d]);
    }
    return ACCEL_SUCCESS;
}

// TODO: gracefully handle malloc failure in this function.
// TODO: this should return error types instead of being void.
// Follow-up: find usages of this method.
void handle_recording_tick(accel_gesture *gesture, uint32_t dimensions) {
    if (gesture == NULL) {
        return;
    }
    // TODO: grow exponentially, not linearly. Linear growth allocates too frequently.
    if (gesture->recording_size != 0) {
        gesture->normalized_recording =
            (int32_t **)my_realloc(gesture->normalized_recording, (gesture->recording_size + 1) * sizeof(int32_t *),
                                   gesture->recording_size * sizeof(int32_t *));
        if (gesture->normalized_recording == NULL) {
            return;
        }
    } else {
        gesture->normalized_recording = (int32_t **)malloc(sizeof(int32_t *));
    }
    gesture->normalized_recording[gesture->recording_size] = (int32_t *)malloc(sizeof(int32_t) * dimensions);
    for (uint32_t i = 0; i < dimensions; ++i) {
        // TODO: fix this int/float business.
        // TODO: complain about invalid return values.
        get_latest_frame_moving_avg(gesture->moving_avg_values[i],
                                    &(gesture->normalized_recording[gesture->recording_size][i]));
        gesture->normalized_recording[gesture->recording_size][i] =
            normalize(gesture->normalized_recording[gesture->recording_size][i]);
    }
    ++(gesture->recording_size);
}

int handle_evaluation_tick(accel_state *state, accel_gesture *gesture, int gesture_id) {
    // TODO: load the input at the beginning instead of gesture->recording_size times.
    PRECONDITION_NOT_NULL(gesture);
    uint32_t dimensions = state->dimensions;

    if (gesture->moving_avg_values == NULL || gesture->offsets == NULL) {
        return ACCEL_INTERNAL_ERROR;
    }

    uint16_t i = gesture->recording_size;
    while (i != 0) {
        --i;

        int cost = 0;
        for (uint32_t d = 0; d < dimensions; ++d) {
            int recording_i_d = gesture->normalized_recording[i][d];
            int32_t input_i_d = 0;
            // TODO: complain about invalid return values.
            get_latest_frame_moving_avg(gesture->moving_avg_values[d], &input_i_d);
            input_i_d = normalize(input_i_d);
            if (recording_i_d > input_i_d) {
                cost += recording_i_d - input_i_d;
            } else {
                // recording_i_d <= input_i_d
                cost += input_i_d - recording_i_d;
            }
        }
        if (i == 0) {
            gesture->offsets[i] = cost;
        } else {
            gesture->offsets[i] = MIN((int)(ALPHA * gesture->offsets[i]), cost + gesture->offsets[i - 1]);
        }
    }
    for (i = 1; i < gesture->recording_size; ++i) {
        int cost = 0;
        for (uint32_t d = 0; d < dimensions; ++d) {
            int recording_i_d = gesture->normalized_recording[i][d];
            int32_t input_i_d = 0;
            // TODO: complain about invalid return values.
            get_latest_frame_moving_avg(gesture->moving_avg_values[d], &input_i_d);
            if (recording_i_d > input_i_d) {
                cost += recording_i_d - input_i_d;
            } else {
                // recording_i_d <= input_i_d
                cost += input_i_d - recording_i_d;
            }
        }
        gesture->offsets[i] = MIN(gesture->offsets[i], gesture->offsets[i - 1] + cost);
    }
    if (state->callback != NULL) {
        if (state->state->threshold == 0) {
            return ACCEL_PARAM_ERROR;
        }
        float avg_affinity = ((float)gesture->offsets[gesture->recording_size - 1]) / gesture->recording_size;
        if (avg_affinity < state->state->threshold) {
            bool reset;
            int retval = state->callback(state, gesture_id, (int32_t)avg_affinity, &reset);
            if (reset == true) {
                reset_gesture(gesture, dimensions);
            }
            return retval;
        }
    }
    return ACCEL_SUCCESS;
}

int accel_process_timer_tick(accel_state *state, int *accel_data) {
    PRECONDITION_VALID_STATE(state);
    PRECONDITION_NOT_NULL(accel_data);

    int retcode = ACCEL_SUCCESS;
    for (int gesture_id = 0; gesture_id < state->state->num_gestures_saved; ++gesture_id) {
        accel_gesture *gesture = state->state->gestures[gesture_id];
        if (gesture == NULL) {
            retcode = ACCEL_INTERNAL_ERROR;
            continue;
        }
        if (!gesture->is_recording && !gesture->is_recorded) {
            retcode = ACCEL_INTERNAL_ERROR;
            continue;
        }
        if (gesture->moving_avg_values == NULL) {
            retcode = ACCEL_INTERNAL_ERROR;
            continue;
        }
        // If the moving average is at a final line.
        bool avg_line = false;
        int returned = ACCEL_SUCCESS;
        for (uint32_t d = 0; d < state->dimensions && returned == 0; ++d) {
            returned = append_to_moving_avg(gesture->moving_avg_values[d], accel_data[d], &avg_line);
        }
        if (returned != ACCEL_SUCCESS) {
            retcode = returned;
            continue;
        }

        if (!avg_line) {
            continue;
        }

        returned = ACCEL_SUCCESS;
        if (gesture->is_recording) {
            handle_recording_tick(gesture, state->dimensions);
        } else if (gesture->is_recorded) {
            returned = handle_evaluation_tick(state, gesture, gesture_id);
            if (returned != ACCEL_SUCCESS) {
                retcode = returned;
            }
        } else {
            retcode = ACCEL_INTERNAL_ERROR;
            continue;
        }
    }
    return retcode;
}

int accel_find_most_likely_gesture(accel_state *state, uint16_t *gesture_id, int *offset) {
    PRECONDITION_VALID_STATE(state);
    PRECONDITION_NOT_NULL(gesture_id);
    PRECONDITION_NOT_NULL(offset);

    *gesture_id = UINT16_MAX;
    *offset = ACCEL_NO_VALID_GESTURE;

    if (state->state->num_gestures_saved == 0) {
        return ACCEL_NO_VALID_GESTURE;
    }

    if (state->state->gestures == NULL) {
        return ACCEL_INTERNAL_ERROR;
    }

    for (uint16_t i = 0; i < state->state->num_gestures_saved; ++i) {
        accel_gesture *gesture = state->state->gestures[i];

        // TODO: Should this be tested?
        if (gesture == NULL) {
            return ACCEL_INTERNAL_ERROR;
        }

        // Both should be the default or changed at the same time. We have a programming error otherwise.
        if ((*gesture_id == UINT16_MAX) != (*offset == ACCEL_NO_VALID_GESTURE)) {
            return ACCEL_INTERNAL_ERROR;
        }

        if (!gesture->is_recorded) {
            // This gesture is not ready yet.
            continue;
        }

        if (gesture->recording_size == 0) {
            // This gesture will cause an error. May as well skip it and log an error.
            continue;
        }

        if (*offset == ACCEL_NO_VALID_GESTURE || gesture->offsets[gesture->recording_size - 1] < *offset) {
            *offset = gesture->offsets[gesture->recording_size - 1];
            *gesture_id = i;
        }
    }
    if (*gesture_id == UINT16_MAX || *offset == ACCEL_NO_VALID_GESTURE) {
        return ACCEL_NO_VALID_GESTURE;
    }
    return ACCEL_SUCCESS;
}

int accel_reset_affinities_for_gesture(accel_state *state, int gesture_id) {
    PRECONDITION_VALID_STATE(state);
    PRECONDITION_NOT_NULL(state->state);
    PRECONDITION_TRUE_PARAM((state->state->num_gestures_saved > gesture_id));

    accel_gesture *gesture = state->state->gestures[gesture_id];

    PRECONDITION_TRUE_PARAM(!gesture->is_recording);
    PRECONDITION_TRUE_PARAM(gesture->is_recorded);

    return reset_gesture(gesture, state->dimensions);
}
