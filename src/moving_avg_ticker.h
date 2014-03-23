#ifndef ACCEL_AVG_TICKER
#define ACCEL_AVG_TICKER

#ifdef __cplusplus
extern "C" {
#endif

#include "accel.h"

#ifdef PEBBLE
#error // no support yet.
#else
#include <stdint.h>
#endif

#define MOVING_AVG_SUCCESS ACCEL_SUCCESS
#define MOVING_AVG_PARAM_ERROR ACCEL_PARAM_ERROR
#define MOVING_AVG_INTERNAL_ERROR ACCEL_INTERNAL_ERROR
#define MOVING_AVG_MALLOC_ERROR ACCEL_MALLOC_ERROR

typedef struct moving_avg_values {
    // Circular buffer
    uint32_t *wbuf;
    int wbuf_end;
    int wbuf_len;

    int subtotal;
    uint32_t subtotal_size;
    uint32_t max_subtotal_size;
} moving_avg_values;

int allocate_moving_avg(uint32_t num_wbuf, uint32_t subtotal_sizes, moving_avg_values **allocated);

int reset_moving_avg(moving_avg_values *reset);

int append_to_moving_avg(moving_avg_values *value, int appended, bool *isAtEnd);

int get_latest_frame_moving_avg(moving_avg_values *value, int *frame);

int free_moving_avg(moving_avg_values **value);

#ifdef __cplusplus
}
#endif

#endif
