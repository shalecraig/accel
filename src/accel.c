#include <stdlib.h>
#include <string.h>

#include "accel.h"
#include "moving_avg_ticker.h"

#define PRECONDITION_NOT_NULL(foo) \
    if (foo == NULL) { return ACCEL_PARAM_ERROR; }

// Decay rate of values we choose to keep. 1.0 is no decay, 2.0 is a doubling every time we keep them.
// TODO: should we store the affinities as floats instead?
#define ALPHA 1.0

// TODO: check for failed allocation.

// TODO: include these from a header file?
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

// TODO: only define the ARRAY_LENGTH macro conditionally
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

int accel_generate_gesture(accel_state *state, accel_gesture **gesture) {
    PRECONDITION_NOT_NULL(state);
    PRECONDITION_NOT_NULL(gesture);

    size_t gesture_size = sizeof(accel_gesture);
    *gesture = (accel_gesture *) malloc(gesture_size);
    if (*gesture == NULL) {
        return ACCEL_MALLOC_ERROR;
    }
    memset(*gesture, 0, gesture_size);
    (*gesture)->is_recording = false;
    (*gesture)->is_recorded = false;
    (*gesture)->normalized_recording = NULL;

    (*gesture)->moving_avg_values = (moving_avg_values **) calloc(state->dimensions, sizeof(moving_avg_values *));
    if ((*gesture)->moving_avg_values == NULL) {
        free((*gesture));
        *gesture = NULL;
    }
    for (int i=0; i<state->dimensions; ++i) {
        // TODO: these two shouldn't both be the same....
        int result = allocate_moving_avg(state->window_size, state->window_size, &((*gesture)->moving_avg_values[i]));

        if (result != 0) {
            // Abort, since there isn't much we can do if it fails.
            for (int j=0; j<i; ++j) {
                free_moving_avg(&((*gesture)->moving_avg_values[j]));
            }
            free(gesture);
            return result;
        }
    }
    return 0;
}

void accel_destroy_gesture(accel_gesture **gesture) {
    // I suppose I successfully did no work?
    if (gesture == NULL || *gesture == NULL) {
        return;
    }

    // TODO: free more than just this.
    free(*gesture);
    *gesture = NULL;
}

int accel_generate_state(accel_state **state, int dimensions, int window_size) {
    PRECONDITION_NOT_NULL(state);
    if (dimensions <= 0) {
        return ACCEL_PARAM_ERROR;
    }
    if (window_size <= 0) {
        return ACCEL_PARAM_ERROR;
    }

    size_t internal_size = sizeof(accel_state);

    *state = (accel_state *) malloc(internal_size);
    if (state == NULL) {
        return ACCEL_MALLOC_ERROR;
    }

    memset((*state), 0, internal_size);
    (*state)->dimensions = dimensions;
    (*state)->window_size = window_size > 0 ? window_size : 2;
    return 0;
}

int accel_destroy_state(accel_state **state) {
    PRECONDITION_NOT_NULL(state);
    PRECONDITION_NOT_NULL(*state);

    /* TODO: remove all additional fields inside the accel_state variable */
    for (int i=0; i<(*state)->num_gestures_saved; ++i) {
        accel_destroy_gesture(&((*state)->gestures[i]));
    }
    free((*state));
    *state = NULL;
    return 0;
}

int accel_start_record_gesture(accel_state *state, int *gesture) {
    PRECONDITION_NOT_NULL(state);
    PRECONDITION_NOT_NULL(gesture);

    if (state->num_gestures_saved != 0) {
        accel_gesture **tmp = (accel_gesture **)realloc(state->gestures, (state->num_gestures_saved + 1)*sizeof(accel_gesture *));
        if (tmp == NULL) {
            return ACCEL_MALLOC_ERROR;
        }
        state->gestures = tmp;
    } else {
        state->gestures = (accel_gesture **)malloc(sizeof(accel_gesture *));
        if (state->gestures == NULL) {
            return ACCEL_MALLOC_ERROR;
        }
    }
    *gesture = (state->num_gestures_saved)++;

    int result = accel_generate_gesture(state, &(state->gestures[*gesture]));
    if (result != 0) {
        *gesture = -1;
        if (state->num_gestures_saved == 1) {
            free(state->gestures);
            state->gestures = NULL;
        } else {
            accel_gesture ** tmp = (accel_gesture **)realloc(state->gestures, state->num_gestures_saved - 1);
            if (tmp != NULL) {
                // If tmp is null, we don't really care that realloc failed, since a future use of realloc will help us.
                state->gestures = tmp;
            }
        }
        --(state->num_gestures_saved);
        return result;
    }
    state->gestures[*gesture]->is_recording = true;
    return 0;
}

