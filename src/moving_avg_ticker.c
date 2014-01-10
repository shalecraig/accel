#include <string.h>
#include <stdlib.h>

#include "moving_avg_ticker.h"

int allocate_moving_avg(int num_wbuf, int subtotal_sizes, moving_avg_values **allocated) {
    if (*allocated != NULL) {
        // TODO: complain about invalid input.
        return MOVING_AVG_PARAM_ERROR;
    }
    *allocated = NULL;
    if (num_wbuf == 0) {
        // TODO: complain about invalid input.
        return MOVING_AVG_PARAM_ERROR;
    }
    if (subtotal_sizes == 0) {
        // TODO: complain about invalid input.
        return MOVING_AVG_PARAM_ERROR;
    }
    size_t size = sizeof(moving_avg_values);
    if (subtotal_sizes == 0) {
        // TODO: log an error.
        return MOVING_AVG_INTERNAL_ERROR;
    }

    *allocated = (moving_avg_values *) malloc(size);
    if (allocated == NULL) {
        // TODO: info log
        return MOVING_AVG_MALLOC_ERROR;
    }
    memset(*allocated, 0, size);
    (*allocated)->max_subtotal_size = subtotal_sizes;

    int *wbuf = (int *) calloc(num_wbuf, sizeof(int));
    if (wbuf == NULL) {
        free (allocated);
        *allocated = NULL;
        return MOVING_AVG_MALLOC_ERROR;
    }
    (*allocated)->wbuf = wbuf;
    (*allocated)->wbuf_len = num_wbuf;
    return 0;
}

int reset_moving_avg(moving_avg_values *reset) {
    // TODO: complain about invalid input.
    if (reset == NULL) {
        // TODO: complain about invalid input.
        return MOVING_AVG_PARAM_ERROR;
    }
    if (reset->wbuf == NULL) {
        // TODO: complain about the lack of struct integrity.
        return MOVING_AVG_INTERNAL_ERROR;
    }

    // TODO: use unsigned ints instead so we only need to check for equality with 0.
    if (reset->wbuf_len <= 0) {
        // TODO: complain about the lack of struct integrity.
        return MOVING_AVG_INTERNAL_ERROR;
    }

    memset(reset->wbuf, 0, reset->wbuf_len);
    reset->wbuf_end = reset->wbuf_len - 1;
    reset->subtotal = 0;
    reset->subtotal_size = 0;
    return 0;
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

