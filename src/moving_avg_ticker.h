#ifndef ACCEL_AVG_TICKER
#define ACCEL_AVG_TICKER

#include "accel.h"

#define MOVING_AVG_PARAM_ERROR -1
#define MOVING_AVG_INTERNAL_ERROR -2
#define MOVING_AVG_MALLOC_ERROR -2

int allocate_moving_avg(int num_wbuf, int subtotal_sizes, moving_avg_values **allocated);

int reset_moving_avg(moving_avg_values * reset);

int append_to_moving_avg(moving_avg_values *value, int appended, bool *isAtEnd);

int get_latest_frame_moving_avg(moving_avg_values *value);

int free_moving_avg(moving_avg_values **value);

#endif
