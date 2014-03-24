#include <string.h>
#include <stdlib.h>

#include "moving_avg_ticker.h"

// TODO: move this (and the accel stuff) into a header file.
#define PRECONDITION_NOT_NULL(foo)                                                                                     \
    if (foo == NULL) {                                                                                                 \
        return MOVING_AVG_PARAM_ERROR;                                                                                 \
    }

#define PRECONDITION_NULL(foo)                                                                                         \
    if (foo != NULL) {                                                                                                 \
        return MOVING_AVG_PARAM_ERROR;                                                                                 \
    }

// TODO: make this into a macro.
int precondition_valid_moving_avg_values(moving_avg_values *input) {
    PRECONDITION_NOT_NULL(input);

    if (input->wbuf == NULL) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->wbuf_len == 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->wbuf_len < input->wbuf_end) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->subtotal_size >= input->max_subtotal_size) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    if (input->max_subtotal_size <= 0) {
        return MOVING_AVG_INTERNAL_ERROR;
    }
    return MOVING_AVG_SUCCESS;
}

int allocate_moving_avg(uint32_t num_wbuf, uint32_t subtotal_sizes, moving_avg_values **allocated) {
    PRECONDITION_NOT_NULL(allocated);
    PRECONDITION_NULL(*allocated);

    if (*allocated != NULL) {
        return MOVING_AVG_PARAM_ERROR;
    }
    if (num_wbuf == 0) {
        return MOVING_AVG_PARAM_ERROR;
    }
    if (subtotal_sizes == 0) {
        return MOVING_AVG_PARAM_ERROR;
    }
    size_t size = sizeof(moving_avg_values);

    *allocated = (moving_avg_values *)malloc(size);
    if (allocated == NULL) {
        return MOVING_AVG_MALLOC_ERROR;
    }
    memset(*allocated, 0, size);
    (*allocated)->max_subtotal_size = subtotal_sizes;

    int16_t *wbuf = (int16_t *)calloc(num_wbuf, sizeof(int16_t));
    if (wbuf == NULL) {
        // Run away, fast!
        free(allocated);
        *allocated = NULL;
        return MOVING_AVG_MALLOC_ERROR;
    }
    (*allocated)->wbuf = wbuf;
    (*allocated)->wbuf_len = num_wbuf;
    return precondition_valid_moving_avg_values(*allocated);
}

int reset_moving_avg(moving_avg_values *reset) {
    int value = precondition_valid_moving_avg_values(reset);
    if (value != MOVING_AVG_SUCCESS) {
        return value;
    }

    memset(reset->wbuf, 0, reset->wbuf_len);
    reset->wbuf_end = reset->wbuf_len - 1;
    reset->subtotal = 0;
    reset->subtotal_size = 0;
    return MOVING_AVG_SUCCESS;
}

int append_to_moving_avg(moving_avg_values *value, int16_t appended, bool *is_at_end) {
    int is_valid_return_value = precondition_valid_moving_avg_values(value);
    if (is_valid_return_value != MOVING_AVG_SUCCESS) {
        return is_valid_return_value;
    }

    PRECONDITION_NOT_NULL(is_at_end);

    ++value->subtotal_size;
    value->subtotal += appended;
    if (value->subtotal_size != value->max_subtotal_size) {
        *is_at_end = false;
        return MOVING_AVG_SUCCESS;
    }

    value->wbuf_end = (value->wbuf_end + 1) % value->wbuf_len;
    value->wbuf[value->wbuf_end] = value->subtotal;

    value->subtotal = 0;
    value->subtotal_size = 0;
    *is_at_end = true;
    return MOVING_AVG_SUCCESS;
}

int get_latest_frame_moving_avg(moving_avg_values *value, int32_t *frame) {
    int is_valid_return_value = precondition_valid_moving_avg_values(value);
    if (is_valid_return_value != MOVING_AVG_SUCCESS) {
        return is_valid_return_value;
    }

    PRECONDITION_NOT_NULL(frame);

    float sum = 0;
    for (uint32_t i = 0; i < value->wbuf_len; ++i) {
        sum += value->wbuf[i] * 1.0 / value->wbuf_len;
    }
    *frame = (int32_t)sum;
    return MOVING_AVG_SUCCESS;
}

int free_moving_avg(moving_avg_values **value) {
    PRECONDITION_NOT_NULL(value);
    PRECONDITION_NOT_NULL((*value));

    if ((*value)->wbuf != NULL) {
        free((*value)->wbuf);
        (*value)->wbuf = NULL;
    }
    free(*value);
    *value = NULL;
    return MOVING_AVG_SUCCESS;
}
