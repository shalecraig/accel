#include <string.h>
#include <stdlib.h>

#include "moving_avg_ticker.h"

int precondition_valid_moving_avg_values(moving_avg_values *input) {
    if (input == NULL) {
        // TODO: log that this is incorrect input.
        return MOVING_AVG_PARAM_ERROR;
    }
    if (input->wbuf == NULL) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->wbuf_end < 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->wbuf_len <= 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->subtotal_size < 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->subtotal_size >= input->max_subtotal_size) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->max_subtotal_size <= 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    return 0;
}

int allocate_moving_avg(int num_wbuf, int subtotal_sizes, moving_avg_values **allocated) {
    if (*allocated != NULL) {
        // TODO: complain about invalid input.
        return MOVING_AVG_PARAM_ERROR;
    }
    *allocated = NULL;
    // TODO: use an unsigned int instead.
    if (num_wbuf <= 0) {
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
    int value = precondition_valid_moving_avg_values(reset);
    if (value != 0) {return value;}

    memset(reset->wbuf, 0, reset->wbuf_len);
    reset->wbuf_end = reset->wbuf_len - 1;
    reset->subtotal = 0;
    reset->subtotal_size = 0;
    return 0;
}

int append_to_moving_avg(moving_avg_values *value, int appended, bool* is_at_end) {
    int is_valid_return_value = precondition_valid_moving_avg_values(value);
    if (is_valid_return_value != 0) {return is_valid_return_value;}

    if (is_at_end == NULL) {
        // TODO: log this?
        return MOVING_AVG_PARAM_ERROR;
    }

    ++value->subtotal_size;
    value->subtotal += appended;
    if (value->subtotal_size != value->max_subtotal_size) {
        *is_at_end = false;
        return 0;
    }
    value->wbuf_end = (value->wbuf_end + 1) % value->wbuf_len;
    value->wbuf[value->wbuf_end] = value->subtotal;

    value->subtotal = 0;
    value->subtotal_size = 0;
    *is_at_end = true;
    return 0;
}

int get_latest_frame_moving_avg(moving_avg_values *value, float *frame) {
    int is_valid_return_value = precondition_valid_moving_avg_values(value);
    if (is_valid_return_value != 0) {return is_valid_return_value;}

    if (frame == NULL) {
        return MOVING_AVG_PARAM_ERROR;
    }

    int sum = 0;
    for (int i=0; i<value->wbuf_len; ++i) {
        sum += value->wbuf[i];
    }
    *frame = sum * 1.0 / value->wbuf_len;
    return 0;
}

int free_moving_avg(moving_avg_values **value) {
    if (*value == NULL) {
        // TODO: complain about bad input.
        return MOVING_AVG_PARAM_ERROR;
    }
    if ((*value)->wbuf == NULL) {
        // TODO: is this the best way to do this? (complain but do the right thing?)
        free (*value);
        *value = NULL;
        return MOVING_AVG_INTERNAL_ERROR;
    }
    free((*value)->wbuf);
    (*value)->wbuf = NULL;
    free(*value);
    *value = NULL;
    return 0;
}
