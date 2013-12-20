/*
    WARNING: This is in not-even-thinking-about-alpha state.
 */
#include <stdbool.h>
#include <stdlib.h>

#include "muwave.h"
/*
    http://stackoverflow.com/questions/3437404/min-and-MAX-in-c
 */
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/*
TODOs:
    Create header file
    Document
    Check for failed mallocs (malloc will return null.)
    Remove TODOs
 */

/* d-DIMENSION arrays */
int32_t *normalized_data = NULL;
int32_t *acc_data = NULL;

/* Internal sums of the normalized data. MUWAVE_QUAN_MOVING_AVG_SIZE x DIMENSION */
uint32_t i_sum_index = 0;
uint32_t i_num_summed = 0;
int32_t **i_sums = NULL;

uint32_t *affinities = NULL;

/* TODO: allow multiple recordings. */
uint16_t recording_length = 0;
uint16_t recording_arr_length = 0;
int32_t **recording = NULL;

bool is_recording = false;
bool completed_recording = false;

/* TODO: Actually complain, don't just use true/false. */
bool normalize() {
    if (acc_data == NULL) {
        return false;
    }
    if (normalized_data == NULL && i_sums == NULL) {
        /* Setup for the first time. */
        /* TODO: write cleanup routine. */
        normalized_data = malloc(MUWAVE_DIMENSION*sizeof(int32_t));
        i_sums = malloc(sizeof(int32_t*)*MUWAVE_QUAN_MOVING_AVG_SIZE);
        for (uint8_t i=0; i<MUWAVE_QUAN_MOVING_AVG_SIZE; ++i) {
            i_sums[i] = malloc(MUWAVE_DIMENSION*sizeof(int32_t));
        }
    } else if (normalized_data == NULL || i_sums == NULL) {
        return false;
    }
    for (uint16_t i=0; i<MUWAVE_DIMENSION; ++i) {
        i_sums[i_sum_index][i] += acc_data[i];
    }

    ++i_num_summed;
    if (i_num_summed >= MUWAVE_QUAN_POINT_SIZE) {
        i_num_summed = 0;
        for (uint16_t i=0; i<MUWAVE_DIMENSION; ++i) {
            uint32_t super_smooth = 0;
            for (uint16_t j=0; j<MUWAVE_QUAN_MOVING_AVG_SIZE; ++j) {
                super_smooth += i_sums[j][i];
            }
            /* TODO: Clean up the math. */
            if (super_smooth > 20) {
                normalized_data[i] = 16;
            } else if (super_smooth > 10) {
                normalized_data[i] = 10 + (super_smooth - 10) / 10 * 5;
            } else if (super_smooth <= 10 && super_smooth >= -10) {
                normalized_data[i] = super_smooth;
            } else if (super_smooth < -10) {
                normalized_data[i] = -10 + (super_smooth + 10) / 10 * 5;
            } else if (super_smooth < -20) {
                normalized_data[i] = -16;
            } else {
                /* Impossible? */
                return false;
            }
        }
        /* Deal with moving around this circular buffer. */
        i_sum_index = (i_sum_index + 1) % MUWAVE_QUAN_MOVING_AVG_SIZE;
        return true;
    }
    return false;
}

uint16_t tabulate_cost(uint16_t i) {
    uint16_t cost = 0;
    for (uint16_t d=0; d<MUWAVE_DIMENSION; ++d) {
        cost += abs(recording[i][d] - normalized_data[d]);
    }
    return cost;
}

void update_affinities() {
    /* TODO: verify the recording length is set. */
    /* TODO: Exponential decay */
    uint16_t i = 0;
    if (affinities == NULL) {
        /* allocate affinities. */
        affinities = malloc(sizeof(int32_t)*recording_length);
        for (i=recording_length; i>0; --i) {
            /* TODO: This is somewhere around int32_t MAX_SIZE... Figure it out */
            affinities[i] = 1000000000;
        }
    }
    /* Advance the DTW algorithm */
    affinities[0] = 0;
    for (i=recording_length; i>0; --i) {
        uint16_t cost = tabulate_cost(i);
        affinities[i]= cost + MIN(affinities[i], affinities[i-1]);
    }
    for (i=0; i<=recording_length; ++i) {
        uint16_t cost = tabulate_cost(i);
        affinities[i] = MIN(affinities[i], cost+affinities[i-1]);
    }
}

/**
 * Append normalized_data[recording_length] to the recording array, growing the
 * recording array dynamically.
 * @return true if successful, false otherwise
 */
bool append_to_recording() {
    if (!is_recording || completed_recording) {
        /* TODO: actually complain about this */
        return false;
    }
    if (recording_length >= recording_arr_length) {
        /* TODO: prevent and check overflowing 2*recording_arr_length */
        if (recording_arr_length*2 > MUWAVE_RECORD_MAX_LENGTH) {
            return false;
        }
        int32_t **old_rec = recording;
        recording = malloc(sizeof(int32_t*) * recording_arr_length*2);
        uint16_t i=0;
        for (; i<recording_length; ++i) {
            recording[i] = old_rec[i];
        }
        free(old_rec);
        old_rec = NULL;
        for (; i<recording_arr_length*2; ++i) {
            recording[i] = malloc(sizeof(int32_t)*MUWAVE_DIMENSION);
        }
        recording_arr_length *= 2;
    }
    for (uint16_t d=0; d<MUWAVE_DIMENSION; ++d) {
        recording[recording_length][d] = normalized_data[d];
    }
    ++recording_length;
    return true;
}

void begin_recording() {
    uint16_t i=0;
    /* TODO: Reset the affinities array? */
    is_recording = true;
    recording_arr_length = 10;
    recording_length = 0;
    recording = malloc(sizeof(int32_t*) * recording_arr_length);
    for (i=0; i<recording_arr_length; ++i) {
        recording[i] = malloc(sizeof(int32_t)*MUWAVE_DIMENSION);
    }
}

void complete_recording() {
    /* TODO: Reset the affinities array? */
    is_recording = false;
    completed_recording = true;
    for (uint16_t rec_idx = recording_length; rec_idx < recording_arr_length; ++rec_idx) {
        free(recording[rec_idx]);
        recording[rec_idx] = NULL;
    }
    int32_t **old_rec = recording;
    recording = malloc(sizeof(int32_t*) * recording_length);
    for (uint16_t i=0; i<recording_length; ++i) {
        recording[i] = old_rec[i];
    }
    recording_arr_length = recording_length;
    free(old_rec);
    old_rec = NULL;
}

void handle_accel_tick() {
    /* TODO: setup the acc_data */
    bool end_of_summation = normalize();
    if (end_of_summation) {
        if (is_recording) {
            append_to_recording();
        } else {
            update_affinities();
        }
    }
}

void cleanup() {
    if (normalized_data != NULL) {
        free(normalized_data);
        normalized_data = NULL;
    }
    if (i_sums != NULL) {
        for (uint8_t i=0; i<MUWAVE_QUAN_MOVING_AVG_SIZE; ++i) {
            free(i_sums[i]);
            i_sums[i] = NULL;
        }
        free(i_sums);
        i_sums = NULL;
    }
    if (affinities != NULL) {
        free(affinities);
    }
}