// TODO: These arbitrarily chosen constants are from the uWave algorithm's paper, and have nothing to do with my implementation.
// Find better numbers that'll do instead, apparently pebble uses in the range of +-4k, so we should probably scale +-16k
int normalize(int sum) {
    if (sum > 20) {
        return 15;
    } else if (sum > 10) {
        return (sum-10)*5.0+10;
    } else if (sum > -10) {
        return sum;
    } else if (sum > -20) {
        return (sum+10)*5.0-10;
    } else { // sum <= -20
        return -15;
    }
}

// TODO: does this work for zero recorded timestamps?
int accel_end_record_gesture(accel_state *state, int gesture_id) {
    PRECONDITION_NOT_NULL(state);

    // TODO: use an unsigned int instead so we don't need to check for this type of error.
    if (gesture_id < 0) {
        return ACCEL_PARAM_ERROR;
    }
    // TODO: log the user's error.
    if (gesture_id > state->num_gestures_saved) {
        return ACCEL_PARAM_ERROR;
    }
    // TODO: log accel's error.
    if (state->gestures[gesture_id] == NULL) {
        return ACCEL_INTERNAL_ERROR;
    }
    // TODO: log the user's error.
    if (!(state->gestures[gesture_id]->is_recording)) {
        return ACCEL_PARAM_ERROR;
    }
    // TODO: log accel's error.
    if (state->gestures[gesture_id]->is_recorded) {
        return ACCEL_INTERNAL_ERROR;
    }

    accel_gesture *gesture = state->gestures[gesture_id];
    gesture->affinities = (int *) malloc(gesture->recording_size * sizeof(int));
    if (gesture->affinities == NULL) {
        return ACCEL_MALLOC_ERROR;
    }

    gesture->is_recording = false;
    gesture->is_recorded = true;

    for (int i=0; i<gesture->recording_size; ++i) {
        gesture->affinities[i] = INT16_MAX;
    }
    return 0;
}

// TOOD: check for malloc failure in this function.
void handle_recording_tick(accel_gesture *gesture, int dimensions) {
    if (gesture == NULL) { return; }
    // TODO: grow exponentially, not linearly. Linear growth allocates too frequently.
    if (gesture->recording_size != 0) {
        gesture->normalized_recording = (int **) realloc(gesture->normalized_recording, (gesture->recording_size + 1) * sizeof(int *));
    } else {
        gesture->normalized_recording = (int **) malloc(sizeof(int *));
    }
    gesture->normalized_recording[gesture->recording_size] = (int *) malloc(sizeof(int) * dimensions);
    for (int i=0; i<dimensions; ++i) {
        // TODO: fix this int/float business.
        // TODO: check resultant output.
        get_latest_frame_moving_avg(gesture->moving_avg_values[i], &(gesture->normalized_recording[gesture->recording_size][i]));
        gesture->normalized_recording[gesture->recording_size][i] = normalize(gesture->normalized_recording[gesture->recording_size][i]);
    }
    ++gesture->recording_size;
}

int handle_evaluation_tick(accel_gesture *gesture, int dimensions) {
    PRECONDITION_NOT_NULL(gesture);

    if (gesture->moving_avg_values == NULL) {
        // Internal because the gesture's avg values shouldn't be touched by end users.
        return ACCEL_INTERNAL_ERROR;
    }

    if (gesture->affinities == NULL) {
        // Internal because the gesture's affinities shouldn't be touched by end users.
        return ACCEL_INTERNAL_ERROR;
    }

    int i = gesture->recording_size;
    while (i != 0) {
        --i;

        int cost = 0;
        for (int d=0; d<dimensions; ++d) {
            int recording_i_d = gesture->normalized_recording[i][d];
            int input_i_d = 0;
            // TODO: complain about invalid return values.
            get_latest_frame_moving_avg(gesture->moving_avg_values[d], &input_i_d);
            if (recording_i_d > input_i_d) {
                cost += recording_i_d - input_i_d;
            } else {
                // recording_i_d <= input_i_d
                cost += input_i_d - recording_i_d;
            }
        }
        if (i == 0) {
            // TODO: should just be =cost
            gesture->affinities[i] = cost;
        } else {
            gesture->affinities[i] = MIN(ALPHA * gesture->affinities[i], cost+gesture->affinities[i-1]);
        }
    }
    for (i=1; i<gesture->recording_size; ++i) {
        // TODO: actually tabulate the cost.
        int cost = 0;
        for (int d=0; d<dimensions; ++d) {
            int recording_i_d = gesture->normalized_recording[i][d];
            int input_i_d = 0;
            // TODO: complain about invalid return values.
            get_latest_frame_moving_avg(gesture->moving_avg_values[d], &input_i_d);
            if (recording_i_d > input_i_d) {
                cost += recording_i_d - input_i_d;
            } else {
                // recording_i_d <= input_i_d
                cost += input_i_d - recording_i_d;
            }
        }
        gesture->affinities[i] = MIN(gesture->affinities[i], gesture->affinities[i-1] + cost);
    }
    return 0;
}

