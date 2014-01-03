#include <stdlib.h>
#include <string.h>

#include "muwave.h"

/* Creation and deletion of muwave state objects. */
muwave_state *muwave_generatestate(int dimensions, int window_size) {

    size_t internal_size = sizeof(muwave_state);

    muwave_state *state = (muwave_state *) malloc(internal_size);
    memset(state, 0, internal_size);
    state->error_bit = false;
    state->dimensions = dimensions;
    state->window_size = window_size > 0 ? window_size : 2;
    return state;
}

void muwave_destroy_gesture(muwave_gesture *gesture) {
    if (gesture == NULL) {
        return;
    }
    // TODO: free more than just this.
    free(gesture);
    gesture = NULL;
}

void muwave_destroy_state(muwave_state *state) {
    if (state == NULL) { return; }
    /* TODO: remove all additional fields inside the muwave_state variable */
    for (int i=0; i<state->num_gestures_saved; ++i) {
        muwave_destroy_gesture(state->gestures[i]);
    }
    free(state);
    state = NULL;
}

int muwave_start_record_gesture(muwave_state *state) {
    if (state->num_gestures_saved != 0) {
        state->gestures = (muwave_gesture **)realloc(state->gestures, (state->num_gestures_saved + 1)*sizeof(muwave_gesture *));
    } else {
        state->gestures = (muwave_gesture **)malloc(sizeof(muwave_gesture *));
    }
    int gesture_id = ++state->num_gestures_saved;

    size_t gesture_size = sizeof(muwave_gesture);
    state->gestures[gesture_id] = malloc(gesture_size);
    memset(state->gestures[gesture_id], 0, gesture_size);
    state->gestures[gesture_id]->is_recording = true;
    state->gestures[gesture_id]->is_recorded = false;
    return gesture_id;
}

// hand-tuned utility function
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
void muwave_end_record_gesture(muwave_state *state, int gesture_id) {
    // TODO: Check with a macro for error bit, nullity with error logging.
    if (state == NULL) {return;}
    // TODO: use specific types in c instead of this stuff.
    if (gesture_id < 0) {return;}
    // TODO: complain about an invalid value.
    if (state->num_gestures_saved <= gesture_id) {return;}
    // TODO: use the assert library instead. This should never happen unless muwave doesn't have integrity.
    if (state->gestures[gesture_id] == NULL) {return;}
    // TODO: complain about this with a message.
    if (state->gestures[gesture_id]->is_recording) {return;}
    // TODO: use the assert library instead. This should never happen unless muwave doesn't have integrity.
    if (state->gestures[gesture_id]->is_recorded) {return;}

    muwave_gesture *gesture = state->gestures[gesture_id];
    gesture->is_recording = false;

    // allocate normalized array

    // TODO: verify if safe to divide and assume a round-down.
    int normalized_recording_len = gesture->recording_size - gesture->recording_size%state->window_size;
    normalized_recording_len = normalized_recording_len / state->window_size;

    gesture->normalized_recording = calloc(normalized_recording_len, sizeof(int*));

    // convert from raw to normalized
    for (int i=0; i<normalized_recording_len; ++i) {
        int16_t *sum = calloc(state->dimensions, sizeof(int16_t));
        int j_end = (i+1)*state->window_size;
        for (int j=i*state->window_size; j<=j_end; ++j) {
            for (int d=0; d<state->dimensions; ++d) {
                sum[d] += gesture->raw_recording[j][d];
            }
        }

        gesture->normalized_recording[i] = malloc(state->dimensions * sizeof(int));
        for (int d=0; d<state->dimensions; ++d) {
            gesture->normalized_recording[i][d] = normalize(sum[d]);
        }
    }

    // free raw
    for(i=0; i<gesture->recording_size; ++i) {
        free(gesture->raw_recording[i]);
        gesture->raw_recording[i] = NULL;
    }
    free(gesture->raw_recording);

    // Set the new recording length
    gesture->recording_size = normalized_recording_len;

    state->gestures[gesture_id]->is_recorded = true;
}

void handle_recording_tick(muwave_gesture *gesture, int *accel_data, int dimensions) {
    if (gesture == NULL) { return; }
    // TODO: grow exponentially, not linearly. Linearlly sucks.
    if (gesture->recording_size != 0) {
        gesture->raw_recording = (int **) realloc(gesture->raw_recording, (gesture->recording_size + 1) * sizeof(int *));
    } else {
        gesture->raw_recording = (int **) malloc(sizeof(int *));
    }
    gesture->raw_recording[gesture->recording_size] = malloc(sizeof(int) * dimensions);
    for (int i=0; i<dimensions; ++i) {
        gesture->raw_recording[gesture->recording_size][i] = accel_data[i];
    }
    ++gesture->recording_size;
}

void handle_evaluation_tick(muwave_gesture *gesture, int *accel_data, int dimensions) {
    // TODO: implement DTW algoritm
}

/**
 * Updates the uWave algorithm's DTW-compare step across all gestures.
 * @param state    state being recorded
 * @param accel_data a array with accelerometer data
 */
void muwave_process_timer_tick(muwave_state *state, int *accel_data) {
    if (state == NULL) { return; }
    if (accel_data == NULL) { return; }
    // TODO: conditionally define an ARRAY_LENGTH macro?
    // #define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
    if (ARRAY_LENGTH(accel_data) != state->dimensions) { return; }

    for (int gesture_iter = 0; gesture_iter < state->num_gestures_saved; ++gesture_iter) {
        muwave_gesture *gesture = state->gestures[gesture_iter];
        // TODO: complain about struct integrity issues.
        if (gesture == NULL) { continue; }
        // TODO: complain about struct integrity issues.
        if (!gesture->is_recording && !gesture->is_recorded) { continue; }

        if (gesture->is_recording) {
            handle_recording_tick(gesture, accel_data, state->dimensions);
        } else if (gesture->is_recorded) {
            handle_evaluation_tick(gesture, accel_data, state->dimensions);
        } else {
            // TODO: complain that we need to have one of these two categories.
            continue;
        }
    }
}
