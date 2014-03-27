#ifndef ACCEL_AVG_TICKER
#define ACCEL_AVG_TICKER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "accel.h"

#define MOVING_AVG_SUCCESS ACCEL_SUCCESS
#define MOVING_AVG_PARAM_ERROR ACCEL_PARAM_ERROR
#define MOVING_AVG_INTERNAL_ERROR ACCEL_INTERNAL_ERROR
#define MOVING_AVG_MALLOC_ERROR ACCEL_MALLOC_ERROR

#ifndef MOVING_AVG_DATA_TYPE
#define MOVING_AVG_DATA_TYPE int32_t
#endif

typedef MOVING_AVG_DATA_TYPE moving_avg_data_type;

typedef struct moving_avg_values {
    // Circular buffer
    moving_avg_data_type *wbuf;
    uint16_t wbuf_end;
    uint16_t wbuf_len;

    moving_avg_data_type subtotal;
    uint16_t subtotal_size;
    uint16_t max_subtotal_size;
} moving_avg_values;

int allocate_moving_avg(uint16_t num_wbuf, uint16_t subtotal_sizes, moving_avg_values **allocated);

int reset_moving_avg(moving_avg_values *reset);

int append_to_moving_avg(moving_avg_values *value, moving_avg_data_type appended, bool *is_at_end);

int get_latest_frame_moving_avg(moving_avg_values *value, moving_avg_data_type *frame);

int free_moving_avg(moving_avg_values **value);

#ifdef __cplusplus
}
#endif

#endif