int accel_process_timer_tick(accel_state *state, int *accel_data) {
    if (state == NULL) { return ACCEL_PARAM_ERROR; }
    if (accel_data == NULL) { return ACCEL_PARAM_ERROR; }
    // TODO: verify this later.
    // if (ARRAY_LENGTH(accel_data) != state->dimensions) { return ACCEL_PARAM_ERROR; }

    int retcode = 0;
    for (int gesture_iter = 0; gesture_iter < state->num_gestures_saved; ++gesture_iter) {
        accel_gesture *gesture = state->gestures[gesture_iter];
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
        int returned = 0;
        for (int d=0; d<state->dimensions && returned == 0; ++d) {
            // TODO: verify that the return code isn't an error.
            returned = append_to_moving_avg(gesture->moving_avg_values[d], accel_data[d], &avg_line);
        }
        if (returned != 0) {
            retcode = returned;
            continue;
        }

        if (!avg_line) { continue; }

        returned = 0;
        if (gesture->is_recording) {
            // TODO: this should return error types instead of being void.
            handle_recording_tick(gesture, state->dimensions);
        } else if (gesture->is_recorded) {
            // TODO: if this returns something non-zero, complain about it.
            returned = handle_evaluation_tick(gesture, state->dimensions);
            if (returned != 0) {
                retcode = returned;
            }
        } else {
            // TODO: complain that we need to have one of these two categories.
            retcode = ACCEL_INTERNAL_ERROR;
            continue;
        }
    }
    return retcode;
}

int accel_find_most_likely_gesture(accel_state *state, int *gesture_id, int *affinity) {
    // TODO: complain about these, do them more formally
    if (state == NULL) {
        return ACCEL_PARAM_ERROR;
    }
    if (gesture_id == NULL) {
        return ACCEL_PARAM_ERROR;
    }
    if (affinity == NULL) {
        return ACCEL_PARAM_ERROR;
    }
    // TODO: error.log
    if (state->num_gestures_saved < 0) {
        return ACCEL_INTERNAL_ERROR;
    }

    if (state->num_gestures_saved == 0) {
        *gesture_id = ACCEL_NO_VALID_GESTURE;
        *affinity = ACCEL_NO_VALID_GESTURE;

        return ACCEL_NO_VALID_GESTURE;
    }

    // TODO: error.log
    if (state->gestures == NULL) {
        return ACCEL_INTERNAL_ERROR;
    }

    *gesture_id = ACCEL_NO_VALID_GESTURE;
    *affinity = ACCEL_NO_VALID_GESTURE;

    // TODO: there's a cleaner way to do some of the state->num_gestures_saved precondition stuff. Should it be explicit?
    // TODO: info.log
    for (int i=0; i<state->num_gestures_saved; ++i) {
        accel_gesture *gesture = state->gestures[i];
        // TODO: log error about integrity of the gestures.

        if ((*gesture_id == ACCEL_NO_VALID_GESTURE || *affinity == ACCEL_NO_VALID_GESTURE) &&
            *gesture_id != *affinity) {
            // TODO: debug/complain about internal consistency.
            continue;
        }

        if (*affinity == ACCEL_NO_VALID_GESTURE ||
            gesture->affinities[gesture->recording_size-1] < *affinity) {
            *affinity = gesture->affinities[gesture->recording_size-1];
            *gesture_id = i;
        }
    }
    if (*gesture_id == ACCEL_NO_VALID_GESTURE ||
        *affinity == ACCEL_NO_VALID_GESTURE) {
        return ACCEL_NO_VALID_GESTURE;
    }
    return 0;
}
