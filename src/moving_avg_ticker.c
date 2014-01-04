#include <string.h>
#include <stdlib.h>

#include "moving_avg_ticker.h"

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

