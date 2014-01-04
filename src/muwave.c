#include <stdlib.h>
#include <string.h>

#include "muwave.h"

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

moving_avg_values *allocate_moving_avg_struct(int num_wbuf, int subtotal_sizes) {
    size_t size = sizeof(moving_avg_values);
    moving_avg_values *returned = (moving_avg_values *) malloc(size);
    memset(returned, 0, size);
    returned->max_subtotal_size = subtotal_sizes;

    int *wbuf = (int *) calloc(num_wbuf, sizeof(int));
    returned->wbuf = wbuf;
    returned->wbuf_len = num_wbuf;
    return returned;
}

void reset_moving_avg(moving_avg_values * reset) {
    // TODO: complain about invalid input.
    if (reset == NULL) { return; }
    if (reset->wbuf != NULL) {
        memset(reset->wbuf, 0, reset->wbuf_len);
    }
    reset->wbuf_end = 0;
    reset->subtotal = 0;
    reset->subtotal_size = 0;
}

bool append_to_moving_avg(moving_avg_values *value, int appended) {
    // TODO: complain about the error
    if (value == NULL) { return false; }
    ++value->subtotal_size;
    value->subtotal += appended;
    if (value->subtotal_size != value->max_subtotal_size) {
        return false;
    }
    value->wbuf_end = (value->wbuf_end + 1) % value->wbuf_len;
    value->wbuf[value->wbuf_end] = value->subtotal;

    value->subtotal = 0;
    value->subtotal_size = 0;
    return true;
}

int get_latest_frame_moving_avg(moving_avg_values *value) {
    // TODO: complain about the invalid input.
    if (value == NULL) { return 0; }
    int sum = 0;
    for (int i=0; i<value->wbuf_len; ++i) {
        sum += value->wbuf[i];
    }
    return sum / value->wbuf_len;
}

muwave_gesture *muwave_generate_gesture(muwave_state *state) {
    size_t gesture_size = sizeof(muwave_gesture);
    muwave_gesture *g = malloc(gesture_size);
    memset(g, 0, gesture_size);
    g->is_recording = false;
    g->is_recorded = false;
    g->raw_recording = (int **) malloc(sizeof(int *));
    g->normalized_recording = NULL;

    // TODO: these shouldn't both be the same....
    g->moving_avg_values = allocate_moving_avg_struct(state->window_size, state->window_size);
    return g;
}

void muwave_destroy_gesture(muwave_gesture *gesture) {
    if (gesture == NULL) {
        return;
    }
    // TODO: free more than just this.
    free(gesture);
    gesture = NULL;
}

/* Creation and deletion of muwave state objects. */
muwave_state *muwave_generate_state(int dimensions, int window_size) {

    size_t internal_size = sizeof(muwave_state);

    muwave_state *state = (muwave_state *) malloc(internal_size);
    memset(state, 0, internal_size);
    state->error_bit = false;
    state->dimensions = dimensions;
    state->window_size = window_size > 0 ? window_size : 2;
    return state;
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

    state->gestures[gesture_id] = muwave_generate_gesture();
    state->gestures[gesture_id]->is_recording = true;
    return gesture_id;
}

// TODO: adjust the hand-tuned utility function using params from the uWave algorithm
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
    // TODO: this can be done much more efficiently, re-implement once testing is up.
    for (int i=0; i<gesture->recording_size; ++i) {
        if (i%state->window_size != state->window_size-1) { continue; }
        int *sum = malloc(state->dimensions * sizeof(int));
        for (int d=0; d<state->dimensions; ++d) {
            int num_counted = 0;
            for (int j=MAX(0, i-state->window_size); j<i; ++j) {
                sum[d] += gesture->raw_recording[j][d];
                ++num_counted;
            }
            sum[d] /= num_counted;
            sum[d] = normalize(sum[d]);
        }
        int normalized_id = (i+1)/state->window_size;
        gesture->normalized_recording[normalized_id] = sum;
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
    // TODO: actually complain about these issues.
    if (gesture == NULL) { return; }
    if (accel_data == NULL) { return; }
    if (ARRAY_LENGTH(accel_data) != dimensions) { return; }

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
