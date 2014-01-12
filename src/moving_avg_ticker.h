#ifndef ACCEL_AVG_TICKER
#define ACCEL_AVG_TICKER

#include "accel.h"

#define MOVING_AVG_PARAM_ERROR ACCEL_PARAM_ERROR
#define MOVING_AVG_INTERNAL_ERROR ACCEL_INTERNAL_ERROR
#define MOVING_AVG_MALLOC_ERROR ACCEL_MALLOC_ERROR

typedef struct moving_avg_values {
    // Circular buffer
    int *wbuf;
    int wbuf_end;
    int wbuf_len;

    int subtotal;
    int subtotal_size;
    int max_subtotal_size;
} moving_avg_values;


int allocate_moving_avg(int num_wbuf, int subtotal_sizes, moving_avg_values **allocated);

int reset_moving_avg(moving_avg_values *reset);

int append_to_moving_avg(moving_avg_values *value, int appended, bool *isAtEnd);

int get_latest_frame_moving_avg(moving_avg_values *value, int *frame);

int free_moving_avg(moving_avg_values **value);

#endif
